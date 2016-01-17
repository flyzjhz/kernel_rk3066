/****************************************************************************
 *   FileName    : SecretIC.c
 *   Description :
 ****************************************************************************
*
*   Copyright (c) Nowada, Inc.
*   ALL RIGHTS RESERVED
*
*	Created: 2011-11-28
*  	Author: Jackson Meng
****************************************************************************/

#include "windows.h"
#include "SHA204.h"
#include "SHA204_lib_return_codes.h"
#include "SHA204_comm.h"
#include "I2C_phys.h"
#include "SHA256.h"
#include "Host_Cal.h"
#include "SHA204_Config.h"

//#ifdef BOOT_AUTH
//extern 	BOOL BootAuthOK;
//#else
//extern 	BOOL BootAuthOK;
//#endif

#ifdef BOOT_AUTH
	BOOL BootAuthOK = FALSE;
	//extern 	int	bPassSecretIC;
	int bPassSecretIC = 0;
#else
	BOOL BootAuthOK = TRUE;
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT8 SHA204_CheckMAC(BOOL bBoard, UINT8 Slot)
{
	UINT8 status = SHA204_SUCCESS;//!< Function execution status, initialized to SUCCES and bitmasked witherror codes as
	unsigned char TxBuf[SHA204_CMD_SIZE_MAX],RxBuf[SHA204_RSP_SIZE_MAX];
	unsigned char NumIn[32],Mac[32];
	unsigned char soft_digest [32];
	unsigned char Key_ID = 0x00;
	unsigned char Secret[32];
	unsigned char TmpBuf[35];

	struct sha204h_nonce_in_out nonce_param;		//!< Parameter for nonce helper function
	struct sha204h_temp_key Temp_key;
	struct sha204h_mac_in_out mac_param;

	int i=0;

	UINT8 OtherDat[13] = {
		0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00};

	////////////////////////////////////////////////////////////////////////////
	Key_ID = Slot;

	if((Slot<0) || (Slot>15))
	{
		return 1;
	}

	memcpy(&Secret[0] ,GPA_BUF[Key_ID],8);
	memcpy(&Secret[8] ,GPB_BUF[Key_ID],8);
	memcpy(&Secret[16],GPC_BUF[Key_ID],8);
	memcpy(&Secret[24],GPD_BUF[Key_ID],8);

	for(i=0; i<32; i++ )
	{
		Secret[i] ^= GPA_Seed[Key_ID][i%2];
	}

	TmpBuf[0] = 0x23;
	memcpy(&TmpBuf[1],Secret,32);
	TmpBuf[0x21] = GPA_Seed[Key_ID][0];
	TmpBuf[0x22] = GPA_Seed[Key_ID][1];

	status = SHA204c_Check_crc(TmpBuf);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204c_Check ERROR\n");

		return 1;
	}

	//Wakeup Deivce
	status = ATSHA204_wakeup_and_validate_device();

	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("ATSHA204_wakeup_and_validate_device ERROR\n");
		return status;
	}

	status = SHA204_Random(TxBuf,RxBuf,0x00);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204_Random ERROR\n");

		return status;
	}
	else
	{
		memcpy(NumIn, &RxBuf[1],32);
	}

	status = SHA204_Nonce(TxBuf,RxBuf,0x00,NumIn);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204_Nonce ERROR\n");

		return status;
	}

	// Prepare parameters and nonce in host.
	nonce_param.mode = NONCE_MODE_SEED_UPDATE;
	nonce_param.num_in = NumIn;
	nonce_param.rand_out = &RxBuf[1];
	nonce_param.temp_key = &Temp_key;
	status = sha204h_nonce(nonce_param);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n sha204h_nonce ERROR\n");

		return status;
	}

	status = SHA204_Mac(TxBuf,RxBuf,0x01,Key_ID,NumIn);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204_Mac ERROR\n");

		return status;
	}
	else
	{
		memcpy(Mac, &RxBuf[1],32);
	}

	//information needed by a host system to calculate the expected challenge response in software
	mac_param.mode = MAC_MODE_BLOCK2_TEMPKEY;
	mac_param.key_id = Key_ID;
	mac_param.challenge = NumIn;
	mac_param.key = Secret;
	mac_param.otp = NULL;
	mac_param.sn = NULL;
	mac_param.response = soft_digest;
	mac_param.temp_key = &Temp_key;

	status = sha204h_mac(mac_param);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n Mac COMM Error:\n");

		return status;
	}

	// Moment of truth!  Compare the chip generated digest found in 'response_buffer' with the host software calculated digest found in 'soft_digest'.
	status = memcmp(soft_digest,&Mac[0],32);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n Mac COMM Error:\n");

		return status;
	}

	OtherDat[2] = Key_ID;

	status = SHA204_Check_Mac(TxBuf,RxBuf,0x00,Key_ID,Temp_key.value,soft_digest,OtherDat);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204_Check_Mac Error:\n");

		return status;
	}

	/* Put the chip to sleep in case you stop to examine buffer contents */
	status = SHA204_sleep();
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204_sleep  ERROR\n");

//		return status;
	}

	if(RxBuf[1] != 0x00)
		return	1;
	else
		return 0;
}

UINT8 ATSHA204_OTP_read(UINT8 slot, UINT8 *pDat)
{
	UINT8 status = SHA204_SUCCESS;
	UINT8 transmit_buffer[SHA204_CMD_SIZE_MAX];
	UINT8 response_buffer[SHA204_RSP_SIZE_MAX];
	UINT8 response[MAC_RSP_SIZE];

	//Wakeup device
	SHA204c_wakeup(response);

	 /*!*	*** WRITE INITIAL OTP DATA INTO THE OTP REGION ***
	 *****************************************************
	 *  Write initial information to the OTP region.  This is the only opportunity to do so.  After locking data and OTP regions, write accesses to this region will be controlled by
	 *  custom access privileges defined in the configuration region.
	 */

	 if(slot == 0)
	 {
		// Write the first 32 bytes of the 64-byte OTP block
		status = SHA204_Execute(SHA204_READ,SHA204_ZONE_OTP|SHA204_ZONE_COUNT_FLAG,OTP_BLOCK_0_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
		if(status != SHA204_SUCCESS)
		{
			KITLOutputDebugString("\n Read OTP0 ERROR\n");
			return status;
		}
	}
	else
	{
		// Write the second 32 bytes of the 64-byte OTP block
		status = SHA204_Execute(SHA204_READ,SHA204_ZONE_OTP|SHA204_ZONE_COUNT_FLAG,OTP_BLOCK_1_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
		if(status != SHA204_SUCCESS)
		{
			KITLOutputDebugString("\n Read OTP1 ERROR\n");
			return status;
		}
	}

	memcpy(pDat, &response_buffer[1],32);

	//Put Device Sleep
	SHA204_sleep();

	return status;
}

UINT8 ATSHA204_OTP_Check(UINT8 slot, unsigned char *pBuf)
{
	UINT8 Buf[32];
	UINT8 status = SHA204_SUCCESS;
	int   i=0;

	status = ATSHA204_OTP_read(slot,Buf);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("OTP0 Check Rd ERROR\n");
		return status;
	}

	for(i=0; i<30; i++ )
	{
		Buf[i] ^= Buf[30+i%2];
	}

	status = SHA204c_Check_crc(Buf);
	if(status != SHA204_SUCCESS)
	{
		KITLOutputDebugString("\n SHA204c_Check_crc ERROR\n");
	}
	else
	{
		memcpy(pBuf,&Buf[14],16);
	}

	return status;
}

BOOL ATSHA204_RdUUID( unsigned char  *pUUID)
{
	if(ATSHA204_OTP_Check(0,pUUID) != 0)
	{
		KITLOutputDebugString("\nRead UUID 0 Error\n");

		if(ATSHA204_OTP_Check(1,pUUID) != 0)
		{
			KITLOutputDebugString("\nRead UUID 1 Error\n");

			return FALSE;
		}
	}

	return TRUE;
}

UINT8 AT204Secret_Init( void )
{
//	UINT8 response[MAC_RSP_SIZE];
	UINT8 status = SHA204_SUCCESS;

	//AT_I2C_Init();

//	status = SHA204_sleep();
//	if(status != SHA204_SUCCESS)
//	{
//		KITLOutputDebugString("SHA204_sleep  ERROR\n");
//	}

//	SHA204c_wakeup(response);

//	status = ATSHA204_wakeup_and_validate_device();
//
//	if(status != SHA204_SUCCESS)
//	{
//		KITLOutputDebugString("ATSHA204_wakeup_and_validate_device ERROR\n");
//	}

//	KITLOutputDebugString("AT204Secret_Init++\n");

	return status;

}

#ifdef  BOOT_AUTH
BOOL AT_Boot_Authenticate(unsigned int RdPassWord,void *pData )
{
	unsigned char Slot[3];
	UINT8 status = SHA204_SUCCESS;

	Slot[0] = RdPassWord&0xff;
	Slot[1] = (RdPassWord>>8)&0xff;
	Slot[2] = (RdPassWord>>16)&0xff;

	status = AT204Secret_Init();

	if( SHA204_CheckMAC(FALSE,Slot[0]) != 0 )
	{
		KITLOutputDebugString("\nFirst Authe err");

		status = AT204Secret_Init();
		if( SHA204_CheckMAC(FALSE,Slot[1]) != 0 )
		{
			KITLOutputDebugString("\nsecond Authe err");

			status = AT204Secret_Init();
			if( SHA204_CheckMAC(FALSE,Slot[2]) != 0 )
			{
				KITLOutputDebugString("\nthird Authe err");
				return FALSE;
			}
		}
	}

	BootAuthOK = TRUE;
	bPassSecretIC++;

	if( pData != NULL )
		 ATSHA204_RdUUID( pData );

	return TRUE;
}
#endif

#ifdef  OS_AUTH
BOOL AT_OS_Authenticate( unsigned int RdPassWord ,void *pData)
{
	unsigned char Slot[3];
	UINT8 status = SHA204_SUCCESS;

	Slot[0] = RdPassWord&0xff;
	Slot[1] = (RdPassWord>>8)&0xff;
	Slot[2] = (RdPassWord>>16)&0xff;

	status = AT204Secret_Init();

	if( SHA204_CheckMAC(FALSE,Slot[1]) != 0 )
	{
		KITLOutputDebugString("\nFirst Authe err");

		status = AT204Secret_Init();
		if( SHA204_CheckMAC(FALSE,Slot[0]) != 0 )
		{
			KITLOutputDebugString("\nsecond Authe err");

			status = AT204Secret_Init();
			if( SHA204_CheckMAC(FALSE,Slot[2]) != 0 )
			{
				KITLOutputDebugString( (_T("INFO: OS auth fail\r\n")));
				if( pData != NULL )
				{
					memset( pData, 0x1f, 800*480*2/2);
				    while(1);
				}
				return FALSE;
			}
		}
	}

	BootAuthOK = TRUE;

	return TRUE;
}

BOOL AT_APP_Authenticate( unsigned int RdPassWord ,void *pData)
{
	unsigned char Slot[3];
	UINT8 status = SHA204_SUCCESS;

	Slot[0] = RdPassWord&0xff;
	Slot[1] = (RdPassWord>>8)&0xff;
	Slot[2] = (RdPassWord>>16)&0xff;

	status = AT204Secret_Init();

	if(status != SHA204_SUCCESS)
		return FALSE;

	if( SHA204_CheckMAC(FALSE,Slot[2]) != 0 )
	{
		RETAILMSG(1,(L "\nFirst Authe err"));
		if( SHA204_CheckMAC(FALSE,Slot[0]) != 0 )
		{
			RETAILMSG(1,(L "\nsecond Authe err"));
			if( SHA204_CheckMAC(FALSE,Slot[2]) != 0 )
			{
				return FALSE;
			}
		}
	}

	BootAuthOK = TRUE;

	return TRUE;
}
#endif

void AT204Secret_Test( void )
{
	UINT8 response[MAC_RSP_SIZE];

	UINT8 status = SHA204_SUCCESS;

	//AT_I2C_Init();

	SHA204c_wakeup(response);

	SHA204c_resync(0x4,response);

	status = ATSHA204_wakeup_and_validate_device();

	if(status != SHA204_SUCCESS)
		KITLOutputDebugString("ATSHA204_wakeup_and_validate_device ERROR\n");

}


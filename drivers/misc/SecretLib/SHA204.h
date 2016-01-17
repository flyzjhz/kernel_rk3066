/****************************************************************************
 *   FileName    : SHA204.h
 *   Description :
 ****************************************************************************
*
*   Copyright (c) Nowada, Inc.
*   ALL RIGHTS RESERVED
*
*	Created: 2011-11-28
*  	Author: Jackson Meng
****************************************************************************/

#ifndef __SECRETIC_H__
#define __SECRETIC_H__

#define SHA204_CRC_SIZE              2	//CRC 2 Byte
#define SHA204_CMD_SIZE_MIN          7	//1Byte Cmdlen+1Byte Opcode+1Byte Param1+
										//2Byte Param+2Byte CRC

#define SHA204_RETRY_COUNT           1

// maximum size of command packet (CheckMac)
#define SHA204_CMD_SIZE_MAX          84

#define SHA204_BUFFER_POS_COUNT      (0)             //!< buffer index of count byte in command or response
#define SHA204_BUFFER_POS_DATA       (1)             //!< buffer index of data in response

//write parameter (additional)
#define WRITE_BUFFER_SIZE_SHORT			(4)						//buffer size for 4 bytes write
#define WRITE_BUFFER_SIZE_LONG			(32)					//buffer size for 32 bytes write
#define WRITE_DATA_START_IDX			(5)						//index for the first data in write buffer
#define WRITE_DATA_END_IDX_4_BYTES		(9)						//index for the last data in 4 bytes write buffer
#define WRITE_DATA_END_IDX_32_BYTES		(37)					//index for the last data in 32 bytes write buffer
#define WRITE_ZONE_MODE_32_BYTES        ((UINT8) 0x80)		//!< write mode: 32 bytes


#define NONCE_PARAM2					((UINT16) 0x0000)		//nonce param2. always zero
#define HMAC_MODE_EXCLUDE_OTHER_DATA	((UINT8) 0x00)		//!< HMAC mode excluded other data
#define HMAC_MODE_INCLUDE_OTP_88		((UINT8) 0x10)		//!< HMAC mode bit   4: include first 88 OTP bits
#define HMAC_MODE_INCLUDE_OTP_64		((UINT8) 0x20)		//!< HMAC mode bit   5: include first 64 OTP bits
#define HMAC_MODE_INCLUDE_SN			((UINT8) 0x40)		//!< HMAC mode bit   6: include serial number
#define DERIVE_KEY_RANDOM_NONCE			((UINT8) 0x00)		//Derive key mode using random nonce
#define MAC_MODE_NO_TEMPKEY				((UINT8) 0x00)		//MAC mode using internal key and challenge to get MAC result
#define LOCK_PARAM2_NO_CRC				((UINT16) 0x0000)		//Lock mode : not using checksum to validate the data written
#define CHECKMAC_PASSWORD_MODE			((UINT8) 0X01)		//CheckMac mode : password check operation

//Key ID in 16 bit boundaries
#define KEY_ID_0						((UINT16) 0x0000)
#define KEY_ID_1						((UINT16) 0x0001)
#define KEY_ID_2						((UINT16) 0x0002)
#define KEY_ID_3						((UINT16) 0x0003)
#define KEY_ID_4						((UINT16) 0x0004)
#define KEY_ID_5						((UINT16) 0x0005)
#define KEY_ID_6						((UINT16) 0x0006)
#define KEY_ID_7						((UINT16) 0x0007)
#define KEY_ID_8						((UINT16) 0x0008)
#define KEY_ID_9						((UINT16) 0x0009)
#define KEY_ID_10						((UINT16) 0x000A)
#define KEY_ID_11						((UINT16) 0x000B)
#define KEY_ID_12						((UINT16) 0x000C)
#define KEY_ID_13						((UINT16) 0x000D)
#define KEY_ID_14						((UINT16) 0x000E)
#define KEY_ID_15						((UINT16) 0x000F)

// *	*** DEVICE Modes Address ***
#define DEVICE_MODES_ADDRESS			((UINT16) 0x0004)
#define DEVICE_MODES_BYTE_SIZE			(4)

// command op-code definitions
#define SHA204_CHECKMAC                 0x28       //!< CheckMac command op-code
#define SHA204_DERIVE_KEY               0x1C       //!< DeriveKey command op-code
#define SHA204_DEVREV                   0x30       //!< DevRev command op-code
#define SHA204_GENDIG                   0x15       //!< GenDig command op-code
#define SHA204_HMAC                     0x11       //!< HMAC command op-code
#define SHA204_LOCK                     0x17       //!< Lock command op-code
#define SHA204_MAC                      0x08       //!< MAC command op-code
#define SHA204_NONCE                    0x16       //!< Nonce command op-code
#define SHA204_PAUSE                    0x01       //!< Pause command op-code
#define SHA204_RANDOM                   0x1B       //!< Random command op-code
#define SHA204_READ                     0x02       //!< Read command op-code
#define SHA204_TEMPSENSE                0x18       //!< TempSense command op-code
#define SHA204_UPDATE_EXTRA             0x20       //!< UpdateExtra command op-code
#define SHA204_WRITE                    0x12       //!< Write command op-code

//////////////////////////////////////////////////////////////////////
// parameter range definitions
#define SHA204_KEY_ID_MAX               (15)         //!< maximum value for key id
#define SHA204_OTP_BLOCK_MAX            (1)        	 //!< maximum value for OTP block


//////////////////////////////////////////////////////////////////////
// command timing definitions for minimum execution times (ms)
//! CheckMAC minimum command delay
#define CHECKMAC_DELAY                  12	//Type 12MS

//! DeriveKey minimum command delay
#define DERIVE_KEY_DELAY                14  //Type 14MS

//! DevRev minimum command delay
#define DEVREV_DELAY                    1	//Type 0.4MS

//! GenDig minimum command delay
#define GENDIG_DELAY                    11  //Type 11MS

//! HMAC minimum command delay
#define HMAC_DELAY                      27  //Type 27MS

//! Lock minimum command delay
#define LOCK_DELAY                      5   //Type 5MS

//! MAC minimum command delay
#define MAC_DELAY                       12  //Type 12MS

//! Nonce minimum command delay
#define NONCE_DELAY                     22  //Type 22MS

//! Pause minimum command delay
#define PAUSE_DELAY                     1   //Type 0.4MS

//! Random minimum command delay
#define RANDOM_DELAY                    11  //Type 11MS

//! Read minimum command delay
#define READ_DELAY                      1   //Type 0.4MS

//! TempSense minimum command delay
#define TEMP_SENSE_DELAY                4   //Type 4MS

//! UpdateExtra minimum command delay
#define UPDATE_DELAY                    4   //Type 4MS

//! Write minimum command delay
#define WRITE_DELAY                     4   //Type 4MS

//////////////////////////////////////////////////////////////////////
// command timing definitions for maximum execution times (ms)
//! CheckMAC maximum execution time
#define CHECKMAC_EXEC_MAX                38  //MAX 38MS

//! DeriveKey maximum execution time
#define DERIVE_KEY_EXEC_MAX              62  //MAX 62MS

//! DevRev maximum execution time
#define DEVREV_EXEC_MAX                  2	 //MAX 2MS

//! GenDig maximum execution time
#define GENDIG_EXEC_MAX                  43  //MAX 43MS

//! HMAC maximum execution time
#define HMAC_EXEC_MAX                    69  //MAX 69MS

//! Lock maximum execution time
#define LOCK_EXEC_MAX                    24  //MAX 24MS

//! MAC maximum execution time
#define MAC_EXEC_MAX                     35  //MAX 35MS

//! Nonce maximum execution time
#define NONCE_EXEC_MAX                   60  //MAX 60MS

//! Pause maximum execution time
#define PAUSE_EXEC_MAX                   2   //MAX 2MS

//! Random maximum execution time
#define RANDOM_EXEC_MAX                  50  //MAX 50MS

//! Read maximum execution time
#define READ_EXEC_MAX                    4   //MAX 4MS

//! TempSense maximum execution time
#define TEMP_SENSE_EXEC_MAX              11  //MAX 11MS

//! UpdateExtra maximum execution time
#define UPDATE_EXEC_MAX                  6   //MAX 6MS

//! Write maximum execution time
#define WRITE_EXEC_MAX                   42  //MAX 42MS

//! maximum command delay
#define SHA204_COMMAND_EXEC_MAX      	 69  //MAX 69MS

#define SHA204_RESPONSE_TIMEOUT     	 37  //MAX 37MS

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Response size definitions
#define SHA204_RSP_SIZE_VAL             7         //!< size of response packet containing four bytes of data
#define SHA204_RSP_SIZE_MIN             4  //!< minimum number of bytes in response
#define SHA204_RSP_SIZE_MAX             35  //!< maximum size of response packet

#define CHECKMAC_RSP_SIZE               SHA204_RSP_SIZE_MIN    //!< response size of DeriveKey command
#define DERIVE_KEY_RSP_SIZE             SHA204_RSP_SIZE_MIN    //!< response size of DeriveKey command
#define DEVREV_RSP_SIZE                 SHA204_RSP_SIZE_VAL    //!< response size of DevRev command returns 4 bytes
#define GENDIG_RSP_SIZE                 SHA204_RSP_SIZE_MIN    //!< response size of GenDig command
#define HMAC_RSP_SIZE                   SHA204_RSP_SIZE_MAX    //!< response size of HMAC command
#define LOCK_RSP_SIZE                   SHA204_RSP_SIZE_MIN    //!< response size of Lock command
#define MAC_RSP_SIZE                    SHA204_RSP_SIZE_MAX    //!< response size of MAC command
#define NONCE_RSP_SIZE_SHORT            SHA204_RSP_SIZE_MIN    //!< response size of Nonce command with mode[0:1] = 3
#define NONCE_RSP_SIZE_LONG             SHA204_RSP_SIZE_MAX    //!< response size of Nonce command
#define PAUSE_RSP_SIZE                  SHA204_RSP_SIZE_MIN    //!< response size of Pause command
#define RANDOM_RSP_SIZE                 SHA204_RSP_SIZE_MAX    //!< response size of Random command
#define READ_4_RSP_SIZE                 SHA204_RSP_SIZE_VAL    //!< response size of Read command when reading 4 bytes
#define READ_32_RSP_SIZE                SHA204_RSP_SIZE_MAX    //!< response size of Read command when reading 32 bytes
#define TEMP_SENSE_RSP_SIZE             SHA204_RSP_SIZE_VAL    //!< response size of TempSense command returns 4 bytes
#define UPDATE_RSP_SIZE                 SHA204_RSP_SIZE_MIN    //!< response size of UpdateExtra command
#define WRITE_RSP_SIZE                  SHA204_RSP_SIZE_MIN    //!< response size of Write command

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// zone definitions
#define SHA204_ZONE_CONFIG              0x00       //!< Configuration zone
#define SHA204_ZONE_OTP                 0x01       //!< OTP (One Time Programming) zone
#define SHA204_ZONE_DATA                0x02       //!< Data zone
#define SHA204_ZONE_COUNT_FLAG          0x80       //!< Zone bit 7 set: Access 32 bytes, otherwise 4 bytes.
#define SHA204_ZONE_ACCESS_4            4      		//!< Read or write 4 bytes.
#define SHA204_ZONE_ACCESS_32           32      	//!< Read or write 32 bytes.
#define SHA204_ADDRESS_MASK_CONFIG      0x1F      	//!< Address bits 5 to 7 are 0 for Configuration zone.
#define SHA204_ADDRESS_MASK_OTP         0x0F      	//!< Address bits 4 to 7 are 0 for OTP zone.
#define SHA204_ADDRESS_MASK             0x007F    	//!< Address bit 7 to 15 are always 0.

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// definitions for command packet indexes common to all commands
#define SHA204_COUNT_IDX                ( 0)                   //!< command packet index for count
#define SHA204_OPCODE_IDX               ( 1)                   //!< command packet index for op-code
#define SHA204_PARAM1_IDX               ( 2)                   //!< command packet index for first parameter
#define SHA204_PARAM2_IDX               ( 3)                   //!< command packet index for second parameter
#define SHA204_DATA_IDX                 ( 5)                   //!< command packet index for second parameter

//////////////////////////////////////////////////////////////////////
// CheckMAC command definitions
#define CHECKMAC_MODE_IDX               SHA204_PARAM1_IDX      //!< CheckMAC command index for mode
#define CHECKMAC_KEYID_IDX              SHA204_PARAM2_IDX      //!< CheckMAC command index for key identifier
#define CHECKMAC_CLIENT_CHALLENGE_IDX   SHA204_DATA_IDX        //!< CheckMAC command index for client challenge
#define CHECKMAC_CLIENT_RESPONSE_IDX    (37)                   //!< CheckMAC command index for client response
#define CHECKMAC_DATA_IDX               (69)                   //!< CheckMAC command index for other data
#define CHECKMAC_COUNT                  (84)                   //!< CheckMAC command packet size
#define CHECKMAC_MODE_MASK              (0x27)       			//!< CheckMAC mode bits 3, 4, 6, and 7 are 0.
#define CHECKMAC_CLIENT_CHALLENGE_SIZE  (32)                   //!< CheckMAC size of client challenge
#define CHECKMAC_CLIENT_RESPONSE_SIZE   (32)                   //!< CheckMAC size of client response
#define CHECKMAC_OTHER_DATA_SIZE        (13)                   //!< CheckMAC size of "other data"

//////////////////////////////////////////////////////////////////////
// DeriveKey command definitions
#define DERIVE_KEY_RANDOM_IDX           SHA204_PARAM1_IDX      //!< DeriveKey command index for random bit
#define DERIVE_KEY_TARGETKEY_IDX        SHA204_PARAM2_IDX      //!< DeriveKey command index for target slot
#define DERIVE_KEY_MAC_IDX              SHA204_DATA_IDX        //!< DeriveKey command index for optional MAC
#define DERIVE_KEY_COUNT_SMALL          SHA204_CMD_SIZE_MIN    //!< DeriveKey command packet size without MAC
#define DERIVE_KEY_COUNT_LARGE          (39)                   //!< DeriveKey command packet size with MAC
#define DERIVE_KEY_RANDOM_FLAG          (4)          			//!< DeriveKey 1. parameter
#define DERIVE_KEY_MAC_SIZE             (32)                   //!< DeriveKey MAC size

//////////////////////////////////////////////////////////////////////
// DevRev command definitions
#define DEVREV_PARAM1_IDX               SHA204_PARAM1_IDX      //!< DevRev command index for 1. parameter (ignored)
#define DEVREV_PARAM2_IDX               SHA204_PARAM2_IDX      //!< DevRev command index for 2. parameter (ignored)
#define DEVREV_COUNT                    SHA204_CMD_SIZE_MIN    //!< DevRev command packet size

//////////////////////////////////////////////////////////////////////
// GenDig command definitions
#define GENDIG_ZONE_IDX                 SHA204_PARAM1_IDX      //!< GenDig command index for zone
#define GENDIG_KEYID_IDX                SHA204_PARAM2_IDX      //!< GenDig command index for key id
#define GENDIG_DATA_IDX                 SHA204_DATA_IDX        //!< GenDig command index for optional data
#define GENDIG_COUNT                    SHA204_CMD_SIZE_MIN    //!< GenDig command packet size without "other data"
#define GENDIG_COUNT_DATA               (11)                   //!< GenDig command packet size with "other data"
#define GENDIG_OTHER_DATA_SIZE          (4)                    //!< GenDig size of "other data"
#define GENDIG_ZONE_OTP                 (1)          			//!< GenDig zone id OTP
#define GENDIG_ZONE_DATA                (2)          			//!< GenDig zone id data

//////////////////////////////////////////////////////////////////////
// HMAC command definitions
#define HMAC_MODE_IDX                   SHA204_PARAM1_IDX      //!< HMAC command index for mode
#define HMAC_KEYID_IDX                  SHA204_PARAM2_IDX      //!< HMAC command index for key id
#define HMAC_COUNT                      SHA204_CMD_SIZE_MIN    //!< HMAC command packet size
#define HMAC_MODE_MASK                  (0x74)       //!< HMAC mode bits 0, 1, 3, and 7 are 0.

//////////////////////////////////////////////////////////////////////
// Lock command definitions
#define LOCK_ZONE_IDX                   SHA204_PARAM1_IDX      //!< Lock command index for zone
#define LOCK_SUMMARY_IDX                SHA204_PARAM2_IDX      //!< Lock command index for summary
#define LOCK_COUNT                      SHA204_CMD_SIZE_MIN    //!< Lock command packet size
#define LOCK_ZONE_NO_CONFIG             (0x01)       			//!< Lock zone is OTP or Data
#define LOCK_ZONE_NO_CRC                (0x80)       			//!< Lock command: Ignore summary.
#define LOCK_PARAM2_NO_CRC				((UINT16) 0x0000)		//Lock mode : not using checksum to validate the data written
#define LOCK_ZONE_MASK                  (0x81)                 //!< Lock parameter 1 bits 2 to 6 are 0.

//////////////////////////////////////////////////////////////////////
// Mac command definitions
#define MAC_MODE_IDX                    SHA204_PARAM1_IDX      //!< MAC command index for mode
#define MAC_KEYID_IDX                   SHA204_PARAM2_IDX      //!< MAC command index for key id
#define MAC_CHALLENGE_IDX               SHA204_DATA_IDX        //!< MAC command index for optional challenge
#define MAC_COUNT_SHORT                 SHA204_CMD_SIZE_MIN    //!< MAC command packet size without challenge
#define MAC_COUNT_LONG                  (39)                   //!< MAC command packet size with challenge
#define MAC_MODE_BLOCK2_TEMPKEY         (0x01)       //!< MAC mode bit   0: second SHA block from TempKey
#define MAC_MODE_BLOCK1_TEMPKEY         (0x02)       //!< MAC mode bit   1: first SHA block from TempKey
#define MAC_MODE_SOURCE_FLAG_MATCH      (0x04)       //!< MAC mode bit   2: match TempKey.SourceFlag
#define MAC_MODE_PASSTHROUGH            (0x07)       //!< MAC mode bit 0-2: pass-through mode
#define MAC_MODE_INCLUDE_OTP_88         (0x10)       //!< MAC mode bit   4: include first 88 OTP bits
#define MAC_MODE_INCLUDE_OTP_64         (0x20)       //!< MAC mode bit   5: include first 64 OTP bits
#define MAC_MODE_INCLUDE_SN             (0x50)       //!< MAC mode bit   6: include serial number
#define MAC_CHALLENGE_SIZE              (32)                   //!< MAC size of challenge
#define MAC_MODE_MASK                   (0x77)       //!< MAC mode bits 3 and 7 are 0.

//////////////////////////////////////////////////////////////////////
// Nonce command definitions
#define NONCE_MODE_IDX                  SHA204_PARAM1_IDX      	//!< Nonce command index for mode
#define NONCE_PARAM2_IDX                SHA204_PARAM2_IDX      	//!< Nonce command index for 2. parameter
#define NONCE_INPUT_IDX                 SHA204_DATA_IDX        	//!< Nonce command index for input data
#define NONCE_COUNT_SHORT               27                   	//!< Nonce command packet size for 20 bytes of data
#define NONCE_COUNT_LONG                39                   	//!< Nonce command packet size for 32 bytes of data
#define NONCE_MODE_MASK                 3          				//!< Nonce mode bits 2 to 7 are 0.
#define NONCE_MODE_SEED_UPDATE          0x00       				//!< Nonce mode: update seed
#define NONCE_MODE_NO_SEED_UPDATE       0x01       				//!< Nonce mode: do not update seed
#define NONCE_MODE_INVALID              0x02       				//!< Nonce mode 2 is invalid.
#define NONCE_MODE_PASSTHROUGH          0x03       				//!< Nonce mode: pass-through
#define NONCE_NUMIN_SIZE                20                   	//!< Nonce data length
#define NONCE_NUMIN_SIZE_PASSTHROUGH    32                   	//!< Nonce data length in pass-through mode (mode = 3)

//////////////////////////////////////////////////////////////////////
// Pause command definitions
#define PAUSE_SELECT_IDX                SHA204_PARAM1_IDX      //!< Pause command index for Selector
#define PAUSE_PARAM2_IDX                SHA204_PARAM2_IDX      //!< Pause command index for 2. parameter
#define PAUSE_COUNT                     SHA204_CMD_SIZE_MIN    //!< Pause command packet size

//////////////////////////////////////////////////////////////////////
// Random command definitions
#define RANDOM_MODE_IDX                 SHA204_PARAM1_IDX      //!< Random command index for mode
#define RANDOM_PARAM2_IDX               SHA204_PARAM2_IDX      //!< Random command index for 2. parameter
#define RANDOM_COUNT                    SHA204_CMD_SIZE_MIN    //!< Random command packet size
#define RANDOM_SEED_UPDATE              0x00       				//!< Random mode for automatic seed update
#define RANDOM_NO_SEED_UPDATE           0x01       				//!< Random mode for no seed update

//////////////////////////////////////////////////////////////////////
// Read command definitions
#define READ_ZONE_IDX                   SHA204_PARAM1_IDX      //!< Read command index for zone
#define READ_ADDR_IDX                   SHA204_PARAM2_IDX      //!< Read command index for address
#define READ_COUNT                      SHA204_CMD_SIZE_MIN    //!< Read command packet size
#define READ_ZONE_MASK                  0x83       				//!< Read zone bits 2 to 6 are 0.
#define READ_ZONE_MODE_32_BYTES         0x80       				//!< Read mode: 32 bytes

//////////////////////////////////////////////////////////////////////
// TempSense command definitions
#define TEMP_SENSE_PARAM1_IDX           SHA204_PARAM1_IDX      //!< TempSense command index for 1. parameter
#define TEMP_SENSE_PARAM2_IDX           SHA204_PARAM2_IDX      //!< TempSense command index for 2. parameter
#define TEMP_SENSE_COUNT                SHA204_CMD_SIZE_MIN    //!< TempSense command packet size

//////////////////////////////////////////////////////////////////////
// UpdateExtra command definitions
#define UPDATE_MODE_IDX                  SHA204_PARAM1_IDX     //!< UpdateExtra command index for mode
#define UPDATE_VALUE_IDX                 SHA204_PARAM2_IDX     //!< UpdateExtra command index for new value
#define UPDATE_COUNT                     SHA204_CMD_SIZE_MIN   //!< UpdateExtra command packet size
#define UPDATE_CONFIG_BYTE_86            (0x01)      //!< UpdateExtra mode: update Config byte 86

//////////////////////////////////////////////////////////////////////
// Write command definitions
#define WRITE_ZONE_IDX                  SHA204_PARAM1_IDX      //!< Write command index for zone
#define WRITE_ADDR_IDX                  SHA204_PARAM2_IDX      //!< Write command index for address
#define WRITE_VALUE_IDX                 SHA204_DATA_IDX        //!< Write command index for data
#define WRITE_MAC_VS_IDX                ( 9)                   //!< Write command index for MAC following short data
#define WRITE_MAC_VL_IDX                (37)                   //!< Write command index for MAC following long data
#define WRITE_COUNT_SHORT               (11)                   //!< Write command packet size with short data and no MAC
#define WRITE_COUNT_LONG                (39)                   //!< Write command packet size with long data and no MAC
#define WRITE_COUNT_SHORT_MAC           (43)                   //!< Write command packet size with short data and MAC
#define WRITE_COUNT_LONG_MAC            (71)                   //!< Write command packet size with long data and MAC
#define WRITE_MAC_SIZE                  (32)                   //!< Write MAC size
#define WRITE_ZONE_MASK                 (0xC3)       //!< Write zone bits 2 to 5 are 0.
#define WRITE_ZONE_WITH_MAC             (0x40)       //!< Write zone bit 6: write encrypted with MAC

/////////////////////////////////////////////////////////////////////
//Slot configuration address
#define SLOT_CONFIG_0_1_ADDRESS			((UINT16) 0x0005)
#define SLOT_CONFIG_2_3_ADDRESS			((UINT16) 0x0006)
#define SLOT_CONFIG_4_5_ADDRESS			((UINT16) 0x0007)
#define SLOT_CONFIG_6_7_ADDRESS			((UINT16) 0x0008)
#define SLOT_CONFIG_8_9_ADDRESS			((UINT16) 0x0009)
#define SLOT_CONFIG_10_11_ADDRESS		((UINT16) 0x000A)
#define SLOT_CONFIG_12_13_ADDRESS		((UINT16) 0x000B)
#define SLOT_CONFIG_14_15_ADDRESS		((UINT16) 0x000C)

//////////////////////////////////////////////////////////////////////
//Slot key address
#define SLOT_0_ADDRESS					((UINT16) 0x0000)
#define SLOT_1_ADDRESS					((UINT16) 0x0008)
#define SLOT_2_ADDRESS					((UINT16) 0x0010)
#define SLOT_3_ADDRESS					((UINT16) 0x0018)
#define SLOT_4_ADDRESS					((UINT16) 0x0020)
#define SLOT_5_ADDRESS					((UINT16) 0x0028)
#define SLOT_6_ADDRESS					((UINT16) 0x0030)
#define SLOT_7_ADDRESS					((UINT16) 0x0038)
#define SLOT_8_ADDRESS					((UINT16) 0x0040)
#define SLOT_9_ADDRESS					((UINT16) 0x0048)
#define SLOT_10_ADDRESS					((UINT16) 0x0050)
#define SLOT_11_ADDRESS					((UINT16) 0x0058)
#define SLOT_12_ADDRESS					((UINT16) 0x0060)
#define SLOT_13_ADDRESS					((UINT16) 0x0068)
#define SLOT_14_ADDRESS					((UINT16) 0x0070)
#define SLOT_15_ADDRESS					((UINT16) 0x0078)

//////////////////////////////////////////////////////////////////////
/*!
 *	*** LAST KEY USE ADDRESS AND SIZE ***
 */
#define LAST_KEY_USE_ADDRESS			((UINT16) 0x0011)		// Word 17
#define LAST_KEY_USE_BYTE_SIZE			((UINT8) 0x10)		// 16 bytes

////////////////////////////////////////////////////////////
/*!
 *	*** USER EXTRA, SELECTOR, and LOCK bytes address
 */
#define EXTRA_SELECTOR_LOCK_ADDRESS		((UINT16) 0x0015)		// Word 21

//////////////////////////////////////////////////////////////////////
#define CONFIG_READ_SHORT				((UINT8)0x00)
#define CONFIG_READ_LONG				((UINT8)0x80)

#define OTP_READ_SHORT					((UINT8)0x01)
#define OTP_READ_LONG					((UINT8)0x81)
#define OTP_BLOCK_0_ADDRESS				((UINT16)0x0000)			//!< Base address of the first 32 bytes of the OTP region
#define OTP_BLOCK_1_ADDRESS				((UINT16)0x0008)			//!< Base address of the second 32 bytes of the OTP region

#define DATA_READ_SHORT					((UINT8)0x02)
#define DATA_READ_LONG					((UINT8)0x82)

#define CONFIG_BLOCK_0_ADDRESS			((UINT16)0x0000)
#define CONFIG_BLOCK_1_ADDRESS			((UINT16)0x0008)
#define CONFIG_BLOCK_2_ADDRESS			((UINT16)0x0010)

//////////////////////////////////////////////////////////////////////
const static UINT8 GPA_BUF [16][8] =
{
{0xF3, 0xAB, 0xF5, 0xC3, 0x42, 0xD4, 0x45, 0x7F},
{0xC7, 0x24, 0x2F, 0x99, 0xF7, 0x70, 0xC6, 0x4E},
{0x5A, 0x6B, 0x50, 0xE7, 0x99, 0x97, 0x32, 0xEB},
{0x96, 0xD1, 0x80, 0x90, 0xA4, 0xD3, 0xFA, 0xDF},
{0x69, 0x18, 0x63, 0x9E, 0x5D, 0x5F, 0xFD, 0xE4},
{0xE4, 0x12, 0x3B, 0x75, 0x0B, 0xBD, 0xFD, 0x27},
{0x83, 0x32, 0xC9, 0x20, 0x9C, 0x8A, 0x9C, 0x56},
{0xD8, 0xC0, 0x97, 0xE5, 0xC3, 0x52, 0x9F, 0x5C},
{0x29, 0x1D, 0xBE, 0x24, 0xC3, 0x99, 0x90, 0x1F},
{0x34, 0xB9, 0x8C, 0x23, 0xEF, 0xE2, 0xD5, 0x5A},
{0x83, 0xDD, 0xB8, 0xE7, 0x48, 0x5D, 0x0B, 0x6C},
{0xC9, 0x09, 0x6C, 0x8F, 0x81, 0x0C, 0xEE, 0xD9},
{0x7B, 0xBB, 0x44, 0xDE, 0x20, 0xC2, 0x16, 0x61},
{0xAE, 0x45, 0x2A, 0x67, 0x41, 0xE1, 0xB9, 0xEC},
{0x1C, 0xFB, 0xF4, 0x22, 0x1E, 0xE9, 0xB1, 0x52},
{0xF1, 0xD7, 0x03, 0x5A, 0x30, 0x87, 0x40, 0x25}
};

//////////////////////////////////////////////////////////////////////
void AT204Secret_Test( void );

UINT8 SHA204_Execute(UINT8 op_code, UINT8 param1, UINT16 param2,
			UINT8 datalen1, UINT8 *data1, UINT8 datalen2, UINT8 *data2, UINT8 datalen3, UINT8 *data3,
			UINT8 tx_size, UINT8 *tx_buffer, UINT8 rx_size, UINT8 *rx_buffer);

UINT8 SHA204_Check_Mac(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 mode, UINT8 key_id, UINT8 *client_challenge, UINT8 *client_response, UINT8 *other_data);

UINT8 SHA204_Derive_Key(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 random, UINT8 target_key, UINT8 *mac);

UINT8 SHA204_Dev_Rev(UINT8 *tx_buffer, UINT8 *rx_buffer);

UINT8 SHA204_Gen_Dig(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 zone, UINT8 key_id, UINT8 *other_data);

UINT8 SHA204_Hmac(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT16 key_id);

UINT8 SHA204_Lock(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 zone, UINT16 summary);

UINT8 SHA204_Mac(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 mode, UINT16 key_id, UINT8 *challenge);

UINT8 SHA204_Nonce(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT8 *numin);

UINT8 SHA204_Pause(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 selector);

UINT8 SHA204_Random(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode);

UINT8 SHA204_Read(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 zone, UINT16 address);

UINT8 SHA204_Temp_Sense(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT16 *temp_raw);

UINT8 SHA204_Update_extra(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT8 new_value);

UINT8 SHA204_Write(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 zone, UINT16 address, UINT8 *new_value, UINT8 *mac);

UINT8 ATSHA204_wakeup_and_validate_device(void);

#endif /*__SECRETIC_H__*/
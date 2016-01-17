/****************************************************************************
 *   FileName    : I2C_phys.c
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
//#include <asm/arch/i2c.h>
//#include <drvlib.h>
//#include <cspi2c.h>
/////////////////////////////////////////////////////////////////////

#if 0
#define  OUTPUT_FUNC_IN      printk("------>in %s  \n", __func__)
#define  OUTPUT_FUNC_OUT     printk("------>out %s  \n", __func__)
#else
#define  OUTPUT_FUNC_IN      
#define  OUTPUT_FUNC_OUT     
#endif

#define WAIT_COUNT   (2000*15)

/** This function sends a I2C packet enclosed by a I2C start and stop to a SHA204 device.
 *
 *         This function combines a I2C packet send sequence that is common to all packet types.
 *         Only if word_address is #I2C_PACKET_FUNCTION_NORMAL, count and buffer parameters are
 *         expected to be non-zero.
 * @return status of the operation
 */

extern struct i2c_client *atsha204a_client;

enum i2c_word_address {
	SHA204_I2C_PACKET_FUNCTION_RESET,  //!< Reset device.
	SHA204_I2C_PACKET_FUNCTION_SLEEP,  //!< Put device into Sleep mode.
	SHA204_I2C_PACKET_FUNCTION_IDLE,   //!< Put device into Idle mode.
	SHA204_I2C_PACKET_FUNCTION_NORMAL  //!< Write / evaluate data that follow this word address byte.
};


int rk30_i2c_send(struct i2c_client *client, u8 reg_addr, u32 len, u8 *buffer)
{
	struct i2c_msg msg;
	int ret = -1, i = 0;
	int retries = 0;
	char pbuf[128] = {0};
	
	pbuf[0] = reg_addr;
	
	memcpy(&pbuf[1], buffer, len);

	msg.addr = client->addr;
	msg.flags =  !I2C_M_RD;
	msg.len = len + 1;
	msg.buf = &pbuf[0];
	msg.scl_rate = 10* 1000;
	msg.udelay = 5;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret != 1)
	{
		ret = 0xF0;
		printk(KERN_ERR"error func: %s , line: %d \n", __func__, __LINE__);
	}
	else
	{
		ret = 0x00;
	}

	return ret;
}

int rk_i2c_recv(struct i2c_client *client, u8 reg_addr, u32 len, u8 *buffer)
{
	struct i2c_msg msgs[2];
	s32 ret=-1;
	s32 retries = 0;

	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr  = client->addr;
	msgs[0].len   = 1;
	msgs[0].buf   = &reg_addr;          
	msgs[0].scl_rate = 10 * 1000;    
	msgs[0].udelay = 5;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = client->addr;
	msgs[1].len   = len;
	msgs[1].buf   = &buffer[0];
	msgs[1].scl_rate = 10 * 1000;
	msgs[1].udelay = 5;

	while(retries < 5)
	{
		ret = i2c_transfer(client->adapter, msgs, 2);
		if(ret == 2)break;
		retries++;
	}

	if((retries >= 5)){
		ret = 0xF0;
		printk(KERN_ERR"error func: %s , line: %d \n", __func__, __LINE__);
	}
	else
	{
		ret = 0x00;
	}
	
OUTPUT_FUNC_OUT;
	return ret;

}

u32 AT_ICIsReady(struct i2c_client *client) //make sure i2c is ready
{
	return 0; 
}

UINT8 SHA204_wakeup(void)
{
	return AT_ICIsReady(atsha204a_client); 
}

static UINT8 SHA204_I2C_send(UINT8 word_address, UINT8 count, UINT8 *buffer)
{
	return rk30_i2c_send(atsha204a_client, word_address, count, buffer);
}

/** rief This I2C function resynchronizes communication.
 */
UINT8 SHA204_resync_platform(struct i2c_client *client, UINT8 size, UINT8 *response)
{
        struct i2c_msg msgs;
	int retries = -1, ret = -1;
	UINT8 nine_clocks = 0x7F;

	msgs.flags = I2C_M_RD;
    	msgs.addr  = nine_clocks;
    	msgs.len   = 0;
    	msgs.buf   = NULL;            
    	msgs.scl_rate = 10 * 1000;    
    	msgs.udelay = 5; 

	while(retries < 5) 
	{    
		ret = i2c_transfer(client->adapter, &msgs, 1);
		if(ret == 1)break;
		retries++;
	}    

	msgs.flags = I2C_M_RD;
    	msgs.addr  = client->addr;
    	msgs.len   = 0;
    	msgs.buf   = NULL;            ///buf[0] buf[1] is reg addr
    	msgs.scl_rate = 10 * 1000;    // for Rockchip
    	msgs.udelay = 5; 

	while(retries < 5) 
	{    
		ret = i2c_transfer(client->adapter, &msgs, 1);
		if(ret == 1)break;
		retries++;
	}    

	if(retries >= 5)
	{    
		ret = 0xE0;   //just for suit original interface
		printk("%s : %d: test read error\n", __func__, __LINE__);
	}    
	else
	{
		ret = 0;
	}

	return ret; 
}

UINT8 SHA204_resync(UINT8 size, UINT8 *response)
{
	return SHA204_resync_platform(atsha204a_client, size, response);
}

/** This I2C function puts the SHA204 device into low-power state.
 *  \return status of the operation
 */
UINT8 SHA204_sleep(void)
{
	return SHA204_I2C_send(SHA204_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}

/**This I2C function sends a command to the device.
 */
UINT8 SHA204_Send_command(UINT8 count, UINT8 *command)
{
	return SHA204_I2C_send(SHA204_I2C_PACKET_FUNCTION_NORMAL, count, command);
}

/** This TWI function receives a response from the SHA204 device. */
UINT8 SHA204_Receive_response_platform(struct i2c_client *client, u8 size, u8 *response)
{
    struct i2c_msg msgs;
    u8 count; 
    s32 ret = 0; 
    s32 retries = 0; 
    
    memset(response, 0, size);

    msgs.flags = I2C_M_RD;
    msgs.addr  = client->addr;
    msgs.len   = 1;
    msgs.buf   = &response[0];            ///buf[0] buf[1] is reg addr
    msgs.scl_rate = 10 * 1000;    // for Rockchip
    msgs.udelay = 5; 

    ret = i2c_transfer(client->adapter, &msgs, 1);
    if(ret != 1){    
	ret = 0xE0;  
        printk("%d: test read error\n", __LINE__);
    }    

    //printk("response[0] = 0x%x size = %d\n", response[0], size);

    count = response[0];
    if ((count < 4) || (count > size))
    {
        printk("\nline: %d func : %s ;count  %d \n", __LINE__, __func__, count);
	return 0xE4;
    }

    retries = 0;
    msgs.flags = I2C_M_RD;
    msgs.addr  = client->addr;
    msgs.len   = count - 1;
    msgs.buf   = &response[1];            
    msgs.scl_rate = 10 * 1000;    // for Rockchip
    msgs.udelay = 5; 

    while(retries < 5) 
    {    
        ret = i2c_transfer(client->adapter, &msgs, 1);
        if(ret == 1)break;
        retries++;
    }    
    if(retries >= 5)
    {    
	ret = 0xE0;   //just for suit original interface
        printk("%d: test read error\n", __LINE__);
    }    
/*
    for(retries = 1; retries < count; retries++) 
    {
	printk("response[%d] = 0x%x \n", retries, response[retries]);
    }
*/
    if(ret == 1)
	ret = 0;
    return ret; 

}

UINT8 SHA204_Receive_response(UINT8 size, UINT8 *response)
{
	return SHA204_Receive_response_platform(atsha204a_client, size, response);
}


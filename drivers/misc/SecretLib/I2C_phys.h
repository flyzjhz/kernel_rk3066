/****************************************************************************
 *   FileName    : I2C_phys.h
 *   Description :
 ****************************************************************************
*
*   Copyright (c) Nowada, Inc.
*   ALL RIGHTS RESERVED
*
*	Created: 2011-11-28
*  	Author: Jackson Meng
****************************************************************************/

#ifndef I2C_PHYS_H_
#define I2C_PHYS_H_

// error codes for physical hardware dependent module
#define I2C_FUNCTION_RETCODE_SUCCESS     0x00 //!< Communication with device succeeded.
#define I2C_FUNCTION_RETCODE_COMM_FAIL   0xF0 //!< Communication with device failed.
#define I2C_FUNCTION_RETCODE_TIMEOUT     0xF1 //!< Communication timed out.
#define I2C_FUNCTION_RETCODE_NACK        0xF8 //!< TWI nack

UINT8 SHA204_wakeup(void);
UINT8 SHA204_Receive_response(UINT8 size, UINT8 *response);
UINT8 SHA204_resync(UINT8 size, UINT8 *response);
UINT8 SHA204_sleep(void);
BOOL  AT_I2C_Init(void);
UINT8 SHA204_Send_command(UINT8 count, UINT8 *command);

#endif

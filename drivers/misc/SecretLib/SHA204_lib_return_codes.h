/****************************************************************************
 *   FileName    : SHA204_lib_return_codes.h
 *   Description :
 ****************************************************************************
*
*   Copyright (c) Nowada, Inc.
*   ALL RIGHTS RESERVED
*
*	Created: 2011-11-28
*  	Author: Jackson Meng
****************************************************************************/

#ifndef SHA204_LIB_RETURN_CODES_H
#define SHA204_LIB_RETURN_CODES_H


/** \todo Use same values for same meanings for SHA204 and AES132.
 * */

#define SHA204_SUCCESS              0x00 //!< Function succeeded.
#define SHA204_PARSE_ERROR          0xD2 //!< response status byte indicates parsing error
#define SHA204_CMD_FAIL             0xD3 //!< response status byte indicates command execution error
#define SHA204_STATUS_CRC           0xD4 //!< response status byte indicates CRC error
#define SHA204_STATUS_UNKNOWN       0xD5 //!< response status byte is unknown
#define SHA204_FUNC_FAIL            0xE0 //!< Function could not execute due to incorrect condition / state.
#define SHA204_GEN_FAIL             0xE1 //!< unspecified error
#define SHA204_BAD_PARAM            0xE2 //!< bad argument (out of range, null pointer, etc.)
#define SHA204_INVALID_ID           0xE3 //!< invalid device id, id not set
#define SHA204_INVALID_SIZE         0xE4 //!< Count value is out of range or greater than buffer size.
#define SHA204_BAD_CRC              0xE5 //!< incorrect CRC received
#define SHA204_RX_FAIL              0xE6 //!< Timed out while waiting for response. Number of bytes received is > 0.
#define SHA204_RX_NO_RESPONSE       0xE7 //!< Not an error while the Command layer is polling for a command response.
#define SHA204_RESYNC_WITH_WAKEUP   0xE8 //!< re-synchronization succeeded, but only after generating a Wake-up

#define SHA204_COMM_FAIL            0xF0 //!< Communication with device failed. Same as in hardware dependent modules.
#define SHA204_TIMEOUT              0xF1 //!< Timed out while waiting for response. Number of bytes received is 0.

#endif

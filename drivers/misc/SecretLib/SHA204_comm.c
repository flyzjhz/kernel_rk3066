/****************************************************************************
 *   FileName    : SHA204_comm.c
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
#include "SHA204_comm.h"                // definitions and declarations for the Communication module
#include "SHA204_lib_return_codes.h"    // declarations of function return codes
#include "SHA204.h"
#include "I2C_phys.h"


///////////////////////////////////////////////////////////////////static
void Delay_ms_old( int iTime)
{
	volatile int i,j;
	volatile int Count=0;

	i = iTime;

    while( i-- >0 )
    {
		Count = 0x9000;

    	for(j=0; j<Count; j++);
    }
}

void Delay_ms( int iTime)
{
	mdelay(iTime);
}

/** \brief This function calculates CRC.
 *
 * \param[in] length number of bytes in buffer
 * \param[in] data pointer to data for which CRC should be calculated
 * \param[out] crc pointer to 16-bit CRC
 */
void SHA204c_Calculate_crc(UINT8 length, UINT8 *data, UINT8 *crc)
{
	UINT8 counter;
	UINT16 crc_register = 0;
	UINT16 polynom = 0x8005;
	UINT8 shift_register;
	UINT8 data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
	  for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
		 data_bit = (data[counter] & shift_register) ? 1 : 0;
		 crc_bit = crc_register >> 15;

		 // Shift CRC to the left by 1.
		 crc_register <<= 1;

		 if ((data_bit ^ crc_bit) != 0)
			crc_register ^= polynom;
	  }
	}
	crc[0] = (UINT8) (crc_register & 0x00FF);
	crc[1] = (UINT8) (crc_register >> 8);
}


/** \brief This function checks the consistency of a response.
 * \param[in] response pointer to response
 * \return status of the consistency check
 */
UINT8 SHA204c_Check_crc(UINT8 *response)
{
	UINT8 crc[SHA204_CRC_SIZE];
	UINT8 count = response[SHA204_BUFFER_POS_COUNT];

	count -= SHA204_CRC_SIZE;
	SHA204c_Calculate_crc(count, response, crc);

	return (crc[0] == response[count] && crc[1] == response[count + 1])
		? SHA204_SUCCESS : SHA204_BAD_CRC;
}


/** \brief This function wakes up a SHA204 device
 *         and receives a response.
 *  \param[out] response pointer to four-byte response
 *  \return status of the operation
 */
UINT8 SHA204c_wakeup(UINT8 *response)
{
	UINT8 ret_code = SHA204_wakeup();
	if (ret_code != SHA204_SUCCESS)
		return ret_code;

	ret_code = SHA204_Receive_response(SHA204_RSP_SIZE_MIN, response);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;

	// Verify status response.
	if (response[SHA204_BUFFER_POS_COUNT] != SHA204_RSP_SIZE_MIN)
		ret_code = SHA204_INVALID_SIZE;
	else if (response[SHA204_BUFFER_POS_STATUS] != SHA204_STATUS_BYTE_WAKEUP)
		ret_code = SHA204_COMM_FAIL;
	else {
		if ((response[SHA204_RSP_SIZE_MIN - SHA204_CRC_SIZE] != 0x33)
					|| (response[SHA204_RSP_SIZE_MIN + 1 - SHA204_CRC_SIZE] != 0x43))
			ret_code = SHA204_BAD_CRC;
	}

	if (ret_code != SHA204_SUCCESS)
		KITLOutputDebugString("\n Wakeup error 0x%x\n",ret_code);

	return ret_code;
}

/** \brief This function re-synchronizes communication.
 *
  Be aware that succeeding only after waking up the
  device could mean that it had gone to sleep and lost
  its TempKey in the process.\n
  Re-synchronizing communication is done in a maximum of
  three steps:
  <ol>
    <li>
      Try to re-synchronize without sending a Wake token.
      This step is implemented in the Physical layer.
    </li>
    <li>
      If the first step did not succeed send a Wake token.
    </li>
    <li>
      Try to read the Wake response.
    </li>
  </ol>
 *
 * \param[in] size size of response buffer
 * \param[out] response pointer to Wake-up response buffer
 * \return status of the operation
 */
UINT8 SHA204c_resync(UINT8 size, UINT8 *response)
{
	// Try to re-synchronize without sending a Wake token
	// (step 1 of the re-synchronization process).
	UINT8 ret_code = SHA204_resync(size, response);
	if (ret_code == SHA204_SUCCESS)
		return ret_code;

	// We lost communication. Send a Wake pulse and try
	// to receive a response (steps 2 and 3 of the
	// re-synchronization process).
	(void) SHA204_sleep();
	ret_code = SHA204_wakeup();

	// Translate a return value of success into one
	// that indicates that the device had to be woken up
	// and might have lost its TempKey.
	return (ret_code == SHA204_SUCCESS ? SHA204_RESYNC_WITH_WAKEUP : ret_code);
}


/** \brief This function runs a communication sequence:
 * Append CRC to tx buffer, send command, delay, and verify response after receiving it.
 *
 * The first byte in tx buffer must be the byte count of the packet.
 * If CRC or count of the response is incorrect, or a command byte got "nacked" (TWI),
 * this function requests re-sending the response.
 * If the response contains an error status, this function resends the command.
 *
 * \param[in] tx_buffer pointer to command
 * \param[in] rx_size size of response buffer
 * \param[out] rx_buffer pointer to response buffer
 * \param[in] execution_delay Start polling for a response after this many ms .
 * \param[in] execution_timeout polling timeout in ms
 * \return status of the operation
 */
UINT8 SHA204c_Send_and_receive(UINT8 *tx_buffer, UINT8 rx_size, UINT8 *rx_buffer,
			UINT8 execution_delay, UINT8 execution_timeout)
{
	UINT8 ret_code = SHA204_FUNC_FAIL;
	UINT8 ret_code_resync;
	UINT8 n_retries_send;
	UINT8 n_retries_receive;
	UINT8 i;
	UINT8 status_byte;
	UINT8 count = tx_buffer[SHA204_BUFFER_POS_COUNT];
	UINT8 count_minus_crc = count - SHA204_CRC_SIZE;
	//UINT16 execution_timeout_us = (UINT16) (execution_timeout * 1000) + SHA204_RESPONSE_TIMEOUT;
	//volatile UINT16 timeout_countdown;

	// Append CRC.
	SHA204c_Calculate_crc(count_minus_crc, tx_buffer, tx_buffer + count_minus_crc);

	// Retry loop for sending a command and receiving a response.
	n_retries_send = SHA204_RETRY_COUNT + 1;

	while ((n_retries_send-- > 0) && (ret_code != SHA204_SUCCESS))
	{
		// Send command.
		ret_code = SHA204_Send_command(count, tx_buffer);
		if (ret_code != SHA204_SUCCESS)
		{
			if (SHA204c_resync(rx_size, rx_buffer) == SHA204_RX_NO_RESPONSE)
			{
				// The device seems to be dead in the water.
				return ret_code;
			}
			else
				continue;
		}

		// Wait minimum command execution time and then start polling for a response.
		Delay_ms(execution_delay+50);

		// Retry loop for receiving a response.
		n_retries_receive = SHA204_RETRY_COUNT + 1;
		while (n_retries_receive-- > 0)
		{

			// Reset response buffer.
			for (i = 0; i < rx_size; i++)
				rx_buffer[i] = 0;

			// Poll for response.
//			timeout_countdown = execution_timeout_us;
//			do
//			{
				ret_code = SHA204_Receive_response(rx_size, rx_buffer);
//				timeout_countdown -= SHA204_RESPONSE_TIMEOUT;
//			} while ((timeout_countdown > SHA204_RESPONSE_TIMEOUT) && (ret_code == SHA204_RX_NO_RESPONSE));

			if (ret_code == SHA204_RX_NO_RESPONSE)
			{
				// We did not receive a response. Re-synchronize and send command again.
				if (SHA204c_resync(rx_size, rx_buffer) == SHA204_RX_NO_RESPONSE)
					// The device seems to be dead in the water.
					return ret_code;
				else
					break;
			}

			// Check whether we received a valid response.
			if (ret_code == SHA204_INVALID_SIZE)
			{
				// We see 0xFF for the count when communication got out of sync.
				ret_code_resync = SHA204c_resync(rx_size, rx_buffer);
				if (ret_code_resync == SHA204_SUCCESS)
					// We did not have to wake up the device. Try receiving response again.
					continue;
				if (ret_code_resync == SHA204_RESYNC_WITH_WAKEUP)
					// We could re-synchronize, but only after waking up the device.
					// Re-send command.
					break;
				else
					// We failed to re-synchronize.
					return ret_code;
			}

			// We received a response of valid size.
			// Check the consistency of the response.
			ret_code = SHA204c_Check_crc(rx_buffer);
			if (ret_code == SHA204_SUCCESS)
			{
				// Received valid response.
				if (rx_buffer[SHA204_BUFFER_POS_COUNT] > SHA204_RSP_SIZE_MIN)
					// Received non-status response. We are done.
					return ret_code;

				// Received status response.
				status_byte = rx_buffer[SHA204_BUFFER_POS_STATUS];

				// Translate the three possible device status error codes
				// into library return codes.
				if (status_byte == SHA204_STATUS_BYTE_PARSE)
					return SHA204_PARSE_ERROR;
				if (status_byte == SHA204_STATUS_BYTE_EXEC)
					return SHA204_CMD_FAIL;
				if (status_byte == SHA204_STATUS_BYTE_COMM)
				{
					// In case of the device status byte indicating a communication
					// error this function exits the retry loop for receiving a response
					// and enters the overall retry loop
					// (send command / receive response).
					ret_code = SHA204_STATUS_CRC;
					break;
				}

				// Received status response from CheckMAC, DeriveKey, GenDig,
				// Lock, Nonce, Pause, UpdateExtra, or Write command.
				return ret_code;
			}
			else
			{
				KITLOutputDebugString("\n  SHA204c_Check_crc OK:");
				// Received response with incorrect CRC.
				ret_code_resync = SHA204c_resync(rx_size, rx_buffer);
				if (ret_code_resync == SHA204_SUCCESS)
					// We did not have to wake up the device. Try receiving response again.
					continue;
				if (ret_code_resync == SHA204_RESYNC_WITH_WAKEUP)
					// We could re-synchronize, but only after waking up the device.
					// Re-send command.
					break;
				else
					// We failed to re-synchronize.
					return ret_code;
			} // block end of check response consistency

		} // block end of receive retry loop

	} // block end of send and receive retry loop

	return ret_code;
}

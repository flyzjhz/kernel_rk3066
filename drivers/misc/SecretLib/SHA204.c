/****************************************************************************
 *   FileName    : SHA204.c
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

//************************************************************************

/* The current device revision */
const UINT8 SHA204_DEVREV_VALUE[4] = {0x00, 0x00, 0x00, 0x04}; /* The device revision - currently DevRev 4 */

/** \brief This function creates a command packet, sends it, and receives its response.
 *
 * \param[in] op_code command op-code
 * \param[in] param1 first parameter
 * \param[in] param2 second parameter
 * \param[in] datalen1 number of bytes in first data block
 * \param[in] data1 pointer to first data block
 * \param[in] datalen2 number of bytes in second data block
 * \param[in] data2 pointer to second data block
 * \param[in] datalen3 number of bytes in third data block
 * \param[in] data3 pointer to third data block
 * \param[in] tx_size size of tx buffer
 * \param[in] tx_buffer pointer to tx buffer
 * \param[in] rx_size size of rx buffer
 * \param[out] rx_buffer pointer to rx buffer
 * \return status of the operation
 */
UINT8 SHA204_Execute(UINT8 op_code, UINT8 param1, UINT16 param2,
			UINT8 datalen1, UINT8 *data1, UINT8 datalen2, UINT8 *data2, UINT8 datalen3, UINT8 *data3,
			UINT8 tx_size, UINT8 *tx_buffer, UINT8 rx_size, UINT8 *rx_buffer)
{
	UINT8 poll_delay, poll_timeout, response_size;
	UINT8 *p_buffer;
	UINT8 len;

	// Supply delays and response size.
	switch (op_code)
	{
	case SHA204_CHECKMAC:
		poll_delay = CHECKMAC_DELAY;
		poll_timeout = CHECKMAC_EXEC_MAX - CHECKMAC_DELAY;
		response_size = CHECKMAC_RSP_SIZE;
		break;

	case SHA204_DERIVE_KEY:
		poll_delay = DERIVE_KEY_DELAY;
		poll_timeout = DERIVE_KEY_EXEC_MAX - DERIVE_KEY_DELAY;
		response_size = DERIVE_KEY_RSP_SIZE;
		break;

	case SHA204_DEVREV:
		poll_delay = DEVREV_DELAY;
		poll_timeout = DEVREV_EXEC_MAX - DEVREV_DELAY;
		response_size = DEVREV_RSP_SIZE;
		break;

	case SHA204_GENDIG:
		poll_delay = GENDIG_DELAY;
		poll_timeout = GENDIG_EXEC_MAX - GENDIG_DELAY;
		response_size = GENDIG_RSP_SIZE;
		break;

	case SHA204_HMAC:
		poll_delay = HMAC_DELAY;
		poll_timeout = HMAC_EXEC_MAX - HMAC_DELAY;
		response_size = HMAC_RSP_SIZE;
		break;

	case SHA204_LOCK:
		poll_delay = LOCK_DELAY;
		poll_timeout = LOCK_EXEC_MAX - LOCK_DELAY;
		response_size = LOCK_RSP_SIZE;
		break;

	case SHA204_MAC:
		poll_delay = MAC_DELAY;
		poll_timeout = MAC_EXEC_MAX - MAC_DELAY;
		response_size = MAC_RSP_SIZE;
		break;

	case SHA204_NONCE:
		poll_delay = NONCE_DELAY;
		poll_timeout = NONCE_EXEC_MAX - NONCE_DELAY;
		response_size = param1 == 0x03					//pass-through
							? SHA204_RSP_SIZE_MIN : SHA204_RSP_SIZE_MAX;
		break;

	case SHA204_PAUSE:
		poll_delay = PAUSE_DELAY;
		poll_timeout = PAUSE_EXEC_MAX - PAUSE_DELAY;
		response_size = PAUSE_RSP_SIZE;
		break;

	case SHA204_RANDOM:
		poll_delay = RANDOM_DELAY;
		poll_timeout = RANDOM_EXEC_MAX - RANDOM_DELAY;
		response_size = RANDOM_RSP_SIZE;
		break;

	case SHA204_READ:
		poll_delay = READ_DELAY;
		poll_timeout = READ_EXEC_MAX - READ_DELAY;
		response_size = (param1 & SHA204_ZONE_COUNT_FLAG)				//pass-through
							? READ_32_RSP_SIZE : READ_4_RSP_SIZE;
		break;

	case SHA204_TEMPSENSE:
		poll_delay = TEMP_SENSE_DELAY;
		poll_timeout = TEMP_SENSE_EXEC_MAX - TEMP_SENSE_DELAY;
		response_size = TEMP_SENSE_RSP_SIZE;
		break;

	case SHA204_UPDATE_EXTRA:
		poll_delay = UPDATE_DELAY;
		poll_timeout = UPDATE_EXEC_MAX - UPDATE_DELAY;
		response_size = UPDATE_RSP_SIZE;
		break;

	case SHA204_WRITE:
		poll_delay = WRITE_DELAY;
		poll_timeout = WRITE_EXEC_MAX - WRITE_DELAY;
		response_size = WRITE_RSP_SIZE;
		break;

	default:
		poll_delay = 0;
		poll_timeout = SHA204_COMMAND_EXEC_MAX;
		response_size = rx_size;
	}

	// Assemble command.
	len = datalen1 + datalen2 + datalen3 + SHA204_CMD_SIZE_MIN;
	p_buffer = tx_buffer;
	*p_buffer++ = len;
	*p_buffer++ = op_code;
	*p_buffer++ = param1;
	*p_buffer++ = param2 & 0xFF;
	*p_buffer++ = param2 >> 8;

	if (datalen1 > 0)
	{
		memcpy(p_buffer, data1, datalen1);
		p_buffer += datalen1;
	}
	if (datalen2 > 0)
	{
		memcpy(p_buffer, data2, datalen2);
		p_buffer += datalen1;
	}
	if (datalen3 > 0)
	{
		memcpy(p_buffer, data3, datalen3);
		p_buffer += datalen3;
	}

	SHA204c_Calculate_crc(len - SHA204_CRC_SIZE, tx_buffer, p_buffer);

	// Send command and receive response.
	return SHA204c_Send_and_receive(&tx_buffer[0], response_size,
				&rx_buffer[0],	poll_delay, poll_timeout);
}

/** This function sends a CheckMAC command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode selects the hash inputs
 * \param[in]  key_id slot index of key
 * \param[in]  client_challenge pointer to client challenge (ignored if mode bit 0 is set)
 * \param[in]  client_response pointer to client response
 * \param[in]  other_data pointer to 13 bytes of data used in the client command
 * \return status of the operation
 */
UINT8 SHA204_Check_Mac(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 mode, UINT8 key_id, UINT8 *client_challenge, UINT8 *client_response, UINT8 *other_data)
{
	if (// no null pointers allowed
		!tx_buffer || !rx_buffer || !client_response || !other_data
		// No reserved bits should be set.
		|| (mode | CHECKMAC_MODE_MASK) != CHECKMAC_MODE_MASK
		// key_id > 15 not allowed
		|| key_id > SHA204_KEY_ID_MAX)
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = CHECKMAC_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_CHECKMAC;
	tx_buffer[CHECKMAC_MODE_IDX] = mode & CHECKMAC_MODE_MASK;
	tx_buffer[CHECKMAC_KEYID_IDX]= key_id;
	tx_buffer[CHECKMAC_KEYID_IDX + 1] = 0;
	if (client_challenge == NULL)
		memset(&tx_buffer[CHECKMAC_CLIENT_CHALLENGE_IDX], 0, CHECKMAC_CLIENT_CHALLENGE_SIZE);
	else
		memcpy(&tx_buffer[CHECKMAC_CLIENT_CHALLENGE_IDX], client_challenge, CHECKMAC_CLIENT_CHALLENGE_SIZE);

	memcpy(&tx_buffer[CHECKMAC_CLIENT_RESPONSE_IDX], client_response, CHECKMAC_CLIENT_RESPONSE_SIZE);
	memcpy(&tx_buffer[CHECKMAC_DATA_IDX], other_data, CHECKMAC_OTHER_DATA_SIZE);

	return SHA204c_Send_and_receive(&tx_buffer[0], CHECKMAC_RSP_SIZE, &rx_buffer[0],
				CHECKMAC_DELAY, CHECKMAC_EXEC_MAX - CHECKMAC_DELAY);
}

/**This function sends a DeriveKey command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  random type of source key (0: slot; 1: parent key)
 * \param[in]  target_key slot index of key (0..15); not used if random is 1
 * \param[in]  mac pointer to optional MAC
 * \return status of the operation
 */
UINT8 SHA204_Derive_Key(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 random, UINT8 target_key, UINT8 *mac)
{
	if (!tx_buffer || !rx_buffer || ((random & ~DERIVE_KEY_RANDOM_FLAG) != 0)
				 || (target_key > SHA204_KEY_ID_MAX))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_OPCODE_IDX] = SHA204_DERIVE_KEY;
	tx_buffer[DERIVE_KEY_RANDOM_IDX] = random;
	tx_buffer[DERIVE_KEY_TARGETKEY_IDX] = target_key;
	tx_buffer[DERIVE_KEY_TARGETKEY_IDX + 1] = 0;
	if (mac != NULL)
	{
		memcpy(&tx_buffer[DERIVE_KEY_MAC_IDX], mac, DERIVE_KEY_MAC_SIZE);
		tx_buffer[SHA204_COUNT_IDX] = DERIVE_KEY_COUNT_LARGE;
	}
	else
		tx_buffer[SHA204_COUNT_IDX] = DERIVE_KEY_COUNT_SMALL;

	return SHA204c_Send_and_receive(&tx_buffer[0], DERIVE_KEY_RSP_SIZE, &rx_buffer[0],
				DERIVE_KEY_DELAY, DERIVE_KEY_EXEC_MAX - DERIVE_KEY_DELAY);
}

/**This function sends a DevRev command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \return status of the operation
 */
UINT8 SHA204_Dev_Rev(UINT8 *tx_buffer, UINT8 *rx_buffer)
{
	if (!tx_buffer || !rx_buffer)
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = DEVREV_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_DEVREV;

	// Parameters are 0.
	tx_buffer[DEVREV_PARAM1_IDX] =
	tx_buffer[DEVREV_PARAM2_IDX] =
	tx_buffer[DEVREV_PARAM2_IDX + 1] = 0;

	return SHA204c_Send_and_receive(&tx_buffer[0], DEVREV_RSP_SIZE, &rx_buffer[0],
				DEVREV_DELAY, DEVREV_EXEC_MAX - DEVREV_DELAY);
}

/**This function sends a GenDig command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  zone 1: OTP zone, 2: data zone
 * \param[in]  key_id zone 1: OTP block; zone 2: key id
 * \param[in]  other_data pointer to 4 bytes of data when using CheckOnly key
 * \return status of the operation
 */
UINT8 SHA204_Gen_Dig(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 zone, UINT8 key_id, UINT8 *other_data)
{
	if (!tx_buffer || !rx_buffer
				|| ((zone != GENDIG_ZONE_OTP) && (zone != GENDIG_ZONE_DATA)))
		return SHA204_BAD_PARAM;

	if (((zone == GENDIG_ZONE_OTP) && (key_id > SHA204_OTP_BLOCK_MAX))
				|| ((zone == GENDIG_ZONE_DATA) && (key_id > SHA204_KEY_ID_MAX)))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_OPCODE_IDX] = SHA204_GENDIG;
	tx_buffer[GENDIG_ZONE_IDX] = zone;
	tx_buffer[GENDIG_KEYID_IDX] = key_id;
	tx_buffer[GENDIG_KEYID_IDX + 1] = 0;
	if (other_data != NULL)
	{
		memcpy(&tx_buffer[GENDIG_DATA_IDX], other_data, GENDIG_OTHER_DATA_SIZE);
		tx_buffer[SHA204_COUNT_IDX] = GENDIG_COUNT_DATA;
	}
	else
		tx_buffer[SHA204_COUNT_IDX] = GENDIG_COUNT;

	return SHA204c_Send_and_receive(&tx_buffer[0], GENDIG_RSP_SIZE, &rx_buffer[0],
				GENDIG_DELAY, GENDIG_EXEC_MAX - GENDIG_DELAY);

}

/**This function sends an HMAC command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode
 * \param[in]  key_id slot index of key
 * \return status of the operation
 */
UINT8 SHA204_Hmac(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT16 key_id)
{
	if (!tx_buffer || !rx_buffer || ((mode & ~HMAC_MODE_MASK) != 0))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = HMAC_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_HMAC;
	tx_buffer[HMAC_MODE_IDX] = mode;

	// Although valid key identifiers are only
	// from 0 to 15, all 16 bits are used in the HMAC message.
	tx_buffer[HMAC_KEYID_IDX] = key_id & 0xFF;
	tx_buffer[HMAC_KEYID_IDX + 1] = key_id >> 8;

	return SHA204c_Send_and_receive(&tx_buffer[0], HMAC_RSP_SIZE, &rx_buffer[0],
				HMAC_DELAY, HMAC_EXEC_MAX - HMAC_DELAY);
}

/**This function sends a Lock command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  zone zone id to lock
 * \param[in]  summary zone digest
 * \return status of the operation
 */
UINT8 SHA204_Lock(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 zone, UINT16 summary)
{
	if (!tx_buffer || !rx_buffer || ((zone & ~LOCK_ZONE_MASK) != 0)
				|| ((zone & LOCK_ZONE_NO_CRC) && (summary != 0)))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = LOCK_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_LOCK;
	tx_buffer[LOCK_ZONE_IDX] = zone & LOCK_ZONE_MASK;
	tx_buffer[LOCK_SUMMARY_IDX]= summary & 0xFF;
	tx_buffer[LOCK_SUMMARY_IDX + 1]= summary >> 8;

	return SHA204c_Send_and_receive(&tx_buffer[0], LOCK_RSP_SIZE, &rx_buffer[0],
				LOCK_DELAY, LOCK_EXEC_MAX - LOCK_DELAY);
}

/**This function sends a MAC command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode selects message fields
 * \param[in]  key_id slot index of key
 * \param[in]  challenge pointer to challenge (not used if mode bit 0 is set)
 * \return status of the operation
 */
UINT8 SHA204_Mac(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 mode, UINT16 key_id, UINT8 *challenge)
{
	if (!tx_buffer || !rx_buffer || ((mode & ~MAC_MODE_MASK) != 0)
				|| (((mode & MAC_MODE_BLOCK2_TEMPKEY) == 0) && !challenge))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = MAC_COUNT_SHORT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_MAC;
	tx_buffer[MAC_MODE_IDX] = mode;
	tx_buffer[MAC_KEYID_IDX] = key_id & 0xFF;
	tx_buffer[MAC_KEYID_IDX + 1] = key_id >> 8;
	if ((mode & MAC_MODE_BLOCK2_TEMPKEY) == 0)
	{
		memcpy(&tx_buffer[MAC_CHALLENGE_IDX], challenge, MAC_CHALLENGE_SIZE);
		tx_buffer[SHA204_COUNT_IDX] = MAC_COUNT_LONG;
	}

	return SHA204c_Send_and_receive(&tx_buffer[0], MAC_RSP_SIZE, &rx_buffer[0],
				MAC_DELAY, MAC_EXEC_MAX - MAC_DELAY);
}

/**This function sends a Nonce command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode controls the mechanism of the internal random number generator and seed update
 * \param[in]  numin pointer to system input\n
 *             (mode = 3: 32 bytes same as in TempKey;\n
 *              mode < 2: 20 bytes\n
 *              mode == 2: not allowed)
 * \return status of the operation
 */
UINT8  SHA204_Nonce(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT8 *numin)
{
	UINT8 rx_size;

	if (!tx_buffer || !rx_buffer || !numin
				|| (mode > NONCE_MODE_PASSTHROUGH) || (mode == NONCE_MODE_INVALID))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_OPCODE_IDX] = SHA204_NONCE;
	tx_buffer[NONCE_MODE_IDX] = mode;

	// 2. parameter is 0.
	tx_buffer[NONCE_PARAM2_IDX] = tx_buffer[NONCE_PARAM2_IDX + 1] = 0;

	if (mode != NONCE_MODE_PASSTHROUGH)
	{
		memcpy(&tx_buffer[NONCE_INPUT_IDX], numin, NONCE_NUMIN_SIZE);
		tx_buffer[SHA204_COUNT_IDX] = NONCE_COUNT_SHORT;
		rx_size = NONCE_RSP_SIZE_LONG;
	}
	else
	{
		memcpy(&tx_buffer[NONCE_INPUT_IDX], numin, NONCE_NUMIN_SIZE_PASSTHROUGH);
		tx_buffer[SHA204_COUNT_IDX] = NONCE_COUNT_LONG;
		rx_size = NONCE_RSP_SIZE_SHORT;
	}

	return SHA204c_Send_and_receive(&tx_buffer[0], rx_size, &rx_buffer[0],
				NONCE_DELAY, NONCE_EXEC_MAX - NONCE_DELAY);
}

/**This function sends a Pause command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  selector Devices not matching this value will pause.
 * \return status of the operation
 */
UINT8 SHA204_Pause(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 selector)
{
	if (!tx_buffer || !rx_buffer)
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = PAUSE_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_PAUSE;
	tx_buffer[PAUSE_SELECT_IDX] = selector;

	// 2. parameter is 0.
	tx_buffer[PAUSE_PARAM2_IDX] =
	tx_buffer[PAUSE_PARAM2_IDX + 1] = 0;

	return SHA204c_Send_and_receive(&tx_buffer[0], PAUSE_RSP_SIZE, &rx_buffer[0],
				PAUSE_DELAY, PAUSE_EXEC_MAX - PAUSE_DELAY);
}

/**This function sends a Random command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode 0: update seed; 1: no seed update
 * \return status of the operation
 */
UINT8 SHA204_Random(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode)
{
	if (!tx_buffer || !rx_buffer || (mode > RANDOM_NO_SEED_UPDATE))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = RANDOM_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_RANDOM;
	tx_buffer[RANDOM_MODE_IDX] = mode & RANDOM_SEED_UPDATE;

	// 2. parameter is 0.
	tx_buffer[RANDOM_PARAM2_IDX] = tx_buffer[RANDOM_PARAM2_IDX + 1] = 0;


	return SHA204c_Send_and_receive(&tx_buffer[0], RANDOM_RSP_SIZE, &rx_buffer[0],
				RANDOM_DELAY, RANDOM_EXEC_MAX - RANDOM_DELAY);
}

/** \brief This function sends a Read command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  zone 0: Configuration; 1: OTP; 2: Data
 * \param[in]  byte address address to read from\n
 * \return status of the operation
 */
UINT8 SHA204_Read(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 zone, UINT16 address)
{
	UINT8 rx_size;

	if (!tx_buffer || !rx_buffer || ((zone & ~READ_ZONE_MASK) != 0)
				|| ((zone & READ_ZONE_MODE_32_BYTES) && (zone == SHA204_ZONE_OTP)))
		return SHA204_BAD_PARAM;

	if (zone & SHA204_ZONE_DATA)
	{
		address >>= 2;
		if (address & 1)
			// If we would just mask this bit, we would
			// read from an address that was not intended.
			return SHA204_BAD_PARAM;
	}

	tx_buffer[SHA204_COUNT_IDX] = READ_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_READ;
	tx_buffer[READ_ZONE_IDX] = zone;
	tx_buffer[READ_ADDR_IDX] = (UINT8) (address & SHA204_ADDRESS_MASK);
	tx_buffer[READ_ADDR_IDX + 1] = 0;

	rx_size = (zone & SHA204_ZONE_COUNT_FLAG) ? READ_32_RSP_SIZE : READ_4_RSP_SIZE;

	return SHA204c_Send_and_receive(&tx_buffer[0], rx_size, &rx_buffer[0],
				READ_DELAY, READ_EXEC_MAX - READ_DELAY);
}

/** \brief This function sends a TempSense command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[out]  temp_raw pointer to raw temperature value\n
 *              T(C) = 0.855 * (temp_raw + T(offset) - 334)\n
 *                 whereT(offset) is a field in the configuration zone.
 * \return status of the operation
 */
UINT8 SHA204_Temp_Sense(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT16 *temp_raw)
{
	UINT8 ret_code;
	UINT8 *temp_data;
	UINT16 temp_high, temp_low;

	if (!tx_buffer || !rx_buffer || !temp_raw)
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = TEMP_SENSE_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_TEMPSENSE;

	// Parameters are 0.
	tx_buffer[TEMP_SENSE_PARAM1_IDX] =
	tx_buffer[TEMP_SENSE_PARAM2_IDX] =
	tx_buffer[TEMP_SENSE_PARAM2_IDX + 1] = 0;

	ret_code = SHA204c_Send_and_receive(&tx_buffer[0], TEMP_SENSE_RSP_SIZE, &rx_buffer[0],
				TEMP_SENSE_DELAY, TEMP_SENSE_EXEC_MAX - TEMP_SENSE_DELAY);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;

	// Calculate temp_raw.
	temp_data = &rx_buffer[SHA204_BUFFER_POS_DATA];
	temp_high = *temp_data++ << 8;
	temp_high += *temp_data++;
	temp_low = *temp_data++ << 8;
	temp_low += *temp_data;
	*temp_raw = temp_high - temp_low;

	return ret_code;
}

/** This function sends an UpdateExtra command to the device.
* \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  mode 0: update Configuration zone byte 85; 1: byte 86
 * \param[in]  new_value byte to write
 * \return status of the operation
 */
UINT8 SHA204_Update_extra(UINT8 *tx_buffer, UINT8 *rx_buffer, UINT8 mode, UINT8 new_value)
{
	if (!tx_buffer || !rx_buffer || (mode > UPDATE_CONFIG_BYTE_86))
		return SHA204_BAD_PARAM;

	tx_buffer[SHA204_COUNT_IDX] = UPDATE_COUNT;
	tx_buffer[SHA204_OPCODE_IDX] = SHA204_UPDATE_EXTRA;
	tx_buffer[UPDATE_MODE_IDX] = mode;
	tx_buffer[UPDATE_VALUE_IDX] = new_value;
	tx_buffer[UPDATE_VALUE_IDX + 1] = 0;

	return SHA204c_Send_and_receive(&tx_buffer[0], UPDATE_RSP_SIZE, &rx_buffer[0],
				UPDATE_DELAY, UPDATE_EXEC_MAX - UPDATE_DELAY);
}

/** This function sends a Write command to the device.
 * \param[in]  tx_buffer pointer to transmit buffer
 * \param[out] rx_buffer pointer to receive buffer
 * \param[in]  zone 0: Configuration; 1: OTP; 2: Data
 * \param[in]  address address to write to\n
 * \param[in]  new_value pointer to 32 (zone bit 7 set) or 4 bytes of data
 * \param[in]  mac pointer to MAC (ignored if zone is unlocked)
 * \return status of the operation
 */
UINT8 SHA204_Write(UINT8 *tx_buffer, UINT8 *rx_buffer,
			UINT8 zone, UINT16 address, UINT8 *new_value, UINT8 *mac)
{
	UINT8 *p_command;
	UINT8 count;

	if (!tx_buffer || !rx_buffer || !new_value || ((zone & ~WRITE_ZONE_MASK) != 0))
		return SHA204_BAD_PARAM;

	if (zone & SHA204_ZONE_DATA)
	{
		address >>= 2;
		if (address & 1)
			// If we would just mask this bit, we would
			// write to an address that was not intended.
			return SHA204_BAD_PARAM;
	}

	p_command = &tx_buffer[SHA204_OPCODE_IDX];
	*p_command++ = SHA204_WRITE;
	*p_command++ = zone;
	*p_command++ = (UINT8) (address & SHA204_ADDRESS_MASK);
	*p_command++ = 0;

	count = (zone & SHA204_ZONE_COUNT_FLAG) ? SHA204_ZONE_ACCESS_32 : SHA204_ZONE_ACCESS_4;
	memcpy(p_command, new_value, count);
	p_command += count;

	if (mac != NULL)
	{
		memcpy(p_command, mac, WRITE_MAC_SIZE);
		p_command += WRITE_MAC_SIZE;
	}

	// Supply count.
	tx_buffer[SHA204_COUNT_IDX] = (UINT8) (p_command - &tx_buffer[0] + SHA204_CRC_SIZE);

	return SHA204c_Send_and_receive(&tx_buffer[0], WRITE_RSP_SIZE, &rx_buffer[0],
				WRITE_DELAY, WRITE_EXEC_MAX - WRITE_DELAY);
}

UINT8 ATSHA204_wakeup_and_validate_device(void)
{

	UINT8 status = SHA204_SUCCESS;
	UINT8 transmit_buffer[0x07];
	UINT8 response_buffer[0x07];
	UINT8 tries = 5;
	UINT8 i;

	/* Identify the device on the communication bus and initialize the interface */

	/* Wake up the device or abort operation if unsuccessful */
	while(tries)
	{
		status = SHA204c_wakeup(&response_buffer[0]);
		if(status == SHA204_SUCCESS) break;
		tries--;
		if(tries == 0)
		{
			return status;
		}
	}

	// Use the DevRev command to check communication to chip by validating value received.
	// Note that DevRev value is not constant over future revisions of the chip so failure
	// of this function may not mean bad connection.
	status |= SHA204_Execute(SHA204_DEVREV, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, NULL, sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
//	status |= SHA204_sleep();  /* Put the chip to sleep in case you stop to examine buffer contents */

#if 0
	// validate the received value for DevRev
	for(i=0; i<4; i++)
	{
		printk("response_buffer[%d] = %d\n", i + 1, response_buffer[i + 1]);
		if( response_buffer[i+1] != SHA204_DEVREV_VALUE[i])
		{
			status |= SHA204_FUNC_FAIL;
		}
	}
#endif

	return status;
}

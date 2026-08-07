/**
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * @file mbox_checksum.c
 * @brief Mailbox CRC-16 message checksum
 */

#include "mbox_checksum.h"

#include <stdint.h>
#include <string.h>

#define MBOX_CRC16_POLYNOM 0x8005U
#define MBOX_CRC16_MASK    0xFFFFU

static uint16_t mbox_crc16_update_byte(uint16_t crc, uint8_t data)
{
	uint16_t value = (uint16_t)(crc ^ ((uint16_t)data << 8));
	int bit;

	for (bit = 0; bit < 8; bit++) {
		if (value & 0x8000U)
			value = (uint16_t)((value << 1) ^ MBOX_CRC16_POLYNOM);
		else
			value = (uint16_t)(value << 1);
	}

	return value & MBOX_CRC16_MASK;
}

static uint16_t mbox_crc16(const void *data, size_t len)
{
	const uint8_t *bytes = data;
	uint16_t crc = MBOX_CRC16_INIT;
	size_t index;

	for (index = 0; index < len; index++)
		crc = mbox_crc16_update_byte(crc, bytes[index]);

	return crc;
}

size_t mbox_checksum_size(const MboxPostMsg *msg)
{
	return sizeof(MboxPostMsg) - MAX_ITEM + msg->payload.payload_size;
}

uint16_t mbox_checksum_calculate(MboxPostMsg *msg)
{
	uint32_t saved_checksum;
	size_t crc_len;
	uint16_t crc;

	if (!msg || msg->payload.payload_size > MAX_ITEM)
		return 0;

	saved_checksum = msg->checksum;
	crc_len = mbox_checksum_size(msg);
	msg->checksum = 0;
	crc = mbox_crc16(msg, crc_len);
	msg->checksum = saved_checksum;

	return crc;
}

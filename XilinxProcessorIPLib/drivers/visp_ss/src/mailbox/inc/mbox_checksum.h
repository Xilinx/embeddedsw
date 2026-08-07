/**
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * @file mbox_checksum.h
 * @brief Header of mailbox CRC-16 message checksum
 */
#ifndef _MBOX_CHECKSUM_H_
#define _MBOX_CHECKSUM_H_

#include <stddef.h>
#include <stdint.h>

#include "mbox_fifo.h"

#define MBOX_CRC16_INIT 0x4F4EU

/**
 * @brief Number of bytes covered by the checksum: header + used payload only
 * @param msg Pointer to MboxPostMsg
 * @return Byte count to feed into the CRC (not sizeof(MboxPostMsg))
 */
size_t mbox_checksum_size(const MboxPostMsg *msg);

/**
 * @brief Calculate the CRC-16 checksum of a message
 * @param msg Pointer to MboxPostMsg (checksum field is zeroed then restored during calculation)
 * @return Calculated 16-bit CRC value
 */
uint16_t mbox_checksum_calculate(MboxPostMsg *msg);

#endif // _MBOX_CHECKSUM_H_

/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_onfi.c
* @brief Implements the ONFI specific functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 1.17  tb     10/01/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup nandpsu_api NANDPSU APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xnandpsu_onfi.h"
#include "xnandpsu.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* @brief Calculates the ONFI parameter page CRC.
*
* Covers the bytes from offset StartOff up to, but not including, offset
* Length of the buffer.
*
* @param[in]	ParamBuf Pointer to the ONFI parameter page buffer.
* @param[in]	StartOff Starting offset in the buffer for the CRC.
* @param[in]	Length End offset (exclusive) in the buffer for the CRC.
*
* @return	Calculated 16-bit CRC value.
*
******************************************************************************/
u32 XNandPsu_OnfiParamPageCrc(u8 *ParamBuf, u32 StartOff, u32 Length)
{
	const u32 CrcInit = 0x4F4EU;
	const u32 Order = 16U;
	const u32 Polynom = 0x8005U;
	u32 i, j, c, Bit;
	u32 Crc = CrcInit;
	u32 DataIn;
	u32 DataByteCount = 0U;
	u32 CrcMask, CrcHighBit;

	CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) - (u32)1) << (u32)1) | (u32)1;
	CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
	/*
	 * CRC covers the data bytes between byte 0 and byte 253
	 * (ONFI 1.0, section 5.4.1.36)
	 */
	for (i = StartOff; i < Length; i++) {
		DataIn = *(ParamBuf + i);
		c = (u32)DataIn;
		DataByteCount++;
		j = 0x80U;
		while (j != 0U) {
			Bit = Crc & CrcHighBit;
			Crc <<= 1U;
			if ((c & j) != 0U) {
				Bit ^= CrcHighBit;
			}
			if (Bit != 0U) {
				Crc ^= Polynom;
			}
			j >>= 1U;
		}
		Crc &= CrcMask;
	}
	return Crc;
}
/** @} */

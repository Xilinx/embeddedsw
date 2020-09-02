/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_onfi.c
* @addtogroup nandpsu_v1_7
* @{
*
* This file contains the implementation of ONFI specific functions.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* </pre>
*
******************************************************************************/

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
* This function calculates ONFI parameter page CRC.
*
* @param	Parambuf is a pointer to the ONFI parameter page buffer.
* @param	StartOff is the starting offset in buffer to calculate CRC.
* @param	Length is the number of bytes for which CRC is calculated.
*
* @return
*		CRC value.
* @note
*		None.
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

	CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) -(u32)1) << (u32)1) | (u32)1;
	CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
	/*
	 * CRC covers the data bytes between byte 0 and byte 253
	 * (ONFI 1.0, section 5.4.1.36)
	 */
	for(i = StartOff; i < Length; i++) {
		DataIn = *(ParamBuf + i);
		c = (u32)DataIn;
		DataByteCount++;
		j = 0x80U;
		while(j != 0U) {
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

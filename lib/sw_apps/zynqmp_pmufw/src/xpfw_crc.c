/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_config.h"
#include "xpfw_crc.h"

#ifdef ENABLE_IPI_CRC
/*****************************************************************************/
/**
*
* This function calculates the CRC for the data
*
* @param	BufAddr - buffer on which CRC is calculated
* @param	BufSize - size of the buffer
*
* @return	Checksum - 16 bit CRC value
*
* @note		None.
*
******************************************************************************/
u32 XPfw_CalculateCRC(u32 BufAddr, u32 BufSize)
{
	const u32 CrcInit = 0x4F4EU;
	const u32 Order = 16U;
	const u32 Polynom = 0x8005U;
	u32 i;
	u32 j;
	u32 c;
	u32 Bit;
	u32 Crc = CrcInit;
	u32 DataIn;
	u32 CrcMask, CrcHighBit;

	CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) -(u32)1) << (u32)1) | (u32)1;
	CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
	for(i = 0U; i < BufSize; i++) {
		DataIn = Xil_In8(BufAddr + i);
		c = (u32)DataIn;
		j = 0x80U;
		while(j != 0U) {
			Bit = Crc & CrcHighBit;
			Crc <<= 1U;
			if((c & j) != 0U) {
				Bit ^= CrcHighBit;
			}
			if(Bit != 0U) {
				Crc ^= Polynom;
			}
			j >>= 1U;
		}
		Crc &= CrcMask;
	}
	return Crc;
}
#endif /* ENABLE_IPI_CRC */


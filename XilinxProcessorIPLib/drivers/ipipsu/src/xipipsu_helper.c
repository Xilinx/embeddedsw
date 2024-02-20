/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xipipsu_helper.c
* @addtogroup ipipsu_api IPIPSU APIs
* @{
*
* The xipipsu_helper.c file contains the implementation of the XIpiPsu_CalculateCRC
* and XIpiPsu_GetBufferIndex.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	 Changes
* ----- ------ --------  ----------------------------------------------
* 2.14	ht	06/13/23 Restructured the code for more modularity
*       ht	07/28/23 Fix MISRA-C warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xipipsu_buf.h"

/************************** Constant Definitions *****************************/
#ifdef ENABLE_IPI_CRC
#define POLYNOM					0x8005U /**< Polynomial */
#define INITIAL_CRC_VAL			0x4F4EU		/**< Initial crc value */
#define CRC16_MASK				0xFFFFU /**< CRC mask */
#define CRC16_HIGH_BIT_MASK		0x8000U		/**< CRC high bit mask */
#define NUM_BITS_IN_BYTE		0x8U		/**< 8 bits in a byte */
#endif

/************************** Variable Definitions *****************************/

/****************************************************************************/
#ifdef ENABLE_IPI_CRC
/**
 * @brief Calculate CRC for IPI buffer data
 *
 * @param	BufAddr - buffer on which CRC is calculated
 * @param	BufSize - size of the buffer in bytes
 *
 * @return	Checksum - 16 bit CRC value
 */
u32 XIpiPsu_CalculateCRC(u32 BufAddr, u32 BufSize)
{
	u32 Crc16 = INITIAL_CRC_VAL;
	u32 DataIn;
	u32 Idx = 0;
	u32 Bits = 0;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = 0U; Idx < BufSize; Idx++) {
		/* Move byte into MSB of 16bit CRC */
		DataIn = (u32)Xil_In8(BufAddr + Idx);
		Crc16 ^= (DataIn << NUM_BITS_IN_BYTE);

		/* Process each bit of 8 bit value */
		for (Bits = 0; Bits < NUM_BITS_IN_BYTE; Bits++) {
			Temp1Crc = ((Crc16 << 1U) ^ POLYNOM);
			Temp2Crc = Crc16 << 1U;

			if ((Crc16 & CRC16_HIGH_BIT_MASK) != 0) {
				Crc16 = Temp1Crc;
			} else {
				Crc16 = Temp2Crc;
			}
		}
		Crc16 &= CRC16_MASK;
	}
	return Crc16;
}
#endif


/****************************************************************************/
/**
 * @brief	Gets the Buffer Index for a CPU specified by Mask
 *
 * @param	InstancePtr Pointer to current IPI instance
 * @param	CpuMask Mask of the CPU form which Index is required
 *
 * @return	Buffer Index value if CPU Mask is valid
 * 			XIPIPSU_MAX_BUFF_INDEX+1 if not valid
 *
 */
u32 XIpiPsu_GetBufferIndex(const XIpiPsu *InstancePtr, u32 CpuMask)
{
	u32 BufferIndex;
	u32 Index;
	/* Init Index with an invalid value */
	BufferIndex = XIPIPSU_MAX_BUFF_INDEX + 1U;

	/*Search for CPU in the List */
	for (Index = 0U; Index < InstancePtr->Config.TargetCount; Index++) {
		/*If we find the CPU , then set the Index and break the loop*/
		if (InstancePtr->Config.TargetList[Index].Mask == CpuMask) {
			BufferIndex = InstancePtr->Config.TargetList[Index].BufferIndex;
			break;
		}
	}

	/* Return the Index */
	return BufferIndex;
}
/** @} */

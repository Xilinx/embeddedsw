/******************************************************************************
* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xsem_ebd_search.c
*
* This file contains API that can be used to check whether a particular Bit
* is Essential or not.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   hv   05/17/2022   Initial creation
* </pre>
*
*/
/*****************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
/************************** Variable Definitions *****************************/
u8 FrameOffset[6][4] = {{3, 5, 7, 9},
					  {11, 13, 15, 17},
					  {19, 21, 23, 25},
					  {27, 29, 31, 33},
					  {35, 37, 39, 41},
					  {43, 45, 47, 49}};
extern volatile int XSem_EbdBuffer[];
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This API can be used to check whether a particular bit is
 * an Essential bit or not.
 *
 * @param	BtIndex[In]	:	Block type (Valid Inputs: 0, 3, 4, 5)
 * 		RowIndex[In]	:	Row (Valid Inputs: 0 to 3)
 * 		FAddr[In]		:	Frame Address (Valid Inputs: Refer note)
 * 		QwordIndex[In]	:	Qword (Valid Inputs: 0 to 24)
 * 		BitIndex[In]	:	Bit (Valid Inputs: 0 to 127)
 *
 * @return	0U : Input Bit is essential
 * 		1U : Input Bit is not essential
 * 		2U : Invalid Input
 *
 * @note
 *	- Total number of frames in a row is not same for all rows.
 *
 *	- XSem_CmdCfrGetTotalFrames API is provided to know the total number of
 * frames in a row for each block. Output param (FrameCntPtr) of
 * XSem_CmdCfrGetTotalFrames API is updated with total number of frames of each
 * block type for the input row. If a particular block in a row has 0 frames,
 * then error injection shall not be performed. Range of Frame number: 0 to
 * (FrameCntPtr[n] - 1) where n is block type with range 0 to 6.
 *****************************************************************************/
u32 XSem_EbdLookUp(u8 BtIndex, u8 RowIndex,\
		u32 FAddr, u32 QwordIndex, u32 BitIndex)
{
	u32 RetValue = 0u;
	u32 WordIndex = 0U;
	u32 InputMask = 0U;
	u32 FrameMap = 0U;
	u32 FrameMask = 0U;
	u32 LastFrame = 0U;

	xil_printf("Searching for Block type = 0x%08x  Row = 0x%08x  "
			"FAddr = 0x%08x  Qword = 0x%08x Bit = 0x%08x\n", \
			BtIndex, RowIndex, FAddr, QwordIndex, BitIndex);

	LastFrame = XSem_EbdBuffer[FrameOffset[BtIndex][RowIndex] - 1];

	/* Validate Inputs */
	if ((BtIndex > 5U) || (RowIndex > 3U) || (FAddr >= LastFrame) ||
			(QwordIndex > 24U) || (BitIndex > 127U)){
		xil_printf("Invalid Input\n");
		RetValue = 2U;
		goto END;
	}

	/* Convert Qword/bit to Word/bit */
	WordIndex = BitIndex/32;
	/* Prepare Input mask */
	InputMask = (1 << (BitIndex - WordIndex*32));

	FrameMap = XSem_EbdBuffer[FrameOffset[BtIndex][RowIndex]] + FAddr;
	FrameMask = XSem_EbdBuffer[XSem_EbdBuffer[FrameMap] + QwordIndex*4 + WordIndex];

	xil_printf("InputMask = 0x%08x\n", InputMask);
	xil_printf("FrameMask = 0x%08x\n", FrameMask);
	/**
	 * Check if Input bit is set or not,
	 * If Set, then the bit is essential,
	 * else, not essential
	 */
	if( (FrameMask & InputMask) != 0U){
		/*Return 1 if bit is essential */
		RetValue = 1U;
	} else {
		/*Return 0 if bit is not essential */
		RetValue = 0U;
	}
END:
	return RetValue;
}

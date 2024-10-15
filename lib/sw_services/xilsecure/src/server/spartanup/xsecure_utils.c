/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_utils.c
* This file contains common functionalities required for xilsecure Versalnet library
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.1   kpt     08/18/24 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function takes the hardware core out of reset
 *
 * @param	BaseAddress	- Base address of the core
 * @param	Offset		- Offset of the reset register
 *
 *****************************************************************************/
void XSecure_ReleaseReset(UINTPTR BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_UNSET);
}

/*****************************************************************************/
/**
 * @brief	This function places the hardware core into the reset
 *
 * @param	BaseAddress	- Base address of the core
 * @param	Offset		- Offset of the reset register
 *
 *****************************************************************************/
void XSecure_SetReset(UINTPTR BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
}

/***************************************************************************/
/**
 * @brief	This function copies data from 64 bit address Src to 64 bit
 * address Dst
 *
 * @param	DstAddr is the 64 bit destination address
 * @param	SrcAddr is the 64 bit source address
 * @param	Cnt is the number of bytes of data to be copied
 *
 ******************************************************************************/
void XSecure_MemCpy64(u64 DstAddr, u64 SrcAddr, u32 Cnt)
{
	u64 Dst = DstAddr;
	u64 Src = SrcAddr;
	u32 Count = Cnt;

	if (((Dst & XSECURE_WORD_ALIGN_MASK) == 0U) &&
		((Src & XSECURE_WORD_ALIGN_MASK) == 0U)) {
		while (Count >= sizeof (int)) {
			XSecure_Out64(Dst, XSecure_In64(Src));
			Dst += sizeof(int);
			Src += sizeof(int);
			Count -= (u32)sizeof(int);
		}
	}
	while (Count > 0U) {
		XSecure_OutByte64(Dst, XSecure_InByte64(Src));
		Dst += 1U;
		Src += 1U;
		Count -= 1U;
	}
}

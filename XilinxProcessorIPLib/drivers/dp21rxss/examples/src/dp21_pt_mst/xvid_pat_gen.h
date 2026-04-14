/*******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xvid_pat_gen.h
*
* This is the header file for the Video Pattern Generator register access.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.6   GM    03/15/26  Initial release.
*</pre>
*
*****************************************************************************/

#ifndef XVID_PAT_GEN_H_
#define XVID_PAT_GEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/***************** Macros (Inline Functions) Definitions ********************/
/** @name Register access macro definitions.
  * @{
  */
#define XPatGen_In32 Xil_In32
#define XPatGen_Out32 Xil_Out32
/* @} */

/******************************************************************************/
/**
 * This is a low-level function that writes to the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to write to.
 * @param	Data is the 32-bit data to write to the specified register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 *		void XPatGen_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XPatGen_WriteReg(BaseAddress, RegOffset, Data) \
	XPatGen_Out32((BaseAddress) + (RegOffset), (Data))

/******************************************************************************/
/**
 * This is a low-level function that reads from the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to be read from.
 *
 * @return	The 32-bit value of the specified register.
 *
 * @note	C-style signature:
 *		u32 XPatGen_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XPatGen_ReadReg(BaseAddress, RegOffset) \
	XPatGen_In32((BaseAddress) + (RegOffset))

#ifdef __cplusplus
}
#endif

#endif /* XVID_PAT_GEN_H_ */

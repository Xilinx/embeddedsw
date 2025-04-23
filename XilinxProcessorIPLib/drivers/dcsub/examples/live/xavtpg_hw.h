/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_io.h"

#define XAV_PATGEN_EN                       0x0
#define XAV_PATGEN_VSYNC_POLARITY           0x4
#define XAV_PATGEN_HSYNC_POLARITY           0x8
#define XAV_PATGEN_ENABLE_POLARITY          0xC
#define XAV_PATGEN_VSYNC_WIDTH              0x10
#define XAV_PATGEN_VERT_BACK_PORCH          0x14
#define XAV_PATGEN_VERT_FRONT_PORCH         0x18
#define XAV_PATGEN_VRES                     0x1C
#define XAV_PATGEN_HSYNC_WIDTH              0x20
#define XAV_PATGEN_HORIZ_BACK_PORCH         0x24
#define XAV_PATGEN_HORIZ_FRONT_PORCH        0x28
#define XAV_PATGEN_HRES                     0x2C
#define XAV_PATGEN_TC_HSBLNK                0x44
#define XAV_PATGEN_TC_HSSYNC                0x48
#define XAV_PATGEN_TC_HESYNC                0x4C
#define XAV_PATGEN_TC_HEBLNK                0x50
#define XAV_PATGEN_TC_VSBLNK                0x54
#define XAV_PATGEN_TC_VSSYNC                0x58
#define XAV_PATGEN_TC_VESYNC                0x5C
#define XAV_PATGEN_TC_VEBLNK                0x60
#define XAV_PATGEN_FORMAT_BPC               0x300
#define XAV_PATGEN_MODE_PATTERN             0x308

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XAvTpg_In32 Xil_In32
#define XAvTpg_Out32 Xil_Out32
/* @} */

/******************************************************************************/
/**
 * This is a low-level function that reads from the specified register.
 *
 * @param       BaseAddress is the base address of the device.
 * @param       RegOffset is the register offset to be read from.
 *
 * @return      The 32-bit value of the specified register.
 *
 * @note        C-style signature:
 *              u32 XAVTPG_ReadReg(u32 BaseAddress, u32 RegOffset
 *
*******************************************************************************/
#define XAvTpg_ReadReg(BaseAddress, RegOffset) \
	XAvTpg_In32((BaseAddress) + (RegOffset))

/******************************************************************************/
/**
 * This is a low-level function that writes to the specified register.
 *
 * @param       BaseAddress is the base address of the device.
 * @param       RegOffset is the register offset to write to.
 * @param       Data is the 32-bit data to write to the specified register.
 *
 * @return      None.
 *
 * @note        C-style signature:
 *              void XAVTPG_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XAvTpg_WriteReg(BaseAddress, RegOffset, Data) \
	XAvTpg_Out32((BaseAddress) + (RegOffset), (Data))

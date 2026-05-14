/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xavpg.h
*
* This header file contains AV Pattern Generator declarations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---- ---  --------  --------------------------------------------------
* 1.00 vsa  05/01/26  Initial version
*
* </pre>
*
******************************************************************************/
#ifndef __XAVPG_H__
#define __XAVPG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_io.h"
#include "mmi_dpdc_example.h"

#define XAV_PATGEN_EN                       0x0
#define XAV_PATGEN_VRES                     0x1C
#define XAV_PATGEN_HRES                     0x2C
#define XAV_PATGEN_FORMAT_BPC               0x300
#define XAV_PATGEN_MODE_PATTERN             0x308

#define XAVPATGEN_PPC_DUAL  2
#define XAVPATGEN_PPC_QUAD  4

#define XAVPATGEN_BPC_8     1
#define XAVPATGEN_BPC_10    2
#define XAVPATGEN_BPC_12    3

#define XAVPATGEN_CS_RGB      0
#define XAVPATGEN_CS_YUV422   1

#define XAVPATGEN_COL_BT601         0
#define XAVPATGEN_COL_BT709         1

#define XAVPATGEN_PIXFMT_SHIFT          1
#define XAVPATGEN_COLORIMETRY_SHIFT     4
#define XAVPATGEN_BPC_SHIFT             5
#define XAVPATGEN_PPC_QUAD_MASK         (1 << 9)
#define XAVPATGEN_PPC_DUAL_MASK         (1 << 8)

#define XAVPATGEN_INST_0    0
#define XAVPATGEN_INST_1    1
#define XAVPATGEN_INST_2    2
#define XAVPATGEN_INST_3    3

#define XAVPATGEN_INST_MAX_COUNT  4
#define XAVPATGEN_INST_LIVE_EX_MAX_COUNT  2

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XAvpg_In32 Xil_In32
#define XAvpg_Out32 Xil_Out32
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
 *              u32 XAvpg_ReadReg(u32 BaseAddress, u32 RegOffset
 *
*******************************************************************************/
#define XAvpg_ReadReg(BaseAddress, RegOffset) \
	XAvpg_In32((BaseAddress) + (RegOffset))

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
 *              void XAvpg_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XAvpg_WriteReg(BaseAddress, RegOffset, Data) \
	XAvpg_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes *******************************/

void XAvpgSetConfig(UINTPTR BaseAddr, AvpgRunConfig *avpgcfg, u32 width, u32 height);

#ifdef __cplusplus
}
#endif

#endif /* __XAVPG_H__ */

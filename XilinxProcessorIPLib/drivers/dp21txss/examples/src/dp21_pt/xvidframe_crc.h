/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xvidframe_crc.h
*
* This is the main header file for the Xilinx HDCP abstraction layer.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  GM   17/07/17 First Release
*</pre>
*
*****************************************************************************/

#ifndef SRC_XVIDFRAME_CRC_H_
#define SRC_XVIDFRAME_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/
#ifdef XPAR_VIDEO_FRAME_CRC_BASEADDR
#define VIDEO_FRAME_CRC_EN
#endif

#define XPAR_VIDEO_FRAME_CRC_BASEADDR \
	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR

/* Register Offsets */
#define VIDEO_FRAME_CRC_CONFIG			0x00
#define VIDEO_FRAME_CRC_VALUE_G_R		0x04
#define VIDEO_FRAME_CRC_VALUE_B			0x08
#define VIDEO_FRAME_CRC_ACTIVE_COUNTS	0x0C

#define VIDEO_FRAME_CRC_CLEAR			0x10
#define VIDEO_FRAME_CRC_PXLMODE_MASK	0x7
#define VIDEO_FRAME_CRC_R_Y_COMP_MASK	0xFFFF
#define VIDEO_FRAME_CRC_G_CR_COMP_MASK	0xFFFF0000
#define VIDEO_FRAME_CRC_B_CB_COMP_MASK	0xFFFF
#define VIDEO_FRAME_CRC_G_CR_COMP_SHIFT	16

/************************** Variable Declaration ****************************/

/**************************** Type Definitions ******************************/
typedef struct {
        u8  TEST_CRC_CNT;
        u8  TEST_CRC_SUPPORTED;
        u8  TEST_CRC_START_STOP;
        u16 Pixel_r;
        u16 Pixel_g;
        u16 Pixel_b;
        u8  Mode_422;
        u32 Base_Addr;
} Video_CRC_Config;

/***************** Macros (Inline Functions) Definitions ********************/
/** @name Register access macro definitions.
  * @{
  */
#define XVidFrameCrc_In32 Xil_In32
#define XVidFrameCrc_Out32 Xil_Out32
/* @} */

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
 *		u32 XVidFrameCrc_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XVidFrameCrc_ReadReg(BaseAddress, RegOffset) \
	XVidFrameCrc_In32((BaseAddress) + (RegOffset))

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
 *		void XVidFrameCrc_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XVidFrameCrc_WriteReg(BaseAddress, RegOffset, Data) \
	XVidFrameCrc_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes *****************************/
int XVidFrameCrc_Initialize(Video_CRC_Config *VidFrameCRC);
void XVidFrameCrc_Reset(Video_CRC_Config *VidFrameCRC);
void XVidFrameCrc_Report(Video_CRC_Config *VidFrameCRC);

#ifdef __cplusplus
}
#endif


#endif /* SRC_XVIDFRAME_CRC_H_ */

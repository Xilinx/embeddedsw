/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

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

/** Base address of Video Frame CRC peripheral */
#define XPAR_VIDEO_FRAME_CRC_BASEADDR \
	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR

/* Register Offsets */
/** Video Frame CRC configuration register offset */
#define VIDEO_FRAME_CRC_CONFIG			0x00
/** Video Frame CRC value for Green/Red components register offset */
#define VIDEO_FRAME_CRC_VALUE_G_R		0x04
/** Video Frame CRC value for Blue component register offset */
#define VIDEO_FRAME_CRC_VALUE_B			0x08
/** Video Frame CRC active pixel counts register offset */
#define VIDEO_FRAME_CRC_ACTIVE_COUNTS		0x0C
/** Video Frame CRC misses register offset */
#define VIDEO_FRAME_CRC_MISSES			0x10

/** Video Frame CRC clear control value */
#define VIDEO_FRAME_CRC_CLEAR			0x10
/** Video Frame CRC pixel mode mask */
#define VIDEO_FRAME_CRC_PXLMODE_MASK		0x7
/** Video Frame CRC R/Y component mask */
#define VIDEO_FRAME_CRC_R_Y_COMP_MASK		0xFFFF
/** Video Frame CRC G/Cr component mask */
#define VIDEO_FRAME_CRC_G_CR_COMP_MASK		0xFFFF0000
/** Video Frame CRC B/Cb component mask */
#define VIDEO_FRAME_CRC_B_CB_COMP_MASK		0xFFFF
/** Video Frame CRC G/Cr component shift value */
#define VIDEO_FRAME_CRC_G_CR_COMP_SHIFT		16

/************************** Variable Declaration ****************************/

/**************************** Type Definitions ******************************/
/**
 * Video CRC configuration structure
 */
typedef struct {
	u8  TEST_CRC_CNT;               /**< CRC test count */
	u8  TEST_CRC_SUPPORTED;         /**< CRC test supported flag */
	u8  TEST_CRC_START_STOP;        /**< CRC test start/stop control */
	u16 Pixel_r;                    /**< Red/Y pixel component value */
	u16 Pixel_g;                    /**< Green/Cr pixel component value */
	u16 Pixel_b;                    /**< Blue/Cb pixel component value */
	u8  Mode_422;                   /**< YUV 4:2:2 mode flag */
} Video_CRC_Config;

/***************** Macros (Inline Functions) Definitions ********************/
/** @name Register access macro definitions.
  * @{
  */
/** Read 32-bit register value */
#define XVidFrameCrc_In32 Xil_In32
/** Write 32-bit register value */
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
void XVidFrameCrc_Reset(void);
void XVidFrameCrc_Report(void);

#ifdef __cplusplus
}
#endif


#endif /* SRC_XVIDFRAME_CRC_H_ */

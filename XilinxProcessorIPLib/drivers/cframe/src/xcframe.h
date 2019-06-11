/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF PLRCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEPLNT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcframe.h
*
* This is the file which contains header files related to CFRAME block
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XCFRAME_H
#define XCFRAME_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"

#include "xcframe_hw.h"

/************************** Constant Definitions *****************************/
/* Cframe command types */
#define XCFRAME_CMD_REG_WCFG			(0x1U)
#define XCFRAME_CMD_REG_ROWON		(0x2U)
#define XCFRAME_CMD_REG_ROWOFF		(0x3U)
#define XCFRAME_CMD_REG_RCFG			(0x4U)
#define XCFRAME_CMD_REG_DLPARK		(0x5U)
#define XCFRAME_CMD_REG_CRCC			(0x6U)
#define XCFRAME_CMD_REG_HCLEAN		(0x7U)
#define XCFRAME_CMD_REG_HCLEANR		(0x8U)
#define XCFRAME_CMD_REG_TRIMB		(0xAU)
#define XCFRAME_CMD_REG_TRIMU		(0xBU)

#define XCFRAME_FRAME_OFFSET				(0x2000U)

/** @name Frame Num
 * @{
 */
typedef enum
{
	XCFRAME_FRAME_0 = 0,
	XCFRAME_FRAME_1,
	XCFRAME_FRAME_2,
	XCFRAME_FRAME_3,
	XCFRAME_FRAME_4,
	XCFRAME_FRAME_5,
	XCFRAME_FRAME_6,
	XCFRAME_FRAME_7,
	XCFRAME_FRAME_8,
	XCFRAME_FRAME_9,
	XCFRAME_FRAME_10,
	XCFRAME_FRAME_11,
	XCFRAME_FRAME_12,
	XCFRAME_FRAME_13,
	XCFRAME_FRAME_14,
	XCFRAME_FRAME_BCAST
} XCframe_FrameNo;

/**************************** Type Definitions *******************************/
/**
 * This typedef contains 128 bit value for CFRAME registers
 */
typedef struct
{
	u32 Word0;
	u32 Word1;
	u32 Word2;
	u32 Word3;
} Xuint128 __attribute__ ((aligned(16)));

/**
* This typedef contains configuration information for a CFRAME core.
* Each CFRAME core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
} XCframe_Config;

/******************************************************************************/
/**
*
* The XCframe driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCframe_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
}XCframe;

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef XCFRAME_DEBUG
#define XCframe_Printf(...)	xil_printf(...)
#else
#define XCframe_Printf(...)
#endif

/************************** Function Prototypes ******************************/
XCframe_Config *XCframe_LookupConfig(u16 DeviceId);
s32 XCframe_CfgInitialize(XCframe *InstancePtr, XCframe_Config *CfgPtr,
			u32 EffectiveAddr);
s32 XCframe_SelfTest(XCframe *InstancePtr);
void XCframe_WriteReg(XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo, Xuint128 *Value128);
void XCframe_WriteCmd(XCframe *InstancePtr,	XCframe_FrameNo CframeNo, u32 Cmd);
void XCframe_VggTrim(XCframe *InstancePtr,	Xuint128 *TrimVal);
void XCframe_CramTrim(XCframe *InstancePtr,	u32 TrimValue);
void XCframe_BramTrim(XCframe *InstancePtr, u32 TrimValue);
void XCframe_UramTrim(XCframe *InstancePtr, u32 TrimValue);
void XCframe_SetReadParam(XCframe *InstancePtr,
		XCframe_FrameNo CframeNo, u32 CframeLen);

#ifdef __cplusplus
}
#endif

#endif  /* XCFRAME_H */

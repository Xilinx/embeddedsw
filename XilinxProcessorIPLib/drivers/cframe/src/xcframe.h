/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  bsv  06/11/2019 Added XCframe_ClearCframeErr API
* 1.02  bsv  17/02/2020 XCframe_SafetyWriteReg API added
* 1.03  bsv  07/15/2021 Fix doxygen warnings
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
/**@cond cframe_internal
 * @{
 */

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
#define XCFRAME_CMD_REG_RCRC		(0xEU)
#define XCFRAME_CMD_REG_RDALL		(0x10U)

#define XCFRAME_FRAME_OFFSET				(0x2000U)

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
 * @}
 * @endcond
 */

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
/**@cond cframe_internal
 * @{
 */

#ifdef XCFRAME_DEBUG
#define XCframe_Printf(...)	xil_printf(...)
#else
#define XCframe_Printf(...)
#endif

/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/
XCframe_Config *XCframe_LookupConfig(u16 DeviceId);
s32 XCframe_CfgInitialize(XCframe *InstancePtr, XCframe_Config *CfgPtr,
			u32 EffectiveAddr);
s32 XCframe_SelfTest(XCframe *InstancePtr);
void XCframe_WriteReg(XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo, Xuint128 *Val);
void XCframe_WriteCmd(XCframe *InstancePtr,	XCframe_FrameNo CframeNo, u32 Cmd);
void XCframe_VggTrim(XCframe *InstancePtr,	Xuint128 *TrimVal);
void XCframe_CramTrim(XCframe *InstancePtr,	u32 TrimValue);
void XCframe_BramTrim(XCframe *InstancePtr, u32 TrimValue);
void XCframe_UramTrim(XCframe *InstancePtr, u32 TrimValue);
void XCframe_SetReadParam(XCframe *InstancePtr,
			XCframe_FrameNo CframeNo, u32 CframeLen);
void XCframe_ReadReg(XCframe *InstancePtr, u32 AddrOffset,
			XCframe_FrameNo FrameNo, u32* ValPtr);
void XCframe_ClearCframeErr(XCframe *InstancePtr);
int XCframe_SafetyWriteReg(XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo, Xuint128 *Val);
#ifdef __cplusplus
}
#endif

#endif  /* XCFRAME_H */

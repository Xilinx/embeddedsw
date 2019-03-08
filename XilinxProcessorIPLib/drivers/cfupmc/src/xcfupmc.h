/******************************************************************************
*
* Copyright (C) 2017-2019 Xilinx, Inc.  All rights reserved.
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
* @file xcfupmc.h
*
* This is the file which contains code for CFU block.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
* 2.00  bsv  03/01/2019 Added error handling APIs
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XCFUPMC_H
#define XCFUPMC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"

#include "cfu_apb.h"
#include "xcfupmc_hw.h"

/************************** Constant Definitions *****************************/

/* CFU key hole register address */
//#define CFU_STREAM_ADDR     (0xF1300000)
//#define CFU_FDRO_ADDR		(0xF1302000)
/* Address updates after RTL HW40 */
#define CFU_STREAM_ADDR     (0xF12C0000)
#define CFU_FDRO_ADDR		(0xF12C2000)

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for a CFU core.
* Each CFU core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
} XCfupmc_Config;

/******************************************************************************/
/**
*
* The XCfupmc driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCfupmc_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
	u32 DeCompress;
	u32 Crc32Check;
	u32 Crc32Val;
	u32 Crc8Dis;
}XCfupmc;

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * This function clears CFU ISR
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
* @note		None.
*		C-style signature:
*		XStatus XCfupmc_ClearIsr(XCfupmc *InstancePtr, IsrMask)
*
******************************************************************************/
#define XCfupmc_ClearIsr(InstancePtr, IsrMask) \
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, \
			       CFU_APB_CFU_ISR, IsrMask)

/*****************************************************************************/
/**
* This function reads CFU ISR
*
* @param	InstancePtr is a pointer to the XCfupmc instance.
*
* @return	None
*
* @note		None.
*		C-style signature:
*		XStatus XCfupmc_ReadIsr(XCfupmc *InstancePtr)
*
******************************************************************************/
#define XCfupmc_ReadIsr(InstancePtr) \
	XCfupmc_ReadReg(InstancePtr->Config.BaseAddress, \
			       CFU_APB_CFU_ISR)

/*****************************************************************************/
/**
* This function reads CFU Status
*
* @param	InstancePtr is a pointer to the XCfupmc instance.
*
* @return	None
*
* @note		None.
*		C-style signature:
*		XStatus XCfupmc_ReadStatus(XCfupmc *InstancePtr)
*
******************************************************************************/
#define XCfupmc_ReadStatus(InstancePtr) \
	XCfupmc_ReadReg(InstancePtr->Config.BaseAddress, \
			       CFU_APB_CFU_STATUS)

#ifdef XCFUPMC_DEBUG
#define XCfupmc_Printf(...)	xil_printf(__VA_ARGS__)
#else
#define XCfupmc_Printf(...)
#endif

/************************** Function Prototypes ******************************/
XCfupmc_Config *XCfupmc_LookupConfig(u16 DeviceId);
s32 XCfupmc_CfgInitialize(XCfupmc *InstancePtr, XCfupmc_Config *CfgPtr,
			u32 EffectiveAddr);
s32 XCfupmc_SelfTest(XCfupmc *InstancePtr);
void XCfupmc_MaskRegWrite(XCfupmc *InstancePtr, u32 Addr, u32 Mask, u32 Val);
void XCfupmc_SetParam(XCfupmc *InstancePtr);
s32 XCfupmc_CheckParam(XCfupmc *InstancePtr);
s32 XCfupmc_WaitForStreamBusy(XCfupmc *InstancePtr);
void XCfupmc_SetGlblSigEn(XCfupmc *InstancePtr, u8 Enable);
void XCfupmc_GlblSeqInit(XCfupmc *InstancePtr);
void XCfupmc_StartGlblSeq(XCfupmc *InstancePtr);
void XCfupmc_EndGlblSeq(XCfupmc *InstancePtr);
void XCfupmc_Reset(XCfupmc *InstancePtr);
void XCfupmc_WaitForStreamDone(XCfupmc *InstancePtr);
void XCfupmc_CfuErrHandler(XCfupmc *InstancePtr);
void XCfupmc_CfiErrHandler(XCfupmc *InstancePtr);
void XCfupmc_ExtErrorHandler(XCfupmc *InstancePtr);
#ifdef __cplusplus
}
#endif

#endif  /* XCFUPMC_H */

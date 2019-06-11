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
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file xcframe_hw.h
* @addtogroup cframe_v1_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx CFRAME core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kc  22/10/17 First release
* </pre>
*
******************************************************************************/

#ifndef XCFRAME_HW_H_
#define XCFRAME_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros	*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xcframe.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/
/** @name Registers offsets
 * @{
 */
#define XCFRAME_CRC_OFFSET				(0x000U)
#define XCFRAME_FAR_OFFSET				(0x010U)
#define XCFRAME_FAR_SFR_OFFSET			(0x020U)
#define XCFRAME_FAR_MFW_OFFSET			(0x030U)
#define XCFRAME_FDRI_OFFSET				(0x040U)
#define XCFRAME_FRCNT_OFFSET			(0x050U)
#define XCFRAME_CMD_OFFSET				(0x060U)
#define XCFRAME_MASK_OFFSET				(0x070U)
#define XCFRAME_CTL_OFFSET				(0x080U)
#define XCFRAME_CRAM_WR_OFFSET			(0x090U)
#define XCFRAME_CRAM_RD_OFFSET			(0x0A0U)
#define XCFRAME_CRAM_TRIM_OFFSET		(0x0B0U)
#define XCFRAME_COE_TRIM_OFFSET			(0x0C0U)
#define XCFRAME_SVDOPT_OFFSET			(0x0D0U)
#define XCFRAME_SEUOPT_OFFSET			(0x0E0U)
#define XCFRAME_SEU_SEL_SCAN_OFFSET		(0x0F0U)
#define XCFRAME_SEU_START_CNT_OFFSET	(0x100U)
#define XCFRAME_SEU_SWCRC_OFFSET		(0x110U)
#define XCFRAME_TESTMODE_OFFSET			(0x120U)
#define XCFRAME_BRDOPT_OFFSET			(0x130U)
#define XCFRAME_VGG_TRIM_OFFSET			(0x140U)
/*@}*/


/***************** Macros (Inline Functions) Definitions *********************/
#define XCframe_In32		Xil_In32	/**< Input operation */
#define XCframe_Out32		Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the CFRAME core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XCframe_ReadReg32(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCframe_ReadReg32(BaseAddress, RegOffset) \
		XCframe_In32(BaseAddress + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the CFRAME core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XCframe_WriteReg32(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XCframe_WriteReg32(BaseAddress, RegOffset, Data) \
		XCframe_Out32(BaseAddress + (u32)(RegOffset), (u32)(Data))


#ifdef __cplusplus
}

#endif


#endif /* End of protection macro */
/** @} */

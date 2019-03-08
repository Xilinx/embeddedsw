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
* @file xcfupmc_hw.h
* @addtogroup cfupmc_v1_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx CFU core.
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

#ifndef XCFUPMC_HW_H_
#define XCFUPMC_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros	*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "cfu_apb.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XCfupmc_In32		Xil_In32	/**< Input operation */
#define XCfupmc_Out32		Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the CFU core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XCfupmc_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCfupmc_ReadReg(BaseAddress, RegOffset) \
		XCfupmc_In32((u32)(RegOffset))
		//XCfupmc_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the CFU core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfupmc_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XCfupmc_WriteReg(BaseAddress, RegOffset, Data) \
		XCfupmc_Out32((u32)(RegOffset), (u32)(Data))
		//XCfupmc_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))


#ifdef __cplusplus
}

#endif


#endif /* End of protection macro */
/** @} */

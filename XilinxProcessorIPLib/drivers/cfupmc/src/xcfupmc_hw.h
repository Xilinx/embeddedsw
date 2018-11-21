/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcfupmc_hw.h
* @addtogroup cfupmc_v1_1
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

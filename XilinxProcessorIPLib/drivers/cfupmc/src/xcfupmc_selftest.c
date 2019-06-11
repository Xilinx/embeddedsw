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
* @file xcfupmc_selftest.c
* @addtogroup cfupmc_v1_0
* @{
*
* This file contains a diagnostic self-test function for the CFU driver.
* Refer to the header file xcfupmc.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   kc   22/10/17 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfupmc.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This function runs a self-test on the driver and hardware device.
* @param	InstancePtr is a pointer to the XCfupmc instance.
*
* @return
*		- XST_SUCCESS if the self-test passed.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
s32 XCfupmc_SelfTest(XCfupmc *InstancePtr)
{
	u32 Data;
	s32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XCfupmc_ReadReg(InstancePtr->Config.BaseAddress,
					(u32)(CFU_APB_CFU_PROTECT));

	/* Changing Endianess of Source channel */

	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			(u32)(CFU_APB_CFU_PROTECT),
			CFU_APB_CFU_PROTECT_ACTIVE_MASK);

	if ((XCfupmc_ReadReg(InstancePtr->Config.BaseAddress,
		(u32)(CFU_APB_CFU_PROTECT)) &
			(u32)(CFU_APB_CFU_PROTECT_ACTIVE_MASK)) ==
				(CFU_APB_CFU_PROTECT_ACTIVE_MASK)) {
		Status = (s32)(XST_SUCCESS);
	}
	else {
		Status = (s32)(XST_FAILURE);
	}

	/* Changes made are being reverted back */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			(u32)(CFU_APB_CFU_PROTECT), Data);

	return Status;

}
/** @} */

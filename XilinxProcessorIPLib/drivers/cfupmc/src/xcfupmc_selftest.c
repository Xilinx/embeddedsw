/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcfupmc_selftest.c
* @addtogroup cfupmc_v1_1
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

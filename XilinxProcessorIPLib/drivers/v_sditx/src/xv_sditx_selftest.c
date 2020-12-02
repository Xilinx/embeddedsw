/******************************************************************************
* Copyright (C)2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditx_selftest.c
* @addtogroup xv_sditx_v3_1
* @{
*
* Contains diagnostic/self-test functions for the SDI Tx Controller core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xv_sditx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

#define SDITX_RST_CTRL_DEFAULT	0x000
#define SDITX_MDL_CTRL_DEFAULT	0x117000

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the mode control
* register valus have the default values.
*
* @param	InstancePtr is a pointer to the XV_SdiTx instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note		None
*
******************************************************************************/
u32 XV_SdiTx_SelfTest(XV_SdiTx *InstancePtr)
{
	u32 Result, RegValue;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XV_SdiTx_StopSdi(InstancePtr);

	if (Result == XST_FAILURE)
		return XST_FAILURE;

	RegValue = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_RST_CTRL_OFFSET);
	if (RegValue != SDITX_RST_CTRL_DEFAULT)
		return XST_FAILURE;

	RegValue = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_MDL_CTRL_OFFSET);
	if (RegValue != SDITX_MDL_CTRL_DEFAULT)
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */

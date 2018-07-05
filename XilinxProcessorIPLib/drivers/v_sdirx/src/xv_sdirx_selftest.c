/******************************************************************************
* Copyright (C)2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirx_selftest.c
* @addtogroup xv_sdirx_v2_0
* @{
*
* Contains diagnostic/self-test functions for the SDI Rx Controller core.
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
#include "xv_sdirx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

#define SDIRX_RST_CTRL_DEFAULT	0x0000
#define SDIRX_MDL_CTRL_DEFAULT	0x3F70

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the LaneCount
* present in register matches the one from the generated file.
*
* @param	InstancePtr is a pointer to the XV_SdiRx instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note		None
*
******************************************************************************/
u32 XV_SdiRx_SelfTest(XV_SdiRx *InstancePtr)
{
	u32 Result, RegValue;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XV_SdiRx_Stop(InstancePtr);

	if (Result == XST_FAILURE)
		return XST_FAILURE;

	RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_RST_CTRL_OFFSET);
	if (RegValue != SDIRX_RST_CTRL_DEFAULT)
		return XST_FAILURE;

	RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_MDL_CTRL_OFFSET);
	if (RegValue != SDIRX_MDL_CTRL_DEFAULT)
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */

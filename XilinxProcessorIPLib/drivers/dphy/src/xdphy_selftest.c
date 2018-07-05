/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdphy_selftest.c
*
* @addtogroup dphy_v1_4
* @{
*
* Contains diagnostic/self-test functions for the XDphy component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/09/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xdphy.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if HS Timeout value
* present in register matches the one from the generated file.
*
* @param 	InstancePtr is a pointer to the XDphy instance.
*
* @return
* 		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note 	None.
*
******************************************************************************/
u32 XDphy_SelfTest(XDphy *InstancePtr)
{
	u32 Result;

	Result = XDphy_GetInfo(InstancePtr, XDPHY_HANDLE_HSTIMEOUT);

	if ((InstancePtr->Config.HSTimeOut) == Result) {
		return XST_SUCCESS;
	}
	else {
		return XST_FAILURE;
	}
}
/** @} */

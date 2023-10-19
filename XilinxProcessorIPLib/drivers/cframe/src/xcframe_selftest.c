/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcframe_selftest.c
* @addtogroup cframe Overview
* @{
*
* This file contains a diagnostic self-test function for the CFU driver.
* Refer to the header file xcframe.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   kc   10/22/2017 First release
* 1.5   mss  09/04/2023 Fixed MISRA-C violation 8.13
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcframe.h"

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
* @param	InstancePtr is a pointer to the XCframe instance.
*
* @return
*		- XST_SUCCESS if the self-test passed.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
s32 XCframe_SelfTest(const XCframe *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* TODO write self test */

	return XST_SUCCESS;

}
/** @} */

/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       pre  01/16/2025 Fixed warning
* 2.1   gnr  07/13/2026 Removed TODO comment
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
* This function is a dummy function and placeholder for a self-test
* on the Cframe driver and hardware device.
* @param	InstancePtr is a pointer to the XCframe instance.
*
* @return Always XST_SUCCESS
*
******************************************************************************/
s32 XCframe_SelfTest(const XCframe *InstancePtr)
{
	(void)InstancePtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Place holder for Cframe self test */

	return XST_SUCCESS;

}
/** @} */

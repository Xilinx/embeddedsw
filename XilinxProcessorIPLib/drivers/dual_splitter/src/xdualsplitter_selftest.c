/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdualsplitter_selftest.c
* @addtogroup dual_splitter Overview
* @{
*
* This file contains self test function for the Dual Splitter core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdualsplitter.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on DualSplitter core registers.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
s32 XDualSplitter_SelfTest(XDualSplitter *InstancePtr)
{
	s32 Status = XST_SUCCESS;
	u16 Width;
	u16 Height;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* set image size width x height */
	XDualSplitter_SetImageSize(InstancePtr, 2160, 1920);

	/* Retrieve image size width x height set previously */
	XDualSplitter_GetImageSize(InstancePtr, &Height, &Width);

	if ((Width != 1920) && (Height != 2160)) {
		Status = XST_FAILURE;
	}

	return Status;
}
/** @} */

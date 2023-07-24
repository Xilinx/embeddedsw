/******************************************************************************
* Copyright (C)2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi_selftest.c
* @addtogroup csi Overview
* @{
*
* Contains diagnostic/self-test functions for the CSI Rx Controller core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 06/18/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xcsi.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the LaneCount
* present in register matches the one from the generated file.
*
* @param	InstancePtr is a pointer to the XCsi instance.
*
* @return
* 		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note		None
*
******************************************************************************/
u32 XCsi_SelfTest(XCsi *InstancePtr)
{
	u32 Result, Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCsi_GetBitField(InstancePtr->Config.BaseAddr, XCSI_PCR_OFFSET,
			XCSI_PCR_MAXLANES_MASK,XCSI_PCR_MAXLANES_SHIFT);
	if ((InstancePtr->Config.MaxLanesPresent - 1) == Result) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}

	return Status;
}
/** @} */

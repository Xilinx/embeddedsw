/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi_selftest.c
* @addtogroup dsi_v1_3
* @{
*
* Contains diagnostic/self-test functions for the XDsi Controller component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0 ram 02/11/16 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi.h"

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
* @param	InstancePtr is a pointer to the XDsi instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to GUI parameter
*
* @note		None
*
******************************************************************************/
u32 XDsi_SelfTest(XDsi *InstancePtr)
{
	u8 Result;
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XDsi_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_ACTLANES_MASK, XDSI_PCR_ACTLANES_SHIFT);
	if ((InstancePtr->Config.DsiLanes - 1) == Result) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}
	return Status;
}
/** @} */

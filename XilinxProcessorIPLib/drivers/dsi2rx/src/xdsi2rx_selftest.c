/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rx_selftest.c
* @addtogroup dsi2rx Overview
* @{
*
* Contains diagnostic/self-test functions for the XDsi Controller component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0 Kunal 18/4/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi2rx.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the Pixel mode
* has been set.
*
* @param	InstancePtr is a pointer to the XDsi2Rx instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to GUI parameter
*
* @note		None
*
******************************************************************************/
u32 XDsi2Rx_SelfTest(XDsi2Rx *InstancePtr)
{
	u8 Result;
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XDsi2Rx_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_COREENB_MASK, XDSI2RX_CCR_COREENB_SHIFT);
	if (Result) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}
	return Status;
}
/** @} */

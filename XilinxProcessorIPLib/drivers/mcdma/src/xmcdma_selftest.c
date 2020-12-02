/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma_selftest.c
* @addtogroup mcdma_v1_5
* @{
*
* This file contains the self-test function for the MCDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xmcdma.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This file contains a diagnostic self-test function for the MCDMA driver.
* Refer to the header file xmcdma.h for more detailed information.
*
* @param	InstancePtr is a pointer to XMcDma instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*		- XST_FAILURE if the test is failed.
*
* @note		None.
*
******************************************************************************/
s32 XMcdma_SelfTest(XMcdma *InstancePtr)
{
	u32 TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Reset the device */
	XMcDma_Reset(InstancePtr);

	TimeOut = XMCDMA_LOOP_COUNT;

	while (TimeOut) {

		if(XMcdma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		xil_printf("Self Test failed\r\n");

		return XST_FAILURE;
	}

	return (XST_SUCCESS);
}
/** @} */

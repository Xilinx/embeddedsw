/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_selftest.c
* @addtogroup axidma_v9_13
* @{
*
* Contains diagnostic/self-test functions for the XAxiDma component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 8.1 	adk  29/01/15 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xaxidma.h"


/************************** Constant Definitions *****************************/
#define XAXIDMA_RESET_TIMEOUT   500

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test perform a
* reset of the DMA device and checks the device is coming out of reset or not
*
* @param	InstancePtr is a pointer to the XAxiDma instance.
*
* @return
* 		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the device is not coming out of reset.
*
* @note
*     None.
*
******************************************************************************/
int XAxiDma_Selftest(XAxiDma * InstancePtr)
{
	int TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XAxiDma_Reset(InstancePtr);

	/* At the initialization time, hardware should finish reset quickly
	 */
	TimeOut = XAXIDMA_RESET_TIMEOUT;

	while (TimeOut) {

		if(XAxiDma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut)
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */

/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_selftest.c
* @addtogroup AXIDMA Overview
* @{
* @brief Implements diagnostic and self-test functions for AXI DMA.
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
#define XAXIDMA_RESET_TIMEOUT   500 /**< Maximum iterations to wait for reset completion. */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* @brief Runs a self-test on the driver/device. This test perform a.
* reset of the DMA device and checks the device is coming out of reset or not
*
* @param[in] InstancePtr is a pointer to the XAxiDma instance.
*
* @retval XST_SUCCESS Self-test passed.
* @retval XST_FAILURE Self-test failed.
*
******************************************************************************/
int XAxiDma_Selftest(XAxiDma *InstancePtr)
{
	int TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XAxiDma_Reset(InstancePtr);

	/* At the initialization time, hardware should finish reset quickly
	 */
	TimeOut = XAXIDMA_RESET_TIMEOUT;

	while (TimeOut) {

		if (XAxiDma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/** @} */

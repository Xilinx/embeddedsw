/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_selftest.c
* @addtogroup AXIDMA Overview
* @{
* @brief Demonstrates the AXI DMA self-test example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID	/**< Device ID of the AXI DMA under test. */
#endif
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int AxiDMASelfTestExample(u16 DeviceId);
#else
int AxiDMASelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;	/**< Driver instance for the AXI DMA under test. */


/*****************************************************************************/
/**
* @brief Invokes the example function and reports the execution status.
*
* @return XST_SUCCESS if Example finished successfully, or XST_FAILURE if Example failed.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
#ifndef SDT
	Status = AxiDMASelfTestExample(DMA_DEV_ID);
#else
	Status = AxiDMASelfTestExample(XPAR_XAXIDMA_0_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("AxiDMASelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran AxiDMASelfTest Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* @brief Resets the DMA device and verifies idle state and checks the device is.
* coming out of reset or not.
*
* @if SDT
* @param[in] BaseAddress contains the base address of the device.
* @else
* @param[in] DeviceId is the DMA device id.
* @endif
*
* @return
*		- XST_SUCCESS if channel reset is successful
*		- XST_FAILURE if channel reset fails.
*
******************************************************************************/
#ifndef SDT
int AxiDMASelfTestExample(u16 DeviceId)
#else
int AxiDMASelfTestExample(UINTPTR BaseAddress)
#endif
{
	XAxiDma_Config *CfgPtr;
	int Status = XST_SUCCESS;

#ifndef SDT
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
#else
	CfgPtr = XAxiDma_LookupConfig(BaseAddress);
#endif
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XAxiDma_Selftest(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/** @} */

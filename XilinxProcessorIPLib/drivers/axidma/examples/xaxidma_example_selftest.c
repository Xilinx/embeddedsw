/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_selftest.c
 *
 * This file demonstrates the example to do selftest on the device.
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
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int AxiDMASelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;


/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
	Status = AxiDMASelfTestExample(DMA_DEV_ID);

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
* This function performance a reset of the DMA device and checks the device is
* coming out of reset or not.
*
* @param	DeviceId is the DMA device id.
*
* @return
*		- XST_SUCCESS if channel reset is successful
*		- XST_FAILURE if channel reset fails.
*
* @note		None.
*
******************************************************************************/
int AxiDMASelfTestExample(u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status = XST_SUCCESS;

	CfgPtr = XAxiDma_LookupConfig(DeviceId);
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

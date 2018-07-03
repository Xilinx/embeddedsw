/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxivdma_example_selftest.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 6.2   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation of examples.
* 6.5   rsp  05/12/17 Fix compilation error.Rename macro for dma device id.
*                     CR-990803
* </pre>
*
*****************************************************************************/
/***************************** Include Files *********************************/
#include "xaxivdma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xil_printf.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

 #ifndef TESTAPP_GEN
#define DMA_DEV_ID		XPAR_AXIVDMA_0_DEVICE_ID
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int AxiVDMASelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiVdma AxiVdma;


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
	Status = AxiVDMASelfTestExample(DMA_DEV_ID);

	if (Status != XST_SUCCESS) {

		xil_printf("AxiVDMASelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran AxiVDMASelfTest Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function performance a reset of the VDMA device and checks the device is
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
int AxiVDMASelfTestExample(u16 DeviceId)
{
	XAxiVdma_Config *Config;
	int Status = XST_SUCCESS;

	Config = XAxiVdma_LookupConfig(DeviceId);
	if (!Config) {
			return XST_FAILURE;
	}

	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(&AxiVdma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XAxiVdma_Selftest(&AxiVdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

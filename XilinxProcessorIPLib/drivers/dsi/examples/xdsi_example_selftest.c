/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xdsi_example_selftest.c
*
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.1  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
*      ms  04/05/17 Modified Comment lines in functions to
*                   recognize it as documentation block for doxygen
*                   generation of examples.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xdsi.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/* Device hardware build related constants. */
#ifndef TESTAPP_GEN
#define DSI_DEV_ID	XPAR_DSI_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 DsiSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

/* Device instance definitions */
XDsi Dsi;

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

	/* Run the self test example */
	Status = DsiSelfTestExample(DSI_DEV_ID);

	if (Status != XST_SUCCESS) {

		xil_printf("DsiSelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DsiSelfTest Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function will perform DSI self test prints GUI parameters
*
* @param	DeviceId is the DSI Controller Device id.
*
* @return
*		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 DsiSelfTestExample(u32 DeviceId)
{
	XDsi_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XDsi_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XDsi_CfgInitialize(&Dsi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDsi_SelfTest(&Dsi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

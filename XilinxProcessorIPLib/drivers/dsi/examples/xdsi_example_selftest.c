/******************************************************************************
*
* (c) Copyright 2016 Xilinx, Inc. All rights reserved.
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
*
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

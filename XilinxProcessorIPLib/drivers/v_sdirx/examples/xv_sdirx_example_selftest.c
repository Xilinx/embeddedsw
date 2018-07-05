/******************************************************************************
*
* (c) Copyright 2017 Xilinx, Inc. All rights reserved.
*
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*

*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirx_example_selftest.c
* @addtogroup v_sdirx_v1_0
* @{
*
* This file contains a design example using the XV_SdiRx driver. It performs a
* self test on the SDI Rx driver that will test its sub-cores self test
* functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_sdirx.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define XV_SDIRX_DEV_ID XPAR_XV_SDIRX_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 XV_SdiRxSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XV_SdiRx Sdi;

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
int main(void)
{
	int Status;

	xil_printf("\n\r--- Entering main() --- \n\r");

	/* Run the poll example for simple transfer */
	Status = XV_SdiRxSelfTestExample(XV_SDIRX_DEV_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("XV_SdiRxSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XV_SdiRxSelfTest Example\n\r");

	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Rx core initialization is successful and
* the default register values are matching
*
* @param	DeviceId is the XV_SdiRx Controller Device id.
*
* @return
*		- XST_SUCCESS if Rx initialixation and self test are successsful
*		- XST_FAILURE if either Rx initialixation and self test fails
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRxSelfTestExample(u32 DeviceId)
{
	XV_SdiRx_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XV_SdiRx_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XV_SdiRx_CfgInitialize(&Sdi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XV_SdiRx_SelfTest(&Sdi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/** @} */

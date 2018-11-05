/******************************************************************************
*
* (c) Copyright 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsi_example_selftest.c
*
* This file contains an example using the XCsi driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver	Who	Date	Changes
* ----- ------ -------- -----------------------------------------------
* 1.1   ms  01/23/17 Modified xil_printf statement in main function to
*                    ensure that "Successfully ran" and "Failed" strings are
*                    available in all examples. This is a fix for CR-965028.
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation of examples.
* </pre>
******************************************************************************/
/***************************** Include Files *********************************/
#include "xcsi.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define CSI2RX_DEV_ID	XPAR_CSI_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 CsiSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XCsi Csi;

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

	xil_printf("\n\r--- Entering main() --- \n\r");

	/* Run the poll example for simple transfer */
	Status = CsiSelfTestExample(CSI2RX_DEV_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("CsiSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CsiSelfTest Example\n\r");

	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Max Lane count from the generated file matches
* the value present in the protocol configuration register
*
* @param	DeviceId is the CSI Controller Device id.
*
* @return
* 		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 CsiSelfTestExample(u32 DeviceId)
{
	XCsi_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XCsi_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XCsi_CfgInitialize(&Csi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XCsi_SelfTest(&Csi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

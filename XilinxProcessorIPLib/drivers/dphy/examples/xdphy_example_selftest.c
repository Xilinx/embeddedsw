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
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xdphy_example_selftest.c
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.1   ms     01/23/17  Modified xil_printf statement in main function to
*                        ensure that "Successfully ran" and "Failed" strings
*                        are available in all examples. This is a fix for
*                        CR-965028.
*       ms     04/05/17  Modified Comment lines in functions to
*                        recognize it as documentation block for doxygen
*                        generation of examples.
* </pre>
*
*****************************************************************************/
/***************************** Include Files *********************************/

#include "xdphy.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define DPHY_DEV_ID XPAR_DPHY_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 DphySelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

/*
 * Device instance definitions
 */
XDphy Dphy;

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
	Status = DphySelfTestExample(DPHY_DEV_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("DphySelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DphySelfTest Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the HS_TIMEOUT from the generated file matches
* the value present in the corresponding configuration register
*
* @param 	DeviceId is the DPHY device id.
*
* @return
* 		- XST_SUCCESS if values match
*		- XST_FAILURE if values differ.
*
* @note		None.
*
******************************************************************************/
u32 DphySelfTestExample(u32 DeviceId)
{
	XDphy_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = (XDphy_Config *) XDphy_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XDphy_CfgInitialize(&Dphy, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDphy_GetRegIntfcPresent(&Dphy);
	if (Status == 0) {
		return XST_FAILURE;
	}

	Status = XDphy_SelfTest(&Dphy);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

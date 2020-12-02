/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
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

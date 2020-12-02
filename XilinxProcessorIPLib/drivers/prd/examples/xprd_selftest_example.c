/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_selftest_example.c
*
* This file contains a design example using the PR Decoupler driver to do
* self test on the device.
*
* @note		None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date	     Changes
* ---- ---- ------------   -----------------------------------------------
* 1.00  ms   7/14/2016      First release
*       ms    04/05/2017    Modified comment lines notation in functions to
*                           avoid unnecessary description displayed
*                           while generating doxygen.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define PRD_DEVICE_ID	XPAR_PR_DECOUPLER_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XPrd_SelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XPrd Prd;		/* Instance of the PR Decoupler */

/*****************************************************************************/
/**
*
* This function is used to call the example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/*
	 * Run the PR Decoupler self test example, specify the Device ID
	 * that is generated in xparameters.h
	 */
	Status = XPrd_SelfTestExample((u16)PRD_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PR Decoupler Selftest example is failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PR Decoupler Selftest example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the PR Decoupler device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XPrd component.
*
* @param	DeviceId is the XPAR_<prd_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 XPrd_SelfTestExample(u16 DeviceId)
{
	u32 Status;
	XPrd_Config *CfgPtr;

	/*
	 * Initialize the PR Decoupler driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	 CfgPtr = XPrd_LookupConfig(DeviceId);
	 if (NULL == CfgPtr) {
		return XST_FAILURE;
	 }

	Status = XPrd_CfgInitialize(&Prd, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XPrd_SelfTest(&Prd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

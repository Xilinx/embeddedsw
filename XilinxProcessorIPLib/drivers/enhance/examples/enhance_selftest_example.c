/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/*****************************************************************************/
/**
*
* @file enhance_selftest_example.c
*
* This file contains an example using the XEnhance driver to do self test
* on the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a drg/jz 01/13/10 First Release
* 1.03a  sg    08/14/12 Updated the example for CR 666306. Modified
*		     	the device ID to use the first Device Id
*			Removed the printf at the start of the main
* 7.0   adk    02/19/14 Modified function names as per guidelines
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"
#include "xparameters.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 XEnhanceSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XEnhance InstancePtr;		/**< Instance of the Enhance Device */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/*
	 * Run the selftest example
	 */
	Status = XEnhanceSelfTestExample((u16)XPAR_ENHANCE_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ENHANCE Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ENHANCE Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XEnhance driver.
*
*
* @param	DeviceId is the XPAR_<ENHANCE_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
u32 XEnhanceSelfTestExample(u16 DeviceId)
{
	u32 Status;
	XEnhance_Config *Config;

	/*
	 * Initialize the ENHANCE driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XEnhance_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XEnhance_CfgInitialize(&InstancePtr, Config,
			Config->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XEnhance_SelfTest(&InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	return XST_SUCCESS;
}

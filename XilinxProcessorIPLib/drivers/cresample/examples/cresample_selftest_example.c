/******************************************************************************
* Copyright (C)2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file cresample_selftest_example.c
*
* This file contains an example using the XCRESAMPLE driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 4.0   adk    03/12/14 First release.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcresample.h"
#include "xparameters.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CRESAMPLE_DEVICE_ID	XPAR_CRESAMPLE_0_DEVICE_ID
					/**< CRESAMPLE Device ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/************************** Function Prototypes ******************************/


int XCresampleSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


XCresample Cresample;		/**<Instance of the CRESAMPLE Device */
/*****************************************************************************/
/**
*
* Main function to call the example.
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
	int Status;

	/* Run the selftest example */
	Status = XCresampleSelfTestExample((u16)(CRESAMPLE_DEVICE_ID));
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		xil_printf("Cresample Selftest Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Cresample Selftest Example\r\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XCRESAMPLE driver.
*
*
* @param	DeviceId is the XPAR_<CRESAMPLE_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XCresampleSelfTestExample(u16 DeviceId)
{
	int Status;
	XCresample_Config *Config;

	/*
	 * Initialize the CRESAMPLE driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCresample_LookupConfig(DeviceId);

	/* Checking Config variable */
	if (NULL == Config) {
		return (XST_FAILURE);
	}

	Status = XCresample_CfgInitialize(&Cresample, Config,
						Config->BaseAddress);
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XCresample_SelfTest(&Cresample);

	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);

}

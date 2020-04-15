/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file ccm_selftest_example.c
*
* This file contains an example using the XCCM driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who     Date     Changes
* ----- ------- -------- -----------------------------------------------
* 6.00  adk     03/06/14 First Release.
*                        Implimented XCcmSelfTestExample function.
*                        Adherence to Xilinx coding guidelines.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"
#include "xparameters.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CCM_DEVICE_ID		XPAR_CCM_0_DEVICE_ID	/**< CCM Device ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


int XCcmSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


XCcm Ccm;		/**<Instance of the CCM Device */
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if succesful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
	Status = XCcmSelfTestExample((u16)CCM_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CCM Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CCM Selftest Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XCCM driver.
*
*
* @param	DeviceId is the XPAR_<CCM_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if succesful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XCcmSelfTestExample(u16 DeviceId)
{
	int Status;
	XCcm_Config *Config;

	/*
	 * Initialize the CCM driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCcm_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCcm_CfgInitialize(&Ccm, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */

	Status = XCcm_SelfTest(&Ccm);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file tpg_selftest_example.c
*
* This file contains an example using the XTpg driver to do self test
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
* 3.0   adk    01/15/14  First release
*       ms     04/10/17  Modified filename tag to include the file in doxygen
*                        examples.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtpg.h"
#include "xparameters.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define TPG_DEVICE_ID              XPAR_TPG_0_DEVICE_ID	/**< TPG Device ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


int XTpgSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


XTpg Tpg;		/**<Instance of the TPG Device */
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if XTpgSelfTestExample is successful.
* 		- XST_FAILURE if XTpgSelfTestExample is failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 *  Run the self test example
	 */
	Status = XTpgSelfTestExample((u16)TPG_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("TPG Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TPG Selftest Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XTpg driver.
*
*
* @param	DeviceId is the XPAR_<TPG_instance>_DEVICE_ID value from
*           	xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
* 		- XST_FAILURE if failed.
*
* @note		None
*
******************************************************************************/
int XTpgSelfTestExample(u16 DeviceId)
{
	int Status;
	XTpg_Config *Config;
	/*
	 * Initialize the TPG driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XTpg_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XTpg_CfgInitialize(&Tpg, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */

	Status = XTpg_SelfTest(&Tpg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

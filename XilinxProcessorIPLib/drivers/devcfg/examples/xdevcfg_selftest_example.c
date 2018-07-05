/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xdevcfg_selftest_example.c
*
* This file contains an self test example showing the usage of the Device
* Configuration Interface Hardware and driver (XDevCfg).
*
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  sdm    05/25/11 First release.
* </pre>
*
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xparameters.h"
#include "xdevcfg.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define DCFG_DEVICE_ID		XPAR_XDCFG_0_DEVICE_ID


/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/

int DcfgSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XDcfg DcfgInstance;		/* Device Configuration Interface Instance */

/******************************************************************************/
/**
*
* Main function to call the Device Configuration Interface Selftest example.
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None
*
*******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("DevCfg Selftest Example \r\n");

	/*
	 * Call the example, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = DcfgSelfTestExample(DCFG_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DevCfg Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DevCfg Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a selftest on the Device Configuration device and
* XDevCfg driver as an example. The purpose of this function is to illustrate
* the usage of the XDevCfg driver.
*
*
* @param	DeviceId is the XPAR_<XDCFG_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
int DcfgSelfTestExample(u16 DeviceId)
{
	int Status;
	XDcfg_Config *ConfigPtr;

	/*
	 * Initialize the Device Configuration Interface driver.
	 */
	ConfigPtr = XDcfg_LookupConfig(DeviceId);

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XDcfg_CfgInitialize(&DcfgInstance, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Self Test.
	 */
	Status = XDcfg_SelfTest(&DcfgInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}

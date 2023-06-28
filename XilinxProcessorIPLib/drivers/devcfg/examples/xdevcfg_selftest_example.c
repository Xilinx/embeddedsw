/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 3.8   Nava   06/21/23 Added support for system device-tree flow.
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
#ifndef SDT
#define DCFG_DEVICE_ID          XPAR_XDCFG_0_DEVICE_ID
#else
#define DCFG_BASEADDR           XPAR_XDEVCFG_0_BASEADDR
#endif

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/
#ifndef SDT
int DcfgSelfTestExample(u16 DeviceId);
#else
int DcfgSelfTestExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = DcfgSelfTestExample(DCFG_DEVICE_ID);
#else
	Status = DcfgSelfTestExample(DCFG_BASEADDR);
#endif
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
#ifndef SDT
int DcfgSelfTestExample(u16 DeviceId)
#else
int DcfgSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XDcfg_Config *ConfigPtr;

	/*
	 * Initialize the Device Configuration Interface driver.
	 */
#ifndef SDT
	ConfigPtr = XDcfg_LookupConfig(DeviceId);
#else
	ConfigPtr = XDcfg_LookupConfig(BaseAddress);
#endif

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

/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xwdtps_selftest_example.c
*
* This file contains a design example using the System Watchdog Timer driver
* (XWdtPs) and hardware device.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a sdm    05/27/11 First release
* 3.6   sb     06/27/23 Added support for system device-tree flow.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdtps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define WDT_DEVICE_ID		XPAR_XWDTPS_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int WdtPsSelfTestExample(u16 DeviceId);
#else
int WdtPsSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XWdtPs Watchdog;		/* Instance of WatchDog Timer  */

/*****************************************************************************/
/**
* Main function to call the self test example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("WDT SelfTest Example Test\r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
#ifndef SDT
	Status = WdtPsSelfTestExample(WDT_DEVICE_ID);
#else
	Status = WdtPsSelfTestExample(XPAR_XWDTPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("WDT SelfTest Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran WDT SelfTest Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function does a minimal test on the watchdog timer device and driver
* as a design example. The purpose of this function is to illustrate how to use
* the XWdtPs driver.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
* @param	DeviceId is the unique device id of the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int WdtPsSelfTestExample(u16 DeviceId)
#else
int WdtPsSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XWdtPs_Config *ConfigPtr;

	/*
	 * Initialize the watchdog timer so that it is ready to use
	 */
#ifndef SDT
	ConfigPtr = XWdtPs_LookupConfig(DeviceId);
#else
	ConfigPtr = XWdtPs_LookupConfig(BaseAddress);
#endif
	if (NULL == ConfigPtr) {
		return XST_FAILURE;
	}
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XWdtPs_CfgInitialize(&Watchdog, ConfigPtr,
				      ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the SelfTest
	 */
	Status = XWdtPs_SelfTest(&Watchdog);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xwdttb_gwdt_selftest_example.c
*
* This file contains an example for  using the Generic Watchdog Timer
* hardware and driver
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sne   02/04/19 Initial release
* 5.7   sb    07/12/23 Added support for system device-tree flow.
* </pre>
*
*****************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdttb.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define WDTTB_DEVICE_ID         XPAR_WDTTB_0_DEVICE_ID
#endif
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int GWdtTbSelfTestExample(u16 DeviceId);
#else
int GWdtTbSelfTestExample(UINTPTR BaseAddress);
#endif
/************************** Variable Definitions *****************************/

XWdtTb GWatchdog; /* The instance of the WatchDog Timer  */
/*****************************************************************************/
/*
 * Main function to call the example.This function is not included if the
 * example is generated from the TestAppGen test tool.
 *
 *
 * @return
 *               - XST_SUCCESS if successful.
 *               - XST_FAILURE if unsuccessful.
 *
 * @note         None.
 *
 ******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the GWDT Self Test example , specify the device ID that is generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = GWdtTbSelfTestExample(WDTTB_DEVICE_ID);
#else
	Status = GWdtTbSelfTestExample(XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("GWDT self test example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran GWDT self test example\n\r");

	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
 * This function does a minimal test on Generic watchdog timer device and
 * driver as a design example. The purpose of this function is to illustrate
 * how to use the XwdtTb component.
 *
 * @param        DeviceId is the XPAR_<WDTTB_instance>_DEVICE_ID value from
 *               xparameters.h.
 *
 * @return
 *               - XST_SUCCESS if successful.
 *               - XST_FAILURE if unsuccessful.
 *
 * @note         None.
 *
 ****************************************************************************/

#ifndef SDT
int GWdtTbSelfTestExample(u16 DeviceId)
#else
int GWdtTbSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XWdtTb_Config *Config;

	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XWdtTb_LookupConfig(DeviceId);
#else
	Config = XWdtTb_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}
	/*
	 * Initialize the watchdog timer and window WDT driver so that
	 * it is ready to use.
	 */
	Status = XWdtTb_CfgInitialize(&GWatchdog, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XWdtTb_SelfTest(&GWatchdog);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Reset all the Register of GWDT */
	Status = XWdtTb_Stop(&GWatchdog);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

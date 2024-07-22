/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xwdttb_selftest_example.c
*
* This file contains a example for  using the Watchdog Timer Timebase
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
* 1.00a sv   04/27/05 Initial release for TestApp integration.
* 2.00a ktn  22/10/09 Updated the example to use the HAL APIs/macros.
* 4.0   sha  02/04/16 Added debug messages.
*                     Calling XWdtTb_LookupConfig and XWdtTb_CfgInitialize
*                     functions instead of XWdtTb_Initialize for
*                     initialization.
* 4.4   sne  02/12/19 Added support for Versal
* 4.5	sne  09/27/19 Updated example file to support AXI Timebase WDT and
*		      WWDT.
* 5.0	sne  03/11/20 Added XWdtTb_ConfigureWDTMode api to configure mode.
* 5.7	sb   07/12/23 Added support for system device-tree flow.
* 5.9	ht   07/22/24 Add support for peripheral tests in SDT flow.
*
* </pre>
*
*****************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdttb.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TIMEBASE_WDT_DEVICE_ID  XPAR_WDTTB_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int WdtTbSelfTestExample(u16 DeviceId);
#else
int WdtTbSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XWdtTb WatchdogTimebaseInstance; /* The instance of the WatchDog Time Base */

/*****************************************************************************/
/**
* Main function to call the example.This function is not included if the
* example is generated from the TestAppGen test tool.
*
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the Self Test example , specify the device ID that is generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = WdtTbSelfTestExample(TIMEBASE_WDT_DEVICE_ID);
#else
	Status = WdtTbSelfTestExample(XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("WDTTB self test example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran WDTTB self test example.\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the watchdog timer timebase device and
* driver as a design example.  The purpose of this function is to illustrate
* how to use the XWdtTb component.
*
* This function assumes that the reset output of the watchdog timer
* timebase device is not connected to the reset of the processor. The function
* allows the watchdog timer to timeout such that a reset will occur if it is
* connected.  It the interrupt output is connected to an interrupt input, the
* user must handle the interrupts appropriately.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
*
* @param	DeviceId is the XPAR_<WDTB_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int WdtTbSelfTestExample(u16 DeviceId)
#else
int WdtTbSelfTestExample(UINTPTR BaseAddress)
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
	 * Initialize the watchdog timer and timebase driver so that
	 * it is ready to use.
	 */
	Status = XWdtTb_CfgInitialize(&WatchdogTimebaseInstance, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*Enable Window Watchdog Feature in WWDT*/
#ifndef SDT
	if (!WatchdogTimebaseInstance.Config.IsPl) {
#else
	if (!(strcmp(WatchdogTimebaseInstance.Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		XWdtTb_ConfigureWDTMode(&WatchdogTimebaseInstance, XWT_WWDT);
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XWdtTb_SelfTest(&WatchdogTimebaseInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xscuwdt_polled_example.c
*
* This file contains a design example using the Xilinx SCU Private Watchdog
* Timer driver (XScuWdt) and hardware device in watchdog mode.  This test
* illustrates how to initialize the watchdog device and restart it periodially
* to avoid the assertion of the WDRESETREQ pin.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1.00a sdm  01/15/10 First release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xscuwdt.h"
#include "xil_printf.h"
#include <sleep.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define WDT_DEVICE_ID		XPAR_SCUWDT_0_DEVICE_ID

#define WDT_LOAD_VALUE		0xFFFF0000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int ScuWdtPolledExample(XScuWdt * WdtInstancePtr, u16 DeviceId);

/************************** Variable Definitions *****************************/

XScuWdt Watchdog;		/* Cortex SCU Private WatchDog Timer Instance */

/*****************************************************************************/
/**
*
* Main function to call the Scu Priver Wdt polled mode example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SCU WDT Polled Mode Example Test \r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = ScuWdtPolledExample(&Watchdog, WDT_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SCU WDT Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SCU WDT Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the watchdog timer device and driver.
* The purpose of this function is to illustrate how to use the XScuWdt driver.
*
* @param	WdtInstancePtr is a pointer to the instance of XScuWdt driver.
* @param	DeviceId is the unique device id of the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int ScuWdtPolledExample(XScuWdt * WdtInstancePtr, u16 DeviceId)
{
	int Status;
	XScuWdt_Config *ConfigPtr;
	int Count = 0;

	/*
	 * Initialize the SCU Private Wdt driver so that it is ready to use.
	 */
	ConfigPtr = XScuWdt_LookupConfig(DeviceId);

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XScuWdt_CfgInitialize(WdtInstancePtr, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Put the watchdog timer in watchdog mode.
	 */
	XScuWdt_SetWdMode(WdtInstancePtr);

	/*
	 * Load the watchdog counter register.
	 */
	XScuWdt_LoadWdt(WdtInstancePtr, WDT_LOAD_VALUE);

	/*
	 * Start the ScuWdt device.
	 */
	XScuWdt_Start(WdtInstancePtr);

	while (Count < 20) {
		/*
		 * We have nothing to do here. Sleep for some time and then
		 * restart the watchdog such that the WDRESETREQ pin is not
		 * asserted. If the WDRESETREQ pin is asserted and the system
		 * is reset, then the watchdog device/driver is not working as
		 * expected.
		 * The user application needs to call the XScuWdt_RestartWdt
		 * periodially to avoid the watchdog from being timed-out.
		 */
		sleep(1);
		Count++;
		XScuWdt_RestartWdt(WdtInstancePtr);
	}

	return XST_SUCCESS;
}

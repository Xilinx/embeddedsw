/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xwdtps_polled_example.c
*
* This file contains a design example using the System Watchdog Timer driver
* (XWdtPs) and hardware device. This function assumes that the reset output
* of the Watchdog timer device is not connected to the reset of the processor.
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
* 1.00a ecm/jz 01/15/10 First release
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
#define WDT_DEVICE_ID  		XPAR_XWDTPS_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int WdtPsPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XWdtPs Watchdog;		/* Instance of WatchDog Timer  */

/*****************************************************************************/
/**
*
* Main function to call the Wdt polled mode example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("WDT Polled Mode Example Test\r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = WdtPsPolledExample(WDT_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("WDT Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran WDT Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the System Watchdog Timer device and
* driver as a design example.  The purpose of this function is to illustrate
* how to use the XWdtPs driver.
*
* This function assumes that the reset output of the Watchdog timer
* device is not connected to the reset of the processor. The function
* allows the Watchdog Timer to timeout such that a reset will occur if it is
* connected.  If the interrupt output is connected to an interrupt input, the
* user must handle the interrupts appropriately if the output is enabled.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the Watchdog Timer to expire.
*
* @param	DeviceId is the unique device id of the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int WdtPsPolledExample(u16 DeviceId)
{
	int Status;
	u32 ExpiredTimeDelta1 = 0;
	u32 ExpiredTimeDelta2 = 0;
	XWdtPs_Config *ConfigPtr;
	u32 EffectiveAddress;	/* This can be the virtual address */

	/*
	 * Initialize the Watchdog Timer so that it is ready to use
	 */
	ConfigPtr = XWdtPs_LookupConfig(DeviceId);

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	EffectiveAddress = ConfigPtr->BaseAddress;
	Status = XWdtPs_CfgInitialize(&Watchdog, ConfigPtr,
				       EffectiveAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the initial counter restart to the smallest value (0).
	 */
	XWdtPs_SetControlValue(&Watchdog,
				(u8) XWDTPS_COUNTER_RESET, (u8) 0);


	/*
	 * Set the initial Divider ratio at the smallest value.
	 */
	XWdtPs_SetControlValue(&Watchdog,
				(u8) XWDTPS_CLK_PRESCALE,
				(u8) XWDTPS_CCR_PSCALE_0008);

	/*
	 * Disable the RESET output.
	 */
	XWdtPs_DisableOutput(&Watchdog, XWDTPS_RESET_SIGNAL);

	/*
	 * Start the Watchdog Timer.
	 */
	XWdtPs_Start(&Watchdog);

	/*
	 * Restart the Watchdog Timer.
	 */
	XWdtPs_RestartWdt(&Watchdog);

	/*
	 * Determine how long it takes for the Watchdog Timer to expire
	 * for later processing.
	 */
	while (1) {
		if (!(XWdtPs_IsWdtExpired(&Watchdog))) {
			ExpiredTimeDelta1++;
		} else {
			break;
		}
	}

	/*
	 * Change the initial restart count value and make sure the WDT takes longer.
	 */
	XWdtPs_SetControlValue(&Watchdog,
				(u8) XWDTPS_COUNTER_RESET, (u8) 1);


	XWdtPs_RestartWdt(&Watchdog);

	/*
	 * Wait for the WDT to timeout again, should take longer this time.
	 */
	while (1) {
		if (!(XWdtPs_IsWdtExpired(&Watchdog))) {
			ExpiredTimeDelta2++;
		} else {
			break;
		}
	}


	/*
	 * Verify that the Watchdog Timer took longer.
	 */
	if (ExpiredTimeDelta2 <= ExpiredTimeDelta1) {
		return XST_FAILURE;
	}

	/*
	 * Verify that the Watchdog Timer does not timeout when restarted
	 * all the time, wait more than twice the amount of time it took for it
	 * to expire in the previous test.
	 */
	ExpiredTimeDelta1 = 0;

	while (1) {
		/*
		 * Restart the Watchdog Timer as a normal application would.
		 */
		XWdtPs_RestartWdt(&Watchdog);

		ExpiredTimeDelta1++;
		/*
		 * If more time has gone past than it took for it to expire
		 * when not restarted in the previous test, then stop as the
		 * restarting worked.
		 */
		if (ExpiredTimeDelta1 > (ExpiredTimeDelta2 * 2)) {
			break;
		}

		/*
		 * If the Watchdog Timer expired, then the test failed.
		 */
		if (XWdtPs_IsWdtExpired(&Watchdog)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

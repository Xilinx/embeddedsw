/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xwdttb_example.c
*
* This file contains a design example using the Watchdog Timer Timebase driver
* (XWdtTb) and hardware device.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/13/02 First release
* 1.00b sv   04/26/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  12/02/09 Updated the example to use the HAL APIs/macros.
*		      Updated this example to check for Watchdog timer reset
*		      condition instead of timer expiry state to avoid a race
* 		      condition
* 4.0   sha  02/04/16 Added debug messages.
*                     Updated WatchdogTimebase.RegBaseAddress ->
*                     WatchdogTimebase.Config.BaseAddr.
*                     Calling XWdtTb_LookupConfig and XWdtTb_CfgInitialize
*                     functions instead of XWdtTb_Initialize for
*                     initialization.
* 4.5   nsk 08/07/19  Add macro to support testapp generation for polled mode
* 5.7   sb  07/12/23  Added support for system device-tree flow.
* 5.9   ht  07/22/24  Add support for peripheral tests in SDT flow.
* 5.10  ht  10/28/24  Fix compilation warnings in SDT flow peripheral tests.
*</pre>
******************************************************************************/

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
#define WDTTB_DEVICE_ID		XPAR_WDTTB_0_DEVICE_ID
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int WdtTbExample(u16 DeviceId);
#else
int WdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XWdtTb WatchdogTimebase; /* Instance of WatchDog Timer Base */

/*****************************************************************************/
/**
* Main function to call the Wdttb driver example.
*
*
* @return
*		- XST_SUCCESS if example ran successfully.
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
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
#ifndef SDT
	Status = WdtTbExample(WDTTB_DEVICE_ID);
#else
	Status = WdtTbExample(&WatchdogTimebase, XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("WDTTB example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran WDTTB example \n\r");

	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
* This function tests the functioning of the TimeBase WatchDog Timer module
* in the polled mode.
*
* After one expiration of the WDT timeout interval, the WDT state bit is set to
* one in the status register. If the state bit is not cleared (by writing a 1 to
* the state bit) before the next expiration of the timeout interval, a WDT reset
* is generated.
*
* This function checks for Watchdog timer reset condition in two timer expiry
* state.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
* @param	DeviceId is the XPAR_<WDTB_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if WRS bit is not set in next two subsequent
*		timer expiry state.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int WdtTbExample(u16 DeviceId)
#else
int WdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	int Count = 0;
	XWdtTb_Config *Config;

#ifdef SDT
	(void*)WdtTbInstancePtr;
#endif
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
	Status = XWdtTb_CfgInitialize(&WatchdogTimebase, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XWdtTb_SelfTest(&WatchdogTimebase);
	if (Status != XST_SUCCESS) {
		xil_printf("Example:Self test failed\n\r");
		return XST_FAILURE;
	}

	/*
	 * Start the watchdog timer, the timebase is automatically reset
	 * when this occurs.
	 */
	XWdtTb_Start(&WatchdogTimebase);

	/*
	 * Verify Whether the WatchDog Reset Status has been set in the next two
	 * expiry state.
	 */
	while (1) {
		xil_printf(".");

		/*
		 * If the watchdog timer expired, then restart it.
		 */
		if (XWdtTb_IsWdtExpired(&WatchdogTimebase)) {

			/*
			 * Restart the watchdog timer as a normal application
			 * would
			 */
			XWdtTb_RestartWdt(&WatchdogTimebase);
			Count++;
			xil_printf("\n\rRestart kick %d\n\r", Count);
		}

		/*
		 * Check whether the WatchDog Reset Status has been set.
		 * If this is set means then the test has failed
		 */
		if (XWdtTb_ReadReg(WatchdogTimebase.Config.BaseAddr,
				   XWT_TWCSR0_OFFSET) & XWT_CSR0_WRS_MASK) {

			/*
			 * Stop the watchdog timer
			 */
			XWdtTb_Stop(&WatchdogTimebase);
			return XST_FAILURE;
		}

		/*
		 * Check whether the WatchDog timer expires two times.
		 * If the timer expires two times then the test is passed.
		 */
		if (Count == 2) {
			break;
		}
	}
	return XST_SUCCESS;
}



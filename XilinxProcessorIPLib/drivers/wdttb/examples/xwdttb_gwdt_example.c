/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xwdttb_gwdt_example.c
* This file contains a example for  using the Generic Watchdog Timer
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
* 1.0   sne   03/04/19 Initial release for Generic Watchdog Timer.
* 5.0	sne   01/31/20 Removed compare value registers write while configuring
*		       Generic watchdog window.
* 5.7	sb    07/12/23 Added support for system device-tree flow.
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
int GWdtTbExample(u16 DeviceId);
#else
int GWdtTbExample(UINTPTR BaseAddress);
#endif
/************************** Variable Definitions *****************************/

XWdtTb GWatchdog;       /* The instance of the WatchDog Time Base */
#define WDTPSV_GWOR_COUNT     0x00110000U  /*Generic Watchdog offset value*/

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
	 * Run the Generic Watchdog  example , specify the device ID that is generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = GWdtTbExample(WDTTB_DEVICE_ID);
#else
	Status = GWdtTbExample(XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Generic WdtTb example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Generic WdtTb example \n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *This function tests the functioning of the Generic watch dog Timer Feature
 * in Polled mode
 * In this function polls up to First window completion, once we reached to
 * generic_wdt_interrupt  point it will Refresh.
 * In this function once we reach to Second window time out point
 * It means generating generic_wdt_reset ,We are going to Refresh.
 *
 * @param        DeviceId is the XPAR_<WDTPSV_instance>_DEVICE_ID value from
 *                xparameters.h.
 * @return
 *               - XST_SUCCESS, in window, there is no bad event.
 *               - XST_FAILURE, otherwise.
 *
 * @note         None.
 *
 ****************************************************************************/

#ifndef SDT
int GWdtTbExample(u16 DeviceId)
#else
int GWdtTbExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	int RefreshReg = 0;
	XWdtTb_Config *Config;

	/*
	 * Initialize the WDTPSV driver so that it's ready to use look up
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
	 * Initialize the watchdog timer  driver so that
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
	xil_printf("\nSelf test completed \n\r");

	/* Update GWOR Register */
	XWdtTb_SetGenericWdtWindow(&GWatchdog, WDTPSV_GWOR_COUNT);

	/*
	 * Start the watchdog timer, the General Watchdog  is automatically reset
	 * when this occurs.
	 */
	XWdtTb_Start(&GWatchdog);

	/*
	 * Verify Whether the Generic WatchDog Refresh Status has been set in the next two
	 * expire state.
	 */
	while (1) {
		xil_printf(".");
		/* Refresh  General Watchdog timer if generic_wdt_interrupt occurred */
		if (RefreshReg <= 2) {
			if (XWdtTb_IsGenericWdtFWExpired(&GWatchdog)) {
				XWdtTb_RestartWdt(&GWatchdog);
				RefreshReg++;
				xil_printf("\n Refresh kick%d\n\r", RefreshReg);
			}
		}
		/*
		 * If the watchdog timer expired, then restart it.
		 */
		if (XWdtTb_IsWdtExpired(&GWatchdog)) {

			xil_printf("\n\rGeneric watchdog second window expired\n\r");
			XWdtTb_Stop(&GWatchdog);
			break;
		}

	}

	return XST_SUCCESS;
}

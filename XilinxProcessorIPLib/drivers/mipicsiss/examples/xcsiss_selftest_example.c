/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss_selftest_example.c
*
* This file contains a design example using the XCsiSs driver. It performs a
* self test on the MIPI CSI Rx Subsystem that will test its sub-cores
* self test functions.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/29/15 Initial release
* 1.1 ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings
*                  are available in all examples. This is a fix for
*                  CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsiss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the MIPI CSI Rx Subsystem instance to be used */
#ifndef TESTAPP_GEN
#define XCSISS_DEVICE_ID	XPAR_CSISS_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 CsiSs_SelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

XCsiSs CsiSsInst;	/* The MIPI CSI Rx Subsystem instance.*/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This is the main function for XCsiSs self test example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the self test example passed.
*		- XST_FAILURE if the self test example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	u32 Status;

	xil_printf("---------------------------------\n\r");
	xil_printf("MIPI CSI RX Subsystem self test example\n\r");
	xil_printf("---------------------------------\n\r\n\r");

	Status = CsiSs_SelfTestExample(XCSISS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI CSI RX Subsystem self test example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI CSI RX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function is the main entry point for the self test example using the
* XCsiSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	DeviceId is the unique device ID of the MIPI CSI RX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if any of MIPI CSI RX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of MIPI CSI RX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 CsiSs_SelfTestExample(u32 DeviceId)
{
	u32 Status;
	XCsiSs_Config *ConfigPtr;

	/* Obtain the device configuration for the MIPI CSI RX Subsystem */
	ConfigPtr = XCsiSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the CsiSsInst's Config
	 * structure. */
	Status = XCsiSs_CfgInitialize(&CsiSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI CSI RX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XCsiSs_SelfTest(&CsiSsInst);

	return Status;
}

/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_selftest_example.c
*
* This file contains a design example using the XDpRxSs driver. It performs a
* self test on the DisplayPort transmitter Subsystem Hierarchical IP that will
* test its sub-cores self test functions.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 07/29/15 Initial release.
* 4.00 ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the DisplayPort Receiver Subsystem HIP instance
 * to be used
 */
#ifndef SDT
#define XDPRXSS_DEVICE_ID		XPAR_DPRXSS_0_DEVICE_ID
#endif
/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/
#ifndef SDT
u32 DpRxSs_SelfTestExample(u16 DeviceId);
#else
u32 DpRxSs_SelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs self test example.
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
int main()
{
	u32 Status;

	xil_printf("---------------------------------\n\r");
	xil_printf("DisplayPort RX Subsystem self test example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("---------------------------------\n\r\n\r");
#ifndef SDT
	Status = DpRxSs_SelfTestExample(XDPRXSS_DEVICE_ID);
#else
	Status = DpRxSs_SelfTestExample(XPAR_DPRXSS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem self test example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort RX Subsystem self test example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the self test example using the
* XDpRxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if any of DisplayPort RX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of DisplayPort RX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
u32 DpRxSs_SelfTestExample(u16 DeviceId)
#else
u32 DpRxSs_SelfTestExample(UINTPTR BaseAddress)
#endif
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
#ifndef SDT
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XDpRxSs_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpRxSsInst's Config
	 * structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XDpRxSs_SelfTest(&DpRxSsInst);

	return Status;
}

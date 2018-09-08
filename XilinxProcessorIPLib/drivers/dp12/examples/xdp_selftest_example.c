/*******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_selftest_example.c
 *
 * Contains a design example using the XDp driver. It performs a self test on
 * the DisplayPort TX/RX core that will compare many of the DisplayPort core's
 * registers against their default reset values.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * 5.1   ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xparameters.h"
#include "xil_printf.h"

/**************************** Constant Definitions ****************************/

#define DP_DEVICE_ID XPAR_DISPLAYPORT_0_DEVICE_ID

/**************************** Function Prototypes *****************************/

u32 Dp_SelfTestExample(XDp *InstancePtr, u16 DeviceId);

/*************************** Variable Declarations ****************************/

XDp DpInstance;

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDp selftest example.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the self test example passed.
 *		- XST_FAILURE if the self test example was unsuccessful - the
 *		  DisplayPort TX's registers do not match their default values
 *		  or no DisplayPort TX instance was found.
 *
 * @note	None.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	Status = Dp_SelfTestExample(&DpInstance, DP_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("XDp_SelfTest failed, check register values.\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran XDp_SelfTest\n");
	return Status;
}

/******************************************************************************/
/**
 * The main entry point for the selftest example using the XDp driver. This
 * function will check whether or not the DisplayPort TX's registers are at
 * their default reset values to ensure that the core is in a known and working
 * state.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort TX's registers are at their
 *		  default reset values.
 *		- XST_FAILURE if the DisplayPort TX's registers do not match
 *		  their default values or no DisplayPort TX instance was found.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dp_SelfTestExample(XDp *InstancePtr, u16 DeviceId)
{
	u32 Status;
	XDp_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort TX core. */
	ConfigPtr = XDp_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDp_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	/* Run the self test. */
	Status = XDp_SelfTest(InstancePtr);
	return Status;
}

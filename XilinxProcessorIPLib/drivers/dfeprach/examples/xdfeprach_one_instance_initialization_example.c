/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_one_instance_initialization_example.c
*
* This file contains a load coefficients example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfeprach_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function runs the DFE Prach device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Activate the device.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfePrach_SelfTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfePrach_Cfg Cfg;
	XDfePrach *InstancePtr = NULL;
	XDfePrach_Version SwVersion;
	XDfePrach_Version HwVersion;
	XDfePrach_Init Init;

	printf("\r\nPrach \"One Instance Initialization\" Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfePrach_InstanceInit(XDFEPRACH_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfePrach_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfePrach_Reset(InstancePtr);
	XDfePrach_Configure(InstancePtr, &Cfg);
	XDfePrach_Initialize(InstancePtr, &Init);
	XDfePrach_Activate(InstancePtr, false);

	XDfePrach_Deactivate(InstancePtr);
	XDfePrach_InstanceClose(InstancePtr);

	printf("Prach \"One Instance Initialization\" Example: Pass\r\n");

	return XST_SUCCESS;
}

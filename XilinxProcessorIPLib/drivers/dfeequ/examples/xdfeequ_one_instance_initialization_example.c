/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_one_instance_initialization_example.c
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
#include "xdfeequ_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function runs the DFE Equalizer device using the driver APIs.
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
int XDfeEqu_SelfExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeEqu_Cfg Cfg;
	XDfeEqu *InstancePtr = NULL;
	XDfeEqu_Version SwVersion;
	XDfeEqu_Version HwVersion;
	XDfeEqu_EqConfig Config;

	printf("\r\nEqualizer \"One Instance Initialization\" Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeEqu_InstanceInit(XDFEEQU_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeEqu_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfeEqu_Reset(InstancePtr);
	XDfeEqu_Configure(InstancePtr, &Cfg);
	Config.DatapathMode = 0;
	XDfeEqu_Initialize(InstancePtr, &Config);
	XDfeEqu_Activate(InstancePtr, false);

	XDfeEqu_Deactivate(InstancePtr);
	XDfeEqu_InstanceClose(InstancePtr);
	printf("Equalizer \"One Instance Initialization\" Example: Pass\r\n");

	return XST_SUCCESS;
}

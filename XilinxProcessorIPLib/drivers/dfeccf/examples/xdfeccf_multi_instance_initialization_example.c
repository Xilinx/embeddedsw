/******************************************************************************
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_multi_instance_initialization_example.c
*
* This file contains a two instances example.
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
#include "xdfeccf_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function runs two instances of DFE Channel Filter.
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
int XDfeCcf_MultiInstancesExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr1 = NULL;
	XDfeCcf *InstancePtr2 = NULL;
	XDfeCcf_Version SwVersion;
	XDfeCcf_Version HwVersion;
	XDfeCcf_Init Init;

	printf("\r\nChannel Filter \"Multi Instances Initialization\" Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr1 = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);
	InstancePtr2 = XDfeCcf_InstanceInit(XDFECCF_NODE2_NAME);

	/* Get SW and HW version numbers */
	XDfeCcf_GetVersions(InstancePtr1, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfeCcf_Reset(InstancePtr1);
	XDfeCcf_Configure(InstancePtr1, &Cfg);
	XDfeCcf_Initialize(InstancePtr1, &Init);
	XDfeCcf_Activate(InstancePtr1, false);

	XDfeCcf_Reset(InstancePtr2);
	XDfeCcf_Configure(InstancePtr2, &Cfg);
	XDfeCcf_Initialize(InstancePtr2, &Init);
	XDfeCcf_Activate(InstancePtr2, false);

	XDfeCcf_Deactivate(InstancePtr1);
	XDfeCcf_Deactivate(InstancePtr2);
	XDfeCcf_InstanceClose(InstancePtr1);
	XDfeCcf_InstanceClose(InstancePtr2);

	printf("Channel Filter \"Multi Instances Initialization\" Example: Pass\r\n");
	return XST_SUCCESS;
}

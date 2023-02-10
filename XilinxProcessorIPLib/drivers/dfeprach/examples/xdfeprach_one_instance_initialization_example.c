/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_one_instance_initialization_example.c
*
* This file contains One instance initialisation example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
* 1.2   dc     11/19/21 Update doxygen documentation
* 1.3   dc     02/07/22 Configure 2 CC and 3 RC examples
* 1.5   dc     01/02/23 Multiband registers update
*
* </pre>
* @addtogroup Overview
* @{
*
*****************************************************************************/
/** @cond nocomments */
/***************************** Include Files ********************************/
#include "xdfeprach_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/** @endcond */
/****************************************************************************/
/**
*
* This example runs the DFE Prach device using the driver APIs.
* The example goes through the following steps:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Activate the device.
*	- Deactivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
/** //! [testexample1] */
int XDfePrach_SelfTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfePrach_Cfg Cfg = {
		{ 0, 0, 0, 0 },
	};
	XDfePrach *InstancePtr = NULL;
	XDfePrach_Version SwVersion = { 0 };
	XDfePrach_Version HwVersion = { 0 };
	XDfePrach_Init Init = { { { 0, { 0 } }, { 0, { 0 } }, { 0, { 0 } } },
				1,
				0 };

	printf("\n\rPrach \"One Instance Initialization\" Example - Start\n\n\r");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of PRACH driver */
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

	printf("\n\rPrach \"One Instance Initialization\" Example: Pass\n\n\r");

	return XST_SUCCESS;
}
/** //! [testexample1] */
/** @} */

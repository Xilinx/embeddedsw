/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_pass_through_example.c
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
*	- Set the triggers
*	- Activate the device.
*	- Load an equalizer coefficients.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfeEqu_PassThroughExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeEqu_Cfg Cfg;
	XDfeEqu *InstancePtr = NULL;
	XDfeEqu_EqConfig Config;
	XDfeEqu_TriggerCfg TriggerCfg;
	XDfeEqu_Coefficients Coeffs = { 1U, 0, { 0, 0, 0, 0, 0, 0, 0, 0,
						 0, 0, 0, 0, 0, 0, 0, 0,
						 0, 0, 0, 0, 0, 0, 0, 0 } };
	u32 ChannelField = 0xffU;
	u32 Shift;
	u32 Mode = 1U;
	XDfeEqu_Version SwVersion;
	XDfeEqu_Version HwVersion;

	printf("\r\nEqualizer \"Pass Through\" Example - Start\r\n");

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
	/* Reset */
	XDfeEqu_Reset(InstancePtr);
	/* Configure */
	XDfeEqu_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Config.DatapathMode = 0;
	XDfeEqu_Initialize(InstancePtr, &Config);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.Update.Mode = 1U;
	TriggerCfg.Update.TuserEdgeLevel = 1U;
	TriggerCfg.Update.TUSERBit = 0;
	XDfeEqu_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeEqu_Activate(InstancePtr, false);

	/* Set coefficents */
	Shift = 5;
	Coeffs.Coefficients[0] = (1 << 15) - 1;
	XDfeEqu_LoadCoefficients(InstancePtr, ChannelField, Mode, Shift,
				 &Coeffs);

	/* Close and exit */
	XDfeEqu_Deactivate(InstancePtr);
	XDfeEqu_InstanceClose(InstancePtr);
	printf("Equalizer \"Pass Through\" Example: Pass\r\n");
	return XST_SUCCESS;
}

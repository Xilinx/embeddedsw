/******************************************************************************
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_pass_through_example.c
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
* This function runs the DFE Channel Filter device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Load a channel filter coefficients.
*	- Add CC.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfeCcf_PassThroughExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_Init Init;
	u32 Shift;
	u32 CCID;
	u32 BitSequence;
	XDfeCcf_TriggerCfg TriggerCfg;
	XDfeCcf_Coefficients Coeffs = {
		0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};
	XDfeCcf_CarrierCfg CarrierCfg = { 0, 0, 0, 1, 0, 0, 0 };
	XDfeCcf_Version SwVersion;
	XDfeCcf_Version HwVersion;

	printf("\r\nChannel Filter \"Pass Through\" Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);

	/* Get SW and HW version numbers */
	XDfeCcf_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeCcf_Reset(InstancePtr);
	/* Configure */
	XDfeCcf_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Init.GainStage = 1;
	Init.Sequence.Length = 16;
	XDfeCcf_Initialize(InstancePtr, &Init);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.CCUpdate.Mode = 1U;
	TriggerCfg.CCUpdate.TuserEdgeLevel = 1U;
	TriggerCfg.CCUpdate.TUSERBit = 0;
	XDfeCcf_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeCcf_Activate(InstancePtr, false);

	/* Set coefficents */
	Shift = 5;
	Coeffs.Num = 7U;
	Coeffs.Symmetric = 1U;
	Coeffs.Value[1] = 1 << 15;
	Coeffs.Value[2] = 1 << 13;
	XDfeCcf_LoadCoefficients(InstancePtr, 1, Shift, &Coeffs);

	/* Clear events */
	XDfeCcf_ClearEventStatus(InstancePtr);
	usleep(10000U); /* Give trigger time to finish */

	/* Add component carrier */
	CCID = 0;
	BitSequence = 0xffff;
	CarrierCfg.Rate = 16U;
	CarrierCfg.Gain = round(1024 * 8);
	CarrierCfg.ImagCoeffSet = 0;
	CarrierCfg.RealCoeffSet = 0;
	XDfeCcf_AddCC(InstancePtr, CCID, BitSequence, &CarrierCfg);

	/* Close and exit */
	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);

	printf("Channel Filter \"Pass Through\" Example: Pass\r\n");
	return XST_SUCCESS;
}

/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_multiAddCC_pass_through_example.c
*
* This file contains a load coefficients example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.2   dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include <unistd.h>
#include "xdfemix_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function runs the DFE Mixer device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Get current configuration.
*	- Add CC.
*	- Trigger configuration update.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfeMix_MultiAddCCExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeMix_Cfg Cfg;
	XDfeMix_CCCfg CurrentCCCfg;
	XDfeMix *InstancePtr = NULL;
	XDfeMix_Init Init;
	u32 CCID;
	u32 CCSeqBitmap = 0xffff;
	u32 AntennaId;
	u32 AntennaGain;
	double FreqMhz;
	double NcoFreqMhz;
	double FrequencyControlWord;
	XDfeMix_TriggerCfg TriggerCfg;
	XDfeMix_CarrierCfg CarrierCfg;
	XDfeMix_NCO NCO;
	XDfeMix_Version SwVersion;
	XDfeMix_Version HwVersion;
	XDfeMix_InterruptMask ClearMask;
	u32 ret = XST_FAILURE;

	printf("\r\nMixer \"Multi AddCC Pass Through\" Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of Mixer driver */
	InstancePtr = XDfeMix_InstanceInit(XDFEMIX_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeMix_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeMix_Reset(InstancePtr);
	/* Configure */
	XDfeMix_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Init.Sequence.Length = 16;
	XDfeMix_Initialize(InstancePtr, &Init);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.CCUpdate.Mode = 1U;
	TriggerCfg.CCUpdate.TuserEdgeLevel = 1U;
	TriggerCfg.CCUpdate.TUSERBit = 0;
	XDfeMix_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeMix_Activate(InstancePtr, false);

	/* Set antenna gain */

	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Set antenna 0 gain */
	AntennaId = 0;
	AntennaGain = 1U;
	XDfeMix_SetAntennaGain(InstancePtr, AntennaId, AntennaGain);

	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Set antenna 0 gain */
	AntennaId = 1U;
	XDfeMix_SetAntennaGain(InstancePtr, AntennaId, AntennaGain);

	/* Add component carrier */
	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Add CC */
	CCID = 0;
	CarrierCfg.DUCDDCCfg.NCOIdx = 0;
	CarrierCfg.DUCDDCCfg.CCGain = 3U;
	NCO.NCOGain = 0;
	FreqMhz = 40;
	NcoFreqMhz = 491.52;
	FrequencyControlWord = floor((FreqMhz / NcoFreqMhz) * 0x100000000);
	NCO.FrequencyCfg.FrequencyControlWord = FrequencyControlWord;

	XDfeMix_GetCurrentCCCfg(InstancePtr, &CurrentCCCfg);
	if (XST_SUCCESS == XDfeMix_AddCCtoCCCfg(InstancePtr, &CurrentCCCfg,
						CCID, CCSeqBitmap, &CarrierCfg,
						&NCO)) {
		ret = XST_SUCCESS;
	}

	/* Close and exit */
	XDfeMix_Deactivate(InstancePtr);
	XDfeMix_InstanceClose(InstancePtr);
	printf("Mixer \"Pass Through\" Example: Pass\r\n");
	return ret;
}

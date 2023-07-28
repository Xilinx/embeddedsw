/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm_AddCC_example.c
*
* This file contains a two instances example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   dc     11/21/22 Initial version
* 1.1   dc     07/27/23 Output delay in ccid slots
*
* </pre>
* @addtogroup dfeofdm Overview
* @{
*
*****************************************************************************/
/** @cond nocomments */
/***************************** Include Files ********************************/
#include <unistd.h>
#include "xdfeofdm_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/

/** @endcond */
/****************************************************************************/
/**
*
* This function runs the DFE OFDM device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Add CC.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
/** //! [testexample1] */
int XDfeOfdm_AddCCExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeOfdm_Cfg Cfg;
	XDfeOfdm *InstancePtr = NULL;
	XDfeOfdm_Init Init;
	u32 CCID;
	u32 BitSequence;
	XDfeOfdm_TriggerCfg TriggerCfg;
	XDfeOfdm_CarrierCfg CarrierCfg = {};
	XDfeOfdm_Status Status;
	XDfeOfdm_FTSequence FTSeq;
	u32 Return;
	XDfeOfdm_Version SwVersion;
	XDfeOfdm_Version HwVersion;

	printf("\r\n\nOFDM Example \"AddCC Initialization\" - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}
	memset(&FTSeq, 0, sizeof(FTSeq));
	/* Initialize the instance of OFDM driver */
	InstancePtr = XDfeOfdm_InstanceInit(XDFEOFDM_NODE1_NAME);

	/* Get SW and HW version numbers */
	XDfeOfdm_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeOfdm_Reset(InstancePtr);
	/* Configure */
	XDfeOfdm_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Init.CCSequenceLength = 16;
	XDfeOfdm_Initialize(InstancePtr, &Init);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 0U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.CCUpdate.Mode = 0U;
	TriggerCfg.CCUpdate.TuserEdgeLevel = 0U;
	TriggerCfg.CCUpdate.TUSERBit = 0;
	XDfeOfdm_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeOfdm_Activate(InstancePtr, false);
	u32 val = XDfeOfdm_ReadReg(InstancePtr, 0x20);
	printf("\r\n Active: %x", val);

	printf("\r\n Node Name: %s", InstancePtr->NodeName);
	printf("\r\n Base Address: 0x%lx", InstancePtr->Config.BaseAddr);
	printf("\r\n NumAntenna: %u", InstancePtr->Config.NumAntenna);
	printf("\r\n AntennaInterleave: %u",
	       InstancePtr->Config.AntennaInterleave);

	printf("\r\n VMaj: %u", Cfg.Version.Major);
	printf("\r\n VMin: %u", Cfg.Version.Minor);
	printf("\r\n VRev: %u", Cfg.Version.Revision);
	printf("\r\n VPatch: %u", Cfg.Version.Patch);
	printf("\r\n NumAntennamp: %u", Cfg.ModelParams.NumAntenna);
	printf("\r\n AntennaInterleave (Model Params): %u\n",
	       Cfg.ModelParams.AntennaInterleave);

	/* Clear event status */
	Status.SaturationCCID = XDFEOFDM_ISR_CLEAR;
	Status.SaturationCount = XDFEOFDM_ISR_CLEAR;
	Status.CCUpdate = XDFEOFDM_ISR_CLEAR;
	Status.FTCCSequenceError = XDFEOFDM_ISR_CLEAR;
	Status.Saturation = XDFEOFDM_ISR_CLEAR;
	XDfeOfdm_ClearEventStatus(InstancePtr, &Status);
	usleep(10000U); /* Give trigger time to finish */

	/* Add component carrier */
	CCID = 0;
	BitSequence = 0x17;
	CarrierCfg.Numerology = 1;
	CarrierCfg.FftSize = 2048;
	CarrierCfg.NumSubcarriers = 1;
	CarrierCfg.ScaleFactor = 1;
	CarrierCfg.CommsStandard = 0;
	CarrierCfg.OutputDelay = 1;
	FTSeq.Length = 2;
	Return = XDfeOfdm_AddCC(InstancePtr, CCID, BitSequence, &CarrierCfg,
				&FTSeq);

	if (Return == XST_SUCCESS) {
		printf("Add CC done!\n\r");
	} else {
		printf("Add CC failed!\n\r");
		printf("OFDM \"AddCC\" Example: Fail\r\n");
		return XST_FAILURE;
	}

	/* Shutdown the block */
	XDfeOfdm_Deactivate(InstancePtr);
	XDfeOfdm_InstanceClose(InstancePtr);

	printf("OFDM Example \"AddCC\": Pass\r\n");
	return XST_SUCCESS;
}
/** //! [testexample1] */
/** @} */

/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_2CC_3RC_dynamic_example.c
*
* This file contains dynamic setting of 2 CC and 3 RC example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.3   dc     02/07/22 Configure 2 CC and 3 RC examples
* 1.4   dc     04/26/22 Add dynamic config example
* 1.5   dc     12/14/22 Update multiband register arithmetic
*       dc     01/02/23 Multiband registers update
* 1.6   dc     06/15/23 Configure all trigger parameters in examples
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
#define XDFEPRACH_NUMBER_CC 2U
#define XDFEPRACH_NUMBER_RC 3U

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
*	- Get current Component Carrier (CC) configuration.
*	- Get current RACH Channel (RC) configuration.
*	- add 2 Component Carriers: CCID={5,3}
*	- add 3 RACH Channel: RCID={2,4,6}
*	- Trigger configuration update.
*	- Deactivate the device.
*
* @return
*	- XST_SUCCESS if the example has completed successfully.
*	- XST_FAILURE if the example has failed.
*
****************************************************************************/
/** //! [testexample4] */
int XDfePrach_2CC3RCDynamicTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfePrach_Cfg Cfg = {
		{ 0, 0, 0, 0 },
	};
	XDfePrach *InstancePtr = NULL;
	XDfePrach_Version SwVersion = { 0 };
	XDfePrach_Version HwVersion = { 0 };
	XDfePrach_TriggerCfg TriggerCfg = { 0 };
	XDfePrach_StatusMask StatusMask = { 0 };
	/* Sequence: length 4, ID=0, Use static Schedule, length may depend
	   on core generics (antenna interleave) */
	XDfePrach_Init Init = { { { 0, { 0 } }, { 0, { 0 } }, { 0, { 0 } } },
				1,
				0 };
	/* Full CC config structure */
	XDfePrach_CCCfg CCCfg = { 0 };
	/* Full RC config structure */
	XDfePrach_RCCfg RCCfg = { 0 };
	int Index;
	int fir_stg;
	int RC_idx;
	int CC_idx;

	XDfePrach_CarrierCfg CarrierCfg[XDFEPRACH_NUMBER_CC]; /* CC settings */
	u32 CCSeqBitmap[XDFEPRACH_NUMBER_CC]; /* Insertion location */
	s32 CCID[XDFEPRACH_NUMBER_CC]; /* CC number */

	/* set up an RCIDs */
	s32 RCID[XDFEPRACH_NUMBER_RC]; /* RCID number */
	s32 PhysRachChannel[XDFEPRACH_NUMBER_RC]; /* The physical location of
		the RACH decimation - Must match RCID in this release */
	XDfePrach_DDCCfg
		DdcCfg[XDFEPRACH_NUMBER_RC]; /* decimation configuration */
	XDfePrach_NCO
		NcoCfg[XDFEPRACH_NUMBER_RC]; /* demodulation configuration */

	/* set up "first" CC */
	CCID[0] = 5; /* Select the CCID to add */
	CarrierCfg[0].SCS = 0; /* Configure this CC: 15kHz SCS */
	CCSeqBitmap[0] = 0x0001U; /* set up insertion location. Just use 1 slot
		in the CC sequence, because only a 30.72MHz channel */

	/* set up "second" CC */
	CCID[1] = 3; /* different CCID */
	CarrierCfg[1].SCS = 0; /* Configure this CC: 15kHz SCS */
	CCSeqBitmap[1] = 0x0002U; /* Also single slot, but in second location */

	/* In Dynamic mode, only the RCID and PhysRachChannel it is allocated
	   to need to be set. All other fields are set through the Dynamic
	   Control Packet on the s_axis_sched* interface. Hence the removal
	   of a number of the settings found in xdfeprach_2CC_3RC_example.c. */

	/* set up an RCID "2" from CCID 5 */
	RCID[0] = 2; /* Select RCID which the RACH capture will run on. */
	PhysRachChannel[0] = RCID[0]; /* the physical location of the RACH
		decimation - Must match RCID in this release. */

	/* set up an another RCID "4" from CCID 5 (different freq offset) */
	RCID[1] = 4; /* Select RCID which the RACH capture will run on. */
	PhysRachChannel[1] = RCID[1]; /* the physical location of the RACH
		decimation - Must match RCID in this release. */

	/* set up an an RCID "6" from CCID 3 (different freq offset) */
	RCID[2] = 6; /* Select RCID which the RACH capture will run on. */
	PhysRachChannel[2] = RCID[2]; /* the physical location of the RACH */

	printf("\r\nPrach \"2xCC and 3xRC\" Dynamic Example - Start\n\n\r");

	/* set up decimation and demodulation structures: */
	for (Index = 0; Index < XDFEPRACH_NUMBER_RC; Index++) {
		DdcCfg[Index].DecimationRate =
			0; /* Not needed in Dynamic mode.*/
		DdcCfg[Index].UserSCS = 0;
		for (fir_stg = 0; fir_stg < 6; fir_stg++) {
			DdcCfg[Index].RachGain[fir_stg] = 0; /* 0dB FIR gain */
		}
		/* demodulation configuration: */
		NcoCfg[Index].UserFreq = 0; /* Not needed in Dynamic mode.*/
		NcoCfg[Index].PhaseOffset = 0; /* 0 phase offset */
		NcoCfg[Index].PhaseAcc = 0; /* 0 initial phase */
		NcoCfg[Index].NcoGain = 0; /* 0dB NCO gain */
		NcoCfg[Index].DualModCount = 0; /* 0 dual mod count init */
		NcoCfg[Index].DualModSel = 0; /* 0 dual mod sel init */
	}

	XDfePrach_Schedule StaticSchedule;
	StaticSchedule.PatternPeriod = 0; /* Not needed in Dynamic mode.*/
	StaticSchedule.FrameID = 0; /* Not needed in Dynamic mode.*/
	StaticSchedule.SubframeID = 0; /* Not needed in Dynamic mode.*/
	StaticSchedule.SlotId = 0; /* Not needed in Dynamic mode.*/
	StaticSchedule.Duration = 0; /* Not needed in Dynamic mode.*/
	StaticSchedule.Repeats = 0; /* Not needed in Dynamic mode.*/

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n\r");
		return XST_FAILURE;
	}
	metal_set_log_level(METAL_LOG_WARNING);

	/* Initialize the instance of PRACH driver */
	InstancePtr = XDfePrach_InstanceInit(XDFEPRACH_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfePrach_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\n\r", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\n\r",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */

	/* Reset the core */
	XDfePrach_Reset(InstancePtr);
	/* Get the hardware configuration from the PRACH core to determine
	   what can be sent */
	XDfePrach_Configure(InstancePtr, &Cfg);
	/* Set the sequence length based upon the AntennaInterleave: */
	Init.Sequence[0].Length = 16U / Cfg.ModelParams.NumAntennaSlots[0];
	/* Disable the static Schedule - this makes a number of registers
	   Read Only to the AXI interface - they will expect to be programmed
	   via the s_axis_sched* AXI-S interface */
	Init.EnableStaticSchedule = 0;

	/* Initialise the core */
	XDfePrach_Initialize(InstancePtr, &Init);

	/* Set up the Frame trigger here. Must match with s_axis_din_tuser
	   bit to mark the frame timing. Setting the trigger can occur later
	   in the programme flow, as long as it is set before the tuser bit
	   arrives */
	TriggerCfg.Activate.TUSERBit = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 3U;
	TriggerCfg.Activate.StateOutput = 1U;
	TriggerCfg.Activate.Mode = 0U; /* "Immediate trigger" is set for test
		purposes, in practical mode would be 1 the "single shot"
		trigger */

	TriggerCfg.RachUpdate.TUSERBit = 2U;
	TriggerCfg.RachUpdate.TuserEdgeLevel = 3U;
	TriggerCfg.RachUpdate.StateOutput = 1U;
	TriggerCfg.RachUpdate.Mode = 0U; /* "Immediate trigger" is set for test
		purposes, in practical mode would be 1 the "single shot"
		trigger */

	TriggerCfg.FrameInit[0].TUSERBit = 3U;
	TriggerCfg.FrameInit[0].TuserEdgeLevel = 3U;
	TriggerCfg.FrameInit[0].StateOutput = 1U;
	TriggerCfg.FrameInit[0].Mode = 2U;

	TriggerCfg.FrameInit[1].TUSERBit = 3U;
	TriggerCfg.FrameInit[1].TuserEdgeLevel = 3U;
	TriggerCfg.FrameInit[1].StateOutput = 1U;
	TriggerCfg.FrameInit[1].Mode = 2U;

	TriggerCfg.FrameInit[2].TUSERBit = 3U;
	TriggerCfg.FrameInit[2].TuserEdgeLevel = 3U;
	TriggerCfg.FrameInit[2].StateOutput = 1U;
	TriggerCfg.FrameInit[2].Mode = 2U;
	XDfePrach_SetTriggersCfg(InstancePtr, &TriggerCfg);
	printf("Frame Init Trigger Configured\n\r");

	/* Clear out the rach update flags and mask to allow the addcc to
	 proceed. */
	StatusMask.RachUpdate = 1U;
	XDfePrach_ClearEventStatus(InstancePtr, &StatusMask);
	printf("Mask/Clear Trigger Status\n\r");

	/* Move core to activate */
	XDfePrach_Activate(InstancePtr, false);

	/* Create CC sequences */
	/* Create the "empty" carrier config */
	XDfePrach_GetEmptyCCCfg(InstancePtr, &CCCfg);

	/* Add CCs to CCCfg. CC setup is the same for both Dynamic and Static modes */
	for (CC_idx = 0; CC_idx < XDFEPRACH_NUMBER_CC; CC_idx++) {
		if (XST_SUCCESS ==
		    XDfePrach_AddCCtoCCCfg(InstancePtr, &CCCfg, CCID[CC_idx],
					   CCSeqBitmap[CC_idx],
					   &CarrierCfg[CC_idx])) {
			printf("Adding CC Configuration for CCID %0d to PRACH\n\r",
			       CCID[CC_idx]);
		} else {
			printf("ERROR: Failed to Add CCID %0d to PRACH\n\r",
			       CCID[CC_idx]);
			return XST_FAILURE;
		}
	}

	/* Create RCIDs.
	   Get an empty RCCfg structure because first update from reset.*/
	XDfePrach_GetEmptyRCCfg(InstancePtr, &RCCfg);
	printf("Get the current (empty) RCCfg structure\n\r");

	/* Add RCIDs to RCCfg. In Dynamic mode, there are a number of registers
	   which are Read Only. This means they become don't care in
	   the software - writing them will have no effect. These registers are
	   the ones configured through the DdcCfg, NcoCfg and StaticSchedule
	   structures. The target CC is also don't care, since this is also
	   a dynamic field, specified by the dynamic control packet.
	   The value in the DCP must correspond to a CC which was programmed
	   above. The RCID and PhysRachChannel must be configured in Dynamic
	   mode. This is so the RCID requested in the DCP has a physical
	   Channel allocated in the core. */
	for (RC_idx = 0; RC_idx < XDFEPRACH_NUMBER_RC; RC_idx++) {
		if (XST_SUCCESS ==
		    XDfePrach_AddRCtoRCCfg(
			    InstancePtr, &RCCfg,
			    0, /* CC this RCID is being captured from - Don't
			          care for Dynamic */
			    RCID[RC_idx], /* RCID number */
			    PhysRachChannel[RC_idx], /* physical channel number */
			    &DdcCfg[RC_idx], /* Decimation Configuration -
			                        don't care for Dynamic */
			    &NcoCfg[RC_idx], /* NCO Configuration - don't care
			                        for Dynamic */
			    &StaticSchedule, /* Static schedule - don't care
			                         for Dynamic */
			    &CCCfg)) {
			printf("Adding Configuration for RCID %0d to PRACH\n\r",
			       RCID[RC_idx]);
		} else {
			printf("ERROR: Failed to Add RCID %0d to PRACH\n\r",
			       RCID[RC_idx]);
			return XST_FAILURE;
		}
	}
	/* Update the newly built config in the registers. This function
	   call sets the "rach_update" trigger. */
	XDfePrach_SetNextCfg(InstancePtr, &CCCfg, &RCCfg);
	printf("Sent new configuration to the Core\n\r");

	printf("\r\nPrach \"2xCC and 3xRC Dynamic \" Example - Pass\n\n\r");

	/*  Shutdown the block */
	XDfePrach_Deactivate(InstancePtr);
	XDfePrach_InstanceClose(InstancePtr);

	return XST_SUCCESS;
}
/** //! [testexample4] */
/** @} */

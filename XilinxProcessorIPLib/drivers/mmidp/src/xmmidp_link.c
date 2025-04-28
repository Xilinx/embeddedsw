/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_link.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0    ck   03/14/25  Initial Release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <xstatus.h>
#include <sleep.h>

#include "xmmidp.h"

#define XMMIDP_CR_TIMEOUT_COUNT 50

/******************************************************************************/
/**
 * This function enables fast link training in the core controller
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Phy width val to be used over the main link based on
 *              one of the following selects:
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_FastLinkTrainEnable(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->LinkConfig.FastLinkTrainEn = 0x1;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_DEFAULT_FAST_LINK_TRAIN_EN_MASK,
				  XMMIDP_CCTL0_DEFAULT_FAST_LINK_TRAIN_EN_SHIFT,
				  InstancePtr->LinkConfig.FastLinkTrainEn);
}

/******************************************************************************/
/**
 * This function disables fast link training in the core controller
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_FastLinkTrainDisable(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->LinkConfig.FastLinkTrainEn = 0x0;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_DEFAULT_FAST_LINK_TRAIN_EN_MASK,
				  XMMIDP_CCTL0_DEFAULT_FAST_LINK_TRAIN_EN_SHIFT,
				  InstancePtr->LinkConfig.FastLinkTrainEn);
}

/******************************************************************************/
/**
 * This function gets the max lane count supported from DPCD
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetRxMaxLaneCount(XMmiDp *InstancePtr)
{
	u32 DpcdVal = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_MAX_LANE_COUNT,
		       1, &DpcdVal);

	InstancePtr->RxConfig.MaxNumLanes =
		(DpcdVal & XMMIDP_DPCD_MAX_LANE_COUNT_MASK)
		>> XMMIDP_DPCD_MAX_LANE_COUNT_SHIFT;

	InstancePtr->RxConfig.MaxLaneCount =
		GetLaneCount(InstancePtr, InstancePtr->RxConfig.MaxNumLanes);

	InstancePtr->RxConfig.EnhancedFrameCap =
		(DpcdVal & XMMIDP_DPCD_ENHANCED_FRAME_CAP_MASK)
		>> XMMIDP_DPCD_ENHANCED_FRAME_CAP_SHIFT;

	InstancePtr->RxConfig.PostLtAdjReqSupported =
		(DpcdVal & XMMIDP_DPCD_POST_LT_ADJ_REQ_SUPPORTED_MASK)
		>> XMMIDP_DPCD_POST_LT_ADJ_REQ_SUPPORTED_SHIFT;

	InstancePtr->RxConfig.Tps3Supported =
		(DpcdVal & XMMIDP_DPCD_TPS3_SUPPORTED_MASK)
		>> XMMIDP_DPCD_TPS3_SUPPORTED_SHIFT;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function gets the max link rate supported from Rx DPCD register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetRxMaxLinkRate(XMmiDp *InstancePtr)
{
	u32 Status = 0;
	u32 MaxLinkBW = 0;

	Status = XMmiDp_AuxRead(InstancePtr,
				XMMIDP_DPCD_MAX_LINK_RATE, 1, &MaxLinkBW);
	if ( Status != XST_SUCCESS ) {
		xil_printf("AuxRead Err: %d\n", Status);
		return Status;
	}

	InstancePtr->RxConfig.MaxLinkBW = MaxLinkBW;
	InstancePtr->RxConfig.MaxLinkRate =
		GetLinkRate(InstancePtr, MaxLinkBW);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function initates link training sequence with the Rx
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_StartLinkXmit(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	XMmiDp_SetPhyLaneCount(InstancePtr,
			       InstancePtr->RxConfig.MaxLaneCount);

	XMmiDp_SetPhyLinkRate(InstancePtr,
			      InstancePtr->RxConfig.MaxLinkRate);

	XMmiDp_SetPhyVoltageSwing(InstancePtr, InstancePtr->LinkConfig.VsLevel);
	XMmiDp_SetPhyPreEmphasis(InstancePtr, InstancePtr->LinkConfig.PeLevel);

	XMmiDp_SetPhyTrainingPattern(InstancePtr, PHY_NO_TRAIN);
	XMmiDp_SetDpcdTrainingPattern(InstancePtr, PHY_NO_TRAIN);

	XMmiDp_SetPhyXmitEnable(InstancePtr);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function configures DPCD Rx Field
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_SetSinkDpcdLinkCfgField(XMmiDp *InstancePtr)
{
	u32 MaxLinkBW = InstancePtr->RxConfig.MaxLinkBW;
	u32 ChannelCodingSet = InstancePtr->LinkConfig.ChannelCodingSet;

	/* Link BW Set */
	XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_LINK_BW_SET,
			0x1, &MaxLinkBW);

	/* Lane Count Set */
	XMmiDp_DpcdReadModifyWrite(InstancePtr,
				   XMMIDP_DPCD_LANE_COUNT_SET,
				   XMMIDP_DPCD_LANE_COUNT_SET_MASK,
				   XMMIDP_DPCD_LANE_COUNT_SET_SHIFT,
				   InstancePtr->RxConfig.MaxNumLanes);

	/* Spread Amp */
	XMmiDp_DpcdReadModifyWrite(InstancePtr,
				   XMMIDP_DPCD_DOWNSPREAD_CTRL,
				   XMMIDP_DPCD_SPREAD_AMP_MASK,
				   XMMIDP_DPCD_SPREAD_AMP_SHIFT,
				   InstancePtr->LinkConfig.SpreadAmp);

	/* Main Link Channel Coding Set */
	XMmiDp_AuxWrite(InstancePtr,
			XMMIDP_DPCD_MAIN_LINK_CHANNEL_CODING_SET,
			0x1, &ChannelCodingSet);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function configures CCTL enhance framing enable bit
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XDpPSu14_EnableCctlEnhanceFraming(XMmiDp *InstancePtr)
{

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_MASK,
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_SHIFT, 0x1);

}

/******************************************************************************/
/**
 * This function configures CCTL enhance framing enable bit
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XDpPSu14_DisableCctlEnhanceFraming(XMmiDp *InstancePtr)
{

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_MASK,
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_SHIFT, 0x0);

}

/******************************************************************************/
/**
 * This function reads Training Aux Read Interval register of sink DPCD
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GetDpcdTrainingAuxRdInterval(XMmiDp *InstancePtr)
{
	u32 TrainingAuxRdInterval = 0;

	XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_TRAINING_AUX_RD_INTERVAL,
		       1, &TrainingAuxRdInterval);

	InstancePtr->RxConfig.TrainingAuxRdInterval =
		(TrainingAuxRdInterval & XMMIDP_DPCD_TRAINING_AUX_RD_INTERVAL_MASK)
		>> XMMIDP_DPCD_TRAINING_AUX_RD_INTERVAL_SHIFT;

	InstancePtr->RxConfig.ExtendedReceiverCap =
		(TrainingAuxRdInterval & XMMIDP_DPCD_EXTENDED_REEIVER_CAPABILITY_FIELD_PRESENT_MASK)
		>> XMMIDP_DPCD_EXTENDED_REEIVER_CAPABILITY_FIELD_PRESENT_SHIFT;

}

/******************************************************************************/
/**
 * This function returngs the training aux interval
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *              - WaitTime.
 *
 * @note        None.
*******************************************************************************/
u32 XMmiDp_GetTrainingDelay(XMmiDp *InstancePtr)
{

	u32 WaitTime = 400;

	if (InstancePtr->RxConfig.TrainingAuxRdInterval) {
		return (WaitTime * InstancePtr->RxConfig.TrainingAuxRdInterval * 10);
	} else {
		return WaitTime;
	}

}

/******************************************************************************/
/**
 * This function will do a burst AUX read from the RX device over the AUX
 * channel. The contents of the status registers will be stored for later use by
 * XDp_TxCheckClockRecovery, XDp_TxCheckChannelEqualization, and
 * XDp_TxAdjVswingPreemp.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *              - XST_SUCCESS if the AUX read was successful.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetDpcdLaneStatusAdjReqs(XMmiDp *InstancePtr)
{
	u32 Status;

	/* Read and store 4 bytes of lane status and 2 bytes of adjustment
	 * requests. */
	Status = XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_LANE0_1_STATUS,
				6, InstancePtr->RxConfig.LaneStatusAdjReqs);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the clock recovery sequence during link training was
 * successful - the RX device's link clock and data recovery unit has realized
 * and maintained the frequency lock for all lanes currently in use.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the RX device's clock recovery PLL has
 *                achieved frequency lock for all lanes in use.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_CheckClockRecovery(XMmiDp *InstancePtr, u8 LaneCount)
{
	/* Check that all LANEx_CR_DONE bits are set. */
	switch (LaneCount) {
		case PHY_LANES_4:

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE2_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE2_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE3_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE3_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			InstancePtr->LinkConfig.CrDoneCnt =
				XMMIDP_LANE_ALL_CR_DONE;

		case PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			InstancePtr->LinkConfig.CrDoneCnt =
				XMMIDP_LANE_2_CR_DONE;

		case PHY_LANES_1:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			InstancePtr->LinkConfig.CrDoneCnt =
				XMMIDP_LANE_1_CR_DONE;

		default:
			break;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function reads the adjusted voltage swing and pre-emphasis level settings from
 * the Rx DPCD register and modify the drive settings accordingly during link training
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *              - XST_SUCCESS if writing the settings was successful.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
*******************************************************************************/
u32 XMmiDp_AdjVswingPreemp(XMmiDp *InstancePtr)
{
	u32 Status;

	u8 Index;
	u8 VsLevelAdjReq[4];
	u8 PeLevelAdjReq[4];
	u8 AuxData[4];

	InstancePtr->LinkConfig.VsLevelUpdated = FALSE;
	InstancePtr->LinkConfig.PeLevelUpdated = FALSE;

	/* Analyze the adjustment requests for changes in voltage swing and
	 * pre-emphasis levels. */
	VsLevelAdjReq[0] = (InstancePtr->RxConfig.LaneStatusAdjReqs[4] & XMMIDP_DPCD_VSWING_LANE0_MASK) >>
			   XMMIDP_DPCD_VSWING_LANE0_SHIFT;
	VsLevelAdjReq[1] = (InstancePtr->RxConfig.LaneStatusAdjReqs[4] & XMMIDP_DPCD_VSWING_LANE1_MASK) >>
			   XMMIDP_DPCD_VSWING_LANE1_SHIFT;
	VsLevelAdjReq[2] = (InstancePtr->RxConfig.LaneStatusAdjReqs[5] & XMMIDP_DPCD_VSWING_LANE2_MASK) >>
			   XMMIDP_DPCD_VSWING_LANE2_SHIFT;
	VsLevelAdjReq[3] = (InstancePtr->RxConfig.LaneStatusAdjReqs[5] & XMMIDP_DPCD_VSWING_LANE3_MASK) >>
			   XMMIDP_DPCD_VSWING_LANE3_SHIFT;

	PeLevelAdjReq[0] = (InstancePtr->RxConfig.LaneStatusAdjReqs[4] & XMMIDP_DPCD_PREEMP_LANE0_MASK) >>
			   XMMIDP_DPCD_PREEMP_LANE0_SHIFT;
	PeLevelAdjReq[1] = (InstancePtr->RxConfig.LaneStatusAdjReqs[4] & XMMIDP_DPCD_PREEMP_LANE1_MASK) >>
			   XMMIDP_DPCD_PREEMP_LANE1_SHIFT;
	PeLevelAdjReq[2] = (InstancePtr->RxConfig.LaneStatusAdjReqs[5] & XMMIDP_DPCD_PREEMP_LANE2_MASK) >>
			   XMMIDP_DPCD_PREEMP_LANE2_SHIFT;
	PeLevelAdjReq[3] = (InstancePtr->RxConfig.LaneStatusAdjReqs[5] & XMMIDP_DPCD_PREEMP_LANE3_MASK) >>
			   XMMIDP_DPCD_PREEMP_LANE3_SHIFT;

	/* Change the drive settings to match the adjustment requests. Use the
	 * greatest level requested. */
	for (Index = 0; Index < InstancePtr->LinkConfig.NumLanes;
	     Index++) {
		if (VsLevelAdjReq[Index] > InstancePtr->LinkConfig.VsLevel[Index]) {
			InstancePtr->LinkConfig.VsLevel[Index] =
				VsLevelAdjReq[Index];

			InstancePtr->LinkConfig.VsLevelUpdated = TRUE;
		}

		if (PeLevelAdjReq[Index] > InstancePtr->LinkConfig.PeLevel[Index]) {
			InstancePtr->LinkConfig.PeLevel[Index] =
				PeLevelAdjReq[Index];
			InstancePtr->LinkConfig.PeLevelUpdated = TRUE;
		}

	}

	memset(AuxData, 0, 4);
	XMmiDp_SetVswingPreemp(InstancePtr, AuxData);

	/* Configure DPCD TRAINING LANE SET */
	Status = XMmiDp_AuxWrite(InstancePtr,
				 XMMIDP_DPCD_TRAINING_LANE0_SET, 4, AuxData);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets current voltage swing and pre-emphasis level settings from
 * the LinkConfig structure to Phy and DPCD.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       AuxData is a pointer to the array used for preparing a burst
 *              write over the AUX channel.
 *
 * @return
 *              - XST_SUCCESS if writing the settings was successful.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
*******************************************************************************/
void XMmiDp_SetVswingPreemp(XMmiDp *InstancePtr, u8 *AuxData)
{

	for (int Index = 0; Index < InstancePtr->LinkConfig.NumLanes; Index++) {

		/* The maximum voltage swing has been reached. */
		if (InstancePtr->LinkConfig.VsLevel[Index] >= XMMIDP_MAX_VS_LEVEL) {
			AuxData[Index] |= XMMIDP_DPCD_MAX_SWING_REACHED_MASK;
		}

		/* The maximum pre-emphasis level has been reached. */
		if (InstancePtr->LinkConfig.PeLevel[Index] >= XMMIDP_MAX_PE_LEVEL) {
			AuxData[Index] |= XMMIDP_DPCD_MAX_PREEMPHASIS_REACHED_MASK;
		}

		/* Set up the data buffer for writing to the RX device. */
		AuxData[Index] |= (InstancePtr->LinkConfig.PeLevel[Index] <<
				   XMMIDP_DPCD_PREEMPHASIS_SET_SHIFT) |
				  InstancePtr->LinkConfig.VsLevel[Index];

	}

	XMmiDp_SetPhyVoltageSwing(InstancePtr, InstancePtr->LinkConfig.VsLevel);
	XMmiDp_SetPhyPreEmphasis(InstancePtr, InstancePtr->LinkConfig.PeLevel);

	return;
}

/******************************************************************************/
/**
 * This function sets the training pattern to be used during link training for
 * both the DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       Pattern selects the pattern to be used.
 *
 * @return
 *              - XST_SUCCESS if setting the pattern was successful.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_SetTrainingPattern(XMmiDp *InstancePtr, XMmiDp_PhyTrainingPattern Pattern)
{
	u32 Status;
	u8 AuxData[5];
	u8 Val = Pattern;

	memset(AuxData, 0, 5);

	InstancePtr->LinkConfig.ScrambleEn = 0x0;

	XMmiDp_SetPhyTrainingPattern(InstancePtr, Pattern);

	if (Pattern == PHY_TPS4) {
		Val = 0x7;
		InstancePtr->LinkConfig.ScrambleEn = 0x1;
		XMmiDp_PhyScrambleDisable(InstancePtr);
	} else {
		XMmiDp_PhyScrambleEnable(InstancePtr);
	}

	AuxData[0] |= Val << XMMIDP_DPCD_TRAINING_PATTERN_SELECT_SHIFT;
	AuxData[0] |= (InstancePtr->LinkConfig.ScrambleEn) <<
		      XMMIDP_DPCD_SCRAMBLING_DISABLE_SHIFT;

	XMmiDp_SetVswingPreemp(InstancePtr, &AuxData[1]);

	if (Pattern == PHY_NO_TRAIN)
		Status = XMmiDp_AuxWrite(InstancePtr,
					 XMMIDP_DPCD_TRAINING_PATTERN_SET, 1, AuxData);
	else
		Status = XMmiDp_AuxWrite(InstancePtr,
					 XMMIDP_DPCD_TRAINING_PATTERN_SET, 5, AuxData);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the Rx Dpcd link rate as well as Phy link rate
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       LinkRate value to program.
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_SetLinkRate(XMmiDp *InstancePtr, XMmiDp_PhyRate LinkRate)
{
	u32 Status = 0;

	XMmiDp_SetPhyPowerdown(InstancePtr, PHY_POWER_DOWN);
	Status = XMmiDp_PhyWaitReady(InstancePtr);
	if ( Status != XST_SUCCESS ) {
		xil_printf("Phy Busy Timeout %d\n", Status);
		return Status;
	}

	InstancePtr->LinkConfig.LinkRate = LinkRate;
	InstancePtr->LinkConfig.LinkBW = GetLinkBW(InstancePtr, LinkRate);

	XMmiDp_SetPhyLinkRate(InstancePtr, LinkRate);
	XMmiDp_SetPhyPowerdown(InstancePtr, PHY_POWER_ON);

	XMmiDp_SetDpcdLinkRate(InstancePtr);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the Rx Dpcd lane count as well as Phy lane count
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       LaneCount value to program.
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetLaneCount(XMmiDp *InstancePtr, XMmiDp_PhyLanes LaneCount)
{

	InstancePtr->LinkConfig.LaneCount = LaneCount;
	InstancePtr->LinkConfig.NumLanes = GetNumLanes(InstancePtr, LaneCount);

	XMmiDp_SetPhyLaneCount(InstancePtr, LaneCount);
	XMmiDp_SetDpcdLaneCount(InstancePtr);

}

/******************************************************************************/
/**
 * This function runs the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *      0) Start signaling at the minimum voltage swing, pre-emphasis, and post-
 *         cursor levels.
 *      1) Transmit training pattern 1 over the main link with symbol scrambling
 *         disabled.
 *      2) The clock recovery loop. If clock recovery is unsuccessful after
 *         MaxIterations loop iterations, return.
 *      2a) Wait for at least the period of time specified in the RX device's
 *          DisplayPort Configuration Data (DPCD) register,
 *          TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved clock recovery lock. If so, return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the RX device.
 *      2e) Loop back to 2a.
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return      The next training state:
 *              - XMMIDP__TS_CHANNEL_EQUALIZATION if the clock recovery sequence
 *                completed successfully.
 *              - XMMIDP__TS_FAILURE if writing the drive settings to the RX
 *                device was unsuccessful.
 *              - XMMIDP__TS_ADJUST_LINK_RATE if the clock recovery sequence
 *                did not complete successfully.
 *
 * @note        None.
 *
*******************************************************************************/
XMmiDp_TrainingState XMmiDp_TrainingStateClockRecovery(XMmiDp *InstancePtr)
{
	u32 Status;
	u8 SameVsLevelCnt = 0;
	u8 TimeOutCnt = 0;

	memset(InstancePtr->LinkConfig.VsLevel, 0, 4);
	memset(InstancePtr->LinkConfig.PeLevel, 0, 4);

	Status = XMmiDp_SetTrainingPattern(InstancePtr, PHY_TPS1);

	if ( Status != XST_SUCCESS ) {
		return XMMIDP_TS_FAILURE;
	}

	while (TimeOutCnt <= XMMIDP_CR_TIMEOUT_COUNT) {
		XMmiDp_WaitUs(InstancePtr, 100);

		Status = XMmiDp_GetDpcdLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XMMIDP_TS_FAILURE;
		}

		Status = XMmiDp_CheckClockRecovery(InstancePtr,
						   InstancePtr->LinkConfig.LaneCount);

		if (Status == XST_SUCCESS) {
			return XMMIDP_TS_CHANNEL_EQUALIZATION;
		}

		if (InstancePtr->LinkConfig.VsLevelUpdated ||
		    InstancePtr->LinkConfig.PeLevelUpdated) {
			SameVsLevelCnt = 0;

		} else {
			SameVsLevelCnt++;
		}

		if (SameVsLevelCnt >= 5) {
			break;
		}

		if (InstancePtr->LinkConfig.VsLevel[0] == XMMIDP_MAX_VS_LEVEL) {
			break;
		}

		Status = XMmiDp_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XMMIDP_TS_FAILURE;
		}

	}

	if (InstancePtr->LinkConfig.LinkRate ==  XMMIDP_DPCD_LINK_BW_SET_162GBPS) {
		if ((InstancePtr->LinkConfig.CrDoneCnt != XMMIDP_LANE_ALL_CR_DONE) &&
		    (InstancePtr->LinkConfig.CrDoneCnt != XMMIDP_LANE_0_CR_DONE)) {
			Status = XMmiDp_SetTrainingPattern(InstancePtr, PHY_NO_TRAIN);
			XMmiDp_SetLinkRate(InstancePtr,
					   XMMIDP_DPCD_LINK_BW_SET_810GBPS);
			XMmiDp_SetLaneCount(InstancePtr,
					    InstancePtr->LinkConfig.CrDoneCnt);
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->LinkConfig.CrDoneCnt;
			return XMMIDP_TS_CLOCK_RECOVERY;
		}

	}

	return XMMIDP_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training. As a result, the data rate will
 * be downshifted, and training will be re-attempted (starting with clock
 * recovery) at the reduced data rate. If the data rate is already at 1.62 Gbps,
 * a downshift in lane count will be attempted.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return      The next training state:
 *              - XMMIDP_TS_ADJUST_LANE_COUNT if the minimal data rate is
 *                already in use. Re-attempt training at a reduced lane count.
 *              - XMMIDP_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note        None.
 *
*******************************************************************************/
XMmiDp_TrainingState XMmiDp_TrainingStateAdjustLinkRate(XMmiDp *InstancePtr)
{
	u32 Status;

	switch (InstancePtr->LinkConfig.LinkBW) {
		case XMMIDP_DPCD_LINK_BW_SET_810GBPS:
			XMmiDp_SetLinkRate(InstancePtr,
					   PHY_RATE_HBR2_540GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		case XMMIDP_DPCD_LINK_BW_SET_540GBPS:
			XMmiDp_SetLinkRate(InstancePtr,
					   PHY_RATE_HBR_270GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		case XMMIDP_DPCD_LINK_BW_SET_270GBPS:
			XMmiDp_SetLinkRate(InstancePtr,
					   PHY_RATE_RBR_162GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		default:

			Status = XMMIDP_TS_ADJUST_LANE_COUNT;
			break;

	}

	return Status;

}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training, and a minimal data rate of 1.62
 * Gbps was being used. As a result, the number of lanes in use will be reduced,
 * and training will be re-attempted (starting with clock recovery) at this
 * lower lane count.
 *
 * @note        Training will be re-attempted with the maximum data rate being
 *              used with the reduced lane count to train at the main link at
 *              the maximum bandwidth possible.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return      The next training state:
 *              - XMMIDP_TS_FAILURE if only one lane is already in use.
 *              - XMMIDP_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note        None.
 *
*******************************************************************************/
XMmiDp_TrainingState XMmiDp_TrainingStateAdjustLaneCount(XMmiDp *InstancePtr)
{
	u32 Status;

	switch (InstancePtr->LinkConfig.LaneCount) {
		case PHY_LANES_4:
			XMmiDp_SetLaneCount(InstancePtr, PHY_LANES_2);
			XMmiDp_SetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkRate);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;

		case PHY_LANES_2:
			XMmiDp_SetLaneCount(InstancePtr, PHY_LANES_1);
			XMmiDp_SetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkRate);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;

		default:
			Status = XMMIDP_TS_FAILURE;
			break;

	}

	return Status;
}

/******************************************************************************/
/**
 * This function reads Rx DPCD MaxDownspread register bits
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GetDpcdMaxDownspread(XMmiDp *InstancePtr)
{
	u32 MaxDownspread = 0;

	XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_MAX_DOWNSPREAD
		       , 1, &MaxDownspread);

	InstancePtr->RxConfig.Tps4Supported = (MaxDownspread & XMMIDP_DPCD_TPS4_SUPPORTED_MASK)
					      >> XMMIDP_DPCD_TPS4_SUPPORTED_SHIFT;

	InstancePtr->RxConfig.NoAuxLinkTraining =
		(MaxDownspread & XMMIDP_DPCD_NO_AUX_TRANSACTION_LINK_TRAINING_MASK)
		>> XMMIDP_DPCD_NO_AUX_TRANSACTION_LINK_TRAINING_SHIFT;

	InstancePtr->RxConfig.MaxDownspread = (MaxDownspread & XMMIDP_DPCD_MAX_DOWNSPREAD_MASK)
					      >> XMMIDP_DPCD_MAX_DOWNSPREAD_SHIFT;

}

/******************************************************************************/
/**
 * This function reads Rx DPCD revision number
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GetDpcdRev(XMmiDp *InstancePtr)
{
	u32 DpcdRev = 0;

	XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_REV, 1, &DpcdRev);

	InstancePtr->RxConfig.DpcdRev = (DpcdRev & XMMIDP_DPCD_REV_MAJOR_NUM_MASK)
					>> XMMIDP_DPCD_REV_MAJOR_NUM_SHIFT;

}

/******************************************************************************/
/**
 * This function reads Rx DPCD capability registers to initiate link training
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *              - XST_SUCCESS if the training process succeeded.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetRxCapabilities(XMmiDp *InstancePtr)
{
	u8 MaxLinkBW = 0x0;

	XMmiDp_GetDpcdRev(InstancePtr);

	XMmiDp_GetRxMaxLinkRate(InstancePtr);

	XMmiDp_GetRxMaxLaneCount(InstancePtr);

	XMmiDp_GetDpcdMaxDownspread(InstancePtr);

	XMmiDp_GetDpcdTrainingAuxRdInterval(InstancePtr);

	if (InstancePtr->RxConfig.ExtendedReceiverCap) {
		XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_MAX_LINK_RATE_EXTENDED, 1, &MaxLinkBW);

		InstancePtr->RxConfig.MaxLinkBW = (InstancePtr->RxConfig.MaxLinkBW <= MaxLinkBW)
						  ? MaxLinkBW : InstancePtr->RxConfig.MaxLinkBW;
		InstancePtr->RxConfig.MaxLinkRate =
			GetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkBW);
	}

	InstancePtr->LinkConfig.CrDoneCnt = InstancePtr->RxConfig.MaxLaneCount;
	InstancePtr->LinkConfig.CrDoneOldState = InstancePtr->RxConfig.MaxLaneCount;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the channel equalization sequence during link training was
 * successful - the RX device has achieved channel equalization, symbol lock,
 * and interlane alignment for all lanes currently in use.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the RX device has achieved channel
 *                equalization symbol lock, and interlane alignment for all
 *                lanes in use.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_CheckChannelEqualization(XMmiDp *InstancePtr, u8 LaneCount)
{

	u8 InterlaneAlign = 0;

	switch (LaneCount) {
		case PHY_LANES_4:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE3_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE3_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE2_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE2_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

		case PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

		case PHY_LANES_1:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

		default:
			break;

	}

	switch (LaneCount) {
		case PHY_LANES_4:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE3_SYMBOL_LOCKED_MASK) >>
			      XMMIDP_DPCD_LANE3_SYMBOL_LOCKED_SHIFT)) {
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE2_SYMBOL_LOCKED_MASK) >>
			      XMMIDP_DPCD_LANE2_SYMBOL_LOCKED_SHIFT)) {
				return XST_FAILURE;
			}

		case PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_SYMBOL_LOCKED_MASK) >>
			      XMMIDP_DPCD_LANE1_SYMBOL_LOCKED_SHIFT)) {
				return XST_FAILURE;
			}

		case PHY_LANES_1:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_SYMBOL_LOCKED_MASK) >>
			      XMMIDP_DPCD_LANE0_SYMBOL_LOCKED_SHIFT)) {
				return XST_FAILURE;
			}

		default:
			break;

	}

	InterlaneAlign = (InstancePtr->RxConfig.LaneStatusAdjReqs[2]
			  & XMMIDP_DPCD_INTERLANE_ALIGN_DONE_MASK)
			 >> XMMIDP_DPCD_INTERLANE_ALIGN_DONE_SHIFT;
	if (!(InterlaneAlign)) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function runs the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *      0) Start signaling with the same drive settings used at the end of the
 *         clock recovery sequence.
 *      1) Transmit training pattern 2 (or 3) over the main link with symbol
 *         scrambling disabled.
 *      2) The channel equalization loop. If channel equalization is
 *         unsuccessful after 5 loop iterations, return.
 *      2a) Wait for at least the period of time specified in the RX device's
 *          DisplayPort Configuration Data (DPCD) register,
 *          TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved channel equalization, symbol lock,
 *          and interlane alignment. If so, return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the RX device.
 *      2e) Loop back to 2a.
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return      The next training state:
 *              - XDP_TX_TS_SUCCESS if training succeeded.
 *              - XDP_TX_TS_FAILURE if writing the drive settings to the RX
 *                device was unsuccessful.
 *              - XDP_TX_TS_ADJUST_LINK_RATE if, after 5 loop iterations, the
 *                channel equalization sequence did not complete successfully.
 *
 * @note        None.
 *
*******************************************************************************/
XMmiDp_TrainingState XMmiDp_TrainingStateChannelEqualization(XMmiDp *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u32 DelayUs;
	u32 LoopCount = 0;
	u8 CrFailed = 0;
	u8 CeFailed = 0;

	DelayUs = XMmiDp_GetTrainingDelay(InstancePtr);

	if (InstancePtr->RxConfig.Tps4Supported) {
		Status =  XMmiDp_SetTrainingPattern(InstancePtr, PHY_TPS4);
	} else if (InstancePtr->RxConfig.Tps3Supported) {
		Status = XMmiDp_SetTrainingPattern(InstancePtr, PHY_TPS3);
	} else {
		Status = XMmiDp_SetTrainingPattern(InstancePtr, PHY_TPS2);
	}

	if (Status != XST_SUCCESS) {
		return XMMIDP_TS_FAILURE;
	}

	while (LoopCount < 5) {
		XMmiDp_WaitUs(InstancePtr, DelayUs);

		Status = XMmiDp_GetDpcdLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XMMIDP_TS_FAILURE;
		}

		Status = XMmiDp_CheckClockRecovery(InstancePtr,
						   InstancePtr->LinkConfig.LaneCount);
		if (Status != XST_SUCCESS) {
			CrFailed = 1;

		}

		Status = XMmiDp_CheckChannelEqualization(InstancePtr,
			 InstancePtr->LinkConfig.LaneCount);

		if (Status == XST_SUCCESS) {
			CeFailed = 0;
			return XMMIDP_TS_SUCCESS;

		} else {
			CeFailed = 1;
		}

		Status = XMmiDp_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XMMIDP_TS_FAILURE;

		}

		LoopCount++;
	}

	if (CrFailed) {
		InstancePtr->LinkConfig.CrDoneOldState = InstancePtr->RxConfig.MaxLaneCount;
		return XMMIDP_TS_ADJUST_LINK_RATE;
	} else if (InstancePtr->LinkConfig.LaneCount == 1 && (CeFailed)) {
		InstancePtr->LinkConfig.LaneCount =
			InstancePtr->RxConfig.MaxLaneCount;
		InstancePtr->LinkConfig.NumLanes =
			GetNumLanes(InstancePtr, InstancePtr->LinkConfig.LaneCount);

		InstancePtr->LinkConfig.CrDoneOldState =
			InstancePtr->RxConfig.MaxLaneCount;
		return XMMIDP_TS_ADJUST_LINK_RATE;
	} else if (InstancePtr->LinkConfig.LaneCount > 1 && (CeFailed)) {
		return XMMIDP_TS_ADJUST_LANE_COUNT;
	} else {
		InstancePtr->LinkConfig.CrDoneOldState =
			InstancePtr->RxConfig.MaxLaneCount;

		return XMMIDP_TS_ADJUST_LINK_RATE;
	}

}

/******************************************************************************/
/**
 * This function checks if the Rx DisplayPort Configuration Data (DPCD)
 * indicates the receiver has achieved and maintained clock recovery, channel
 * equalization, symbol lock, and interlane alignment for all lanes currently in
 * use.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the RX device has maintained clock recovery,
 *                channel equalization, symbol lock, and interlane alignment.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_CheckLinkStatus(XMmiDp *InstancePtr, u8 LaneCount)
{
	u32 Status;
	u8 RetryCount = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (!XMmiDp_IsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Retrieve AUX info. */
	do {
		/* Get lane and adjustment requests. */
		Status = XMmiDp_GetDpcdLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XST_FAILURE;
		}

		/* Check if the link needs training. */
		if ((XMmiDp_CheckClockRecovery(
			     InstancePtr, LaneCount) == XST_SUCCESS) &&
		    (XMmiDp_CheckChannelEqualization(
			     InstancePtr, LaneCount) == XST_SUCCESS)) {
			return XST_SUCCESS;
		}

		RetryCount++;
	} while (RetryCount < 5); /* Retry up to 5 times. */

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * This function runs the link training process. It is implemented as a state
 * machine, with each state returning the next state. First, the clock recovery
 * sequence will be run; if successful, the channel equalization sequence will
 * run. If either the clock recovery or channel equalization sequence failed,
 * the link rate or the number of lanes used will be reduced and training will
 * be re-attempted. If training fails at the minimal data rate, 1.62 Gbps with
 * a single lane, training will no longer re-attempt and fail.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *              - XST_SUCCESS if the training process succeeded.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_RunTraining(XMmiDp *InstancePtr)
{

	u32 Status = 0;
	XMmiDp_TrainingState TrainingState = XMMIDP_TS_CLOCK_RECOVERY;

	while (1) {
		switch (TrainingState) {
			case XMMIDP_TS_CLOCK_RECOVERY:
				TrainingState =
					XMmiDp_TrainingStateClockRecovery(InstancePtr);
				break;
			case XMMIDP_TS_ADJUST_LINK_RATE:
				TrainingState =
					XMmiDp_TrainingStateAdjustLinkRate(InstancePtr);
				break;
			case XMMIDP_TS_ADJUST_LANE_COUNT:
				TrainingState =
					XMmiDp_TrainingStateAdjustLaneCount(InstancePtr);
				break;
			case XMMIDP_TS_CHANNEL_EQUALIZATION:
				TrainingState =
					XMmiDp_TrainingStateChannelEqualization(InstancePtr);
			default:
				break;
		}

		if (TrainingState == XMMIDP_TS_SUCCESS) {
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->RxConfig.MaxLaneCount;
			InstancePtr->LinkConfig.CrDoneCnt =
				InstancePtr->RxConfig.MaxLaneCount;
			break;

		} else if (TrainingState == XMMIDP_TS_FAILURE) {
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->RxConfig.MaxLaneCount;
			InstancePtr->LinkConfig.CrDoneCnt =
				InstancePtr->RxConfig.MaxLaneCount;

			return XST_FAILURE;
		}
		if ((TrainingState == XMMIDP_TS_ADJUST_LINK_RATE) ||
		    (TrainingState == XMMIDP_TS_ADJUST_LANE_COUNT)) {
			Status = XMmiDp_SetTrainingPattern(InstancePtr, PHY_NO_TRAIN);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}

	}

	/* Post LT ADJ */
	u32 Data = 0;
	Status = XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_LANE_COUNT_SET, 1, &Data);
	Data |= 0x20;
	XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_LANE_COUNT_SET, 1, &Data);

	Status = XMmiDp_CheckLinkStatus(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
/** @} */

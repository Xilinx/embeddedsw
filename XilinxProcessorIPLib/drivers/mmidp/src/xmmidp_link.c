/*******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#define XMMIDP_MAX_VSLEVEL_CNT 5
#define XMMIDP_LINK_CR_DONE_AUX_RD_INTERVAL 100
#define XMMIDP_TRAINING_AUX_RD_BASE_DELAY_US 400
#define XMMIDP_TRAINING_AUX_RD_DELAY_MULT 10
#define XMMIDP_LANE_STATUS_ADJ_REQS_SIZE 6
#define XMMIDP_EQ_MAX_ITERATIONS 5
#define XMMIDP_LINK_STATUS_MAX_RETRIES 5
#define XMMIDP_TRAINING_PATTERN_SET_SIZE (XMMIDP_MAX_NUM_LANES + 1)

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

	InstancePtr->LinkConfig.FastLinkTrainEn = XMMIDP_FAST_LINK_ENABLE;

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

	InstancePtr->LinkConfig.FastLinkTrainEn = XMMIDP_FAST_LINK_DISABLE;

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
	u32 DpcdVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_MAX_LANE_COUNT,
		       1, &DpcdVal);

	InstancePtr->RxConfig.MaxNumLanes =
		(DpcdVal & XMMIDP_DPCD_MAX_LANE_COUNT_MASK)
		>> XMMIDP_DPCD_MAX_LANE_COUNT_SHIFT;

	InstancePtr->RxConfig.MaxLaneCount =
		XMmiDp_GetLaneCount(InstancePtr, InstancePtr->RxConfig.MaxNumLanes);

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
	u32 Status;
	u32 MaxLinkBW = 0;

	Status = XMmiDp_AuxRead(InstancePtr,
				XMMIDP_DPCD_MAX_LINK_RATE, 1, &MaxLinkBW);
	if ( Status != XST_SUCCESS ) {
		xil_printf("AuxRead Err: %d\r\n", Status);
		return Status;
	}

	InstancePtr->RxConfig.MaxLinkBW = MaxLinkBW;
	InstancePtr->RxConfig.MaxLinkRate =
		XMmiDp_GetLinkRate(InstancePtr, MaxLinkBW);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function reads MST_CAP  from Rx DPCD register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetRxMstModeCap(XMmiDp *InstancePtr)
{
	u32 Status;
	u32 MstCap = 0;

	Status = XMmiDp_AuxRead(InstancePtr,
				XMMIDP_DPCD_MSTM_CAP, 1, &MstCap);

	if ( Status != XST_SUCCESS ) {
		xil_printf("AuxRead Err: %d\r\n", Status);
		return Status;
	}

	InstancePtr->RxConfig.MstCap = (MstCap & XMMIDP_DPCD_MSTM_CAP_MST_CAP_MASK);

	return XST_SUCCESS;
}
/******************************************************************************/
/**
 * This function initiates link training sequence with the Rx
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
	XMmiDp_SetPhyLaneCount(InstancePtr,
			       InstancePtr->RxConfig.MaxLaneCount);

	XMmiDp_SetPhyLinkRate(InstancePtr,
			      InstancePtr->RxConfig.MaxLinkRate);

	XMmiDp_SetPhyVoltageSwing(InstancePtr, InstancePtr->LinkConfig.VsLevel);
	XMmiDp_SetPhyPreEmphasis(InstancePtr, InstancePtr->LinkConfig.PeLevel);

	XMmiDp_SetPhyTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);
	XMmiDp_SetDpcdTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);

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
	u8 MstmCtrl = (XMMIDP_DPCD_UPSTREAM_IS_SRC_MASK |
		       XMMIDP_DPCD_UP_REQ_EN_MASK) |
		      InstancePtr->RxConfig.MstCap;

	/* Link BW Set */
	XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_LINK_BW_SET,
			0x1, &MaxLinkBW);

	/* Lane Count Set */
	XMmiDp_DpcdReadModifyWrite(InstancePtr,
				   XMMIDP_DPCD_LANE_COUNT_SET,
				   XMMIDP_DPCD_LANE_COUNT_SET_MASK,
				   XMMIDP_DPCD_LANE_COUNT_SET_SHIFT,
				   InstancePtr->RxConfig.MaxNumLanes);

	/* Enhanced Frame Enable -- set bit 7 of DPCD_LANE_COUNT_SET */
	if (InstancePtr->RxConfig.EnhancedFrameCap) {
		XMmiDp_DpcdReadModifyWrite(InstancePtr,
					   XMMIDP_DPCD_LANE_COUNT_SET,
					   XMMIDP_DPCD_ENHANCED_FRAME_EN_MASK,
					   XMMIDP_DPCD_ENHANCED_FRAME_EN_SHIFT,
					   0x1);
	}

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

	/* MultiStream Mode support*/
	XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_MSTM_CTRL, 0x1, &MstmCtrl);

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
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_SHIFT,
				  XMMIDP_ENHANCE_FRAMING_ENABLE);

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
				  XMMIDP_CCTL0_ENHANCE_FRAMING_EN_SHIFT,
				  XMMIDP_ENHANCE_FRAMING_DISABLE);

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
 * This function returns the training aux interval. The lowest time is 400us.
 * All other WaitTimes are direct multiples of lowest time and value read from
 * DPCD register.
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

	u32 WaitTime = XMMIDP_TRAINING_AUX_RD_BASE_DELAY_US;

	if (InstancePtr->RxConfig.TrainingAuxRdInterval) {
		return (WaitTime * InstancePtr->RxConfig.TrainingAuxRdInterval *
			XMMIDP_TRAINING_AUX_RD_DELAY_MULT);
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
				XMMIDP_LANE_STATUS_ADJ_REQS_SIZE,
				InstancePtr->RxConfig.LaneStatusAdjReqs);

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
		case XMMIDP_PHY_LANES_4:

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_0_CR_DONE;
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_1_CR_DONE;
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE2_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE2_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_2_CR_DONE;
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[1] &
			       XMMIDP_DPCD_LANE3_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE3_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_3_CR_DONE;
				return XST_FAILURE;
			}

		InstancePtr->LinkConfig.CrDoneCnt =
			XMMIDP_LANE_ALL_CR_DONE;
		/* Fall through */

	case XMMIDP_PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_0_CR_DONE;
				return XST_FAILURE;
			}

			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_1_CR_DONE;
				return XST_FAILURE;
			}

		InstancePtr->LinkConfig.CrDoneCnt =
			XMMIDP_LANE_2_CR_DONE;
		/* Fall through */

	case XMMIDP_PHY_LANES_1:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CR_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CR_DONE_SHIFT)) {
				InstancePtr->LinkConfig.CrDoneCnt =
					XMMIDP_LANE_0_CR_DONE;
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
	u8 VsLevelAdjReq[XMMIDP_MAX_NUM_LANES];
	u8 PeLevelAdjReq[XMMIDP_MAX_NUM_LANES];
	u8 AuxData[XMMIDP_MAX_NUM_LANES];

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

	/* Change the drive settings to match the adjustment requests. */
	for (Index = 0; Index < InstancePtr->LinkConfig.NumLanes;
	     Index++) {
		if (VsLevelAdjReq[Index] != InstancePtr->LinkConfig.VsLevel[Index]) {
			InstancePtr->LinkConfig.VsLevel[Index] =
				VsLevelAdjReq[Index];

			InstancePtr->LinkConfig.VsLevelUpdated = TRUE;
		}

		if (PeLevelAdjReq[Index] != InstancePtr->LinkConfig.PeLevel[Index]) {
			InstancePtr->LinkConfig.PeLevel[Index] =
				PeLevelAdjReq[Index];
			InstancePtr->LinkConfig.PeLevelUpdated = TRUE;
		}

	}

	memset(AuxData, 0, XMMIDP_MAX_NUM_LANES);
	XMmiDp_SetVswingPreemp(InstancePtr, AuxData);

	/* Configure DPCD TRAINING LANE SET */
	Status = XMmiDp_AuxWrite(InstancePtr,
				 XMMIDP_DPCD_TRAINING_LANE0_SET,
				 InstancePtr->LinkConfig.NumLanes, AuxData);

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
	u32 Index;

	for (Index = 0; Index < InstancePtr->LinkConfig.NumLanes; Index++) {

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
	u8 AuxData[XMMIDP_TRAINING_PATTERN_SET_SIZE];
	u8 Val = Pattern;

	memset(AuxData, 0, XMMIDP_TRAINING_PATTERN_SET_SIZE);

	InstancePtr->LinkConfig.ScrambleEn = XMMIDP_SCRAMBLE_ENABLE;

	XMmiDp_SetPhyTrainingPattern(InstancePtr, Pattern);

	if (Pattern == XMMIDP_PHY_TPS4 || Pattern == XMMIDP_PHY_NO_TRAIN) {
		if (Pattern == XMMIDP_PHY_TPS4)
			Val = XMMIDP_DPCD_TPS4_PATTERN_SELECT;
		XMmiDp_PhyScrambleEnable(InstancePtr);
	} else {
		InstancePtr->LinkConfig.ScrambleEn = XMMIDP_SCRAMBLE_DISABLE;
		XMmiDp_PhyScrambleDisable(InstancePtr);
	}

	AuxData[0] |= Val << XMMIDP_DPCD_TRAINING_PATTERN_SELECT_SHIFT;
	AuxData[0] |= (InstancePtr->LinkConfig.ScrambleEn) <<
		      XMMIDP_DPCD_SCRAMBLING_DISABLE_SHIFT;

	XMmiDp_SetVswingPreemp(InstancePtr, &AuxData[1]);

	if (Pattern == XMMIDP_PHY_NO_TRAIN)
		Status = XMmiDp_AuxWrite(InstancePtr,
					 XMMIDP_DPCD_TRAINING_PATTERN_SET, 1, AuxData);
	else
		Status = XMmiDp_AuxWrite(InstancePtr,
					 XMMIDP_DPCD_TRAINING_PATTERN_SET,
					 XMMIDP_TRAINING_PATTERN_SET_SIZE,
					 AuxData);

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

	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_POWER_DOWN);
	Status = XMmiDp_PhyWaitReady(InstancePtr);
	if ( Status != XST_SUCCESS ) {
		xil_printf("Phy Busy Timeout %d\r\n", Status);
		return Status;
	}

	InstancePtr->LinkConfig.LinkRate = LinkRate;
	InstancePtr->LinkConfig.LinkBW = XMmiDp_GetLinkBW(InstancePtr, LinkRate);

	XMmiDp_SetPhyLinkRate(InstancePtr, LinkRate);
	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_POWER_ON);

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
	InstancePtr->LinkConfig.NumLanes = XMmiDp_GetNumLanes(InstancePtr, LaneCount);

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
	u8 i;
	u32 DelayUs;

	DelayUs = XMmiDp_GetTrainingDelay(InstancePtr);

	xil_printf("[CR] Start: Lanes=%d BW=0x%02X Rate=%d Delay=%d us\r\n",
		   InstancePtr->LinkConfig.NumLanes,
		   InstancePtr->LinkConfig.LinkBW,
		   InstancePtr->LinkConfig.LinkRate,
		   DelayUs);

	memset(InstancePtr->LinkConfig.VsLevel, 0, XMMIDP_MAX_NUM_LANES);
	memset(InstancePtr->LinkConfig.PeLevel, 0, XMMIDP_MAX_NUM_LANES);

	Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_TPS1);

	if ( Status != XST_SUCCESS ) {
		xil_printf("[CR] FAIL: SetTrainingPattern TPS1 failed\r\n");
		return XMMIDP_TS_FAILURE;
	}

	while (TimeOutCnt <= XMMIDP_CR_TIMEOUT_COUNT) {
		XMmiDp_WaitUs(InstancePtr, DelayUs);

		Status = XMmiDp_GetDpcdLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("[CR] FAIL: AUX read lane status failed (iter %d)\r\n",
				   TimeOutCnt);
			return XMMIDP_TS_FAILURE;
		}

		xil_printf("[CR] iter=%d DPCD 0x202-0x207:", TimeOutCnt);
		for (i = 0; i < 6; i++) {
			xil_printf(" %02X", InstancePtr->RxConfig.LaneStatusAdjReqs[i]);
		}
		for (i = 0; i < InstancePtr->LinkConfig.NumLanes; i++)
			xil_printf(" L%d:VS=%d/PE=%d", i,
				   InstancePtr->LinkConfig.VsLevel[i],
				   InstancePtr->LinkConfig.PeLevel[i]);
		xil_printf("\r\n");

		Status = XMmiDp_CheckClockRecovery(InstancePtr,
						   InstancePtr->LinkConfig.LaneCount);

		if (Status == XST_SUCCESS) {
			xil_printf("[CR] PASS -> Channel EQ\r\n");
			return XMMIDP_TS_CHANNEL_EQUALIZATION;
		}

		if (InstancePtr->LinkConfig.VsLevelUpdated ||
		    InstancePtr->LinkConfig.PeLevelUpdated) {
			SameVsLevelCnt = 0;

		} else {
			SameVsLevelCnt++;
		}

		if (SameVsLevelCnt >= XMMIDP_MAX_VSLEVEL_CNT) {
			xil_printf("[CR] FAIL: SameVsLevel count reached max (%d)\r\n",
				   XMMIDP_MAX_VSLEVEL_CNT);
			break;
		}

		if (InstancePtr->LinkConfig.VsLevel[0] == XMMIDP_MAX_VS_LEVEL) {
			xil_printf("[CR] FAIL: Max VS level reached (%d)\r\n",
				   XMMIDP_MAX_VS_LEVEL);
			break;
		}

		Status = XMmiDp_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("[CR] FAIL: AdjVswingPreemp failed\r\n");
			return XMMIDP_TS_FAILURE;
		}

		TimeOutCnt++;
	}

	if (TimeOutCnt > XMMIDP_CR_TIMEOUT_COUNT)
		xil_printf("[CR] FAIL: Timeout (%d iters)\r\n", TimeOutCnt);

	if (InstancePtr->LinkConfig.LinkBW == XMMIDP_DPCD_LINK_BW_SET_162GBPS) {
		if ((InstancePtr->LinkConfig.CrDoneCnt != XMMIDP_LANE_ALL_CR_DONE) &&
		    (InstancePtr->LinkConfig.CrDoneCnt != XMMIDP_LANE_0_CR_DONE)) {
			xil_printf("[CR] Partial CR at RBR -- retrying with CrDoneCnt=%d\r\n",
				   InstancePtr->LinkConfig.CrDoneCnt);
			Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);
			XMmiDp_SetLinkRate(InstancePtr,
					   InstancePtr->RxConfig.MaxLinkRate);
			XMmiDp_SetLaneCount(InstancePtr,
					    InstancePtr->LinkConfig.CrDoneCnt);
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->LinkConfig.CrDoneCnt;
			return XMMIDP_TS_CLOCK_RECOVERY;
		}

	}

	xil_printf("[CR] -> Adjust Link Rate\r\n");
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

	xil_printf("[ADJ-RATE] Current LinkBW=0x%02X\r\n",
		   InstancePtr->LinkConfig.LinkBW);

	switch (InstancePtr->LinkConfig.LinkBW) {
		case XMMIDP_DPCD_LINK_BW_SET_810GBPS:
			xil_printf("[ADJ-RATE] HBR3 -> HBR2\r\n");
			XMmiDp_SetLinkRate(InstancePtr,
					   XMMIDP_PHY_RATE_HBR2_540GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		case XMMIDP_DPCD_LINK_BW_SET_540GBPS:
			xil_printf("[ADJ-RATE] HBR2 -> HBR\r\n");
			XMmiDp_SetLinkRate(InstancePtr,
					   XMMIDP_PHY_RATE_HBR_270GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		case XMMIDP_DPCD_LINK_BW_SET_270GBPS:
			xil_printf("[ADJ-RATE] HBR -> RBR\r\n");
			XMmiDp_SetLinkRate(InstancePtr,
					   XMMIDP_PHY_RATE_RBR_162GBPS);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;
		default:
			xil_printf("[ADJ-RATE] Already at min rate -> Adjust Lane Count\r\n");
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

	xil_printf("[ADJ-LANE] Current LaneCount=%d (reg=%d)\r\n",
		   InstancePtr->LinkConfig.NumLanes,
		   InstancePtr->LinkConfig.LaneCount);

	/* If Lane 0 never achieved CR, no point reducing lanes */
	if (!(InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
	      XMMIDP_DPCD_LANE0_CR_DONE_MASK)) {
		xil_printf("[ADJ-LANE] Lane 0 CR not done -> FAILURE\r\n");
		return XMMIDP_TS_FAILURE;
	}

	switch (InstancePtr->LinkConfig.LaneCount) {
		case XMMIDP_PHY_LANES_4:
			xil_printf("[ADJ-LANE] 4 lanes -> 2 lanes (reset to max rate %d)\r\n",
				   InstancePtr->RxConfig.MaxLinkRate);
			XMmiDp_SetLaneCount(InstancePtr, XMMIDP_PHY_LANES_2);
			XMmiDp_SetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkRate);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;

		case XMMIDP_PHY_LANES_2:
			xil_printf("[ADJ-LANE] 2 lanes -> 1 lane (reset to max rate %d)\r\n",
				   InstancePtr->RxConfig.MaxLinkRate);
			XMmiDp_SetLaneCount(InstancePtr, XMMIDP_PHY_LANES_1);
			XMmiDp_SetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkRate);
			Status = XMMIDP_TS_CLOCK_RECOVERY;
			break;

		default:
			xil_printf("[ADJ-LANE] Already at 1 lane -> FAILURE\r\n");
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

	InstancePtr->RxConfig.DpcdRev = (u8)(DpcdRev &
					(XMMIDP_DPCD_REV_MAJOR_NUM_MASK |
					 XMMIDP_DPCD_REV_MINOR_NUM_MASK));

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

	XMmiDp_GetRxMstModeCap(InstancePtr);

	if (InstancePtr->RxConfig.ExtendedReceiverCap) {
		XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_MAX_LINK_RATE_EXTENDED, 1, &MaxLinkBW);

		InstancePtr->RxConfig.MaxLinkBW = (InstancePtr->RxConfig.MaxLinkBW <= MaxLinkBW)
						  ? MaxLinkBW : InstancePtr->RxConfig.MaxLinkBW;
		InstancePtr->RxConfig.MaxLinkRate =
			XMmiDp_GetLinkRate(InstancePtr, InstancePtr->RxConfig.MaxLinkBW);
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
		case XMMIDP_PHY_LANES_4:
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

		/* fall through */

		case XMMIDP_PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE1_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

		/* fall through */

		case XMMIDP_PHY_LANES_1:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE0_CHANNEL_EQ_DONE_MASK) >>
			      XMMIDP_DPCD_LANE0_CHANNEL_EQ_DONE_SHIFT)) {
				return XST_FAILURE;
			}

		default:
			break;

	}

	switch (LaneCount) {
		case XMMIDP_PHY_LANES_4:
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

		/* fall through */

		case XMMIDP_PHY_LANES_2:
			if (!((InstancePtr->RxConfig.LaneStatusAdjReqs[0] &
			       XMMIDP_DPCD_LANE1_SYMBOL_LOCKED_MASK) >>
			      XMMIDP_DPCD_LANE1_SYMBOL_LOCKED_SHIFT)) {
				return XST_FAILURE;
			}

		/* fall through */

		case XMMIDP_PHY_LANES_1:
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
 * section 3.5.1.2.2 of the DisplayPort 1.4a specification document.
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
	u8 TpsUsed;

	DelayUs = XMmiDp_GetTrainingDelay(InstancePtr);

	if (InstancePtr->RxConfig.Tps4Supported &&
	    InstancePtr->LinkConfig.LinkBW == XMMIDP_DPCD_LINK_BW_SET_810GBPS) {
		TpsUsed = 4;
		Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_TPS4);
	} else if (InstancePtr->RxConfig.Tps3Supported &&
		   (InstancePtr->LinkConfig.LinkBW == XMMIDP_DPCD_LINK_BW_SET_810GBPS ||
		    InstancePtr->LinkConfig.LinkBW == XMMIDP_DPCD_LINK_BW_SET_540GBPS)) {
		TpsUsed = 3;
		Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_TPS3);
	} else {
		TpsUsed = 2;
		Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_TPS2);
	}

	xil_printf("[EQ] Start: TPS%d Delay=%d us\r\n", TpsUsed, DelayUs);

	if (Status != XST_SUCCESS) {
		xil_printf("[EQ] FAIL: SetTrainingPattern TPS%d failed\r\n", TpsUsed);
		return XMMIDP_TS_FAILURE;
	}

	while (LoopCount < XMMIDP_EQ_MAX_ITERATIONS) {
		XMmiDp_WaitUs(InstancePtr, DelayUs);

		Status = XMmiDp_GetDpcdLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("[EQ] FAIL: AUX read lane status failed (iter %d)\r\n",
				   LoopCount);
			return XMMIDP_TS_FAILURE;
		}

		xil_printf("[EQ] iter=%d DPCD: %02X %02X %02X %02X %02X %02X\r\n",
			   LoopCount,
			   InstancePtr->RxConfig.LaneStatusAdjReqs[0],
			   InstancePtr->RxConfig.LaneStatusAdjReqs[1],
			   InstancePtr->RxConfig.LaneStatusAdjReqs[2],
			   InstancePtr->RxConfig.LaneStatusAdjReqs[3],
			   InstancePtr->RxConfig.LaneStatusAdjReqs[4],
			   InstancePtr->RxConfig.LaneStatusAdjReqs[5]);

		Status = XMmiDp_CheckClockRecovery(InstancePtr,
						   InstancePtr->LinkConfig.LaneCount);
		if (Status != XST_SUCCESS) {
			xil_printf("[EQ] iter=%d CR lost!\r\n", LoopCount);
			CrFailed = 1;
			break;
		}

		Status = XMmiDp_CheckChannelEqualization(InstancePtr,
			 InstancePtr->LinkConfig.LaneCount);

		if (Status == XST_SUCCESS) {
			CeFailed = 0;
			xil_printf("[EQ] PASS -> SUCCESS\r\n");
			return XMMIDP_TS_SUCCESS;
		} else {
			xil_printf("[EQ] iter=%d EQ not done\r\n", LoopCount);
			CeFailed = 1;
		}

		Status = XMmiDp_AdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("[EQ] FAIL: AdjVswingPreemp failed\r\n");
			return XMMIDP_TS_FAILURE;
		}

		LoopCount++;
	}

	xil_printf("[EQ] Exhausted 5 attempts: CrFailed=%d CeFailed=%d LaneCount=%d\r\n",
		   CrFailed, CeFailed, InstancePtr->LinkConfig.LaneCount);

	if (CrFailed) {
		xil_printf("[EQ] CR failed during EQ -> Adjust Link Rate\r\n");
		InstancePtr->LinkConfig.CrDoneOldState = InstancePtr->RxConfig.MaxLaneCount;
		return XMMIDP_TS_ADJUST_LINK_RATE;
	} else if (InstancePtr->LinkConfig.LaneCount == 1 && (CeFailed)) {
		xil_printf("[EQ] EQ failed at 1 lane -> Adjust Link Rate (reset lanes)\r\n");
		InstancePtr->LinkConfig.LaneCount =
			InstancePtr->RxConfig.MaxLaneCount;
		InstancePtr->LinkConfig.NumLanes =
			XMmiDp_GetNumLanes(InstancePtr, InstancePtr->LinkConfig.LaneCount);

		InstancePtr->LinkConfig.CrDoneOldState =
			InstancePtr->RxConfig.MaxLaneCount;
		return XMMIDP_TS_ADJUST_LINK_RATE;
	} else if (InstancePtr->LinkConfig.LaneCount > 1 && (CeFailed)) {
		xil_printf("[EQ] EQ failed at %d lanes -> Adjust Lane Count\r\n",
			   InstancePtr->LinkConfig.NumLanes);
		return XMMIDP_TS_ADJUST_LANE_COUNT;
	} else {
		xil_printf("[EQ] Unknown state -> Adjust Link Rate\r\n");
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
 * use.The DP 1.4 spec specifies checking for the above bits max 5 times before
 * retraining link at lower link rate.
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

		/* Always verify actual CR and CE lane status bits.
		 * LINK_STATUS_UPDATED only indicates change since last read,
		 * not whether the link is actually trained. */
		if ((XMmiDp_CheckClockRecovery(
			     InstancePtr, LaneCount) == XST_SUCCESS) &&
		    (XMmiDp_CheckChannelEqualization(
			     InstancePtr, LaneCount) == XST_SUCCESS)) {
			return XST_SUCCESS;
		}

		RetryCount++;
	} while (RetryCount < XMMIDP_LINK_STATUS_MAX_RETRIES);

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

	u32 Status;
	u32 Data;
	u32 Round = 0;

	XMmiDp_TrainingState TrainingState = XMMIDP_TS_CLOCK_RECOVERY;

	xil_printf("[TRAIN] Starting training state machine\r\n");

	while (1) {
		Round++;
		switch (TrainingState) {
			case XMMIDP_TS_CLOCK_RECOVERY:
				xil_printf("[TRAIN] Round %d: CLOCK_RECOVERY\r\n", Round);
				TrainingState =
					XMmiDp_TrainingStateClockRecovery(InstancePtr);
				break;
			case XMMIDP_TS_ADJUST_LINK_RATE:
				xil_printf("[TRAIN] Round %d: ADJUST_LINK_RATE\r\n", Round);
				TrainingState =
					XMmiDp_TrainingStateAdjustLinkRate(InstancePtr);
				break;
			case XMMIDP_TS_ADJUST_LANE_COUNT:
				xil_printf("[TRAIN] Round %d: ADJUST_LANE_COUNT\r\n", Round);
				TrainingState =
					XMmiDp_TrainingStateAdjustLaneCount(InstancePtr);
				break;
			case XMMIDP_TS_CHANNEL_EQUALIZATION:
				xil_printf("[TRAIN] Round %d: CHANNEL_EQUALIZATION\r\n", Round);
				TrainingState =
					XMmiDp_TrainingStateChannelEqualization(InstancePtr);
				break;
			default:
				break;
		}

		if (TrainingState == XMMIDP_TS_SUCCESS) {
			xil_printf("[TRAIN] SUCCESS after %d rounds\r\n", Round);
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->RxConfig.MaxLaneCount;
			InstancePtr->LinkConfig.CrDoneCnt =
				InstancePtr->RxConfig.MaxLaneCount;
			break;

		} else if (TrainingState == XMMIDP_TS_FAILURE) {
			xil_printf("[TRAIN] FAILURE after %d rounds\r\n", Round);
			InstancePtr->LinkConfig.CrDoneOldState =
				InstancePtr->RxConfig.MaxLaneCount;
			InstancePtr->LinkConfig.CrDoneCnt =
				InstancePtr->RxConfig.MaxLaneCount;

			return XST_FAILURE;
		}
		if ((TrainingState == XMMIDP_TS_ADJUST_LINK_RATE) ||
		    (TrainingState == XMMIDP_TS_ADJUST_LANE_COUNT)) {
			Status = XMmiDp_SetTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);
			if (Status != XST_SUCCESS) {
				xil_printf("[TRAIN] FAILURE: SetTrainingPattern NO_TRAIN failed\r\n");
				return XST_FAILURE;
			}
		}

	}

	/* Post LT ADJ - only grant if sink supports it */
	if (InstancePtr->RxConfig.PostLtAdjReqSupported) {
		Status = XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_LANE_COUNT_SET,
					1, &Data);
		Data |= XMMIDP_DPCD_POST_LT_ADJ_REQ_GRANTED_MASK;
		XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_LANE_COUNT_SET, 1, &Data);
	}

	Status = XMmiDp_CheckLinkStatus(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
/** @} */

/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_ap.c
* @addtogroup Overview
* @{
*
* Contains the interface functions of the Analogue Path Settings in XRFdc driver.
* Although these features are digital in reality, due to thier position in the
* datapath (being between the crossbar and the converter) they are refered to as
* "analogue path" blocks.
* See xrfdc.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 10.0  cog    11/26/20 Refactor and split files.
*       cog    12/23/20 Fixed issue with IQ QMC on 48dr devices.
*       cog    01/05/21 Signal detector on/off counters needed to be flipped.
*       cog    01/05/21 Second signal detector removed.
*       cog    01/06/21 Added DAC data scaler APIs.
*       cog    01/11/21 Tuning for autocalibration.
* 11.0  cog    05/31/21 Upversion.
*       cog    08/05/21 Fixed issue where VOP initial value was incorrect.
*       cog    08/18/21 Disallow VOP for DC coupled DACs.
* 11.1  cog    11/16/21 Upversion.
*       cog    12/21/21 Read DAC coupling from a register rather than from
*                       the config structure.
*       cog    01/18/22 Refactor connected data components.
*       cog    01/18/22 Added safety checks.
* 12.0  cog    10/26/22 Added API XRFdc_GetCoupling(), this gets the ADC or
*                       DAC coupling.
*       cog    01/07/23 Added VOP support for DC coupled DACs and removed VOP
*                       support for ES1 Parts.
* 13.0  cog    01/15/25 Fixed VOP VCM Drop issue.
*       cog    01/25/25 QMC needs to take the XBar settings into account
*                       for HSADCs.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XRFDC_DAC_LINK_COUPLING_AC 0x0U
#define XRFDC_VOP_ES1_AC_MIN_UA 6435U
#define XRFDC_VOP_ES1_AC_MAX_UA 32000U
#define XRFDC_VOP_AC_MIN_UA 2250U
#define XRFDC_VOP_AC_MAX_UA 40500U
#define XRFDC_VOP_DC_MIN_UA 6400U
#define XRFDC_VOP_DC_MAX_UA 32000U
#define XRFDC_MIN_I_UA_INT 1400U
#define XRFDC_STEP_I_UA 43.75
#define XRFDC_BLDR_GAIN 0x0000U
#define XRFDC_CSCAS_BLDR 0xE000U
#define XRFDC_OPCAS_BIAS 0x001BU
/************************** Function Prototypes ******************************/

/*****************************************************************************/
/*****************************************************************************/
/**
* This API is used to update various QMC settings, eg gain, phase, offset etc.
* QMC settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    QMCSettingsPtr is Pointer to the XRFdc_QMC_Settings structure
*           in which the QMC settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_SetQMCSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_QMC_Settings *QMCSettingsPtr)
{
	u32 Status;
	XRFdc_QMC_Settings *QMCConfigPtr;
	u32 BaseAddr;
	s32 PhaseCorrectionFactor;
	u32 GainCorrectionFactor;
	u32 Index;
	u32 NoOfBlocks;
	u32 Offset;
	u32 MBReg;
	u32 QMCXbarRemote = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(QMCSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Index = Block_Id;

	if (Type == XRFDC_DAC_TILE) {
		MBReg = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id), XRFDC_DAC_MB_CFG_OFFSET,
				    XRFDC_ALT_BOND_MASK);
		if (MBReg == XRFDC_ALT_BOND_MASK) {
			/*Account for internal routing cases*/
			Index = XRFDC_BLK_ID1;
		}
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			QMCXbarRemote = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(Type, Tile_Id, Index), XRFDC_ADC_SWITCH_MATRX_OFFSET, XRFDC_SEL_CB_TO_QMC_MASK);
		}
		if ((InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		    XRFDC_DATA_TYPE_IQ) || (QMCXbarRemote != XRFDC_DISABLED)){
			Index = Block_Id;
			NoOfBlocks = XRFDC_NUM_OF_BLKS3;
			if (Block_Id == XRFDC_BLK_ID1) {
				NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			}
		}
	} else {
		NoOfBlocks = Index + 1U;
	}

	for (; Index < NoOfBlocks;) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			QMCConfigPtr = &InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].QMC_Settings;
		} else {
			QMCConfigPtr = &InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Block_Id].QMC_Settings;
		}

		BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Index);

		if ((QMCSettingsPtr->EnableGain != 0U) && (QMCSettingsPtr->EnableGain != 1U)) {
			metal_log(METAL_LOG_ERROR, "\n Invalid QMC gain option (%u) for %s %u block %u in %s\r\n",
				  QMCSettingsPtr->EnableGain, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
				  Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if ((QMCSettingsPtr->EnablePhase != 0U) && (QMCSettingsPtr->EnablePhase != 1U)) {
			metal_log(METAL_LOG_ERROR, "\n Invalid QMC phase option (%u) for %s %u block %u in %s\r\n",
				  QMCSettingsPtr->EnableGain, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
				  Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if ((QMCSettingsPtr->PhaseCorrectionFactor <= XRFDC_MIN_PHASE_CORR_FACTOR) ||
		    (QMCSettingsPtr->PhaseCorrectionFactor >= XRFDC_MAX_PHASE_CORR_FACTOR)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid QMC Phase Correction factor (%lf) for %s %u block %u in %s\r\n",
				  QMCSettingsPtr->PhaseCorrectionFactor, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
				  Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if ((QMCSettingsPtr->GainCorrectionFactor < XRFDC_MIN_GAIN_CORR_FACTOR) ||
		    (QMCSettingsPtr->GainCorrectionFactor >= XRFDC_MAX_GAIN_CORR_FACTOR)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid QMC Gain Correction factor (%lf) for %s %u block %u in %s\r\n",
				  QMCSettingsPtr->GainCorrectionFactor, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
				  Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		if ((QMCSettingsPtr->EventSource > XRFDC_EVNT_SRC_PL) ||
		    ((QMCSettingsPtr->EventSource == XRFDC_EVNT_SRC_MARKER) && (Type == XRFDC_ADC_TILE))) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid event source selection (%u) for %s %u block %u in %s\r\n",
				  QMCSettingsPtr->EventSource, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
				  Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE) &&
		    ((QMCSettingsPtr->EventSource == XRFDC_EVNT_SRC_SLICE) ||
		     (QMCSettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE))) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Invalid Event Source, event source is not supported in 4GSPS ADC (%u) for ADC %u block %u in %s\r\n",
				QMCSettingsPtr->EventSource, Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET, XRFDC_QMC_CFG_EN_GAIN_MASK,
				QMCSettingsPtr->EnableGain);

		/* Phase Correction factor is applicable to ADC/DAC IQ Pair mode only */
		if (QMCSettingsPtr->EnablePhase == XRFDC_ENABLED) {
			if (((u32)XRFdc_GetConnectedIData(InstancePtr, Type, Tile_Id, Block_Id) == Block_Id) &&
			    (XRFdc_GetConnectedQData(InstancePtr, Type, Tile_Id, Block_Id) != XRFDC_BLK_ID_NONE)) {
				PhaseCorrectionFactor =
					((QMCSettingsPtr->PhaseCorrectionFactor / XRFDC_MAX_PHASE_CORR_FACTOR) *
					 XRFDC_QMC_PHASE_MULT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_PHASE_OFFSET,
						XRFDC_QMC_PHASE_CRCTN_MASK, PhaseCorrectionFactor);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET,
						XRFDC_QMC_CFG_EN_PHASE_MASK,
						(XRFDC_ENABLED << XRFDC_QMC_CFG_PHASE_SHIFT));
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET,
						XRFDC_QMC_CFG_EN_PHASE_MASK, XRFDC_DISABLED);
				metal_log(
					METAL_LOG_WARNING,
					"\n Can't Set QMC phase option (must be I path of IQ pair) for %s %u block %u in %s\r\n",
					(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			}
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET, XRFDC_QMC_CFG_EN_PHASE_MASK,
					XRFDC_DISABLED);
		}

		/* Gain Correction factor */
		GainCorrectionFactor =
			((QMCSettingsPtr->GainCorrectionFactor * XRFDC_QMC_GAIN_MULT) / XRFDC_MAX_GAIN_CORR_FACTOR);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_GAIN_OFFSET, XRFDC_QMC_GAIN_CRCTN_MASK,
				GainCorrectionFactor);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_OFF_OFFSET, XRFDC_QMC_OFFST_CRCTN_MASK,
				QMCSettingsPtr->OffsetCorrectionFactor);

		/* Event Source */
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_QMC_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK,
				QMCSettingsPtr->EventSource);

		if (QMCSettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
			if (Type == XRFDC_ADC_TILE) {
				Offset = XRFDC_ADC_UPDATE_DYN_OFFSET;
			} else {
				Offset = XRFDC_DAC_UPDATE_DYN_OFFSET;
			}
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, Offset, XRFDC_UPDT_EVNT_MASK, XRFDC_UPDT_EVNT_QMC_MASK);
		}
		/* Update the instance with new values */
		QMCConfigPtr->EventSource = QMCSettingsPtr->EventSource;
		QMCConfigPtr->PhaseCorrectionFactor = QMCSettingsPtr->PhaseCorrectionFactor;
		QMCConfigPtr->GainCorrectionFactor = QMCSettingsPtr->GainCorrectionFactor;
		QMCConfigPtr->OffsetCorrectionFactor = QMCSettingsPtr->OffsetCorrectionFactor;
		QMCConfigPtr->EnablePhase = QMCSettingsPtr->EnablePhase;
		QMCConfigPtr->EnableGain = QMCSettingsPtr->EnableGain;
		if ((Type == XRFDC_ADC_TILE) &&
		    (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			Index += XRFDC_BLK_ID2;
		} else {
			Index += XRFDC_BLK_ID1;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* QMC settings are returned back to the caller through this API.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    QMCSettingsPtr Pointer to the XRFdc_QMC_Settings structure
*           in which the QMC settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetQMCSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_QMC_Settings *QMCSettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 MBReg;
	s32 PhaseCorrectionFactor;
	u32 GainCorrectionFactor;
	s32 OffsetCorrectionFactor;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(QMCSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE) &&
	    (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].MixerInputDataType !=
	     XRFDC_DATA_TYPE_IQ)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);
	if (Type == XRFDC_DAC_TILE) {
		MBReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK);
		if (MBReg == XRFDC_ALT_BOND_MASK) {
			/*Account for internal routing cases*/
			BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1);
		}
	}

	QMCSettingsPtr->EnableGain =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET, XRFDC_QMC_CFG_EN_GAIN_MASK);

	/* Phase Correction factor */
	if (((Type == XRFDC_ADC_TILE) &&
	     (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].MixerInputDataType ==
	      XRFDC_DATA_TYPE_IQ)) ||
	    ((Type == XRFDC_DAC_TILE) &&
	     (InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].MixerInputDataType ==
	      XRFDC_DATA_TYPE_IQ))) {
		QMCSettingsPtr->EnablePhase =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET, XRFDC_QMC_CFG_EN_PHASE_MASK) >>
			XRFDC_QMC_CFG_PHASE_SHIFT;
		PhaseCorrectionFactor =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_PHASE_OFFSET, XRFDC_QMC_PHASE_CRCTN_MASK);
		PhaseCorrectionFactor =
			((PhaseCorrectionFactor & XRFDC_QMC_PHASE_CRCTN_SIGN_MASK) != XRFDC_QMC_PHASE_CRCTN_SIGN_MASK) ?
				PhaseCorrectionFactor :
				(s32)((-1 ^ XRFDC_QMC_PHASE_CRCTN_MASK) | PhaseCorrectionFactor);
		QMCSettingsPtr->PhaseCorrectionFactor =
			((PhaseCorrectionFactor * XRFDC_MAX_PHASE_CORR_FACTOR) / XRFDC_QMC_PHASE_MULT);
	} else {
		QMCSettingsPtr->PhaseCorrectionFactor = XRFDC_DISABLED;
		QMCSettingsPtr->EnablePhase = XRFDC_DISABLED;
	}

	/* Gain Correction factor */
	GainCorrectionFactor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_GAIN_OFFSET, XRFDC_QMC_GAIN_CRCTN_MASK);
	QMCSettingsPtr->GainCorrectionFactor =
		((GainCorrectionFactor * XRFDC_MAX_GAIN_CORR_FACTOR) / XRFDC_QMC_GAIN_MULT);
	OffsetCorrectionFactor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_OFF_OFFSET, XRFDC_QMC_OFFST_CRCTN_MASK);
	QMCSettingsPtr->OffsetCorrectionFactor =
		((OffsetCorrectionFactor & XRFDC_QMC_OFFST_CRCTN_SIGN_MASK) != XRFDC_QMC_OFFST_CRCTN_SIGN_MASK) ?
			OffsetCorrectionFactor :
			(s32)((-1 ^ XRFDC_QMC_OFFST_CRCTN_MASK) | OffsetCorrectionFactor);

	/* Event Source */
	QMCSettingsPtr->EventSource =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Coarse delay settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    CoarseDelaySettingsPtr is Pointer to the XRFdc_CoarseDelay_Settings
*           structure in which the CoarseDelay settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_SetCoarseDelaySettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_CoarseDelay_Settings *CoarseDelaySettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	u16 Mask;
	u16 MaxDelay;
	XRFdc_CoarseDelay_Settings *CoarseDelayConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoarseDelaySettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Mask = (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_CRSE_DLY_CFG_MASK : XRFDC_CRSE_DLY_CFG_MASK_EXT;
	MaxDelay = (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_CRSE_DLY_MAX : XRFDC_CRSE_DLY_MAX_EXT;
	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			CoarseDelayConfigPtr =
				&InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].CoarseDelay_Settings;
		} else {
			CoarseDelayConfigPtr =
				&InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Index].CoarseDelay_Settings;
		}

		BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Index);

		if (CoarseDelaySettingsPtr->CoarseDelay > MaxDelay) {
			metal_log(METAL_LOG_ERROR,
				  "\n Requested coarse delay not valid (%u) for %s %u block %u in %s\r\n",
				  CoarseDelaySettingsPtr->CoarseDelay, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
				  Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if ((CoarseDelaySettingsPtr->EventSource > XRFDC_EVNT_SRC_PL) ||
		    ((CoarseDelaySettingsPtr->EventSource == XRFDC_EVNT_SRC_MARKER) && (Type == XRFDC_ADC_TILE))) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid event source selection (%u) for %s %u block %u in %s\r\n",
				  CoarseDelaySettingsPtr->EventSource, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
				  Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE) &&
		    ((CoarseDelaySettingsPtr->EventSource == XRFDC_EVNT_SRC_SLICE) ||
		     (CoarseDelaySettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE))) {
			Status = XRFDC_FAILURE;
			metal_log(
				METAL_LOG_ERROR,
				"\n Invalid Event Source, event source is not supported in 4GSPS ADC (%u) for ADC %u block %u in %s\r\n",
				CoarseDelaySettingsPtr->EventSource, Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
		if (Type == XRFDC_ADC_TILE) {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_CRSE_DLY_CFG_OFFSET, Mask,
					CoarseDelaySettingsPtr->CoarseDelay);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_CRSE_DLY_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK,
					CoarseDelaySettingsPtr->EventSource);
			if (CoarseDelaySettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_UPDATE_DYN_OFFSET,
						XRFDC_UPDT_EVNT_MASK, XRFDC_ADC_UPDT_CRSE_DLY_MASK);
			}
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_CRSE_DLY_CFG_OFFSET, Mask,
					CoarseDelaySettingsPtr->CoarseDelay);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_CRSE_DLY_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK,
					CoarseDelaySettingsPtr->EventSource);
			if (CoarseDelaySettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_UPDATE_DYN_OFFSET,
						XRFDC_UPDT_EVNT_MASK, XRFDC_DAC_UPDT_CRSE_DLY_MASK);
			}
		}
		/* Update the instance with new values */
		CoarseDelayConfigPtr->CoarseDelay = CoarseDelaySettingsPtr->CoarseDelay;
		CoarseDelayConfigPtr->EventSource = CoarseDelaySettingsPtr->EventSource;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Coarse delay settings are returned back to the caller.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    CoarseDelaySettingsPtr Pointer to the XRFdc_CoarseDelay_Settings
*           structure in which the Coarse Delay settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetCoarseDelaySettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_CoarseDelay_Settings *CoarseDelaySettingsPtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoarseDelaySettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);

	if (Type == XRFDC_ADC_TILE) {
		CoarseDelaySettingsPtr->CoarseDelay =
			XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_CRSE_DLY_CFG_OFFSET);
		CoarseDelaySettingsPtr->EventSource =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_CRSE_DLY_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK);
	} else {
		CoarseDelaySettingsPtr->CoarseDelay =
			XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_CRSE_DLY_CFG_OFFSET);
		CoarseDelaySettingsPtr->EventSource =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_CRSE_DLY_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to clear the Sticky bit in threshold config registers.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    ThresholdToUpdate Select which Threshold (Threshold0 or
*           Threshold1 or both) to update.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
u32 XRFdc_ThresholdStickyClear(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 ThresholdToUpdate)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_0) && (ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_1) &&
	    (ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_BOTH)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid ThresholdToUpdate value (%u) for ADC %u block %u in %s\r\n",
			  ThresholdToUpdate, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		    XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = XRFDC_NUM_OF_BLKS3;
			if (Block_Id == XRFDC_BLK_ID1) {
				NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		/* Update for Threshold0 */
		if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_0) ||
		    (ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_CFG_OFFSET, XRFDC_TRSHD0_STIKY_CLR_MASK,
					XRFDC_TRSHD0_STIKY_CLR_MASK);
		}
		/* Update for Threshold1 */
		if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_1) ||
		    (ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_CFG_OFFSET, XRFDC_TRSHD1_STIKY_CLR_MASK,
					XRFDC_TRSHD1_STIKY_CLR_MASK);
		}

		if ((InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			Index += XRFDC_BLK_ID2;
		} else {
			Index += XRFDC_BLK_ID1;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API sets the threshold clear mode. The clear mode can be through
* explicit DRP access (manual) or auto clear (QMC gain update event).
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADCC block number inside the tile. Valid values
*           are 0-3.
* @param    ThresholdToUpdate Select which Threshold (Threshold0 or
*           Threshold1 or both) to update.
* @param    ClrMode can be DRP access (manual) or auto clear (QMC gain
*           update event).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
u32 XRFdc_SetThresholdClrMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 ThresholdToUpdate, u32 ClrMode)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_0) && (ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_1) &&
	    (ThresholdToUpdate != XRFDC_UPDATE_THRESHOLD_BOTH)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid ThresholdToUpdate value (%u) for ADC %u block %u in %s\r\n",
			  ThresholdToUpdate, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((ClrMode != XRFDC_THRESHOLD_CLRMD_MANUAL_CLR) && (ClrMode != XRFDC_THRESHOLD_CLRMD_AUTO_CLR)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Clear mode value (%u) for ADC %u block %u in %s\r\n", ClrMode,
			  Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		    XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = XRFDC_NUM_OF_BLKS3;
			if (Block_Id == XRFDC_BLK_ID1) {
				NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		/* Update for Threshold0 */
		if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_0) ||
		    (ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_CFG_OFFSET);
			if (ClrMode == XRFDC_THRESHOLD_CLRMD_MANUAL_CLR) {
				ReadReg &= ~XRFDC_TRSHD0_CLR_MOD_MASK;
			} else {
				ReadReg |= XRFDC_TRSHD0_CLR_MOD_MASK;
			}
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_CFG_OFFSET, ReadReg);
		}
		/* Update for Threshold1 */
		if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_1) ||
		    (ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_CFG_OFFSET);
			if (ClrMode == XRFDC_THRESHOLD_CLRMD_MANUAL_CLR) {
				ReadReg &= ~XRFDC_TRSHD1_CLR_MOD_MASK;
			} else {
				ReadReg |= XRFDC_TRSHD1_CLR_MOD_MASK;
			}
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_CFG_OFFSET, ReadReg);
		}
		if ((InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			Index += XRFDC_BLK_ID2;
		} else {
			Index += XRFDC_BLK_ID1;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Threshold settings are updated into the relevant registers. Driver structure
* is updated with the new values. There can be two threshold settings:
* threshold0 and threshold1. Both of them are independent of each other.
* The function returns the requested threshold (which can be threshold0,
* threshold1, or both.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    ThresholdSettingsPtr Pointer through which the register settings for
*           thresholds are passed to the API.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
u32 XRFdc_SetThresholdSettings(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
			       XRFdc_Threshold_Settings *ThresholdSettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	XRFdc_Threshold_Settings *ThresholdConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ThresholdSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		    XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = XRFDC_NUM_OF_BLKS3;
			if (Block_Id == XRFDC_BLK_ID1) {
				NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		ThresholdConfigPtr = &InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].Threshold_Settings;
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);

		if ((ThresholdSettingsPtr->UpdateThreshold != XRFDC_UPDATE_THRESHOLD_0) &&
		    (ThresholdSettingsPtr->UpdateThreshold != XRFDC_UPDATE_THRESHOLD_1) &&
		    (ThresholdSettingsPtr->UpdateThreshold != XRFDC_UPDATE_THRESHOLD_BOTH)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid UpdateThreshold value (%u) for ADC %u block %u in %s\r\n",
				  ThresholdSettingsPtr->UpdateThreshold, Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if (((ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_0) ||
		     (ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_BOTH)) &&
		    (ThresholdSettingsPtr->ThresholdMode[0] > XRFDC_TRSHD_HYSTERISIS)) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Requested threshold mode for threshold0 is invalid (%u) for ADC %u block %u in %s\r\n",
				ThresholdSettingsPtr->ThresholdMode[0], Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if (((ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_1) ||
		     (ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_BOTH)) &&
		    (ThresholdSettingsPtr->ThresholdMode[1] > XRFDC_TRSHD_HYSTERISIS)) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Requested threshold mode for threshold1 is invalid (%u) for ADC %u block %u in %s\r\n",
				ThresholdSettingsPtr->ThresholdMode[0], Tile_Id, Block_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		/* Update for Threshold0 */
		if ((ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_0) ||
		    (ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_AVG_LO_OFFSET,
					 (u16)ThresholdSettingsPtr->ThresholdAvgVal[0]);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_AVG_UP_OFFSET,
					 (u16)(ThresholdSettingsPtr->ThresholdAvgVal[0] >> XRFDC_TRSHD0_AVG_UPP_SHIFT));
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_UNDER_OFFSET, XRFDC_TRSHD0_UNDER_MASK,
					ThresholdSettingsPtr->ThresholdUnderVal[0]);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_OVER_OFFSET, XRFDC_TRSHD0_OVER_MASK,
					ThresholdSettingsPtr->ThresholdOverVal[0]);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_CFG_OFFSET, XRFDC_TRSHD0_EN_MOD_MASK,
					ThresholdSettingsPtr->ThresholdMode[0]);

			ThresholdConfigPtr->ThresholdMode[0] = ThresholdSettingsPtr->ThresholdMode[0];
			ThresholdConfigPtr->ThresholdAvgVal[0] = ThresholdSettingsPtr->ThresholdAvgVal[0];
			ThresholdConfigPtr->ThresholdUnderVal[0] = ThresholdSettingsPtr->ThresholdUnderVal[0];
			ThresholdConfigPtr->ThresholdOverVal[0] = ThresholdSettingsPtr->ThresholdOverVal[0];
		}

		/* Update for Threshold1 */
		if ((ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_1) ||
		    (ThresholdSettingsPtr->UpdateThreshold == XRFDC_UPDATE_THRESHOLD_BOTH)) {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_AVG_LO_OFFSET,
					 (u16)ThresholdSettingsPtr->ThresholdAvgVal[1]);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_AVG_UP_OFFSET,
					 (u16)(ThresholdSettingsPtr->ThresholdAvgVal[1] >> XRFDC_TRSHD1_AVG_UPP_SHIFT));
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_UNDER_OFFSET, XRFDC_TRSHD1_UNDER_MASK,
					ThresholdSettingsPtr->ThresholdUnderVal[1]);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_OVER_OFFSET, XRFDC_TRSHD1_OVER_MASK,
					ThresholdSettingsPtr->ThresholdOverVal[1]);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_CFG_OFFSET, XRFDC_TRSHD1_EN_MOD_MASK,
					ThresholdSettingsPtr->ThresholdMode[1]);

			ThresholdConfigPtr->ThresholdMode[1] = ThresholdSettingsPtr->ThresholdMode[1];
			ThresholdConfigPtr->ThresholdAvgVal[1] = ThresholdSettingsPtr->ThresholdAvgVal[1];
			ThresholdConfigPtr->ThresholdUnderVal[1] = ThresholdSettingsPtr->ThresholdUnderVal[1];
			ThresholdConfigPtr->ThresholdOverVal[1] = ThresholdSettingsPtr->ThresholdOverVal[1];
		}
		if ((InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			Index += XRFDC_BLK_ID2;
		} else {
			Index += XRFDC_BLK_ID1;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Threshold settings are read from the corresponding registers and are passed
* back to the caller. There can be two threshold settings:
* threshold0 and threshold1. Both of them are independent of each other.
* The function returns the requested threshold (which can be threshold0,
* threshold1, or both.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    ThresholdSettingsPtr Pointer through which the register settings
*           for thresholds are passed back..
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetThresholdSettings(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
			       XRFdc_Threshold_Settings *ThresholdSettingsPtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ThresholdSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].MixerInputDataType !=
	     XRFDC_DATA_TYPE_IQ)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	/* Threshold mode */
	ThresholdSettingsPtr->UpdateThreshold = XRFDC_UPDATE_THRESHOLD_BOTH;
	ThresholdSettingsPtr->ThresholdMode[0] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_CFG_OFFSET, XRFDC_TRSHD0_EN_MOD_MASK);
	ThresholdSettingsPtr->ThresholdMode[1] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_CFG_OFFSET, XRFDC_TRSHD1_EN_MOD_MASK);

	/* Threshold Average Value */
	ThresholdSettingsPtr->ThresholdAvgVal[0] =
		XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_AVG_LO_OFFSET);
	ThresholdSettingsPtr->ThresholdAvgVal[0] |=
		(XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_AVG_UP_OFFSET) << XRFDC_TRSHD0_AVG_UPP_SHIFT);
	ThresholdSettingsPtr->ThresholdAvgVal[1] =
		XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_AVG_LO_OFFSET);
	ThresholdSettingsPtr->ThresholdAvgVal[1] |=
		(XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_AVG_UP_OFFSET) << XRFDC_TRSHD1_AVG_UPP_SHIFT);

	/* Threshold Under Value */
	ThresholdSettingsPtr->ThresholdUnderVal[0] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_UNDER_OFFSET, XRFDC_TRSHD0_UNDER_MASK);
	ThresholdSettingsPtr->ThresholdUnderVal[1] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_UNDER_OFFSET, XRFDC_TRSHD1_UNDER_MASK);

	/* Threshold Over Value */
	ThresholdSettingsPtr->ThresholdOverVal[0] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD0_OVER_OFFSET, XRFDC_TRSHD0_OVER_MASK);
	ThresholdSettingsPtr->ThresholdOverVal[1] =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TRSHD1_OVER_OFFSET, XRFDC_TRSHD1_OVER_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decoder mode is updated into the relevant registers. Driver structure is
* updated with the new values.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecoderMode Valid values are 1 (Maximum SNR, for non-
*           randomized decoder), 2 (Maximum Linearity, for randomized decoder)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only DAC blocks
*
******************************************************************************/
u32 XRFdc_SetDecoderMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecoderMode)
{
	u32 Status;
	u32 *DecoderModeConfigPtr;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	DecoderModeConfigPtr = &InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Block_Id].DecoderMode;
	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);

	if ((DecoderMode != XRFDC_DECODER_MAX_SNR_MODE) && (DecoderMode != XRFDC_DECODER_MAX_LINEARITY_MODE)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid decoder mode (%u) for DAC %u block %u in %s\r\n", DecoderMode,
			  Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_DECODER_CTRL_OFFSET, XRFDC_DEC_CTRL_MODE_MASK, DecoderMode);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_DECODER_CLK_OFFSET, XRFDC_DEC_CTRL_MODE_MASK, DecoderMode);
	*DecoderModeConfigPtr = DecoderMode;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decoder mode is read and returned back.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecoderModePtr Valid values are 1 (Maximum SNR, for non-randomized
*           decoder), 2 (Maximum Linearity, for randomized decoder)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_GetDecoderMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecoderModePtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DecoderModePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	*DecoderModePtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_DECODER_CTRL_OFFSET, XRFDC_DEC_CTRL_MODE_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get Output Current for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    OutputCurrPtr pointer to return the output current.
*
* @return
*           - Return Output Current for DAC block
*
******************************************************************************/
u32 XRFdc_GetOutputCurr(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *OutputCurrPtr)
{
	u32 Status;
	u32 BaseAddr;
	u16 ReadReg_Cfg2;
	u16 ReadReg_Cfg3;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(OutputCurrPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);

	ReadReg_Cfg3 = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG3_OFFSET, XRFDC_DAC_MC_CFG3_CSGAIN_MASK);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		ReadReg_Cfg2 = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG2_OFFSET,
					   XRFDC_DAC_MC_CFG2_OPCSCAS_MASK);
		if ((ReadReg_Cfg2 == XRFDC_DAC_MC_CFG2_OPCSCAS_32MA) &&
		    (ReadReg_Cfg3 == XRFDC_DAC_MC_CFG3_CSGAIN_32MA)) {
			*OutputCurrPtr = XRFDC_OUTPUT_CURRENT_32MA;
		} else if ((ReadReg_Cfg2 == XRFDC_DAC_MC_CFG2_OPCSCAS_20MA) &&
			   (ReadReg_Cfg3 == XRFDC_DAC_MC_CFG3_CSGAIN_20MA)) {
			*OutputCurrPtr = XRFDC_OUTPUT_CURRENT_20MA;
		} else if ((ReadReg_Cfg2 == 0x0) && (ReadReg_Cfg3 == 0x0)) {
			*OutputCurrPtr = 0x0;
		} else {
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Invalid output current value (%u) for DAC %u block %u in %s\r\n",
				  *OutputCurrPtr, Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	} else {
		*OutputCurrPtr = ((ReadReg_Cfg3 >> XRFDC_DAC_MC_CFG3_CSGAIN_SHIFT) * XRFDC_STEP_I_UA) + XRFDC_MIN_I_UA_INT;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Set the Nyquist zone.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    NyquistZone valid values are 1 (Odd),2 (Even).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_SetNyquistZone(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 NyquistZone)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	u8 CalibrationMode = 0U;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((NyquistZone != XRFDC_ODD_NYQUIST_ZONE) && (NyquistZone != XRFDC_EVEN_NYQUIST_ZONE)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid NyquistZone value (%u) for %s %u block %u in %s\r\n",
			  NyquistZone, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Index);
		if (Type == XRFDC_ADC_TILE) {
			/* Identify calibration mode */
			Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id, Block_Id, &CalibrationMode);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				if (CalibrationMode == XRFDC_CALIB_MODE1) {
					if (NyquistZone == XRFDC_ODD_NYQUIST_ZONE) {
						NyquistZone = XRFDC_EVEN_NYQUIST_ZONE;
					} else {
						NyquistZone = XRFDC_ODD_NYQUIST_ZONE;
					}
				}
			}
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL0_OFFSET);
			if ((NyquistZone % 2U) == 0U) {
				ReadReg |= XRFDC_TI_TISK_ZONE_MASK;
			} else {
				ReadReg &= ~XRFDC_TI_TISK_ZONE_MASK;
			}

			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL0_OFFSET, ReadReg);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].NyquistZone = NyquistZone;
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG0_OFFSET);
			if ((NyquistZone % 2U) == 0U) {
				ReadReg |= XRFDC_MC_CFG0_MIX_MODE_MASK;
			} else {
				ReadReg &= ~XRFDC_MC_CFG0_MIX_MODE_MASK;
			}

			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG0_OFFSET, ReadReg);
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Index].NyquistZone = NyquistZone;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get the Nyquist zone.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    NyquistZonePtr Pointer to return the Nyquist zone.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetNyquistZone(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *NyquistZonePtr)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Block;
	u8 CalibrationMode = 0U;
	u8 MultibandConfig;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(NyquistZonePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Type == XRFDC_ADC_TILE) {
		MultibandConfig = InstancePtr->ADC_Tile[Tile_Id].MultibandConfig;
	} else {
		MultibandConfig = InstancePtr->DAC_Tile[Tile_Id].MultibandConfig;
	}

	if (MultibandConfig != XRFDC_MB_MODE_SB) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	} else {
		Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	}
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Block = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);

	if (Type == XRFDC_ADC_TILE) {
		/* Identify calibration mode */
		Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id, Block, &CalibrationMode);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL0_OFFSET, XRFDC_TI_TISK_ZONE_MASK);
		*NyquistZonePtr = (ReadReg >> XRFDC_TISK_ZONE_SHIFT);
	} else {
		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG0_OFFSET, XRFDC_MC_CFG0_MIX_MODE_MASK);
		*NyquistZonePtr = (ReadReg >> XRFDC_MC_CFG0_MIX_MODE_SHIFT);
	}
	if (*NyquistZonePtr == 0U) {
		*NyquistZonePtr = XRFDC_ODD_NYQUIST_ZONE;
	} else {
		*NyquistZonePtr = XRFDC_EVEN_NYQUIST_ZONE;
	}
	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if ((Type == XRFDC_ADC_TILE) && (CalibrationMode == XRFDC_CALIB_MODE1)) {
			if (*NyquistZonePtr == XRFDC_EVEN_NYQUIST_ZONE) {
				*NyquistZonePtr = XRFDC_ODD_NYQUIST_ZONE;
			} else {
				*NyquistZonePtr = XRFDC_EVEN_NYQUIST_ZONE;
			}
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the Calibration mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    CalibrationMode valid values are 0(Gen 3 only), 1 and 2.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetCalibrationMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u8 CalibrationMode)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	XRFdc_Mixer_Settings Mixer_Settings = { 0 };
	u32 NyquistZone = 0U;
	u8 CalibrationModeReg;
	u32 TimeSkewTuning;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if (InstancePtr->ADC_Tile[Tile_Id].MultibandConfig != XRFDC_MB_MODE_SB) {
			Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
		} else {
			Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
		}
	} else {
		Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	}

	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		return XRFDC_FAILURE;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if ((CalibrationMode != XRFDC_CALIB_MODE1) && (CalibrationMode != XRFDC_CALIB_MODE2)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid Calibration mode value (%u) for ADC %u block %u in %s\r\n",
				  CalibrationMode, Tile_Id, Block_Id, __func__);
			return XRFDC_FAILURE;
		}
		/* Get Mixer Configurations */
		Status = XRFdc_GetMixerSettings(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, &Mixer_Settings);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		/* Get Nyquist Zone */
		Status = XRFdc_GetNyquistZone(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, &NyquistZone);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		for (; Index < NoOfBlocks; Index++) {
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL0_OFFSET);
			ReadReg &= ~XRFDC_TI_DCB_MODE_MASK;
			if (CalibrationMode == XRFDC_CALIB_MODE1) {
				if (((Index % 2U) != 0U) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
					ReadReg |= XRFDC_TI_DCB_MODE1_4GSPS;
				} else if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) {
					ReadReg |= XRFDC_TI_DCB_MODE1_2GSPS;
				}
			}
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL0_OFFSET, ReadReg);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].CalibrationMode =
				CalibrationMode;
		}

		/* Set Nyquist Zone */
		Status = XRFdc_SetNyquistZone(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, NyquistZone);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		/* Set Mixer Configurations */
		Status = XRFdc_SetMixerSettings(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, &Mixer_Settings);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
	} else {
		if ((CalibrationMode != XRFDC_CALIB_MODE1) && (CalibrationMode != XRFDC_CALIB_MODE2) &&
		    (CalibrationMode != XRFDC_CALIB_MODE_AUTO)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Invalid Calibration mode value (%u) for ADC %u block %u in %s\r\n",
				  CalibrationMode, Tile_Id, Block_Id, __func__);
			return XRFDC_FAILURE;
		}
		switch (CalibrationMode) {
		case XRFDC_CALIB_MODE1:
			CalibrationModeReg = XRFDC_CALIB_MODE_NEG_ABS_SUM;
			TimeSkewTuning = XRFDC_TSCB_TUNE_NOT_AUTOCAL;
			break;
		case XRFDC_CALIB_MODE2:
			CalibrationModeReg = XRFDC_CALIB_MODE_ABS_DIFF;
			TimeSkewTuning = XRFDC_TSCB_TUNE_NOT_AUTOCAL;
			break;
		default:
			CalibrationModeReg = XRFDC_CALIB_MODE_MIXED;
			TimeSkewTuning = XRFDC_TSCB_TUNE_AUTOCAL;
			break;
		}
		for (; Index < NoOfBlocks; Index++) {
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL5_OFFSET, XRFDC_CAL_MODES_MASK,
					CalibrationModeReg);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL0_OFFSET, XRFDC_CAL_TSCB_TUNE_MASK,
					TimeSkewTuning);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].CalibrationMode =
				CalibrationMode;
		}
	}

	Status = XRFDC_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
*
* This API is to get the Calibration mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    CalibrationModePtr pointer to get the calibration mode.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetCalibrationMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u8 *CalibrationModePtr)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CalibrationModePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if (InstancePtr->ADC_Tile[Tile_Id].MultibandConfig != XRFDC_MB_MODE_SB) {
			Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
		} else {
			Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
		}
	} else {
		Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	}

	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		if (Block_Id == XRFDC_BLK_ID1) {
			Block_Id = XRFDC_BLK_ID3;
		}
		if (Block_Id == XRFDC_BLK_ID0) {
			Block_Id = XRFDC_BLK_ID1;
		}
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL0_OFFSET, XRFDC_TI_DCB_MODE_MASK);
		*CalibrationModePtr = (ReadReg != 0U) ? XRFDC_CALIB_MODE1 : XRFDC_CALIB_MODE2;
	} else {
		*CalibrationModePtr =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_TISK_CRL5_OFFSET, XRFDC_CAL_MODES_MASK);
		switch (*CalibrationModePtr) {
		case XRFDC_CALIB_MODE_NEG_ABS_SUM:
			*CalibrationModePtr = XRFDC_CALIB_MODE1;
			break;
		case XRFDC_CALIB_MODE_ABS_DIFF:
			*CalibrationModePtr = XRFDC_CALIB_MODE2;
			break;
		default:
			*CalibrationModePtr = XRFDC_CALIB_MODE_AUTO;
			break;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function is used to get the Coupling mode.
 *
 * @param    InstancePtr is a pointer to the XRfdc instance.
 * @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
 * @param    Tile_Id indicates Tile number.
 * @param    Block_Id indicates Block number.
 * @param    ModePtr pointer to get link coupling mode.
 *
 * @return
 *           - XRFDC_SUCCESS if successful.
 *           - XRFDC_FAILURE if error occurs.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 XRFdc_GetCoupling(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
                      u32 *ModePtr)
{
  u32 Status;
  u16 ReadReg;
  u32 BaseAddr;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(ModePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

  if ((Type == XRFDC_DAC_TILE) &&
      (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3)) {
    Status = XRFDC_FAILURE;
    metal_log(METAL_LOG_ERROR,
              "\n DAC coupling for Gen 1/2 devices is not available in %s\r\n",
              __func__);
    goto RETURN_PATH;
  }

  Status =
      XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
  if (Status != XRFDC_SUCCESS) {
    metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
              ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id,
              __func__);
    goto RETURN_PATH;
  }

  if (Type == XRFDC_ADC_TILE) {
    if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) &&
        (Block_Id == XRFDC_BLK_ID1)) {
      Block_Id = XRFDC_BLK_ID2;
    }

    BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

    ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_RXPR_MC_CFG0_OFFSET,
                          XRFDC_RX_MC_CFG0_CM_MASK);
    if (ReadReg == XRFDC_RX_MC_CFG0_CM_MASK) {
      *ModePtr = XRFDC_LINK_COUPLING_AC;
    } else {
      *ModePtr = XRFDC_LINK_COUPLING_DC;
    }

    if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
      *ModePtr = !(*ModePtr); /*logic is inverted for GEN3 devices */
    }
  } else {
    BaseAddr = XRFDC_CTRL_STS_BASE(XRFDC_DAC_TILE, Tile_Id);
    ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CPL_TYPE_OFFSET);
    if (ReadReg == XRFDC_DAC_LINK_COUPLING_AC) {
      *ModePtr = XRFDC_LINK_COUPLING_AC;
    } else {
      *ModePtr = XRFDC_LINK_COUPLING_DC;
    }
  }

  Status = XRFDC_SUCCESS;
RETURN_PATH:
  return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get the Link Coupling mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for 2G, 0-1 for 4G).
* @param    ModePtr pointer to get link coupling mode.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetLinkCoupling(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ModePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	metal_log(METAL_LOG_WARNING,
		"\n %s API Scheduled for deprication in 2024.1, please use the XRFdc_GetCoupling() API\r\n", __func__);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_RXPR_MC_CFG0_OFFSET, XRFDC_RX_MC_CFG0_CM_MASK);
	if (ReadReg != 0U) {
		*ModePtr = XRFDC_LINK_COUPLING_AC;
	} else {
		*ModePtr = XRFDC_LINK_COUPLING_DC;
	}

	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		*ModePtr = !(*ModePtr); /*logic is inverted for GEN3 devices */
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set the IM3 Dither mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    Mode 0: Disable
*                1: Enable
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetDither(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (Mode > XRFDC_DITH_ENABLE) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Dither Mode (%u) for ADC %u block %u in %s\r\n", Mode, Tile_Id,
			  Block_Id, __func__);
		goto RETURN_PATH;
	}
	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG0_OFFSET, XRFDC_RX_MC_CFG0_IM3_DITH_MASK,
				(Mode << XRFDC_RX_MC_CFG0_IM3_DITH_SHIFT));
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get the IM3 Dither mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    ModePtr pointer to get link coupling mode.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetDither(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ModePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG0_OFFSET, XRFDC_RX_MC_CFG0_IM3_DITH_MASK);
	if (ReadReg != 0U) {
		*ModePtr = XRFDC_DITH_ENABLE;
	} else {
		*ModePtr = XRFDC_DITH_DISABLE;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set the ADC Signal Detector Settings.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    SettingsPtr pointer to the XRFdc_Signal_Detector_Settings structure
*           to set the signal detector configurations
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if tile not enabled, or invalid values.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetSignalDetector(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Signal_Detector_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	u16 SignalDetCtrlReg = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested fuctionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);

	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (SettingsPtr->Mode > XRFDC_SIGDET_MODE_RNDM) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Signal Detector Mode (%u) for ADC %u block %u in %s\r\n",
			  SettingsPtr->Mode, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (SettingsPtr->EnableIntegrator > XRFDC_ENABLED) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Signal Detector Integrator Enable (%u) for ADC %u block %u in %s\r\n",
			  SettingsPtr->EnableIntegrator, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (SettingsPtr->HysteresisEnable > XRFDC_ENABLED) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Signal Detector Hysteresis Enable (%u) for ADC %u block %u in %s\r\n",
			  SettingsPtr->HysteresisEnable, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (SettingsPtr->Flush > XRFDC_ENABLED) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Signal Detector Flush Option (%u) for ADC %u block %u in %s\r\n",
			  SettingsPtr->Flush, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (SettingsPtr->TimeConstant > XRFDC_SIGDET_TC_2_18) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Signal Detector Time Constant (%u) for ADC %u block %u in %s\r\n",
			  SettingsPtr->TimeConstant, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	SignalDetCtrlReg |= SettingsPtr->EnableIntegrator << XRFDC_ADC_SIG_DETECT_INTG_SHIFT;
	SignalDetCtrlReg |= SettingsPtr->Flush << XRFDC_ADC_SIG_DETECT_FLUSH_SHIFT;
	SignalDetCtrlReg |= SettingsPtr->TimeConstant << XRFDC_ADC_SIG_DETECT_TCONST_SHIFT;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		SignalDetCtrlReg |= ((SettingsPtr->Mode << 1) | 1) << XRFDC_ADC_SIG_DETECT_MODE_WRITE_SHIFT;
	} else {
		SignalDetCtrlReg |= (SettingsPtr->Mode << 1) << XRFDC_ADC_SIG_DETECT_MODE_WRITE_SHIFT;
	}
	SignalDetCtrlReg |= SettingsPtr->HysteresisEnable << XRFDC_ADC_SIG_DETECT_HYST_SHIFT;

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_CTRL_OFFSET, XRFDC_ADC_SIG_DETECT_MASK,
				SignalDetCtrlReg);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_LEVEL_OFFSET,
				 SettingsPtr->Threshold);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_ON_OFFSET,
				 SettingsPtr->ThreshOnTriggerCnt);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_OFF_OFFSET,
				 SettingsPtr->ThreshOffTriggerCnt);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get the ADC Signal Detector Settings.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    SettingsPtr pointer to the XRFdc_Signal_Detector_Settings structure
*           to get the signal detector configurations
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetSignalDetector(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Signal_Detector_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u16 SignalDetCtrlReg = 0;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}
	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);

	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	SignalDetCtrlReg =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_CTRL_OFFSET, XRFDC_ADC_SIG_DETECT_MASK);
	SettingsPtr->EnableIntegrator =
		(SignalDetCtrlReg & XRFDC_ADC_SIG_DETECT_INTG_MASK) >> XRFDC_ADC_SIG_DETECT_INTG_SHIFT;
	SettingsPtr->Flush = (SignalDetCtrlReg & XRFDC_ADC_SIG_DETECT_FLUSH_MASK) >> XRFDC_ADC_SIG_DETECT_FLUSH_SHIFT;
	SettingsPtr->TimeConstant =
		(SignalDetCtrlReg & XRFDC_ADC_SIG_DETECT_TCONST_MASK) >> XRFDC_ADC_SIG_DETECT_TCONST_SHIFT;
	SettingsPtr->Mode = (SignalDetCtrlReg & XRFDC_ADC_SIG_DETECT_MODE_MASK) >> XRFDC_ADC_SIG_DETECT_MODE_READ_SHIFT;

	SettingsPtr->HysteresisEnable =
		(SignalDetCtrlReg & XRFDC_ADC_SIG_DETECT_HYST_MASK) >> XRFDC_ADC_SIG_DETECT_HYST_SHIFT;
	SettingsPtr->Threshold = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_LEVEL_OFFSET);
	SettingsPtr->ThreshOnTriggerCnt =
		XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_ON_OFFSET);
	SettingsPtr->ThreshOffTriggerCnt =
		XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_OFF_OFFSET);
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to disable Calibration Coefficients override.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    CalibrationBlock indicates the calibration block.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_DisableCoefficientsOverride(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock)
{
	u32 BaseAddr;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (CalibrationBlock == XRFDC_CAL_BLOCK_OCB1)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		switch (CalibrationBlock) {
		case XRFDC_CAL_BLOCK_OCB1:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET, XRFDC_CAL_OCB_EN_MASK,
					XRFDC_DISABLED);
			break;
		case XRFDC_CAL_BLOCK_OCB2:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL3_OFFSET, XRFDC_CAL_OCB_EN_MASK,
					XRFDC_DISABLED);
			break;
		case XRFDC_CAL_BLOCK_GCB:
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_ENFL_MASK, XRFDC_CAL_GCB_ACEN_MASK);
				/*Clear IP Override Coeffs*/
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF0_FAB(Index), XRFDC_CAL_GCB_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF1_FAB(Index), XRFDC_CAL_GCB_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF2_FAB(Index), XRFDC_CAL_GCB_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF3_FAB(Index), XRFDC_CAL_GCB_MASK, XRFDC_DISABLED);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_EN_MASK, XRFDC_DISABLED);
			}
			break;
		case XRFDC_CAL_BLOCK_TSCB:
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_DISABLED);
			}
			break;
		default:
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Invalid Calibration Mode (%u) for ADC %u block %u in %s\r\n",
				  CalibrationBlock, Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set the ADC Calibration Coefficients.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    CalibrationBlock indicates the block to be written to.
* @param    CoeffPtr is pointer to the XRFdc_Calibration_Coefficients structure
*           to set the calibration coefficients.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetCalCoefficients(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock,
			     XRFdc_Calibration_Coefficients *CoeffPtr)
{
	u32 BaseAddr;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;
	u32 HighSpeed;
	u32 Shift;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoeffPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (CalibrationBlock == XRFDC_CAL_BLOCK_OCB1)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	if (CalibrationBlock == XRFDC_CAL_BLOCK_GCB) {
		if ((CoeffPtr->Coeff0 | CoeffPtr->Coeff1 | CoeffPtr->Coeff2 | CoeffPtr->Coeff3) &
		    ~(XRFDC_CAL_GCB_MASK | (XRFDC_CAL_GCB_MASK << XRFDC_CAL_SLICE_SHIFT))) {
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR,
				  "\n Bad GCB Coefficient(s) {%u %u %u %u} for ADC %u block %u in %s\r\n",
				  CoeffPtr->Coeff0, CoeffPtr->Coeff1, CoeffPtr->Coeff2, CoeffPtr->Coeff3, Tile_Id,
				  Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	if (CalibrationBlock == XRFDC_CAL_BLOCK_TSCB) {
		if ((CoeffPtr->Coeff0 | CoeffPtr->Coeff1 | CoeffPtr->Coeff2 | CoeffPtr->Coeff3 | CoeffPtr->Coeff4 |
		     CoeffPtr->Coeff5 | CoeffPtr->Coeff6 | CoeffPtr->Coeff7) &
		    ~(XRFDC_CAL_TSCB_MASK | (XRFDC_CAL_TSCB_MASK << XRFDC_CAL_SLICE_SHIFT))) {
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR,
				  "\n Bad TSCB Coefficient(s) {%u %u %u %u %u %u %u %u} for ADC %u block %u in %s\r\n",
				  CoeffPtr->Coeff0, CoeffPtr->Coeff1, CoeffPtr->Coeff2, CoeffPtr->Coeff3,
				  CoeffPtr->Coeff4, CoeffPtr->Coeff5, CoeffPtr->Coeff6, CoeffPtr->Coeff7, Tile_Id,
				  Block_Id, __func__);
			goto RETURN_PATH;
		}
	}
	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Index = Block_Id;
	HighSpeed = XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id);
	if (HighSpeed == XRFDC_ENABLED) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}
	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		Shift = HighSpeed ? XRFDC_CAL_SLICE_SHIFT * (Index % 2) : 0;
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		switch (CalibrationBlock) {
		case XRFDC_CAL_BLOCK_OCB1:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET, XRFDC_CAL_OCB_EN_MASK,
					XRFDC_ENABLED);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF0, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff0 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF1, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff1 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF2, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff2 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF3, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff3 >> Shift);
			break;
		case XRFDC_CAL_BLOCK_OCB2:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL3_OFFSET, XRFDC_CAL_OCB_EN_MASK,
					XRFDC_ENABLED);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF0, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff0 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF1, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff1 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF2, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff2 >> Shift);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF3, XRFDC_CAL_OCB_MASK,
					CoeffPtr->Coeff3 >> Shift);
			break;
		case XRFDC_CAL_BLOCK_GCB:

			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_ACEN_MASK, XRFDC_DISABLED);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_FLSH_MASK, XRFDC_ENABLED << XRFDC_CAL_GCB_FLSH_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF0_FAB(Index), XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff0 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF1_FAB(Index), XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff1 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF2_FAB(Index), XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff2 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
						XRFDC_CAL_GCB_COEFF3_FAB(Index), XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff3 >> Shift);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_GCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF0, XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff0 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF1, XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff1 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF2, XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff2 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF3, XRFDC_CAL_GCB_MASK,
						CoeffPtr->Coeff3 >> Shift);
			}
			break;
		case XRFDC_CAL_BLOCK_TSCB:
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7_ALT,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff0 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff1 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff2 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff3 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff4 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff5 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff6 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7_ALT,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff7 >> Shift);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7,
						XRFDC_CAL_TSCB_EN_MASK, XRFDC_ENABLED << XRFDC_CAL_TSCB_EN_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff0 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff1 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff2 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff3 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff4 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff5 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff6 >> Shift);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7,
						XRFDC_CAL_TSCB_MASK, CoeffPtr->Coeff7 >> Shift);
			}
			break;
		default:
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Invalid calibration block (%u) for ADC %u block %u in %s\r\n",
				  CalibrationBlock, Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get the ADC Calibration Coefficients.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    CalibrationBlock indicates the block to be read from
* @param    CoeffPtr is pointer to the XRFdc_Calibration_Coefficients structure
*           to get the calibration coefficients.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetCalCoefficients(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock,
			     XRFdc_Calibration_Coefficients *CoeffPtr)
{
	u32 BaseAddr;
	u32 Status;
	u32 Index;
	u32 HighSpeed;
	u32 Shift;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoeffPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	memset(CoeffPtr, 0, sizeof(XRFdc_Calibration_Coefficients));
	Index = Block_Id;
	HighSpeed = XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id);
	if (HighSpeed == XRFDC_ENABLED) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}
	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		Shift = HighSpeed ? XRFDC_CAL_SLICE_SHIFT * (Index % 2) : 0;
		switch (CalibrationBlock) {
		case XRFDC_CAL_BLOCK_OCB1:
			CoeffPtr->Coeff0 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF0, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff1 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF1, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff2 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF2, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff3 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB1_OFFSET_COEFF3, XRFDC_CAL_OCB_MASK)
				<< Shift;
			break;
		case XRFDC_CAL_BLOCK_OCB2:
			CoeffPtr->Coeff0 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF0, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff1 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF1, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff2 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF2, XRFDC_CAL_OCB_MASK)
				<< Shift;
			CoeffPtr->Coeff3 |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_OCB2_OFFSET_COEFF3, XRFDC_CAL_OCB_MASK)
				<< Shift;
			break;
		case XRFDC_CAL_BLOCK_GCB:
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				if (XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_TI_DCB_CRL1_OFFSET,
						XRFDC_CAL_GCB_FLSH_MASK) == XRFDC_DISABLED) {
					CoeffPtr->Coeff0 |=
						(XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF0_ALT,
							     XRFDC_CAL_GCB_FAB_MASK) >>
						 4)
						<< Shift;
					CoeffPtr->Coeff1 |=
						(XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF1_ALT,
							     XRFDC_CAL_GCB_FAB_MASK) >>
						 4)
						<< Shift;
					CoeffPtr->Coeff2 |=
						(XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF2_ALT,
							     XRFDC_CAL_GCB_FAB_MASK) >>
						 4)
						<< Shift;
					CoeffPtr->Coeff3 |=
						(XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF3_ALT,
							     XRFDC_CAL_GCB_FAB_MASK) >>
						 4)
						<< Shift;
				} else {
					CoeffPtr->Coeff0 |=
						XRFdc_RDReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
							    XRFDC_CAL_GCB_COEFF0_FAB(Index), XRFDC_CAL_GCB_MASK)
						<< Shift;
					CoeffPtr->Coeff1 |=
						XRFdc_RDReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
							    XRFDC_CAL_GCB_COEFF1_FAB(Index), XRFDC_CAL_GCB_MASK)
						<< Shift;
					CoeffPtr->Coeff2 |=
						XRFdc_RDReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
							    XRFDC_CAL_GCB_COEFF2_FAB(Index), XRFDC_CAL_GCB_MASK)
						<< Shift;
					CoeffPtr->Coeff3 |=
						XRFdc_RDReg(InstancePtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id),
							    XRFDC_CAL_GCB_COEFF3_FAB(Index), XRFDC_CAL_GCB_MASK)
						<< Shift;
				}
			} else {
				CoeffPtr->Coeff0 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF0,
								XRFDC_CAL_GCB_MASK)
						    << Shift;
				CoeffPtr->Coeff1 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF1,
								XRFDC_CAL_GCB_MASK)
						    << Shift;
				CoeffPtr->Coeff2 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF2,
								XRFDC_CAL_GCB_MASK)
						    << Shift;
				CoeffPtr->Coeff3 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_GCB_OFFSET_COEFF3,
								XRFDC_CAL_GCB_MASK)
						    << Shift;
			}
			break;
		case XRFDC_CAL_BLOCK_TSCB:
			if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
				CoeffPtr->Coeff0 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff1 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff2 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff3 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff4 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff5 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff6 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff7 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7_ALT,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
			} else {
				CoeffPtr->Coeff0 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF0,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff1 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF1,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff2 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF2,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff3 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF3,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff4 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF4,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff5 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF5,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff6 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF6,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
				CoeffPtr->Coeff7 |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CAL_TSCB_OFFSET_COEFF7,
								XRFDC_CAL_TSCB_MASK)
						    << Shift;
			}
			break;
		default:
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Invalid calibration block (%u) for ADC %u block %u in %s\r\n",
				  CalibrationBlock, Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set calibration freeze settings.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    CalFreezePtr pointer to the settings to be applied.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetCalFreeze(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Cal_Freeze_Settings *CalFreezePtr)
{
	u32 BaseAddr;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CalFreezePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (CalFreezePtr->FreezeCalibration > XRFDC_CAL_FREEZE_CALIB) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid FreezeCalibration option (%u) for ADC %u block %u in %s\r\n",
			  CalFreezePtr->FreezeCalibration, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (CalFreezePtr->DisableFreezePin > XRFDC_CAL_FRZ_PIN_DISABLE) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid DisableFreezePin option (%u) for ADC %u block %u in %s\r\n",
			  CalFreezePtr->DisableFreezePin, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CONV_CAL_STGS(Index), XRFDC_CAL_FREEZE_PIN_MASK,
				CalFreezePtr->DisableFreezePin << XRFDC_CAL_FREEZE_PIN_SHIFT);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CONV_CAL_STGS(Index), XRFDC_CAL_FREEZE_CAL_MASK,
				CalFreezePtr->FreezeCalibration << XRFDC_CAL_FREEZE_CAL_SHIFT);
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get calibration freeze settings and status.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number(0-3 for LS, 0-1 for HS).
* @param    CalFreezePtr pointer to be filled the settings/status.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetCalFreeze(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Cal_Freeze_Settings *CalFreezePtr)
{
	u32 BaseAddr;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CalFreezePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);

	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		if (Block_Id == XRFDC_BLK_ID1) {
			Block_Id = XRFDC_BLK_ID2;
		}
	}
	CalFreezePtr->CalFrozen =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CONV_CAL_STGS(Block_Id), XRFDC_CAL_FREEZE_STS_MASK) >>
		XRFDC_CAL_FREEZE_STS_SHIFT;
	CalFreezePtr->DisableFreezePin =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CONV_CAL_STGS(Block_Id), XRFDC_CAL_FREEZE_PIN_MASK) >>
		XRFDC_CAL_FREEZE_PIN_SHIFT;
	CalFreezePtr->FreezeCalibration =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CONV_CAL_STGS(Block_Id), XRFDC_CAL_FREEZE_CAL_MASK) >>
		XRFDC_CAL_FREEZE_CAL_SHIFT;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Set Output Current for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    uACurrent is the current in uA.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Range 6425 - 32000 uA with 25 uA resolution.
******************************************************************************/
u32 XRFdc_SetDACVOP(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 uACurrent)
{
	u32 Status;
	u32 BaseAddr;
	u16 Gen1CompatibilityMode;
	u32 OptIdx;
	u32 MaxCurrent;
	u32 MinCurrent;
	float uACurrentInt;
	float uACurrentNext;
	u32 Code;
	u32 LinkCoupling;
	u32 *BldrOPCBiasPtr;
	u32 *CSCBldrPtr;
	u32 *CSCBiasPtr;

	/* Tuned optimization values*/
	u32 BldrOPCBiasAC[64] = { 22542, 26637, 27661, 27661, 28686, 28686, 29710, 29711, 30735, 30735, 31760,
				31760, 32784, 32785, 33809, 33809, 34833, 34833, 35857, 36881, 37906, 38930,
				38930, 39954, 40978, 42003, 43027, 43027, 44051, 45075, 46100, 47124, 48148,
				49172, 50196, 51220, 52245, 53269, 53269, 54293, 55317, 56342, 57366, 58390,
				58390, 58390, 59415, 59415, 59415, 59415, 60439, 60439, 60439, 60439, 60439,
				60440, 62489, 62489, 63514, 63514, 63514, 64539, 64539, 64539 };
	u32 BldrOPCBiasDC[64] = { 0, 0, 0, 0, 0, 0, 0, 21526, 22550, 23574, 24598, 25622, 26646, 27670, 28694, 29718, 30742, 31766, 32790, 33814, 34838, 35862, 36886, 37910, 38934, 39958, 40982, 42006, 43030, 44054, 45078, 46102, 47126, 48150, 49174, 50198, 51222, 52246, 53270, 54294, 55318, 56342, 57366, 58390, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u32 CSCBldrAC[64] = { 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152,
			    49152, 49152, 49152, 40960, 40960, 40960, 40960, 40960, 40960, 40960, 40960, 40960, 40960,
			    40960, 40960, 40960, 40960, 40960, 40960, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
			    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 24576, 24576, 24576, 24576,
			    24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576 };
	u32 CSCBldrDC[64] = { 0, 0, 0, 0, 0, 0, 0, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 40960, 40960, 40960, 40960, 40960, 40960, 40960, 40960, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 16384, 16384, 16384, 16384, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u32 CSCBiasAC[64] = { 0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
				5,  5,  5,  5,  5,  5,  6,  7,  8,  9,  10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 31, 31, 31, 31 };
	u32 CSCBiasDC[64] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 6, 7, 8, 9, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(XRFDC_DAC_TILE, Tile_Id);
	LinkCoupling = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CPL_TYPE_OFFSET);

	if (LinkCoupling == XRFDC_DAC_LINK_COUPLING_AC) {
		MinCurrent = XRFDC_VOP_AC_MIN_UA;
		MaxCurrent = XRFDC_VOP_AC_MAX_UA;
		BldrOPCBiasPtr = BldrOPCBiasAC;
		CSCBldrPtr = CSCBldrAC;
		CSCBiasPtr = CSCBiasAC;
	} else {
		MinCurrent = XRFDC_VOP_DC_MIN_UA;
		MaxCurrent = XRFDC_VOP_DC_MAX_UA;
		BldrOPCBiasPtr = BldrOPCBiasDC;
		CSCBldrPtr = CSCBldrDC;
		CSCBiasPtr = CSCBiasDC;
	}

	if (uACurrent > MaxCurrent) {
		metal_log(METAL_LOG_ERROR, "\n Invalid current selection (too high - %u) for DAC %u block %u in %s\r\n",
			  uACurrent, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (uACurrent < MinCurrent) {
		metal_log(METAL_LOG_ERROR, "\n Invalid current selection (too low - %u) for DAC %u block %u in %s\r\n",
			  uACurrent, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	uACurrentInt = (float)uACurrent;

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	Gen1CompatibilityMode =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG2_OFFSET, XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK);
	if (Gen1CompatibilityMode == XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK) {
		metal_log(METAL_LOG_ERROR, "\n Invalid compatibility mode is set for DAC %u block %u in %s\r\n",
			  Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_VOP_CTRL_OFFSET,
			(XRFDC_DAC_VOP_CTRL_REG_UPDT_MASK | XRFDC_DAC_VOP_CTRL_TST_BLD_MASK), XRFDC_ENABLED);

	uACurrentNext =
		((float)(XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG3_OFFSET, XRFDC_DAC_MC_CFG3_CSGAIN_MASK) >>
			 XRFDC_DAC_MC_CFG3_CSGAIN_SHIFT) * XRFDC_STEP_I_UA + (float)XRFDC_MIN_I_UA_INT);

	while (uACurrentInt != uACurrentNext) {
		if (uACurrentNext < uACurrentInt) {
			uACurrentNext += uACurrentNext / 10;
			if (uACurrentNext > uACurrentInt)
				uACurrentNext = uACurrentInt;
		} else {
			uACurrentNext -= uACurrentNext / 10;
			if (uACurrentNext < uACurrentInt)
				uACurrentNext = uACurrentInt;
		}
		Code = (u32)((uACurrentNext - XRFDC_MIN_I_UA_INT) / XRFDC_STEP_I_UA);

		OptIdx = (Code & XRFDC_DAC_MC_CFG3_OPT_LUT_MASK) >> XRFDC_DAC_MC_CFG3_OPT_LUT_SHIFT;
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG0_OFFSET,
				XRFDC_DAC_MC_CFG0_CAS_BLDR_MASK, CSCBldrPtr[OptIdx]);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG2_OFFSET,
				(XRFDC_DAC_MC_CFG2_BLDGAIN_MASK | XRFDC_DAC_MC_CFG2_CAS_BIAS_MASK),
				(BldrOPCBiasPtr[OptIdx] | ((Code & XRFDC_DAC_VOP_BLDR_LOW_BITS_MASK)
							<< XRFDC_DAC_MC_CFG3_CSGAIN_SHIFT)));
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG3_OFFSET,
				(XRFDC_DAC_MC_CFG3_CSGAIN_MASK | XRFDC_DAC_MC_CFG3_OPT_MASK),
				((Code << XRFDC_DAC_MC_CFG3_CSGAIN_SHIFT) | CSCBiasPtr[OptIdx]));

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_MC_CFG3_OFFSET, XRFDC_DAC_MC_CFG3_UPDATE_MASK,
				XRFDC_DAC_MC_CFG3_UPDATE_MASK);
#ifdef __BAREMETAL__
		usleep(1);
#else
		metal_sleep_usec(1);
#endif
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Gets VOP compatibility mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    mode is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    EnabledPtr is pointer a that is filled with whether the mode is
*           enabled (1) or disabled(0).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
******************************************************************************/
u32 XRFdc_GetDACCompMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *EnabledPtr)
{
	u32 Status;
	u32 BaseAddr;
	u16 CompatibilityMode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(EnabledPtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	CompatibilityMode =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG2_OFFSET, XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK);
	*EnabledPtr = (CompatibilityMode == XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK) ? XRFDC_ENABLED : XRFDC_DISABLED;
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Sets VOP compatibility mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    mode is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Enable is whether to enable (1) or disable(0) the compatibility
*           mode.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
******************************************************************************/
u32 XRFdc_SetDACCompMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Enable)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (Enable > XRFDC_ENABLED) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Bad enable parameter (%u) for DAC %u block %u in %s\r\n", Enable,
			  Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DAC_MC_CFG2_OFFSET, XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK,
			Enable << XRFDC_DAC_MC_CFG2_GEN1_COMP_SHIFT);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Set DSA for ADC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Attenuation is the attenuation in dB
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note  Range 0 - 11 dB with 0.5 dB resolution ES1 Si.
*        Range 0 - 27 dB with 1 dB resolution for Production Si.
******************************************************************************/
u32 XRFdc_SetDSA(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_DSA_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 Code;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SettingsPtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (SettingsPtr->Attenuation > XRFDC_MAX_ATTEN(InstancePtr->RFdc_Config.SiRevision)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid attenuation selection (too high - %f) in ADC %u block %u %s\r\n",
			  SettingsPtr->Attenuation, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (SettingsPtr->Attenuation < XRFDC_MIN_ATTEN) {
		metal_log(METAL_LOG_ERROR, "\n Invalid current selection (too low - %f) in ADC %u block %u %s\r\n",
			  SettingsPtr->Attenuation, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);

	Code = (u32)((XRFDC_MAX_ATTEN(InstancePtr->RFdc_Config.SiRevision) - SettingsPtr->Attenuation) /
		     XRFDC_STEP_ATTEN(InstancePtr->RFdc_Config.SiRevision));
	for (; Index < NoOfBlocks; Index++) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CONV_DSA_STGS(Index),
				(XRFDC_ADC_DSA_CODE_MASK | XRFDC_ADC_DSA_RTS_PIN_MASK),
				(Code | (SettingsPtr->DisableRTS << XRFDC_ADC_DSA_RTS_PIN_SHIFT)));
	}

	/*trigger*/
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DSA_UPDT_OFFSET, XRFDC_ADC_DSA_UPDT_MASK, XRFDC_ADC_DSA_UPDT_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get DSA for ADC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    AttenuationPtr is the attenuation in dB
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note  Range 0 - 11 dB with 0.5 dB resolution ES1 Si.
*        Range 0 - 27 dB with 1 dB resolution for Production Si.
******************************************************************************/
u32 XRFdc_GetDSA(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_DSA_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u16 EFuse;
	u32 Code;
	u32 RTSENMode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SettingsPtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	EFuse = XRFdc_ReadReg16(InstancePtr, XRFDC_DRP_BASE(XRFDC_ADC_TILE, Tile_Id) + XRFDC_HSCOM_ADDR,
				XRFDC_HSCOM_EFUSE_2_OFFSET);
	if ((EFuse & XRFDC_EXPORTCTRL_DSA) == XRFDC_EXPORTCTRL_DSA) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n API not available - Licensing - for ADC %u block %u in %s\r\n", Tile_Id,
			  Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
		if (Block_Id == XRFDC_BLK_ID1) {
			Block_Id = XRFDC_BLK_ID2;
		}
	}

	BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
	Code = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CONV_DSA_STGS(Block_Id), XRFDC_ADC_DSA_CODE_MASK);
	RTSENMode = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CONV_DSA_STGS(Block_Id), XRFDC_ADC_DSA_RTS_PIN_MASK);

	SettingsPtr->Attenuation = XRFDC_MAX_ATTEN(InstancePtr->RFdc_Config.SiRevision) -
				   (float)(Code * XRFDC_STEP_ATTEN(InstancePtr->RFdc_Config.SiRevision));
	SettingsPtr->DisableRTS = RTSENMode >> XRFDC_ADC_DSA_RTS_PIN_SHIFT;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Checks whether DAC block is available or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return 1 if DAC block is available, otherwise 0.
*
******************************************************************************/
u32 XRFdc_IsDACBlockEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	u32 IsBlockAvail;
	u32 BlockShift;
	u32 BlockEnableReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0U;
	}

	if (Block_Id > XRFDC_BLOCK_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter block number in %s\r\n", __func__);
		return 0U;
	}

	BlockShift = Block_Id + (XRFDC_PATH_ENABLED_TILE_SHIFT * Tile_Id);
	BlockEnableReg = XRFdc_ReadReg(InstancePtr, XRFDC_IP_BASE, XRFDC_DAC_PATHS_ENABLED_OFFSET);
	BlockEnableReg &= (XRFDC_ENABLED << BlockShift);
	IsBlockAvail = BlockEnableReg >> BlockShift;
	return IsBlockAvail;
}

/*****************************************************************************/
/**
*
* Checks whether ADC block is available or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3 in DAC/ADC-2GSPS and 0-1 in ADC-4GSPS.
*
* @return
*           - Return 1 if ADC block is available, otherwise 0.
*
******************************************************************************/
u32 XRFdc_IsADCBlockEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	u32 IsBlockAvail;
	u32 BlockShift;
	u32 BlockEnableReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0U;
	}

	if (Block_Id > XRFDC_BLOCK_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter block number in %s\r\n", __func__);
		return 0U;
	}

	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) {
		if ((Block_Id == 2U) || (Block_Id == 3U)) {
			IsBlockAvail = 0;
			goto RETURN_PATH;
		}
		if (Block_Id == 1U) {
			Block_Id = 2U;
		}
	}

	BlockShift = Block_Id + (XRFDC_PATH_ENABLED_TILE_SHIFT * Tile_Id);
	BlockEnableReg = XRFdc_ReadReg(InstancePtr, XRFDC_IP_BASE, XRFDC_ADC_PATHS_ENABLED_OFFSET);
	BlockEnableReg &= (XRFDC_ENABLED << BlockShift);
	IsBlockAvail = BlockEnableReg >> BlockShift;

RETURN_PATH:
	return IsBlockAvail;
}

/*****************************************************************************/
/**
*
* Checks whether ADC/DAC block is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - XRFDC_SUCCESS if block enabled.
*           - XRFDC_FAILURE if block not enabled.
*
******************************************************************************/
u32 XRFdc_CheckBlockEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 IsBlockAvail;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((Tile_Id > XRFDC_TILE_ID_MAX) || (Block_Id > XRFDC_BLOCK_ID_MAX)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Type == XRFDC_ADC_TILE) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	} else {
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
	} else {
		Status = XRFDC_SUCCESS;
	}
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get Inversesync filter for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return Inversesync filter for DAC block
*           - Return 0 if invalid
*
******************************************************************************/
u32 XRFdc_GetInverseSincFilter(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0U;
	}

	if (Block_Id > XRFDC_BLOCK_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter block number in %s\r\n", __func__);
		return 0U;
	}

	return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Analog_Config[Block_Id].InvSyncEnable;
}

/*****************************************************************************/
/**
*
* Get Mixed mode for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return mixed mode for DAC block
*           - Return 0 if invalid
*
******************************************************************************/
u32 XRFdc_GetMixedMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0U;
	}

	if (Block_Id > XRFDC_BLOCK_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter block number in %s\r\n", __func__);
		return 0U;
	}

	return InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Block_Id].MixedMode;
}

/*****************************************************************************/
/**
*
* Set data scaler for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Enable  valid values are 1 (enable) and 0 (disable).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     DAC blocks only.
******************************************************************************/
u32 XRFdc_SetDACDataScaler(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Enable)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (Enable > XRFDC_ENABLED) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Bad enable parameter (%u) for DAC %u block %u in %s\r\n", Enable,
			  Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DATA_SCALER_OFFSET, XRFDC_DATA_SCALER_MASK, Enable);

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get data scaler for DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    EnablePtr valid values are 1 (enable) and 0 (disable).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     DAC blocks only.
******************************************************************************/
u32 XRFdc_GetDACDataScaler(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *EnablePtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(EnablePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	*EnablePtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DATA_SCALER_OFFSET, XRFDC_DATA_SCALER_MASK);

RETURN_PATH:
	return Status;
}
/** @} */

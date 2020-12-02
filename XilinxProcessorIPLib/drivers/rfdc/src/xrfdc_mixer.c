/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_mixer.c
* @addtogroup rfdc_v8_1
* @{
*
* Contains the interface functions of the Mixer Settings in XRFdc driver.
* See xrfdc.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 5.0   sk     08/06/18 Initial release
* 5.1   cog    01/29/19 Replace structure reference ADC checks with
*                       function.
*       cog    01/29/19 XRFdc_SetCoarseMixer and MixerRangeCheck now need
*                       Tile_id as a parameter.
*       cog    01/29/19 Rename DataType to MixerInputDataType for
*                       readability.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    06/12/19 Fixed issue where positive NCO frequencies were not
*                       being set correctly.
*       cog    07/03/19 Added new off mode for mixers (both mixers off).
*       cog    08/02/19 Formatting changes.
*       cog    09/01/19 Changed the MACRO for turning off the mixer.
*       cog    09/01/19 Fixed issue where going from calibration mode 1 to
*                       any calibration mode caused the course mixer mode to
*                       be incorrect.
*       cog    09/19/19 Calibration mode 1 does not need the frequency shifting workaround
*                       for Gen 3 devices.
* 7.1   cog    11/28/19 Prevent setting non compliant mixer settings when in the bypass
*                       datapath mode.
*       cog    12/20/19 Metal log messages are now more descriptive.
*              12/23/19 Fabric rate is now auto-corrected when changing a miixer from IQ
*                       to real (and vice versa).
*       cog    01/29/20 Fixed metal log typos.
* 8.0   cog    02/10/20 Updated addtogroup.
*       cog    03/05/20 IMR datapath modes require the frequency word to be doubled.
*       cog    03/23/20 Relegate the datapath being in bypass mode to a warning when
*                       getting mixer parameters.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Explicitly set FIFO width when setting the mixer.
*       cog    10/06/20 Should only get calibration mode when setting/getting the
*                       mixer settings for Gen 1/2 devices.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static void XRFdc_SetFineMixer(XRFdc *InstancePtr, u32 BaseAddr, XRFdc_Mixer_Settings *MixerSettingsPtr);
static void XRFdc_SetCoarseMixer(XRFdc *InstancePtr, u32 Type, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				 u32 CoarseMixFreq, XRFdc_Mixer_Settings *MixerSettingsPtr);
static u32 XRFdc_MixerRangeCheck(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_Mixer_Settings *MixerSettingsPtr);
static void XRFdc_MixersOff(XRFdc *InstancePtr, u32 BaseAddr);

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* The API is used to update various mixer settings, fine, coarse, NCO etc.
* Mixer/NCO settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    MixerSettingsPtr Pointer to the XRFdc_Mixer_Settings structure
*           in which the Mixer/NCO settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     FineMixerScale in Mixer_Settings structure can have 3 values.
*           XRFDC_MIXER_SCALE_* represents the valid values.
*           XRFDC_MIXER_SCALE_AUTO - If mixer mode is R2C, Mixer Scale is
*           set to 1 and for other modes mixer scale is set to 0.7
*           XRFDC_MIXER_SCALE_1P0 - To set fine mixer scale to 1.
*           XRFDC_MIXER_SCALE_0P7 - To set fine mixer scale to 0.7.
*
******************************************************************************/
u32 XRFdc_SetMixerSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
			   XRFdc_Mixer_Settings *MixerSettingsPtr)
{
	u32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	double SamplingRate;
	s64 Freq;
	s32 PhaseOffset;
	u32 NoOfBlocks;
	u32 Index;
	XRFdc_Mixer_Settings *MixerConfigPtr;
	u8 CalibrationMode = 0U;
	u32 CoarseMixFreq;
	double NCOFreq;
	u32 NyquistZone = 0U;
	u32 Offset;
	u32 DatapathMode;
	u32 BWDiv = XRFDC_FULL_BW_DIVISOR;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MixerSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}
	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		if (Type == XRFDC_DAC_TILE) {
			DatapathMode = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id),
						   XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
			switch (DatapathMode) {
			case XRFDC_DAC_INT_MODE_FULL_BW_BYPASS:
				Status = XRFDC_FAILURE;
				metal_log(METAL_LOG_ERROR,
					  "\n Can't set mixer as DAC %u DUC %u is in bypass mode in %s\r\n", Tile_Id,
					  Block_Id, __func__);
				goto RETURN_PATH;
			case XRFDC_DAC_INT_MODE_HALF_BW_IMR:
				BWDiv = XRFDC_HALF_BW_DIVISOR;
				break;
			case XRFDC_DAC_INT_MODE_FULL_BW:
			default:
				BWDiv = XRFDC_FULL_BW_DIVISOR;
				break;
			}
		}
	}

	Status = XRFdc_MixerRangeCheck(InstancePtr, Type, Tile_Id, Block_Id, MixerSettingsPtr);
	if (Status != XRFDC_SUCCESS) {
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
			/* ADC */
			MixerConfigPtr =
				&InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].Mixer_Settings;
			SamplingRate = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate;
		} else {
			/* DAC */
			MixerConfigPtr =
				&InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Index].Mixer_Settings;
			SamplingRate = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate / BWDiv;
		}

		BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Index);

		if (SamplingRate <= 0) {
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Incorrect Sampling rate (%2.4f GHz) for %s %u in %s\r\n",
				  SamplingRate, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			goto RETURN_PATH;
		} else {
			metal_log(METAL_LOG_DEBUG, "\n Sampling rate is %2.4f GHz for %s %u in %s\r\n", SamplingRate,
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		}

		SamplingRate *= XRFDC_MILLI;
		/* Set MixerInputDataType for ADC and DAC */
		if (Type == XRFDC_DAC_TILE) {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_ITERP_DATA_OFFSET);
			ReadReg &= ~XRFDC_DAC_INTERP_DATA_MASK;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Index].MixerInputDataType =
				XRFDC_DATA_TYPE_REAL;
			if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
			    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
				ReadReg |= XRFDC_DAC_INTERP_DATA_MASK;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Index].MixerInputDataType =
					XRFDC_DATA_TYPE_IQ;
			}
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_ITERP_DATA_OFFSET, ReadReg);
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_DECI_CONFIG_OFFSET);
			ReadReg &= ~XRFDC_DEC_CFG_MASK;
			if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
				ReadReg |= XRFDC_DEC_CFG_4GSPS_MASK;
			} else if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
				   (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2C)) {
				ReadReg |= XRFDC_DEC_CFG_IQ_MASK;
			} else {
				ReadReg |= XRFDC_DEC_CFG_CHA_MASK;
			}
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_DECI_CONFIG_OFFSET, ReadReg);
			if (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType =
					XRFDC_DATA_TYPE_IQ;
			}
			if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2C) ||
			    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2R)) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType =
					XRFDC_DATA_TYPE_REAL;
			}
		}
		/* Set NCO Phase Mode */
		if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
			if ((Index == XRFDC_BLK_ID0) || (Index == XRFDC_BLK_ID2)) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_PHASE_MOD_OFFSET,
						 XRFDC_NCO_PHASE_MOD_EVEN);
			} else {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_PHASE_MOD_OFFSET,
						 XRFDC_NCO_PHASE_MODE_ODD);
			}
		}

		/* Update NCO, CoarseMix freq based on calibration mode */
		CoarseMixFreq = MixerSettingsPtr->CoarseMixFreq;
		NCOFreq = MixerSettingsPtr->Freq;
		if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (Type == XRFDC_ADC_TILE)) {
			Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id, Block_Id, &CalibrationMode);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if (CalibrationMode == XRFDC_CALIB_MODE1) {
				switch (CoarseMixFreq) {
				case XRFDC_COARSE_MIX_BYPASS:
					CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
					break;
				case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR:
					CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
					break;
				case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO:
					CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
					break;
				case XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR:
					CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
					break;
				default:
					CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
					break;
				}
				NCOFreq -= SamplingRate / 2.0;
			}
		}

		if ((NCOFreq < -(SamplingRate / 2.0)) || (NCOFreq > (SamplingRate / 2.0))) {
			Status = XRFdc_GetNyquistZone(InstancePtr, Type, Tile_Id, Block_Id, &NyquistZone);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			do {
				if (NCOFreq < -(SamplingRate / 2.0)) {
					NCOFreq += SamplingRate;
				}
				if (NCOFreq > (SamplingRate / 2.0)) {
					NCOFreq -= SamplingRate;
				}
			} while ((NCOFreq < -(SamplingRate / 2.0)) || (NCOFreq > (SamplingRate / 2.0)));

			if ((NyquistZone == XRFDC_EVEN_NYQUIST_ZONE) && (NCOFreq != 0)) {
				NCOFreq *= -1;
			}
		}

		/* NCO Frequency */
		Freq = ((NCOFreq * XRFDC_NCO_FREQ_MULTIPLIER) / SamplingRate);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_LOW_OFFSET, (u16)Freq);
		ReadReg = (Freq >> XRFDC_NCO_FQWD_MID_SHIFT) & XRFDC_NCO_FQWD_MID_MASK;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_MID_OFFSET, (u16)ReadReg);
		ReadReg = (Freq >> XRFDC_NCO_FQWD_UPP_SHIFT) & XRFDC_NCO_FQWD_UPP_MASK;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_UPP_OFFSET, (u16)ReadReg);

		/* Phase Offset */
		PhaseOffset = ((MixerSettingsPtr->PhaseOffset * XRFDC_NCO_PHASE_MULTIPLIER) /
			       XRFDC_MIXER_PHASE_OFFSET_UP_LIMIT);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_LOW_OFFSET, (u16)PhaseOffset);

		ReadReg = (PhaseOffset >> XRFDC_NCO_PHASE_UPP_SHIFT) & XRFDC_NCO_PHASE_UPP_MASK;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_UPP_OFFSET, ReadReg);

		switch (MixerSettingsPtr->MixerType) {
		case XRFDC_MIXER_TYPE_COARSE:
			XRFdc_SetCoarseMixer(InstancePtr, Type, BaseAddr, Tile_Id, Index, CoarseMixFreq,
					     MixerSettingsPtr);
			break;
		case XRFDC_MIXER_TYPE_FINE:
			XRFdc_SetFineMixer(InstancePtr, BaseAddr, MixerSettingsPtr);
			break;
		default:
			XRFdc_MixersOff(InstancePtr, BaseAddr);
			break;
		}

		/* Fine Mixer Scale */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET);
		if (MixerSettingsPtr->FineMixerScale == XRFDC_MIXER_SCALE_1P0) {
			ReadReg |= XRFDC_FINE_MIX_SCALE_MASK;
			InstancePtr->UpdateMixerScale = 0x1U;
		} else if (MixerSettingsPtr->FineMixerScale == XRFDC_MIXER_SCALE_0P7) {
			ReadReg &= ~XRFDC_FINE_MIX_SCALE_MASK;
			InstancePtr->UpdateMixerScale = 0x1U;
		} else {
			InstancePtr->UpdateMixerScale = 0x0U;
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, ReadReg);

		/* Event Source */
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_NCO_UPDT_OFFSET, XRFDC_NCO_UPDT_MODE_MASK,
				MixerSettingsPtr->EventSource);
		if (MixerSettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
			if (Type == XRFDC_ADC_TILE) {
				Offset = XRFDC_ADC_UPDATE_DYN_OFFSET;
			} else {
				Offset = XRFDC_DAC_UPDATE_DYN_OFFSET;
			}
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, Offset, XRFDC_UPDT_EVNT_MASK, XRFDC_UPDT_EVNT_NCO_MASK);
		}

		/* Update the instance with new values */
		MixerConfigPtr->EventSource = MixerSettingsPtr->EventSource;
		MixerConfigPtr->PhaseOffset = MixerSettingsPtr->PhaseOffset;
		MixerConfigPtr->MixerMode = MixerSettingsPtr->MixerMode;
		MixerConfigPtr->CoarseMixFreq = MixerSettingsPtr->CoarseMixFreq;
		MixerConfigPtr->Freq = MixerSettingsPtr->Freq;
		MixerConfigPtr->MixerType = MixerSettingsPtr->MixerType;
	}
	/*make sure the datapath side fifo width is correct for the given mode & rate change factor)*/
	Status = XRFdc_ResetInternalFIFOWidth(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* Static API used to do the Mixer Settings range check.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    MixerSettingsPtr Pointer to the XRFdc_Mixer_Settings structure
*           in which the Mixer/NCO settings are passed.
*
* @return
*           - XRFDC_SUCCESS if mixer settings are within the range.
*           - XRFDC_FAILURE if mixer settings are not in valid range
*
* @note     None.
*
******************************************************************************/
static u32 XRFdc_MixerRangeCheck(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_Mixer_Settings *MixerSettingsPtr)
{
	u32 Status;

	if ((MixerSettingsPtr->PhaseOffset >= XRFDC_MIXER_PHASE_OFFSET_UP_LIMIT) ||
	    (MixerSettingsPtr->PhaseOffset <= XRFDC_MIXER_PHASE_OFFSET_LOW_LIMIT)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid phase offset value (%lf) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->PhaseOffset, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->EventSource > XRFDC_EVNT_SRC_PL) ||
	    ((MixerSettingsPtr->EventSource == XRFDC_EVNT_SRC_MARKER) && (Type == XRFDC_ADC_TILE))) {
		metal_log(METAL_LOG_ERROR, "\n Invalid event source selection (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->EventSource, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (MixerSettingsPtr->MixerMode > XRFDC_MIXER_MODE_R2R) {
		metal_log(METAL_LOG_ERROR, "\n Invalid fine mixer mode in (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerMode, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_OFF) &&
	    (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO) &&
	    (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR) &&
	    (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR) &&
	    (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_BYPASS)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid coarse mix frequency value (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->CoarseMixFreq, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (MixerSettingsPtr->FineMixerScale > XRFDC_MIXER_SCALE_0P7) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Mixer Scale (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->PhaseOffset, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}
	if (((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2C) && (Type == XRFDC_DAC_TILE)) ||
	    ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R) && (Type == XRFDC_ADC_TILE))) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Mixer mode (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerMode, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->MixerType != XRFDC_MIXER_TYPE_FINE) &&
	    (MixerSettingsPtr->MixerType != XRFDC_MIXER_TYPE_COARSE) &&
	    (MixerSettingsPtr->MixerType != XRFDC_MIXER_TYPE_OFF)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid Mixer Type (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerType, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE) &&
	    ((MixerSettingsPtr->EventSource == XRFDC_EVNT_SRC_SLICE) ||
	     (MixerSettingsPtr->EventSource == XRFDC_EVNT_SRC_IMMEDIATE))) {
		Status = XRFDC_FAILURE;
		metal_log(
			METAL_LOG_ERROR,
			"\n Invalid Event Source (%u), event source is not supported in 4GSPS ADC for ADC %u block %u in %s\r\n",
			MixerSettingsPtr->EventSource, Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if (((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_COARSE) &&
	     (MixerSettingsPtr->CoarseMixFreq == XRFDC_COARSE_MIX_OFF)) ||
	    ((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_FINE) &&
	     (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_OFF))) {
		Status = XRFDC_FAILURE;
		metal_log(
			METAL_LOG_ERROR,
			"\n Invalid Combination of Mixer type (%u) Mixer mode (%u)/Coarse Mix Frequency (%u) for %s %u block %u in %s\r\n",
			MixerSettingsPtr->MixerType, MixerSettingsPtr->MixerMode, MixerSettingsPtr->CoarseMixFreq,
			(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_COARSE) &&
	    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_OFF)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Combination of Mixer type (%u) and Mixer mode (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerType, MixerSettingsPtr->MixerMode,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_FINE) &&
	    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2R)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Combination of Mixer type (%u) and Mixer mode (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerType, MixerSettingsPtr->MixerMode,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_COARSE) &&
	    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2R) &&
	    (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_BYPASS)) {
		Status = XRFDC_FAILURE;
		metal_log(
			METAL_LOG_ERROR,
			"\n Invalid Combination of Mixer type (%u) and Mixer mode (%u) (non-bypass) for %s %u block %u in %s\r\n",
			MixerSettingsPtr->MixerType, MixerSettingsPtr->MixerMode,
			(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	if ((MixerSettingsPtr->MixerType == XRFDC_MIXER_TYPE_OFF) &&
	    (MixerSettingsPtr->MixerMode != XRFDC_MIXER_MODE_OFF)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Combination of Mixer type (%u) and Mixer mode (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->MixerType, MixerSettingsPtr->MixerMode,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* Static API used to turn off Fine & Coarse Mixers.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddr is ADC or DAC base address.
*
* @return
*           - None
*
* @note     Static API
*
******************************************************************************/
static void XRFdc_MixersOff(XRFdc *InstancePtr, u32 BaseAddr)
{
	/* Coarse Mixer is OFF */
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, XRFDC_MIX_CFG0_MASK, XRFDC_CRSE_MIX_OFF);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, XRFDC_MIX_CFG1_MASK, XRFDC_CRSE_MIX_OFF);
	/* Fine mixer mode is OFF */
	XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, XRFDC_MIXER_MODE_OFF);
}

/*****************************************************************************/
/**
* Static API used to set the Fine Mixer.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddr is ADC or DAC base address.
* @param    MixerSettingsPtr Pointer to the XRFdc_Mixer_Settings structure
*           in which the Mixer/NCO settings are passed.
*
* @return
*           - None
*
* @note     Static API
*
******************************************************************************/
static void XRFdc_SetFineMixer(XRFdc *InstancePtr, u32 BaseAddr, XRFdc_Mixer_Settings *MixerSettingsPtr)
{
	if (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
				(XRFDC_SEL_I_IQ_MASK | XRFDC_SEL_Q_IQ_MASK | XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK),
				(XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK | XRFDC_I_IQ_COS_MINSIN | XRFDC_Q_IQ_SIN_COS));
	} else if (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
				(XRFDC_EN_I_IQ_MASK | XRFDC_SEL_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK),
				(XRFDC_EN_I_IQ_MASK | XRFDC_I_IQ_COS_MINSIN));
	} else if (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2C) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
				(XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK | XRFDC_SEL_I_IQ_MASK | XRFDC_SEL_Q_IQ_MASK |
				 XRFDC_FINE_MIX_SCALE_MASK),
				(XRFDC_EN_I_IQ | XRFDC_EN_Q_IQ | XRFDC_I_IQ_COS_MINSIN | XRFDC_Q_IQ_SIN_COS |
				 XRFDC_FINE_MIX_SCALE_MASK));
	} else {
		/* Fine mixer mode is OFF */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, XRFDC_MIXER_MODE_OFF);
	}

	/* Coarse Mixer is OFF */
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, XRFDC_MIX_CFG0_MASK, XRFDC_CRSE_MIX_OFF);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, XRFDC_MIX_CFG1_MASK, XRFDC_CRSE_MIX_OFF);
}

/*****************************************************************************/
/**
* Static API used to set the Coarse Mixer.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    BaseAddr is ADC or DAC base address.
* @param        Block_Id is ADC/DAC block number inside the tile.
* @param    CoarseMixFreq is ADC or DAC Coarse mixer frequency.
* @param    MixerSettingsPtr Pointer to the XRFdc_Mixer_Settings structure
*           in which the Mixer/NCO settings are passed.
*
* @return
*           - None
*
* @note     Static API
*
******************************************************************************/
static void XRFdc_SetCoarseMixer(XRFdc *InstancePtr, u32 Type, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				 u32 CoarseMixFreq, XRFdc_Mixer_Settings *MixerSettingsPtr)
{
	u16 ReadReg;

	if (CoarseMixFreq == XRFDC_COARSE_MIX_BYPASS) {
		/* Coarse Mix BYPASS */
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, XRFDC_MIX_CFG0_MASK,
				XRFDC_CRSE_MIX_BYPASS);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG1_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			ReadReg |= XRFDC_CRSE_MIX_BYPASS;
		} else {
			ReadReg |= XRFDC_CRSE_MIX_OFF;
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
	} else if (CoarseMixFreq == XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO) {
		/* Coarse Mix freq Fs/2 */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG0_MASK;
		if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
			ReadReg |= XRFDC_CRSE_MIX_I_Q_FSBYTWO;
		} else {
			if ((Block_Id % 2U) == 0U) {
				ReadReg |= XRFDC_CRSE_MIX_BYPASS;
			} else {
				ReadReg |= XRFDC_CRSE_4GSPS_ODD_FSBYTWO;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG1_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_I_Q_FSBYTWO;
			} else {
				ReadReg |=
					((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_BYPASS : XRFDC_CRSE_4GSPS_ODD_FSBYTWO;
			}
		} else {
			ReadReg |= XRFDC_CRSE_MIX_OFF;
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
	} else if (CoarseMixFreq == XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR) {
		/* Coarse Mix freq Fs/4 */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG0_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_I_FSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO :
								     XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
			}
		} else {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_R_I_FSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO : XRFDC_CRSE_MIX_OFF;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG1_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_Q_FSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO :
								     XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
			}
		} else {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_R_Q_FSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_OFF : XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
	} else if (CoarseMixFreq == XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR) {
		/* Coarse Mix freq -Fs/4 */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG0_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_I_MINFSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO :
								     XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
			}
		} else {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_R_I_MINFSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO : XRFDC_CRSE_MIX_OFF;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET);
		ReadReg &= ~XRFDC_MIX_CFG1_MASK;
		if ((MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2C) ||
		    (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_C2R)) {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_Q_MINFSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_I_Q_FSBYTWO :
								     XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
			}
		} else {
			if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
				ReadReg |= XRFDC_CRSE_MIX_R_Q_MINFSBYFOUR;
			} else {
				ReadReg |= ((Block_Id % 2U) == 0U) ? XRFDC_CRSE_MIX_OFF : XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
	} else if (CoarseMixFreq == XRFDC_COARSE_MIX_OFF) {
		/* Coarse Mix OFF */
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, XRFDC_MIX_CFG0_MASK,
				XRFDC_CRSE_MIX_OFF);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, XRFDC_MIX_CFG1_MASK,
				XRFDC_CRSE_MIX_OFF);
	} else {
		metal_log(METAL_LOG_ERROR, "\n Invalid Coarse Mixer frequency (%u) for %s %u block %u in %s\r\n",
			  MixerSettingsPtr->CoarseMixFreq, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
	}

	/* Fine mixer mode is OFF */
	XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, XRFDC_MIXER_MODE_OFF);
}

/*****************************************************************************/
/**
*
* The API returns back Mixer/NCO settings to the caller.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    MixerSettingsPtr Pointer to the XRFdc_Mixer_Settings structure
*           in which the Mixer/NCO settings are passed.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     FineMixerScale in Mixer_Settings structure can have 3 values.
*           XRFDC_MIXER_SCALE_* represents the valid values.
*           XRFDC_MIXER_SCALE_AUTO - If mixer mode is R2C, Mixer Scale is
*           set to 1 and for other modes mixer scale is set to 0.7
*           XRFDC_MIXER_SCALE_1P0 - To set fine mixer scale to 1.
*           XRFDC_MIXER_SCALE_0P7 - To set fine mixer scale to 0.7.
*
******************************************************************************/
u32 XRFdc_GetMixerSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
			   XRFdc_Mixer_Settings *MixerSettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u64 ReadReg;
	u64 ReadReg_Mix1;
	double SamplingRate;
	s64 Freq;
	s32 PhaseOffset;
	u32 Block;
	u8 CalibrationMode = 0U;
	XRFdc_Mixer_Settings *MixerConfigPtr;
	u32 NyquistZone = 0U;
	double NCOFreq;
	u32 FineMixerMode;
	u32 CoarseMixerMode = 0x0;
	u32 BWDiv = XRFDC_FULL_BW_DIVISOR;
	u32 DatapathMode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MixerSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}

	Block = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		if (Block_Id == XRFDC_BLK_ID1) {
			Block_Id = XRFDC_BLK_ID3;
		}
		if (Block_Id == XRFDC_BLK_ID0) {
			Block_Id = XRFDC_BLK_ID1;
		}
	}
	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		if (Type == XRFDC_DAC_TILE) {
			DatapathMode = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id),
						   XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
			switch (DatapathMode) {
			case XRFDC_DAC_INT_MODE_FULL_BW_BYPASS:
				metal_log(METAL_LOG_WARNING, "\n DAC %u DUC %u is in bypass mode in %s\r\n", Tile_Id,
					  Block_Id, __func__);
				BWDiv = XRFDC_FULL_BW_DIVISOR;
				break;
			case XRFDC_DAC_INT_MODE_HALF_BW_IMR:
				BWDiv = XRFDC_HALF_BW_DIVISOR;
				break;
			case XRFDC_DAC_INT_MODE_FULL_BW:
			default:
				BWDiv = XRFDC_FULL_BW_DIVISOR;
				break;
			}
		}
	}
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		SamplingRate = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate;
		MixerConfigPtr = &InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].Mixer_Settings;
	} else {
		/* DAC */
		SamplingRate = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate / BWDiv;
		MixerConfigPtr = &InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].Mixer_Settings;
	}

	if (SamplingRate <= 0) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Incorrect Sampling rate (%2.4f GHz) for %s %u in %s\r\n", SamplingRate,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);
	SamplingRate *= XRFDC_MILLI;
	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG0_OFFSET, XRFDC_MIX_CFG0_MASK);
	ReadReg_Mix1 = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_MXR_CFG1_OFFSET, XRFDC_MIX_CFG1_MASK);
	MixerSettingsPtr->CoarseMixFreq = 0x20;

	/* Identify CoarseMixFreq and CoarseMixerMode */
	if (ReadReg == XRFDC_CRSE_MIX_BYPASS) {
		if (ReadReg_Mix1 == XRFDC_CRSE_MIX_BYPASS) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if (ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2R;
			if (MixerConfigPtr->MixerMode == XRFDC_MIXER_MODE_R2C) {
				CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
			}
		}
	}
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_I_Q_FSBYTWO) && (ReadReg == XRFDC_CRSE_MIX_I_Q_FSBYTWO)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) && (ReadReg == XRFDC_CRSE_MIX_I_Q_FSBYTWO)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
		}
	} else {
		if (ReadReg == XRFDC_CRSE_4GSPS_ODD_FSBYTWO) {
			if (ReadReg_Mix1 == XRFDC_CRSE_4GSPS_ODD_FSBYTWO) {
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
				CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
			} else if (ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) {
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
				CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
			}
		}
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_Q_FSBYFOUR) && (ReadReg == XRFDC_CRSE_MIX_I_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_R_Q_FSBYFOUR) &&
			   (ReadReg == XRFDC_CRSE_MIX_R_I_MINFSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
		}
	} else {
		if ((ReadReg == XRFDC_CRSE_MIX_I_ODD_FSBYFOUR) && (ReadReg_Mix1 == XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if ((ReadReg == XRFDC_CRSE_MIX_OFF) && (ReadReg_Mix1 == XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
		}
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 0) || (Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_I_FSBYFOUR) && (ReadReg == XRFDC_CRSE_MIX_Q_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_R_Q_MINFSBYFOUR) &&
			   (ReadReg == XRFDC_CRSE_MIX_R_I_MINFSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
		}
	} else {
		if ((ReadReg == XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR) && (ReadReg_Mix1 == XRFDC_CRSE_MIX_I_ODD_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
		} else if ((ReadReg == XRFDC_CRSE_MIX_OFF) && (ReadReg_Mix1 == XRFDC_CRSE_MIX_I_ODD_FSBYFOUR)) {
			MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			CoarseMixerMode = XRFDC_MIXER_MODE_R2C;
		}
	}

	if ((ReadReg == XRFDC_CRSE_MIX_OFF) && (ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF)) {
		MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
		CoarseMixerMode = XRFDC_MIXER_MODE_C2C;
	}
	if (MixerSettingsPtr->CoarseMixFreq == 0x20U) {
		metal_log(METAL_LOG_ERROR,
			  "\n Coarse mixer settings not match any of the modes for %s %u block %u in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
	}
	if ((MixerConfigPtr->MixerMode == XRFDC_MIXER_MODE_C2R) && (CoarseMixerMode == XRFDC_MIXER_MODE_C2C)) {
		CoarseMixerMode = XRFDC_MIXER_MODE_C2R;
	}

	/* Identify FineMixerMode */
	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK));
	if (ReadReg == 0xFU) {
		FineMixerMode = XRFDC_MIXER_MODE_C2C;
	} else if (ReadReg == 0x3U) {
		FineMixerMode = XRFDC_MIXER_MODE_C2R;
	} else if (ReadReg == 0x5U) {
		FineMixerMode = XRFDC_MIXER_MODE_R2C;
	} else {
		FineMixerMode = XRFDC_MIXER_MODE_OFF;
	}

	if (FineMixerMode != XRFDC_MIXER_MODE_OFF) {
		MixerSettingsPtr->MixerType = XRFDC_MIXER_TYPE_FINE;
		MixerSettingsPtr->MixerMode = FineMixerMode;
	} else if (MixerSettingsPtr->CoarseMixFreq != XRFDC_COARSE_MIX_OFF) {
		MixerSettingsPtr->MixerType = XRFDC_MIXER_TYPE_COARSE;
		MixerSettingsPtr->MixerMode = CoarseMixerMode;
	} else {
		MixerSettingsPtr->MixerType = XRFDC_MIXER_TYPE_OFF;
		MixerSettingsPtr->MixerMode = XRFDC_MIXER_MODE_OFF;
	}

	/* Identify Fine Mixer Scale */
	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, XRFDC_FINE_MIX_SCALE_MASK);
	if (InstancePtr->UpdateMixerScale == 0x0U) {
		MixerSettingsPtr->FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
	} else if ((ReadReg != 0U) && (InstancePtr->UpdateMixerScale == 0x1U)) {
		MixerSettingsPtr->FineMixerScale = XRFDC_MIXER_SCALE_1P0;
	} else if (InstancePtr->UpdateMixerScale == 0x1U) {
		MixerSettingsPtr->FineMixerScale = XRFDC_MIXER_SCALE_0P7;
	} else {
		metal_log(METAL_LOG_ERROR, "\n Invalid Fine mixer scale in (%u) for %s %u block %u in %s\r\n",
			  InstancePtr->UpdateMixerScale, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	/* Phase Offset */
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_UPP_OFFSET);
	PhaseOffset = ReadReg << XRFDC_NCO_PHASE_UPP_SHIFT;
	PhaseOffset |= XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_LOW_OFFSET);
	PhaseOffset &= XRFDC_NCO_PHASE_MASK;
	PhaseOffset = ((PhaseOffset << 14) >> 14);
	MixerSettingsPtr->PhaseOffset = ((PhaseOffset * 180.0) / XRFDC_NCO_PHASE_MULTIPLIER);

	/* NCO Frequency */
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_UPP_OFFSET);
	Freq = ReadReg << XRFDC_NCO_FQWD_UPP_SHIFT;
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_MID_OFFSET);
	Freq |= ReadReg << XRFDC_NCO_FQWD_MID_SHIFT;
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_NCO_FQWD_LOW_OFFSET);
	Freq |= ReadReg;
	Freq &= XRFDC_NCO_FQWD_MASK;
	Freq = (Freq << 16) >> 16;
	MixerSettingsPtr->Freq = ((Freq * SamplingRate) / XRFDC_NCO_FREQ_MULTIPLIER);

	/* Event Source */
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_NCO_UPDT_OFFSET);
	MixerSettingsPtr->EventSource = ReadReg & XRFDC_NCO_UPDT_MODE_MASK;

	/* Update NCO, CoarseMix freq based on calibration mode */
	NCOFreq = MixerConfigPtr->Freq;

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (Type == XRFDC_ADC_TILE)) {
		Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id, Block, &CalibrationMode);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		if (CalibrationMode == XRFDC_CALIB_MODE1) {
			switch (MixerSettingsPtr->CoarseMixFreq) {
			case XRFDC_COARSE_MIX_BYPASS:
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
				break;
			case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR:
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
				break;
			case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO:
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
				MixerSettingsPtr->MixerMode = (MixerSettingsPtr->MixerMode == XRFDC_MIXER_MODE_R2C) ?
								      XRFDC_MIXER_MODE_R2R :
								      XRFDC_MIXER_MODE_C2C;
				break;
			case XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR:
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
				break;
			default:
				MixerSettingsPtr->CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				break;
			}
			NCOFreq = (MixerConfigPtr->Freq - (SamplingRate / 2.0));
		}
	}

	if ((NCOFreq > (SamplingRate / 2.0)) || (NCOFreq < -(SamplingRate / 2.0))) {
		Status = XRFdc_GetNyquistZone(InstancePtr, Type, Tile_Id, Block, &NyquistZone);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		if ((NyquistZone == XRFDC_EVEN_NYQUIST_ZONE) && (MixerSettingsPtr->Freq != 0)) {
			MixerSettingsPtr->Freq *= -1;
		}

		do {
			if (NCOFreq < -(SamplingRate / 2.0)) {
				NCOFreq += SamplingRate;
				MixerSettingsPtr->Freq -= SamplingRate;
			}
			if (NCOFreq > (SamplingRate / 2.0)) {
				NCOFreq -= SamplingRate;
				MixerSettingsPtr->Freq += SamplingRate;
			}
		} while ((NCOFreq > (SamplingRate / 2.0)) || (NCOFreq < -(SamplingRate / 2.0)));
	}
	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if ((Type == XRFDC_ADC_TILE) && (CalibrationMode == XRFDC_CALIB_MODE1)) {
			MixerSettingsPtr->Freq += (SamplingRate / 2.0);
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/** @} */

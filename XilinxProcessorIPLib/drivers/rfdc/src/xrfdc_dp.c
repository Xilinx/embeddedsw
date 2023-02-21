/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_dp.c
* @addtogroup Overview
* @{
*
* Contains the interface functions of the Digital Path Settings in XRFdc driver.
* See xrfdc.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 10.0  cog    11/26/20 Refactor and split files.
* 11.0  cog    05/31/21 Upversion.
* 11.1  cog    11/16/21 Upversion.
*       cog    11/26/21 Reset clock gaters when setting decimation rate.
*       cog    01/06/22 Check Nyquist zone compatibility when setting the
*                       inverse sinc filter.
*       cog    01/18/22 Added safety checks.
*       cog    01/24/22 Metal log change.
* 12.0  cog    09/01/22 Gen 3 devices now allowed to divide fabric clock
*                       by 1.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XRFdc_IntResetInternalFIFOWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 Channel);
/*****************************************************************************/
/*****************************************************************************/
/**
*
* This function is to set the decimation factor and also update the FIFO write
* words w.r.t to decimation factor for both the actual and observtion FIFOs.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
static u32 XRFdc_SetDecimationFactorInt(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecimationFactor,
					u32 Channel)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	u32 Factor;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	if ((DecimationFactor != XRFDC_INTERP_DECIM_OFF) && (DecimationFactor != XRFDC_INTERP_DECIM_1X) &&
	    (DecimationFactor != XRFDC_INTERP_DECIM_2X) && (DecimationFactor != XRFDC_INTERP_DECIM_4X) &&
	    (DecimationFactor != XRFDC_INTERP_DECIM_8X) &&
	    ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ||
	     ((DecimationFactor != XRFDC_INTERP_DECIM_3X) && (DecimationFactor != XRFDC_INTERP_DECIM_5X) &&
	      (DecimationFactor != XRFDC_INTERP_DECIM_6X) && (DecimationFactor != XRFDC_INTERP_DECIM_10X) &&
	      (DecimationFactor != XRFDC_INTERP_DECIM_12X) && (DecimationFactor != XRFDC_INTERP_DECIM_16X) &&
	      (DecimationFactor != XRFDC_INTERP_DECIM_20X) && (DecimationFactor != XRFDC_INTERP_DECIM_24X) &&
	      (DecimationFactor != XRFDC_INTERP_DECIM_40X)))) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Decimation factor value (%u) for ADC %u block %u in %s\r\n",
			  DecimationFactor, Tile_Id, Block_Id, __func__);
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

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);

		/* Decimation factor */
		Factor = DecimationFactor;
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			if (DecimationFactor == XRFDC_INTERP_DECIM_4X) {
				Factor = 0x3;
			}
			if (DecimationFactor == XRFDC_INTERP_DECIM_8X) {
				Factor = 0x4;
			}
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_OFFSET, XRFDC_DEC_MOD_MASK, Factor);
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_TDD_OFFSET(Channel),
					XRFDC_DEC_MOD_MASK_EXT, Factor);
		}
	}

	XRFdc_IntResetInternalFIFOWidth(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, Channel);

	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		BaseAddr = XRFDC_DRP_BASE(XRFDC_ADC_TILE, Tile_Id) + XRFDC_HSCOM_ADDR;
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1, XRFDC_CLK_NETWORK_CTRL1_EN_SYNC_MASK,
				XRFDC_CLK_NETWORK_CTRL1_EN_SYNC_MASK);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DIV_OFFSET, XRFDC_FAB_CLK_DIV_SYNC_PULSE_MASK,
				XRFDC_FAB_CLK_DIV_SYNC_PULSE_MASK);

		switch (DecimationFactor) {
		case XRFDC_INTERP_DECIM_1X:
		case XRFDC_INTERP_DECIM_2X:
		case XRFDC_INTERP_DECIM_4X:
		case XRFDC_INTERP_DECIM_8X:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_FIFO_START_TDD_OFFSET(Channel),
					XRFDC_ADC_FIFO_DELAY_MASK, XRFDC_FIFO_CHANNEL_ACT);
			break;
		case XRFDC_INTERP_DECIM_3X:
		case XRFDC_INTERP_DECIM_6X:
		case XRFDC_INTERP_DECIM_12X:
		case XRFDC_INTERP_DECIM_5X:
		case XRFDC_INTERP_DECIM_10X:
		case XRFDC_INTERP_DECIM_16X:
		case XRFDC_INTERP_DECIM_20X:
		case XRFDC_INTERP_DECIM_24X:
		case XRFDC_INTERP_DECIM_40X:
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_FIFO_START_TDD_OFFSET(Channel),
					XRFDC_ADC_FIFO_DELAY_MASK,
					XRFDC_ADC_CG_WAIT_CYCLES << XRFDC_ADC_FIFO_DELAY_SHIFT);
			break;
		default:
			metal_log(METAL_LOG_DEBUG, "\n Decimation block is OFF for DAC %u block %u in %s\r\n", Tile_Id,
				  Block_Id, __func__);
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
* This API is to set the decimation factor and also update the FIFO write
* words w.r.t to decimation factor.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
u32 XRFdc_SetDecimationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecimationFactor)
{
	return XRFdc_SetDecimationFactorInt(InstancePtr, Tile_Id, Block_Id, DecimationFactor, XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* This API is to set the decimation factor and also update the FIFO write
* words w.r.t to decimation factor for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only ADC blocks
*
******************************************************************************/
u32 XRFdc_SetDecimationFactorObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecimationFactor)
{
	return XRFdc_SetDecimationFactorInt(InstancePtr, Tile_Id, Block_Id, DecimationFactor, XRFDC_FIFO_CHANNEL_OBS);
}

/*****************************************************************************/
/**
*
* This function will trigger the update event for an event.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Event is for which dynamic update event will trigger.
*           XRFDC_EVENT_* defines the different events.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_UpdateEvent(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 Event)
{
	u32 Status;
	u32 BaseAddr;
	u32 EventSource;
	u32 NoOfBlocks;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
		if ((Event == XRFDC_EVENT_QMC) &&
		    (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ)) {
			Index = Block_Id;
			NoOfBlocks = XRFDC_NUM_OF_BLKS3;
			if (Block_Id == XRFDC_BLK_ID1) {
				NoOfBlocks = XRFDC_NUM_OF_BLKS4;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	if ((Event != XRFDC_EVENT_MIXER) && (Event != XRFDC_EVENT_QMC) && (Event != XRFDC_EVENT_CRSE_DLY)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Event value (%u) for %s %u block %u in %s\r\n", Event,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	for (; Index < NoOfBlocks;) {
		/* Identify the Event Source */
		BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Index);
		if (Event == XRFDC_EVENT_MIXER) {
			Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
			EventSource =
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_NCO_UPDT_OFFSET, XRFDC_NCO_UPDT_MODE_MASK);
		} else if (Event == XRFDC_EVENT_CRSE_DLY) {
			Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
			EventSource = XRFdc_RDReg(InstancePtr, BaseAddr,
						  (Type == XRFDC_ADC_TILE) ? XRFDC_ADC_CRSE_DLY_UPDT_OFFSET :
									     XRFDC_DAC_CRSE_DLY_UPDT_OFFSET,
						  XRFDC_QMC_UPDT_MODE_MASK);
		} else {
			Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
			EventSource =
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_QMC_UPDT_OFFSET, XRFDC_QMC_UPDT_MODE_MASK);
		}
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}

		if ((EventSource == XRFDC_EVNT_SRC_SYSREF) || (EventSource == XRFDC_EVNT_SRC_PL) ||
		    (EventSource == XRFDC_EVNT_SRC_MARKER)) {
			Status = XRFDC_FAILURE;
			metal_log(
				METAL_LOG_ERROR,
				"\n Invalid Event Source (%u), this should be issued external to the driver for %s %u block %u in %s\r\n",
				Event, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
		if (Type == XRFDC_ADC_TILE) {
			if (EventSource == XRFDC_EVNT_SRC_SLICE) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_UPDATE_DYN_OFFSET, 0x1);
			} else {
				BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_UPDT_DYN_OFFSET, 0x1);
			}
		} else {
			if (EventSource == XRFDC_EVNT_SRC_SLICE) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_UPDATE_DYN_OFFSET, 0x1);
			} else {
				BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_UPDT_DYN_OFFSET, 0x1);
			}
		}
		if ((Event == XRFDC_EVENT_QMC) &&
		    (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].MixerInputDataType ==
		     XRFDC_DATA_TYPE_IQ) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
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
* This API is to set the interpolation factor and also update the FIFO read
* words w.r.t to interpolation factor.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    InterpolationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only DAC blocks
*
******************************************************************************/
u32 XRFdc_SetInterpolationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 InterpolationFactor)
{
	u32 Status;
	u32 BaseAddr;
	u8 DataType;
	u32 Factor;
	u32 DatapathMode;
	u32 ReadPtrDelay;
	u32 CGNumerator;
	u32 CGDenominator;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	if ((InterpolationFactor != XRFDC_INTERP_DECIM_OFF) && (InterpolationFactor != XRFDC_INTERP_DECIM_1X) &&
	    (InterpolationFactor != XRFDC_INTERP_DECIM_2X) && (InterpolationFactor != XRFDC_INTERP_DECIM_4X) &&
	    (InterpolationFactor != XRFDC_INTERP_DECIM_8X) &&
	    ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ||
	     ((InterpolationFactor != XRFDC_INTERP_DECIM_3X) && (InterpolationFactor != XRFDC_INTERP_DECIM_5X) &&
	      (InterpolationFactor != XRFDC_INTERP_DECIM_6X) && (InterpolationFactor != XRFDC_INTERP_DECIM_10X) &&
	      (InterpolationFactor != XRFDC_INTERP_DECIM_12X) && (InterpolationFactor != XRFDC_INTERP_DECIM_16X) &&
	      (InterpolationFactor != XRFDC_INTERP_DECIM_20X) && (InterpolationFactor != XRFDC_INTERP_DECIM_24X) &&
	      (InterpolationFactor != XRFDC_INTERP_DECIM_40X)))) {
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid Interpolation factor divider value (%u) for DAC %u block %u in %s\r\n",
			  InterpolationFactor, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);

	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		DatapathMode = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
		if (DatapathMode == XRFDC_DAC_INT_MODE_FULL_BW_BYPASS) {
			Status = XRFDC_FAILURE;
			metal_log(
				METAL_LOG_ERROR,
				"\n Can't set interpolation mode as DUC is in bypass mode for DAC %u block %u in %s\r\n",
				Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	DataType = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_ITERP_DATA_OFFSET);
	if ((DataType == XRFDC_MIXER_MODE_IQ) && (InterpolationFactor == XRFDC_INTERP_DECIM_1X)) {
		Status = XRFDC_FAILURE;
		metal_log(
			METAL_LOG_ERROR,
			"\n Invalid interpolation factor (x1 interpolation factor in IQ mode) for DAC %u block %u in %s\r\n",
			Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	/* Interpolation factor */
	Factor = InterpolationFactor;

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if (InterpolationFactor == XRFDC_INTERP_DECIM_4X) {
			Factor = 0x3;
		}
		if (InterpolationFactor == XRFDC_INTERP_DECIM_8X) {
			Factor = 0x4;
		}
	}
	if (DataType == XRFDC_MIXER_MODE_IQ) {
		Factor |= Factor << ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_INTERP_MODE_Q_SHIFT :
										      XRFDC_INTERP_MODE_Q_SHIFT_EXT);
	}

	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET,
			(InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_INTERP_MODE_MASK :
									 XRFDC_INTERP_MODE_MASK_EXT,
			Factor);

	XRFdc_IntResetInternalFIFOWidth(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_FIFO_CHANNEL_ACT);

	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		switch (InterpolationFactor) {
		case XRFDC_INTERP_DECIM_1X:
		case XRFDC_INTERP_DECIM_2X:
		case XRFDC_INTERP_DECIM_4X:
		case XRFDC_INTERP_DECIM_8X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X1_X2_X4_X8;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X1_X2_X4_X8;
			break;
		case XRFDC_INTERP_DECIM_3X:
		case XRFDC_INTERP_DECIM_6X:
		case XRFDC_INTERP_DECIM_12X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X3_X6_X12;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X3_X6_X12;
			break;
		case XRFDC_INTERP_DECIM_5X:
		case XRFDC_INTERP_DECIM_10X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X5_X10;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X5_X10;
			break;
		case XRFDC_INTERP_DECIM_16X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X16;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X16;
			break;
		case XRFDC_INTERP_DECIM_20X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X20;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X20;
			break;
		case XRFDC_INTERP_DECIM_24X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X24;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X24;
			break;
		case XRFDC_INTERP_DECIM_40X:
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X40;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X40;
			break;
		default:
			metal_log(METAL_LOG_DEBUG, "\n Interpolation block is OFF for DAC %u block %u in %s\r\n",
				  Tile_Id, Block_Id, __func__);
			CGNumerator = XRFDC_CG_CYCLES_TOTAL_X1_X2_X4_X8;
			CGDenominator = XRFDC_CG_CYCLES_KEPT_X1_X2_X4_X8;
			break;
		}
		ReadPtrDelay = ((XRFDC_CG_WAIT_CYCLES * CGNumerator) / CGDenominator) +
			       (((XRFDC_CG_WAIT_CYCLES * CGNumerator) % CGDenominator) ? 1 : 0);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FIFO_START_OFFSET, XRFDC_DAC_FIFO_DELAY_MASK,
				ReadPtrDelay);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Interpolation factor are returned back to the caller.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    InterpolationFactorPtr Pointer to return the interpolation factor
*           for DAC blocks.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_GetInterpolationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *InterpolationFactorPtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InterpolationFactorPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		*InterpolationFactorPtr =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET, XRFDC_INTERP_MODE_I_MASK);
		if (*InterpolationFactorPtr == 0x3U) {
			*InterpolationFactorPtr = XRFDC_INTERP_DECIM_4X;
		} else if (*InterpolationFactorPtr == 0x4U) {
			*InterpolationFactorPtr = XRFDC_INTERP_DECIM_8X;
		}
	} else {
		*InterpolationFactorPtr =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET, XRFDC_INTERP_MODE_I_MASK_EXT);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decimation factor are returned back to the caller for both actual and
* observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactorPtr Pointer to return the Decimation factor
*           for DAC blocks.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
static u32 XRFdc_GetDecimationFactorInt(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecimationFactorPtr,
					u32 Channel)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DecimationFactorPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Block_Id);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		*DecimationFactorPtr =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_TDD_OFFSET(Channel), XRFDC_DEC_MOD_MASK);
		if (*DecimationFactorPtr == 0x3U) {
			*DecimationFactorPtr = XRFDC_INTERP_DECIM_4X;
		} else if (*DecimationFactorPtr == 0x4U) {
			*DecimationFactorPtr = XRFDC_INTERP_DECIM_8X;
		}
	} else {
		*DecimationFactorPtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_TDD_OFFSET(Channel),
						   XRFDC_DEC_MOD_MASK_EXT);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Decimation factor are returned back to the caller.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactorPtr Pointer to return the Decimation factor
*           for DAC blocks.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetDecimationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecimationFactorPtr)
{
	return XRFdc_GetDecimationFactorInt(InstancePtr, Tile_Id, Block_Id, DecimationFactorPtr,
					    XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* Decimation factor are returned back to the caller for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    DecimationFactorPtr Pointer to return the Decimation factor
*           for DAC blocks.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetDecimationFactorObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecimationFactorPtr)
{
	return XRFdc_GetDecimationFactorInt(InstancePtr, Tile_Id, Block_Id, DecimationFactorPtr,
					    XRFDC_FIFO_CHANNEL_OBS);
}

/*****************************************************************************/
/**
*
* This API is used to set the mode for the Inverse-Sinc filter.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Mode valid values are 0(disable),  1(1st Nyquist zone)
			and 2(2nd Nyquist zone).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not enabled/invalid mode.
*
* @note     Only DAC blocks
*
******************************************************************************/
u32 XRFdc_SetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u16 Mode)
{
	u32 Status;
	u32 BaseAddr;
	u32 NyquistZone;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Mode > ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_INV_SYNC_EN_MAX : XRFDC_INV_SYNC_MODE_MAX)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid mode value (%u) for DAC %u block %u in %s\r\n", Mode, Tile_Id,
			  Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_GetNyquistZone(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, &NyquistZone);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Could not determine Nyquist zone for DAC %u block %u in %s\r\n", Tile_Id,
			  Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((Mode != XRFDC_DISABLED) && (Mode != NyquistZone)) {
		metal_log(METAL_LOG_WARNING,
			  "\n Inverse Sinc mode and Nyquist Zone are incompatible for DAC %u block %u in %s\r\n",
			  Tile_Id, Block_Id, __func__);
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	XRFdc_ClrSetReg(
		InstancePtr, BaseAddr, XRFDC_DAC_INVSINC_OFFSET,
		(InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_EN_INVSINC_MASK : XRFDC_MODE_INVSINC_MASK, Mode);
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to get the Inverse-Sinc filter mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is DAC block number inside the tile. Valid values
*           are 0-3.
* @param    ModePtr is a pointer to get the inv-sinc status. valid values
*           are 0(disable),  1(1st Nyquist zone) and 2(2nd Nyquist zone).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only DAC blocks
*
******************************************************************************/
u32 XRFdc_GetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u16 *ModePtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ModePtr != NULL);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	*ModePtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INVSINC_OFFSET,
			       (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_EN_INVSINC_MASK :
										XRFDC_MODE_INVSINC_MASK);
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Checks whether DAC Digital path is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return 1 if DAC digital path is enabled, otherwise 0.
*
******************************************************************************/
u32 XRFdc_IsDACDigitalPathEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	u32 IsDigitalPathAvail;
	u32 DigitalPathShift;
	u32 DigitalPathEnableReg;

	DigitalPathShift = Block_Id + XRFDC_DIGITAL_PATH_ENABLED_SHIFT + (XRFDC_PATH_ENABLED_TILE_SHIFT * Tile_Id);
	DigitalPathEnableReg = XRFdc_ReadReg(InstancePtr, XRFDC_IP_BASE, XRFDC_DAC_PATHS_ENABLED_OFFSET);
	DigitalPathEnableReg &= (XRFDC_ENABLED << DigitalPathShift);
	IsDigitalPathAvail = DigitalPathEnableReg >> DigitalPathShift;
	return IsDigitalPathAvail;
}

/*****************************************************************************/
/**
*
* Checks whether ADC digital path is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3 in DAC/ADC-2GSPS and 0-1 in ADC-4GSPS.
*
* @return
*           - Return 1 if ADC digital path is enabled, otherwise 0.
*
******************************************************************************/
u32 XRFdc_IsADCDigitalPathEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	u32 IsDigitalPathAvail;
	u32 DigitalPathShift;
	u32 DigitalPathEnableReg;

	if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) {
		if ((Block_Id == 2U) || (Block_Id == 3U)) {
			IsDigitalPathAvail = 0;
			goto RETURN_PATH;
		}
		if (Block_Id == 1U) {
			Block_Id = 2U;
		}
	}

	DigitalPathShift = Block_Id + XRFDC_DIGITAL_PATH_ENABLED_SHIFT + (XRFDC_PATH_ENABLED_TILE_SHIFT * Tile_Id);
	DigitalPathEnableReg = XRFdc_ReadReg(InstancePtr, XRFDC_IP_BASE, XRFDC_ADC_PATHS_ENABLED_OFFSET);
	DigitalPathEnableReg &= (XRFDC_ENABLED << DigitalPathShift);
	IsDigitalPathAvail = DigitalPathEnableReg >> DigitalPathShift;

RETURN_PATH:
	return IsDigitalPathAvail;
}

/*****************************************************************************/
/**
*
* Checks whether ADC/DAC Digital path is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - XRFDC_SUCCESS if Digital path is enabled.
*           - XRFDC_FAILURE if Digital path is not enabled.
*
******************************************************************************/
u32 XRFdc_CheckDigitalPathEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 IsBlockAvail;
	u32 Status;

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((Tile_Id > XRFDC_TILE_ID_MAX) || (Block_Id > XRFDC_BLOCK_ID_MAX)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Type == XRFDC_ADC_TILE) {
		IsBlockAvail = XRFdc_IsADCDigitalPathEnabled(InstancePtr, Tile_Id, Block_Id);
	} else {
		IsBlockAvail = XRFdc_IsDACDigitalPathEnabled(InstancePtr, Tile_Id, Block_Id);
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
* This API is to set the divider for clock fabric out.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    FabClkDiv to be set for a tile.
*           XRFDC_FAB_CLK_* defines the valid divider values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC and DAC Tiles
*
******************************************************************************/
u32 XRFdc_SetFabClkOutDiv(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u16 FabClkDiv)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	if ((FabClkDiv != XRFDC_FAB_CLK_DIV1) && (FabClkDiv != XRFDC_FAB_CLK_DIV2) &&
	    (FabClkDiv != XRFDC_FAB_CLK_DIV4) && (FabClkDiv != XRFDC_FAB_CLK_DIV8) &&
	    (FabClkDiv != XRFDC_FAB_CLK_DIV16)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Fabric clock out divider value (%u) for %s %u in %s\r\n",
			  FabClkDiv, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;

	if ((Type == XRFDC_ADC_TILE) && (FabClkDiv == XRFDC_FAB_CLK_DIV1) &&
		(InstancePtr->RFdc_Config.IPType < XRFDC_GEN3)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid clock divider (%u) for ADC %u in %s\r\n", FabClkDiv, Tile_Id,
			  __func__);
		goto RETURN_PATH;
	} else {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DIV_OFFSET, XRFDC_FAB_CLK_DIV_MASK, FabClkDiv);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to get the divider for clock fabric out.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    FabClkDivPtr is a pointer to get fabric clock for a tile.
*           XRFDC_FAB_CLK_* defines the valid divider values.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     API is applicable for both ADC and DAC Tiles
*
******************************************************************************/
u32 XRFdc_GetFabClkOutDiv(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u16 *FabClkDivPtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FabClkDivPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;

	*FabClkDivPtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DIV_OFFSET, XRFDC_FAB_CLK_DIV_MASK);

	if ((*FabClkDivPtr < XRFDC_FAB_CLK_DIV1) || (*FabClkDivPtr > XRFDC_FAB_CLK_DIV16)) {
		*FabClkDivPtr = XRFDC_FAB_CLK_DIV16;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Fabric data rate for the requested DAC block is set by writing to the
* corresponding register. The function writes the number of valid write words
* for the requested DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricWrVldWords is write fabric rate to be set for DAC block.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_SetFabWrVldWords(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricWrVldWords)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	if (FabricWrVldWords > XRFDC_DAC_MAX_WR_FAB_RATE) {
		metal_log(METAL_LOG_ERROR,
			  "\n Requested write valid words is Invalid (%u) for DAC %u block %u in %s\r\n",
			  FabricWrVldWords, Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_WR_MASK,
			FabricWrVldWords);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Fabric data rate for the requested ADC block is set by writing to the
* corresponding register. The function writes the number of valid read words
* for the requested ADC block. This is for both the actual and observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWords is Read fabric rate to be set for ADC block.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
static u32 XRFdc_SetFabRdVldWordsInt(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricRdVldWords, u32 Channel)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) && (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u digital path %u not enabled in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	if (FabricRdVldWords > XRFDC_ADC_MAX_RD_FAB_RATE(InstancePtr->RFdc_Config.IPType)) {
		metal_log(METAL_LOG_ERROR,
			  "\n Requested read valid words is Invalid (%u) for ADC %u block %u in %s\r\n",
			  FabricRdVldWords, Tile_Id, Block_Id, __func__);
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

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_TDD_OFFSET(Channel),
				XRFDC_ADC_FAB_RATE_RD_MASK, (FabricRdVldWords << XRFDC_FAB_RATE_RD_SHIFT));
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Fabric data rate for the requested ADC block is set by writing to the
* corresponding register. The function writes the number of valid read words
* for the requested ADC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWords is Read fabric rate to be set for ADC block.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetFabRdVldWords(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricRdVldWords)
{
	return XRFdc_SetFabRdVldWordsInt(InstancePtr, Tile_Id, Block_Id, FabricRdVldWords, XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* Fabric data rate for the requested ADC block is set by writing to the
* corresponding register. The function writes the number of valid read words
* for the requested ADC block. This is for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWords is Read fabric rate to be set for ADC block.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetFabRdVldWordsObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricRdVldWords)
{
	return XRFdc_SetFabRdVldWordsInt(InstancePtr, Tile_Id, Block_Id, FabricRdVldWords, XRFDC_FIFO_CHANNEL_OBS);
}

/*****************************************************************************/
/**
*
* This function returns the the number of fabric write valid words requested
* for the block. For ADCs this is for both the actual and observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricWrVldWordsPtr Pointer to return the fabric data rate for
*           DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC/DAC blocks
*
******************************************************************************/
static u32 XRFdc_GetFabWrVldWordsInt(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricWrVldWordsPtr,
				     u32 Channel)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FabricWrVldWordsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) || (Type == XRFDC_DAC_TILE)) &&
	    (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u digital path %u not enabled in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);

	if (Type == XRFDC_ADC_TILE) {
		*FabricWrVldWordsPtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_TDD_OFFSET(Channel),
						   XRFDC_ADC_FAB_RATE_WR_MASK);
	} else {
		*FabricWrVldWordsPtr =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_WR_MASK);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* This API returns the number of fabric write valid words requested
* for the block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricWrVldWordsPtr Pointer to return the fabric data rate for
*           DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetFabWrVldWords(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricWrVldWordsPtr)
{
	return XRFdc_GetFabWrVldWordsInt(InstancePtr, Type, Tile_Id, Block_Id, FabricWrVldWordsPtr,
					 XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* This API returns the number of fabric write valid words requested
* for the block. This is for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricWrVldWordsPtr Pointer to return the fabric data rate for
*           DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC blocks only
*
******************************************************************************/
u32 XRFdc_GetFabWrVldWordsObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricWrVldWordsPtr)
{
	return XRFdc_GetFabWrVldWordsInt(InstancePtr, Type, Tile_Id, Block_Id, FabricWrVldWordsPtr,
					 XRFDC_FIFO_CHANNEL_OBS);
}

/*****************************************************************************/
/**
*
* This function returns the number of fabric read valid words requested
* for the block. For ADCs this is for both the actual and observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWordsPtr Pointer to return the fabric data rate for
*           ADC/DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC/DAC blocks
*
******************************************************************************/
static u32 XRFdc_GetFabRdVldWordsInt(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricRdVldWordsPtr,
				     u32 Channel)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FabricRdVldWordsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) || (Type == XRFDC_DAC_TILE)) &&
	    (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u digital path %u not enabled in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE)) {
		Block_Id = XRFDC_BLK_ID2;
	}
	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);

	if (Type == XRFDC_ADC_TILE) {
		*FabricRdVldWordsPtr =
			XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_TDD_OFFSET(Channel));
		*FabricRdVldWordsPtr = (*FabricRdVldWordsPtr) >> XRFDC_FAB_RATE_RD_SHIFT;
		*FabricRdVldWordsPtr &= XRFDC_ADC_FAB_RATE_WR_MASK;
	} else {
		*FabricRdVldWordsPtr = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET);
		*FabricRdVldWordsPtr = (*FabricRdVldWordsPtr) >> XRFDC_FAB_RATE_RD_SHIFT;
		*FabricRdVldWordsPtr &= XRFDC_DAC_FAB_RATE_WR_MASK;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* This API returns the number of fabric read valid words requested
* for the block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWordsPtr Pointer to return the fabric data rate for
*           ADC/DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetFabRdVldWords(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricRdVldWordsPtr)
{
	return XRFdc_GetFabRdVldWordsInt(InstancePtr, Type, Tile_Id, Block_Id, FabricRdVldWordsPtr,
					 XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* This function returns the number of fabric read valid words requested
* for the block. This is for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    FabricRdVldWordsPtr Pointer to return the fabric data rate for
*           ADC/DAC block
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC blocks only
*
******************************************************************************/
u32 XRFdc_GetFabRdVldWordsObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricRdVldWordsPtr)
{
	return XRFdc_GetFabRdVldWordsInt(InstancePtr, Type, Tile_Id, Block_Id, FabricRdVldWordsPtr,
					 XRFDC_FIFO_CHANNEL_OBS);
}
/*****************************************************************************/
/**
*
* Enable and Disable the ADC/DAC FIFO. For ADCs this is for the actual and
* observtion FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
static u32 XRFdc_SetupFIFOInt(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable, u32 Channel)
{
	u32 Status;
	u32 BaseAddr;
	u16 NoOfTiles;
	u16 Index;
	u32 DisableMask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Enable != 0U) && (Enable != 1U)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid enable value (%u) for %s %d in %s\r\n", Enable,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) || (Type == XRFDC_DAC_TILE)) &&
	    (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	/* An input tile if of -1 selects all tiles */
	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = XRFDC_NUM_OF_TILES4;
		Index = XRFDC_TILE_ID0;
	} else {
		NoOfTiles = Tile_Id + 1;
		Index = Tile_Id;
	}

	switch (Channel) {
	case XRFDC_FIFO_CHANNEL_ACT:
	default:
		DisableMask = XRFDC_FIFO_EN_MASK;
		break;
	case XRFDC_FIFO_CHANNEL_OBS:
		DisableMask = XRFDC_FIFO_EN_OBS_MASK;
		break;
	case XRFDC_FIFO_CHANNEL_BOTH:
		DisableMask = (XRFDC_FIFO_EN_MASK | XRFDC_FIFO_EN_OBS_MASK);
		break;
	}

	for (; Index < NoOfTiles; Index++) {
		BaseAddr = XRFDC_CTRL_STS_BASE(Type, Index);

		Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Index);
		if ((Status != XRFDC_SUCCESS) && (Tile_Id != XRFDC_SELECT_ALL_TILES)) {
			metal_log(METAL_LOG_ERROR, "\n %s %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Index, __func__);
			goto RETURN_PATH;
		} else if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Index, __func__);
			continue;
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_FIFO_ENABLE, DisableMask,
					((Enable == XRFDC_ENABLED) ? XRFDC_DISABLED : DisableMask));
		}
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Enable and Disable the ADC/DAC FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_SetupFIFO(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable)
{
	return XRFdc_SetupFIFOInt(InstancePtr, Type, Tile_Id, Enable, XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* Enable and Disable the ADC/DAC FIFO. For ADCs this is for the observtion FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC blocks only
*
******************************************************************************/
u32 XRFdc_SetupFIFOObs(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable)
{
	return XRFdc_SetupFIFOInt(InstancePtr, Type, Tile_Id, Enable, XRFDC_FIFO_CHANNEL_OBS);
}
/*****************************************************************************/
/**
*
* Enable and Disable the ADC/DAC FIFO. Thisis for the actual and observtion FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note    ADC blocks only
*
******************************************************************************/
u32 XRFdc_SetupFIFOBoth(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable)
{
	return XRFdc_SetupFIFOInt(InstancePtr, Type, Tile_Id, Enable, XRFDC_FIFO_CHANNEL_BOTH);
}
/*****************************************************************************/
/**
*
* Current status of ADC/DAC FIFO. For ADCs this is for both the actual and
* observations FIFOs.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    EnablePtr valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetFIFOStatusInt(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 *EnablePtr, u32 Channel)
{
	u32 Status;
	u32 BaseAddr;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(EnablePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (((InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) || (Type == XRFDC_DAC_TILE)) &&
	    (Channel != XRFDC_FIFO_CHANNEL_ACT)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);

	if (Channel == XRFDC_FIFO_CHANNEL_ACT) {
		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_FIFO_ENABLE, XRFDC_FIFO_EN_MASK);
	} else {
		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_FIFO_ENABLE, XRFDC_FIFO_EN_OBS_MASK) >>
			  XRFDC_FIFO_EN_OBS_SHIFT;
	}

	*EnablePtr = (!ReadReg);

	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Current status of ADC/DAC FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    EnablePtr valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetFIFOStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 *EnablePtr)
{
	return XRFdc_GetFIFOStatusInt(InstancePtr, Type, Tile_Id, EnablePtr, XRFDC_FIFO_CHANNEL_ACT);
}
/*****************************************************************************/
/**
*
* Current status of ADC/DAC FIFO. This is for the observation FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    EnablePtr valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC blocks only
*
******************************************************************************/
u32 XRFdc_GetFIFOStatusObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 *EnablePtr)
{
	return XRFdc_GetFIFOStatusInt(InstancePtr, Type, Tile_Id, EnablePtr, XRFDC_FIFO_CHANNEL_OBS);
}
/*****************************************************************************/
/**
* Set the correct FIFO width for current mixer & rate change settings (int).
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id Valid values are 0-3.
* @param    Channel Valid values are 0 for actual FIFO 1 for obs FIFO.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
static void XRFdc_IntResetInternalFIFOWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 Channel)
{
	u32 Factor;
	u32 BaseAddr;
	u32 DataType;
	u32 FabricRate;
	u32 Index;
	u32 NoOfBlocks;

	if (Type == XRFDC_ADC_TILE) {
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

		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			Factor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_OFFSET, XRFDC_DEC_MOD_MASK);
			if (Factor == 0x3U) {
				Factor = XRFDC_INTERP_DECIM_4X;
			} else if (Factor == 0x4U) {
				Factor = XRFDC_INTERP_DECIM_8X;
			}
		} else {
			Factor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_MODE_TDD_OFFSET(Channel),
					     XRFDC_DEC_MOD_MASK_EXT);
		}
		DataType = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DECI_CONFIG_TDD_OFFSET(Channel),
				       XRFDC_DEC_CFG_MASK);
		/* Fabric rate */
		FabricRate =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_OFFSET, XRFDC_ADC_FAB_RATE_WR_MASK);
		if ((DataType == XRFDC_DECIM_2G_IQ_DATA_TYPE) || (DataType == XRFDC_DECIM_4G_DATA_TYPE) ||
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			switch (Factor) {
			case XRFDC_INTERP_DECIM_1X:
				FabricRate = XRFDC_FAB_RATE_8;
				break;
			case XRFDC_INTERP_DECIM_2X:
			case XRFDC_INTERP_DECIM_3X:
				FabricRate = XRFDC_FAB_RATE_4;
				break;
			case XRFDC_INTERP_DECIM_4X:
			case XRFDC_INTERP_DECIM_5X:
			case XRFDC_INTERP_DECIM_6X:
				FabricRate = XRFDC_FAB_RATE_2;
				break;
			case XRFDC_INTERP_DECIM_8X:
			case XRFDC_INTERP_DECIM_10X:
			case XRFDC_INTERP_DECIM_12X:
			case XRFDC_INTERP_DECIM_16X:
			case XRFDC_INTERP_DECIM_20X:
			case XRFDC_INTERP_DECIM_24X:
			case XRFDC_INTERP_DECIM_40X:
				FabricRate = XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) ? XRFDC_FAB_RATE_1 :
											  XRFDC_FAB_RATE_2;
				break;
			default:
				metal_log(METAL_LOG_DEBUG, "\n Decimation block is OFF in ADC %u block %u in %s\r\n",
					  Tile_Id, Block_Id, __func__);
				break;
			}
		} else {
			switch (Factor) {
			case XRFDC_INTERP_DECIM_1X:
				FabricRate = XRFDC_FAB_RATE_4;
				break;
			case XRFDC_INTERP_DECIM_2X:
			case XRFDC_INTERP_DECIM_3X:
				FabricRate = XRFDC_FAB_RATE_2;
				break;
			case XRFDC_INTERP_DECIM_4X:
			case XRFDC_INTERP_DECIM_8X:
			case XRFDC_INTERP_DECIM_5X:
			case XRFDC_INTERP_DECIM_6X:
			case XRFDC_INTERP_DECIM_10X:
			case XRFDC_INTERP_DECIM_12X:
			case XRFDC_INTERP_DECIM_16X:
			case XRFDC_INTERP_DECIM_20X:
			case XRFDC_INTERP_DECIM_24X:
			case XRFDC_INTERP_DECIM_40X:
				FabricRate = XRFDC_FAB_RATE_1;
				break;
			default:
				metal_log(METAL_LOG_DEBUG, "\n Decimation block is OFF in %s\r\n", __func__);
				break;
			}
		}
		for (; Index < NoOfBlocks; Index++) {
			BaseAddr = XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_TDD_OFFSET(Channel),
					XRFDC_ADC_FAB_RATE_WR_MASK, FabricRate);
		}
	} else {
		BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			Factor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET,
					     XRFDC_INTERP_MODE_I_MASK);
			if (Factor == 0x3U) {
				Factor = XRFDC_INTERP_DECIM_4X;
			} else if (Factor == 0x4U) {
				Factor = XRFDC_INTERP_DECIM_8X;
			}
		} else {
			Factor = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET,
					     XRFDC_INTERP_MODE_I_MASK_EXT);
		}

		DataType = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_ITERP_DATA_OFFSET);

		/* Fabric rate */
		FabricRate =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_RD_MASK);
		FabricRate = FabricRate >> XRFDC_FAB_RATE_RD_SHIFT;
		if (DataType == XRFDC_MIXER_MODE_IQ) {
			switch (Factor) {
			case XRFDC_INTERP_DECIM_2X:
			case XRFDC_INTERP_DECIM_3X:
				FabricRate = XRFDC_FAB_RATE_8;
				break;
			case XRFDC_INTERP_DECIM_4X:
			case XRFDC_INTERP_DECIM_5X:
			case XRFDC_INTERP_DECIM_6X:
				FabricRate = XRFDC_FAB_RATE_4;
				break;
			case XRFDC_INTERP_DECIM_8X:
			case XRFDC_INTERP_DECIM_10X:
			case XRFDC_INTERP_DECIM_12X:
			case XRFDC_INTERP_DECIM_16X:
			case XRFDC_INTERP_DECIM_20X:
			case XRFDC_INTERP_DECIM_24X:
			case XRFDC_INTERP_DECIM_40X:
				FabricRate = XRFDC_FAB_RATE_2;
				break;
			default:
				metal_log(METAL_LOG_DEBUG,
					  "\n Interpolation block is OFF for DAC %u block %u in %s\r\n", Tile_Id,
					  Block_Id, __func__);
				break;
			}
		} else {
			switch (Factor) {
			case XRFDC_INTERP_DECIM_1X:
				FabricRate = XRFDC_FAB_RATE_8;
				break;
			case XRFDC_INTERP_DECIM_2X:
			case XRFDC_INTERP_DECIM_3X:
				FabricRate = XRFDC_FAB_RATE_4;
				break;
			case XRFDC_INTERP_DECIM_4X:
			case XRFDC_INTERP_DECIM_5X:
			case XRFDC_INTERP_DECIM_6X:
				FabricRate = XRFDC_FAB_RATE_2;
				break;
			case XRFDC_INTERP_DECIM_8X:
			case XRFDC_INTERP_DECIM_10X:
			case XRFDC_INTERP_DECIM_12X:
			case XRFDC_INTERP_DECIM_16X:
			case XRFDC_INTERP_DECIM_20X:
			case XRFDC_INTERP_DECIM_24X:
			case XRFDC_INTERP_DECIM_40X:
				FabricRate = XRFDC_FAB_RATE_1;
				break;
			default:
				metal_log(METAL_LOG_DEBUG,
					  "\n Interpolation block is OFF for DAC %u block %u in %s\r\n", Tile_Id,
					  Block_Id, __func__);
				break;
			}
		}
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_RD_MASK,
				(FabricRate << XRFDC_FAB_RATE_RD_SHIFT));
	}
}

/*****************************************************************************/
/**
* Set the correct FIFO width for current mixer & rate change settings.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id Valid values are 0-3.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_ResetInternalFIFOWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Invalid type (%u) in %s\r\n", Type, __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	XRFdc_IntResetInternalFIFOWidth(InstancePtr, Type, Tile_Id, Block_Id, XRFDC_FIFO_CHANNEL_ACT);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Set the correct Observation FIFO width for current mixer & rate change
* settings.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id Valid values are 0-3.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     ADC blocks only
*
******************************************************************************/
u32 XRFdc_ResetInternalFIFOWidthObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n ADC %u digital path %u not available in %s\r\n", Tile_Id, Block_Id,
			  __func__);
		goto RETURN_PATH;
	}

	XRFdc_IntResetInternalFIFOWidth(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_FIFO_CHANNEL_OBS);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get Fabric Clock frequency.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
*
* @return
*           - Return Fabric Clock frequency for ADC/DAC tile
*
******************************************************************************/
double XRFdc_GetFabClkFreq(XRFdc *InstancePtr, u32 Type, u32 Tile_Id)
{
	double FabClkFreq;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter type in %s\r\n", __func__);
		return 0.0;
	}
	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0.0;
	}

	if (Type == XRFDC_ADC_TILE) {
		FabClkFreq = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].FabClkFreq;
	} else {
		FabClkFreq = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].FabClkFreq;
	}

	return FabClkFreq;
}

/*****************************************************************************/
/**
*
* Get whether FIFO is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return 1 if FIFO is enabled, otherwise 0.
*
******************************************************************************/
u32 XRFdc_IsFifoEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 FifoEnable;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter type in %s\r\n", __func__);
		return 0U;
	}
	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter tile number in %s\r\n", __func__);
		return 0U;
	}
	if (Block_Id > XRFDC_BLOCK_ID_MAX) {
		metal_log(METAL_LOG_WARNING, "\n Invalid converter block number in %s\r\n", __func__);
		return 0U;
	}

	if (Type == XRFDC_ADC_TILE) {
		FifoEnable =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Digital_Config[Block_Id].FifoEnable;
	} else {
		FifoEnable =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Digital_Config[Block_Id].FifoEnable;
	}

	return FifoEnable;
}

/** @} */

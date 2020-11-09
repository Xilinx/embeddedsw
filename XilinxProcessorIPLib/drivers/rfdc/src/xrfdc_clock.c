/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_clock.c
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
* 6.0   cog    02/17/19 Initial release.
*       cog    03/12/19 Invert clock detection bits to support IP change.
*       cog    03/12/19 Fix bug where incorrect FS, RefClk and were output
*                       divider were being returned.
*       cog    04/09/19 Discriminate between Gen 3 IP and lower for checking
*                       if internal PLL is enabled.
*       cog    04/09/19 Fixed issue where tile was not autostarting after PLL
*                       rate change.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    08/02/19 Formatting changes and added a MACRO for the IP generation.
*       cog    09/18/19 XRFdc_GetClockSourceAPI must handle GEN 3 devices differently
*                       to previous generations.
*       cog    09/18/19 Account for different PLL settings for GEN 3 devices.
*       cog    09/18/19 Fixed issues with clock distribution functionallity.
*       cog    10/02/19 Updated PLL VCO ranges and reset divide bits while bypassing
*                       PLL output divider.
*       cog    10/02/19 Moved new external clock output divider functionallity from
*                       the clock distribution to XRFdc_DynamicPLLConfig() API.
*       cog    10/02/19 Refactor of XRFdc_GetClkDistribution() API.
* 7.1   cog    12/20/19 Metal log messages are now more descriptive.
*       cog    01/08/20 Changed clocking checks to allow ADC distribution to all
*                       ADC tiles.
*       cog    01/29/20 Fixed metal log typos.
* 8.0   cog    02/10/20 Updated addtogroup.
*       cog    03/20/20 Updated PowerState masks for Gen3.
*       cog    04/06/20 Fix GCC warnings.
* 8.1   cog    06/24/20 Upversion.
*       cog    08/11/20 Refactor of clock distribution settings.
*       cog    10/05/20 Change shutdown end state for Gen 3 Quad ADCs to reduce power
*                       consumption.
*       cog    11/09/20 Restrict division of external clock.
*                       PLL must be used if using ADC0, ADC3, DAC0 or DAC3 as a
*                       clock source.
*                       PLL must be used if distributing from DAC to ADC.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/
static u32 PllTuningMatrix[8][4][2] = {
	{ { 0x7F8A, 0x3FFF }, { 0x7F9C, 0x3FFF }, { 0x7FE2, 0x3FFF } },
	{ { 0x7FE9, 0xFFFF }, { 0x7F8E, 0xFFFF }, { 0x7F9C, 0xFFFF } },
	{ { 0x7F95, 0xFFFF }, { 0x7F8E, 0xFFFF }, { 0x7F9A, 0xFFFF }, { 0x7F8C, 0xFFFF } },
	{ { 0x7F95, 0x3FFF }, { 0x7FEE, 0x3FFF }, { 0x7F9A, 0xFFFF }, { 0x7F9C, 0xFFFF } },
	{ { 0x7F95, 0x3FFF }, { 0x7FEE, 0x3FFF }, { 0x7F9A, 0xFFFF }, { 0x7F9C, 0xFFFF } },
	{ { 0x7F95, 0xFFFF }, { 0x7F8E, 0xFFFF }, { 0x7FEA, 0xFFFF }, { 0x7F9C, 0xFFFF } },
	{ { 0x7FE9, 0xFFFF }, { 0x7F8E, 0xFFFF }, { 0x7F9A, 0xFFFF }, { 0x7F9C, 0xFFFF } },
	{ { 0x7FEC, 0xFFFF }, { 0x7FEE, 0x3FFF }, { 0x7F9C, 0xFFFF } }
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static u32 XRFdc_CheckClkDistValid(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr);
static u32 XRFdc_SetPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double RefClkFreq, double SamplingRate);

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function is used to set the clock settings
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type indicates ADC/DAC.
* @param    Tile_Id indicates Tile number (0-3).
* @param    SettingsPtr pointer to set the clock settings
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if no valid distribution found.
*
******************************************************************************/
static u32 XRFdc_SetTileClkSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, XRFdc_Tile_Clock_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u16 PLLSource;
	u16 NetworkCtrlReg;
	u16 DistCtrlReg;
	u16 PLLRefDivReg;
	u32 TileIndex;
	u32 PowerStateMaskReg;

	TileIndex = (Type == XRFDC_DAC_TILE) ? (XRFDC_CLK_DST_TILE_228 - Tile_Id) : (XRFDC_CLK_DST_TILE_224 - Tile_Id);
	if (SettingsPtr->DistributedClock > XRFDC_DIST_OUT_OUTDIV) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Parameter Value for Distribution Out (%u) for %s %u in %s\r\n",
			  SettingsPtr->DistributedClock, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (SettingsPtr->PLLEnable > XRFDC_ENABLED) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Parameter Value for PLLEnable (%u) for %s %u %s\r\n",
			  SettingsPtr->PLLEnable, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((SettingsPtr->SourceTile != TileIndex) && (SettingsPtr->DistributedClock != XRFDC_DIST_OUT_NONE)) {
		metal_log(METAL_LOG_ERROR, "\n Cannot Redistribute Clock in (%s %u is not a source tile)%s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	PLLSource = (SettingsPtr->PLLEnable == XRFDC_ENABLED) ? XRFDC_INTERNAL_PLL_CLK : XRFDC_EXTERNAL_CLK;
	Status = XRFdc_DynamicPLLConfig(InstancePtr, Type, Tile_Id, PLLSource, SettingsPtr->PLLSettings.RefClkFreq,
					SettingsPtr->PLLSettings.SampleRate);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Could not set up PLL settings for %s %u %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}
	(void)XRFdc_GetPLLConfig(InstancePtr, Type, Tile_Id, &(SettingsPtr->PLLSettings));
	SettingsPtr->DivisionFactor = SettingsPtr->PLLSettings.OutputDivider;

	DistCtrlReg = 0;
	PLLRefDivReg = 0;
	NetworkCtrlReg = 0;
	if (SettingsPtr->SourceTile == TileIndex) {
		if (SettingsPtr->DistributedClock == XRFDC_DIST_OUT_NONE) {
			if (SettingsPtr->PLLEnable == XRFDC_DISABLED) {
				PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_OFF;
				NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_DIST_T1;
				if (SettingsPtr->DivisionFactor < 2) {
					/*
					T1 from Self
					No PLL
					Do Not Use PLL Output Divider
					Do Not Distribute
					*/
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_T1_SRC_LOCAL;
					DistCtrlReg |= XRFDC_DIST_CTRL_CLK_T1_SRC_LOCAL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_SRC;
				} else {
					/*
					T1 from Self
					No PLL
					Use PLL Output Divider
					Do Not Distribute
					*/
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV_SRC;
				}
			} else {
				/*
				T1 from Self
				PLL
				Use PLL Output Divider
				Do Not Distribute
				*/
				NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_PLL;
				PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_RX_PLL;
			}
		} else {
			if (SettingsPtr->PLLEnable == XRFDC_DISABLED) {
				NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_DIST_T1;
				PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_OFF;
				if (SettingsPtr->DivisionFactor < 2) {
					/*
					T1 From Distribution (RX back)
					No PLL
					Do Not Use PLL Output Divider
					Send to Distribution
					*/
					PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_OFF;
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_T1_SRC_DIST;
					DistCtrlReg |= XRFDC_DIST_CTRL_TO_T1;
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_LOCAL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_SRC;
				} else if (SettingsPtr->DistributedClock == XRFDC_DIST_OUT_RX) {
					/*
					RX Back From Distribution
					No PLL
					Use PLL Output Divider
					Send to Distribution
					*/
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_INPUT_DIST;
					DistCtrlReg |= XRFDC_DIST_CTRL_TO_PLL_DIV;
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_LOCAL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV_SRC;
				} else {
					/*
					PLL Output Divider Back From Distribution
					No PLL
					Use PLL Output Divider
					Send to Distribution
					*/
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_DIST_T1;
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_T1_SRC_DIST;
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_INPUT_DIST;
					DistCtrlReg |= XRFDC_DIST_CTRL_TO_PLL_DIV;
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_PLL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV_SRC;
				}

			} else {
				/*
				RX Back From Distribution
				PLL
				Use PLL Output Divider
				Send to Distribution
				*/
				if (SettingsPtr->DistributedClock == XRFDC_DIST_OUT_RX) {
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_DIST_T1;
					PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_DIST;
					DistCtrlReg |= XRFDC_DIST_CTRL_TO_PLL_DIV;
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_LOCAL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_RX_PLL;
				} else {
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_T1_SRC_DIST;
					NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_REC_PLL;
					PLLRefDivReg |= XRFDC_PLLOPDIV_INPUT_DIST_LOCAL;
					DistCtrlReg |= XRFDC_DIST_CTRL_TO_T1;
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_PLL;
					PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_RX_PLL;
				}
			}
		}
	} else {
		if (SettingsPtr->PLLEnable == XRFDC_DISABLED) {
			PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_OFF;
			if (SettingsPtr->DivisionFactor > 1) {
				/*
				Source From Distribution
				No PLL
				Use PLL Output Divider
				Do Not Distribute
				*/
				NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_INPUT_DIST;
				DistCtrlReg |= XRFDC_DIST_CTRL_TO_PLL_DIV;
				PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV;
			} else {
				/*
				Source From Distribution
				No PLL
				Do Not Use PLL Output Divider
				Do Not Distribute
				*/
				NetworkCtrlReg |= XRFDC_NET_CTRL_CLK_T1_SRC_DIST;
				DistCtrlReg |= XRFDC_DIST_CTRL_TO_T1;
				PowerStateMaskReg = ((Type == XRFDC_ADC_TILE) ? XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV :
										XRFDC_HSCOM_PWR_STATS_DIST_EXT);
			}
		} else {
			/*
			Source From Distribution
			PLL
			Use PLL Output Divider
			Do Not Distribute
			*/
			PLLRefDivReg |= XRFDC_PLLREFDIV_INPUT_DIST;
			DistCtrlReg |= XRFDC_DIST_CTRL_TO_PLL_DIV;
			PowerStateMaskReg = XRFDC_HSCOM_PWR_STATS_DIST_PLL;
		}
	}

	/*Write to Registers*/
	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
	} else {
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);
	}
	BaseAddr += XRFDC_HSCOM_ADDR;
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK_ALT, DistCtrlReg);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1, XRFDC_HSCOM_NETWORK_CTRL1_MASK, NetworkCtrlReg);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_REFDIV, XRFDC_PLL_REFDIV_MASK, PLLRefDivReg);
	XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET, PowerStateMaskReg);
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to check the distribution chosen is valid
*
* @param        InstancePtr is a pointer to the XRfdc instance.
* @param        DistributionSettingsPtr pointer to the distribution settings struct
*
* @return
*           - XRFDC_SUCCESS if valid.
*           - XRFDC_FAILURE if not valid.
*
******************************************************************************/
static u32 XRFdc_CheckClkDistValid(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr)
{
	u32 Status;
	u32 Type;
	u32 Tile;
	u8 CurrentTile;
	u8 *Source;
	u8 Sources[8] = { DistributionSettingsPtr->DAC[3].SourceTile, DistributionSettingsPtr->DAC[2].SourceTile,
			  DistributionSettingsPtr->DAC[1].SourceTile, DistributionSettingsPtr->DAC[0].SourceTile,
			  DistributionSettingsPtr->ADC[3].SourceTile, DistributionSettingsPtr->ADC[2].SourceTile,
			  DistributionSettingsPtr->ADC[1].SourceTile, DistributionSettingsPtr->ADC[0].SourceTile };
	u16 EFuse;
	XRFdc_Distribution *DistributionPtr;

	/*init for first distribution*/
	DistributionPtr = DistributionSettingsPtr->DistributionStatus;
	Source = Sources;
	DistributionPtr->DistributionSource = DistributionSettingsPtr->DAC[3].SourceTile;
	DistributionPtr->Enabled = XRFDC_ENABLED;
	DistributionPtr->LowerBound = 0;

	for (CurrentTile = XRFDC_CLK_DST_TILE_231; CurrentTile < XRFDC_DIST_MAX; CurrentTile++, Source++) {
		if (CurrentTile < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			Type = XRFDC_DAC_TILE;
			Tile = XRFDC_CLK_DST_TILE_228 - CurrentTile;
		} else { /*ADC*/
			Type = XRFDC_ADC_TILE;
			Tile = XRFDC_CLK_DST_TILE_224 - CurrentTile;
		}
		if (XRFdc_CheckTileEnabled(InstancePtr, Type, Tile) != XRFDC_SUCCESS) {
			if (XRFDC_CLK_DST_TILE_231 != CurrentTile) {
				DistributionPtr++;
			}
			DistributionPtr->Enabled = XRFDC_DISABLED;
			DistributionPtr->LowerBound = CurrentTile;
			DistributionPtr->UpperBound = CurrentTile;
			DistributionPtr->DistributionSource = CurrentTile;
			continue;
		}
		if (*Source < DistributionPtr->DistributionSource) {
			Status = XRFDC_FAILURE; /*SW: no hopovers*/
			metal_log(METAL_LOG_ERROR, "\n Hopping Over Tiles Not Allowed in tile %u s%u d%u %s\r\n",
				  CurrentTile, *Source, DistributionPtr->DistributionSource, __func__);
			goto RETURN_PATH;
		} else if (*Source > DistributionPtr->DistributionSource) {
			DistributionPtr->UpperBound = CurrentTile - 1;
			DistributionPtr++;
			DistributionPtr->Enabled = XRFDC_ENABLED;
			DistributionPtr->DistributionSource = *Source;
			DistributionPtr->LowerBound = CurrentTile;
		}
		if (*Source >= XRFDC_DIST_MAX) {
			Status = XRFDC_FAILURE; /*out of range*/
			metal_log(METAL_LOG_ERROR, "\n Invalid Source value in %s - Out of Range\r\n", __func__);
			goto RETURN_PATH;
		}
		if (Sources[*Source] != *Source) { /*SW: check source is a distributer*/
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n Sourcing from Tile that is not Distributing in %s\r\n",
				  __func__);
			goto RETURN_PATH;
		}

		if (*Source < XRFDC_CLK_DST_TILE_227) {
			if ((DistributionSettingsPtr->DAC[XRFDC_CLK_DST_TILE_228 - *Source].DistributedClock ==
			     XRFDC_DIST_OUT_OUTDIV) ||
			    ((DistributionSettingsPtr->DAC[XRFDC_CLK_DST_TILE_228 - *Source].DistributedClock ==
			      XRFDC_DIST_OUT_RX) &&
			     (DistributionSettingsPtr->DAC[XRFDC_CLK_DST_TILE_228 - *Source].PLLEnable ==
			      XRFDC_DISABLED))) {
				if (Type == XRFDC_ADC_TILE) {
					Status = XRFDC_FAILURE;
					metal_log(
						METAL_LOG_ERROR,
						"\n Distribution full rate clock from DAC to ADC not Supported in %s\r\n",
						__func__);
					goto RETURN_PATH;
				} else if ((*Source == XRFDC_CLK_DST_TILE_228) || (*Source == XRFDC_CLK_DST_TILE_231)) {
					Status = XRFDC_FAILURE;
					metal_log(
						METAL_LOG_ERROR,
						"\n Distribution full rate clock from DAC edge tiles not Supported in %s\r\n",
						__func__);
					goto RETURN_PATH;
				}
			}
		} else {
			if ((DistributionSettingsPtr->ADC[XRFDC_CLK_DST_TILE_224 - *Source].DistributedClock ==
			     XRFDC_DIST_OUT_OUTDIV) ||
			    ((DistributionSettingsPtr->ADC[XRFDC_CLK_DST_TILE_224 - *Source].DistributedClock ==
			      XRFDC_DIST_OUT_RX) &&
			     (DistributionSettingsPtr->ADC[XRFDC_CLK_DST_TILE_224 - *Source].PLLEnable ==
			      XRFDC_DISABLED))) {
				if ((*Source == XRFDC_CLK_DST_TILE_224) || (*Source == XRFDC_CLK_DST_TILE_227)) {
					Status = XRFDC_FAILURE;
					metal_log(
						METAL_LOG_ERROR,
						"\n Distribution full rate clock from ADC edge tiles not Supported in %s\r\n",
						__func__);
					goto RETURN_PATH;
				}
			}
		}
		if ((CurrentTile < XRFDC_CLK_DST_TILE_227) &&
		    (*Source > XRFDC_CLK_DST_TILE_228)) { /*Cut between ADC0 MUX8 && DAC3 STH*/
			metal_log(METAL_LOG_ERROR, "\n DAC Cannot Source from ADC in %s\r\n", __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if (CurrentTile < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			EFuse = XRFdc_ReadReg16(InstancePtr,
						XRFDC_DRP_BASE(XRFDC_DAC_TILE, CurrentTile) + XRFDC_HSCOM_ADDR,
						XRFDC_HSCOM_EFUSE_2_OFFSET);
		} else { /*ADC*/
			EFuse = XRFdc_ReadReg16(InstancePtr,
						XRFDC_DRP_BASE(XRFDC_ADC_TILE, (CurrentTile - XRFDC_CLK_DST_TILE_227)) +
							XRFDC_HSCOM_ADDR,
						XRFDC_HSCOM_EFUSE_2_OFFSET);
		}
		if ((DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->DAC[1].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->DAC[2].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->DAC[3].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->ADC[3].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->ADC[2].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->ADC[1].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].SourceTile != DistributionSettingsPtr->ADC[0].SourceTile) ||
		    (DistributionSettingsPtr->DAC[0].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->DAC[1].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->DAC[2].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->DAC[3].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->ADC[0].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->ADC[1].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->ADC[2].PLLEnable != XRFDC_ENABLED) ||
		    (DistributionSettingsPtr->ADC[3].PLLEnable != XRFDC_ENABLED)) { /*special case that is allowed.*/

			if (EFuse & XRFDC_PREMIUMCTRL_CLKDIST) {
				if ((CurrentTile > XRFDC_CLK_DST_TILE_226) &&
				    (*Source < XRFDC_CLK_DST_TILE_227)) { /*E: no dist past adc2*/
					Status = XRFDC_FAILURE;
					metal_log(METAL_LOG_ERROR, "\n Invalid Configuration in %s\r\n", __func__);
					goto RETURN_PATH;
				}
			}
		}
		DistributionPtr->UpperBound = CurrentTile;
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	if (Status == XRFDC_FAILURE) {
		memset(DistributionSettingsPtr, 0, sizeof(XRFdc_Distribution_Settings));
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to wait for a tile to reach a given state.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type represents ADC or DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    State represents the state which the tile must reach.
*
* @return
*           - XRFDC_SUCCESS if valid.
*           - XRFDC_FAILURE if not valid.
*
******************************************************************************/
static u32 XRFdc_WaitForState(XRFdc *InstancePtr, u32 Type, u32 Tile, u32 State)
{
	u32 DelayCount;
	u32 TileState;

	TileState = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), 0xC, 0xF);
	DelayCount = 0;
	while (TileState < State) {
		if (DelayCount == 10000) {
			metal_log(METAL_LOG_ERROR, "\n timeout error in %s[%u] going to state %u in %s\r\n",
				  (Type ? "DAC" : "ADC"), Tile, State, __func__);
			return XRFDC_FAILURE;
		} else {
			/* Wait for 0.1 msec */
#ifdef __BAREMETAL__
			usleep(100);
#else
			metal_sleep_usec(100);
#endif
			DelayCount++;
			TileState = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), 0xC, 0xF);
		}
	}
	return XRFDC_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to start the distribution tiles
*
* @param        InstancePtr is a pointer to the XRfdc instance.
* @param        DistributionSettingsPtr pointer to the distribution settings struct
*
* @return
*           - XRFDC_SUCCESS if valid.
*           - XRFDC_FAILURE if not valid.
*
******************************************************************************/
static u32 XRFdc_StartUpDist(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr)
{
	u32 Status;
	u32 GlobalTile;
	u32 Type;
	u32 Tile;
	u32 DistributionCount;
	XRFdc_Distribution *Distribution;
	u32 i;
	u32 enables[8];
	u32 BaseAddr;

	Status = XRFDC_SUCCESS;
	for (Distribution = DistributionSettingsPtr->DistributionStatus, DistributionCount = 0;
	     DistributionCount < XRFDC_DIST_MAX; Distribution++, DistributionCount++) {
		if (Distribution->Enabled == XRFDC_DISABLED) {
			continue;
		}
		/*Fully Start Source Tile*/
		if ((Distribution->DistributionSource) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			Type = XRFDC_DAC_TILE;
			Tile = XRFDC_CLK_DST_TILE_228 - Distribution->DistributionSource;
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile);
		} else { /*ADC*/
			Type = XRFDC_ADC_TILE;
			Tile = XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource;
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile);
		}
		BaseAddr += XRFDC_HSCOM_ADDR;
		enables[DistributionCount] = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1, 0x3);
	}

	/*DAC0 Must reach state 4 in Startup FSM*/
	for (i = 0; i < 4; i++) {
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_DAC_TILE, i), 8, 0x10f, 0x10F);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_ADC_TILE, i), 8, 0x10f, 0x10F);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_DAC_TILE, i), 4, 0x1, 1);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_ADC_TILE, i), 4, 0x1, 1);
	}
	for (Distribution = DistributionSettingsPtr->DistributionStatus, DistributionCount = 0;
	     DistributionCount < XRFDC_DIST_MAX; Distribution++, DistributionCount++) {
		if (Distribution->Enabled == XRFDC_DISABLED) {
			continue;
		}
		/*Fully Start Source Tile*/
		if ((Distribution->DistributionSource) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			Type = XRFDC_DAC_TILE;
			Tile = XRFDC_CLK_DST_TILE_228 - Distribution->DistributionSource;
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile);
		} else { /*ADC*/
			Type = XRFDC_ADC_TILE;
			Tile = XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource;
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile);
		}
		BaseAddr += XRFDC_HSCOM_ADDR;
		Status |= XRFdc_WaitForState(InstancePtr, Type, Tile, 7);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1, 0x3, enables[DistributionCount]);
		Status = XRFdc_WaitForState(InstancePtr, Type, Tile, 0xF);
		for (GlobalTile = Distribution->LowerBound; GlobalTile <= Distribution->UpperBound; GlobalTile++) {
			if (GlobalTile < XRFDC_CLK_DST_TILE_227) { /*DAC*/
				Type = XRFDC_DAC_TILE;
				Tile = XRFDC_CLK_DST_TILE_228 - GlobalTile;
			} else { /*ADC*/
				Type = XRFDC_ADC_TILE;
				Tile = XRFDC_CLK_DST_TILE_224 - GlobalTile;
			}
			Status |= XRFdc_WaitForState(InstancePtr, Type, Tile, 0xF);
		}
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set the clock distribution
*
* @param        InstancePtr is a pointer to the XRfdc instance.
* @param        DistributionSettingsPtr pointer to the distribution settings struct
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if could not set distribution.
*
******************************************************************************/
u32 XRFdc_SetClkDistribution(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr)
{
	u32 Status;
	u32 StatusNonBlocking;
	u8 DelayLeft;
	u8 DelayRight;
	s8 Delay;
	s8 ClkDetItr;
	u8 *Delays[8];
	u8 DelayOutSourceLeft;
	u8 DelayOutSourceRight;
	XRFdc_Distribution *Distribution;
	u8 DistributionCount;
	u16 Reg;
	u16 ClkDetectReg;
	u8 FeedBackForInputRight = 0;
	u8 FeedBackForInputLeft = 0;
	u32 Tile;
	u32 Type;
	XRFdc_Tile_Clock_Settings *SettingsPtr;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DistributionSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested fuctionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}
	memset(DistributionSettingsPtr->DistributionStatus, 0, sizeof(DistributionSettingsPtr->DistributionStatus));
	Status = XRFdc_CheckClkDistValid(InstancePtr, DistributionSettingsPtr);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Distribution in %s\r\n", __func__);
		goto RETURN_PATH;
	}
	Delays[0] = &DistributionSettingsPtr->DAC[3].Delay;
	Delays[1] = &DistributionSettingsPtr->DAC[2].Delay;
	Delays[2] = &DistributionSettingsPtr->DAC[1].Delay;
	Delays[3] = &DistributionSettingsPtr->DAC[0].Delay;
	Delays[4] = &DistributionSettingsPtr->ADC[3].Delay;
	Delays[5] = &DistributionSettingsPtr->ADC[2].Delay;
	Delays[6] = &DistributionSettingsPtr->ADC[1].Delay;
	Delays[7] = &DistributionSettingsPtr->ADC[0].Delay;

	for (Tile = 0; Tile < XRFDC_NUM_OF_TILES4; Tile++) {
		Status = XRFdc_CheckTileEnabled(InstancePtr, XRFDC_ADC_TILE, Tile);
		if (Status != XRFDC_SUCCESS) {
			continue;
		}
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_ADC_TILE, Tile), XRFDC_RESTART_STATE_OFFSET,
				XRFDC_PWR_STATE_MASK, (XRFDC_SM_STATE1 << XRFDC_RSR_START_SHIFT) | XRFDC_SM_STATE1);
		/* Trigger restart */
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(XRFDC_ADC_TILE, Tile), XRFDC_RESTART_OFFSET,
			       XRFDC_RESTART_MASK);
		Status |= XRFdc_WaitForState(InstancePtr, XRFDC_ADC_TILE, Tile, XRFDC_SM_STATE1);
		if (Status != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}
	Status = XRFdc_Shutdown(InstancePtr, XRFDC_DAC_TILE, -1);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	StatusNonBlocking = XRFDC_SUCCESS;
	for (Distribution = DistributionSettingsPtr->DistributionStatus, DistributionCount = 0;
	     DistributionCount < XRFDC_DIST_MAX; Distribution++, DistributionCount++) {
		if (Distribution->Enabled == XRFDC_DISABLED) {
			continue;
		}
		DelayLeft = (-Distribution->LowerBound + Distribution->DistributionSource);
		DelayRight = (Distribution->UpperBound - Distribution->DistributionSource);
		DelayOutSourceLeft = 0;
		DelayOutSourceRight = 0;
		Distribution->MaxDelay = 0;
		Distribution->MinDelay = 255;
		Distribution->IsDelayBalanced = 0;
		if ((DelayLeft == 0) && (DelayRight == 0)) { /*self contained*/
			Reg = XRFDC_CLK_DISTR_OFF;
		} else {
			Reg = XRFDC_CLK_DISTR_MUX9_SRC_INT;
			if (DelayLeft == 0) {
				Reg |= XRFDC_CLK_DISTR_MUX8_SRC_NTH;
			} else {
				Reg |= XRFDC_CLK_DISTR_MUX8_SRC_INT;
			}
			if (((Distribution->DistributionSource == XRFDC_CLK_DST_TILE_228) ||
			     (Distribution->DistributionSource == XRFDC_CLK_DST_TILE_224)) &&
			    ((DelayLeft > 1) || (DelayRight > 0))) {
				Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_NTH |
				       XRFDC_CLK_DISTR_MUX7_SRC_INT;
				FeedBackForInputRight = 1;
				FeedBackForInputLeft = 0;
			} else {
				if ((DelayLeft > 1) || ((DelayLeft == 1) && (DelayRight == 1)) ||
				    ((DelayLeft == 1) && (DelayRight == 1))) {
					Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH | XRFDC_CLK_DISTR_MUX6_SRC_NTH |
					       XRFDC_CLK_DISTR_MUX7_SRC_INT;
					DelayOutSourceRight = 2;
					FeedBackForInputRight = 0;
					FeedBackForInputLeft = 1;
				} else {
					FeedBackForInputRight = 1;
					FeedBackForInputLeft = 0;
					if ((DelayRight > 1) &&
					    (Distribution->DistributionSource != XRFDC_CLK_DST_TILE_229)) {
						Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
						Reg |= XRFDC_CLK_DISTR_MUX7_SRC_STH;
						DelayOutSourceLeft = 2;
					} else {
						if ((DelayLeft == 0) &&
						    (Distribution->DistributionSource != XRFDC_CLK_DST_TILE_228)) {
							Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH;
							Reg |= XRFDC_CLK_DISTR_MUX7_SRC_OFF;
						} else {
							Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
							Reg |= XRFDC_CLK_DISTR_MUX7_SRC_INT;
						}
					}
					if (DelayRight == 0) {
						Reg |= XRFDC_CLK_DISTR_MUX6_SRC_OFF;
					} else {
						Reg |= XRFDC_CLK_DISTR_MUX6_SRC_INT;
					}
				}
			}
		}

		*Delays[Distribution->DistributionSource] =
			(Reg == XRFDC_CLK_DISTR_OFF) ? 0 : DelayOutSourceLeft + DelayOutSourceRight + 2;
		Distribution->MaxDelay = MAX(Distribution->MaxDelay, (*Delays[Distribution->DistributionSource]));
		Distribution->MinDelay = MIN(Distribution->MinDelay, (*Delays[Distribution->DistributionSource]));

		/* setup clk detect register */
		ClkDetectReg =
			(XRFDC_CLOCK_DETECT_CLK << ((XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource) << 1));

		if ((Distribution->DistributionSource) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			Type = XRFDC_DAC_TILE;
			Tile = XRFDC_CLK_DST_TILE_228 - Distribution->DistributionSource;
			SettingsPtr = &DistributionSettingsPtr->DAC[Tile];
		} else { /*ADC*/
			Type = XRFDC_ADC_TILE;
			Tile = XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource;
			SettingsPtr = &DistributionSettingsPtr->ADC[Tile];
		}
		XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
				XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
				XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
		StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, SettingsPtr);

		/*Leftmost tile*/
		if (DelayLeft) {
			*Delays[Distribution->LowerBound] = DelayOutSourceLeft + (DelayLeft << 1);
			Distribution->MaxDelay = MAX(Distribution->MaxDelay, (*Delays[Distribution->LowerBound]));
			Distribution->MinDelay = MIN(Distribution->MinDelay, (*Delays[Distribution->LowerBound]));
			Reg = XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX8_SRC_INT |
			      XRFDC_CLK_DISTR_MUX9_SRC_INT;
			if ((Distribution->DistributionSource != XRFDC_CLK_DST_TILE_228) && (DelayLeft == 1) &&
			    (DelayRight == 1)) {
				Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX7_SRC_STH;

			} else {
				Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH | XRFDC_CLK_DISTR_MUX7_SRC_OFF;
			}
			/* setup clk detect register */
			ClkDetectReg = (XRFDC_CLOCK_DETECT_BOTH
					<< ((XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource) << 1));
			for (ClkDetItr = DelayLeft - 1; ClkDetItr > 0; ClkDetItr--) {
				ClkDetectReg |=
					(XRFDC_CLOCK_DETECT_DIST
					 << ((XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource - ClkDetItr))
					     << 1));
			}

			if ((Distribution->DistributionSource - DelayLeft) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
				Type = XRFDC_DAC_TILE;
				Tile = XRFDC_CLK_DST_TILE_228 - (Distribution->DistributionSource - DelayLeft);
				SettingsPtr = &DistributionSettingsPtr->DAC[Tile];
			} else { /*ADC*/
				Type = XRFDC_ADC_TILE;
				Tile = XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource - DelayLeft);
				SettingsPtr = &DistributionSettingsPtr->ADC[Tile];
			}
			XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
					XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
			StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, SettingsPtr);
		}
		/*Rest of tiles left of Distribution->DistributionSource*/
		for (Delay = 1; Delay < DelayLeft; Delay++) {
			Reg = XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX7_SRC_STH |
			      XRFDC_CLK_DISTR_MUX8_SRC_INT | XRFDC_CLK_DISTR_MUX9_SRC_INT;
			if (FeedBackForInputLeft == 0) {
				Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH;
			} else {
				Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
			}
			*Delays[Distribution->DistributionSource - Delay] =
				DelayOutSourceLeft + ((Delay + FeedBackForInputLeft) << 1);
			Distribution->MaxDelay =
				MAX(Distribution->MaxDelay, (*Delays[Distribution->DistributionSource - Delay]));
			Distribution->MinDelay =
				MIN(Distribution->MinDelay, (*Delays[Distribution->DistributionSource - Delay]));
			FeedBackForInputLeft = !FeedBackForInputLeft;

			/* setup clk detect register */
			ClkDetectReg = (XRFDC_CLOCK_DETECT_BOTH
					<< ((XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource) << 1));
			for (ClkDetItr = Delay - 1; ClkDetItr > 0; ClkDetItr--) {
				ClkDetectReg |=
					(XRFDC_CLOCK_DETECT_DIST
					 << ((XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource - ClkDetItr))
					     << 1));
			}

			if ((Distribution->DistributionSource - Delay) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
				Type = XRFDC_DAC_TILE;
				Tile = XRFDC_CLK_DST_TILE_228 - (Distribution->DistributionSource - Delay);
				SettingsPtr = &DistributionSettingsPtr->DAC[Tile];
			} else { /*ADC*/
				Type = XRFDC_ADC_TILE;
				Tile = XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource - Delay);
				SettingsPtr = &DistributionSettingsPtr->ADC[Tile];
			}
			XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
					XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
			StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, SettingsPtr);
		}
		/*Rightmost tile*/
		if (DelayRight) {
			Reg = XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_OFF |
			      XRFDC_CLK_DISTR_MUX7_SRC_OFF | XRFDC_CLK_DISTR_MUX9_SRC_NTH;
			if ((DelayRight > 2) && (DelayRight % 2)) {
				Reg |= XRFDC_CLK_DISTR_MUX8_SRC_INT;
			}
			*Delays[Distribution->UpperBound] = DelayOutSourceRight + (DelayRight << 1);
			Distribution->MaxDelay = MAX(Distribution->MaxDelay, (*Delays[Distribution->UpperBound]));
			Distribution->MinDelay = MIN(Distribution->MinDelay, (*Delays[Distribution->UpperBound]));

			/* setup clk detect register */
			ClkDetectReg = (XRFDC_CLOCK_DETECT_BOTH
					<< ((XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource) << 1));
			for (ClkDetItr = DelayRight - 1; ClkDetItr > 0; ClkDetItr--) {
				ClkDetectReg |=
					(XRFDC_CLOCK_DETECT_DIST
					 << ((XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource + ClkDetItr))
					     << 1));
			}

			if ((Distribution->DistributionSource + DelayRight) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
				Type = XRFDC_DAC_TILE;
				Tile = XRFDC_CLK_DST_TILE_228 - (Distribution->DistributionSource + DelayRight);
				SettingsPtr = &DistributionSettingsPtr->DAC[Tile];
			} else { /*ADC*/
				Type = XRFDC_ADC_TILE;
				Tile = XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource + DelayRight);
				SettingsPtr = &DistributionSettingsPtr->ADC[Tile];
			}
			XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
					XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
			StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, SettingsPtr);
		}
		/*rest of tiles to right*/
		for (Delay = 1; Delay < DelayRight; Delay++) {
			if (((Delay + Distribution->DistributionSource) == XRFDC_CLK_DST_TILE_228) ||
			    (FeedBackForInputRight == 0)) {
				FeedBackForInputRight = 0;
				Reg = XRFDC_CLK_DISTR_MUX4A_SRC_INT;
				*Delays[Distribution->DistributionSource + Delay] = DelayOutSourceRight + (Delay << 1);
			} else {
				Reg = XRFDC_CLK_DISTR_MUX4A_SRC_STH;
				*Delays[Distribution->DistributionSource + Delay] =
					DelayOutSourceRight + ((Delay + 1) << 1);
			}
			Distribution->MaxDelay =
				MAX(Distribution->MaxDelay, (*Delays[Distribution->DistributionSource + Delay]));
			Distribution->MinDelay =
				MIN(Distribution->MinDelay, (*Delays[Distribution->DistributionSource + Delay]));
			FeedBackForInputRight = !FeedBackForInputRight;
			Reg |= XRFDC_CLK_DISTR_MUX6_SRC_NTH | XRFDC_CLK_DISTR_MUX7_SRC_OFF |
			       XRFDC_CLK_DISTR_MUX8_SRC_NTH | XRFDC_CLK_DISTR_MUX9_SRC_NTH;
			if ((Delay > 2) && (Delay % 2)) {
				Reg |= XRFDC_CLK_DISTR_MUX8_SRC_INT;
			}
			/* setup clk detect register */
			ClkDetectReg = (XRFDC_CLOCK_DETECT_BOTH
					<< ((XRFDC_CLK_DST_TILE_224 - Distribution->DistributionSource) << 1));
			for (ClkDetItr = Delay - 1; ClkDetItr > 0; ClkDetItr--) {
				ClkDetectReg |=
					(XRFDC_CLOCK_DETECT_DIST
					 << ((XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource + ClkDetItr))
					     << 1));
			}

			if ((Distribution->DistributionSource + Delay) < XRFDC_CLK_DST_TILE_227) { /*DAC*/
				Type = XRFDC_DAC_TILE;
				Tile = XRFDC_CLK_DST_TILE_228 - (Distribution->DistributionSource + Delay);
				SettingsPtr = &DistributionSettingsPtr->DAC[Tile];
			} else { /*ADC*/
				Type = XRFDC_ADC_TILE;
				Tile = XRFDC_CLK_DST_TILE_224 - (Distribution->DistributionSource + Delay);
				SettingsPtr = &DistributionSettingsPtr->ADC[Tile];
			}
			XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
					XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
			StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, SettingsPtr);
		}
		Distribution->IsDelayBalanced = (Distribution->MaxDelay == Distribution->MinDelay) ? 1 : 0;
	}
	/*start tiles*/
	Status = XRFdc_StartUpDist(InstancePtr, DistributionSettingsPtr);
	Status |= StatusNonBlocking;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to get the clock distribution
*
* @param        InstancePtr is a pointer to the XRfdc instance.
* @param        DistributionSettingsPtr pointer to get the distribution settings
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if no valid distribution found.
*
******************************************************************************/
u32 XRFdc_GetClkDistribution(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr)
{
	u32 Status;
	u32 ClockDetReg;
	u32 DistRegCurrent;
	u32 DistRegSrc;
	s8 CurrentTile;
	u8 DelayOutSourceLeft;
	u8 DelayOutSourceRight;
	XRFdc_Tile_Clock_Settings *ClockSettingsPtr;
	u32 Type;
	u32 SrcType;
	u32 Tile;
	u32 SrcTile;
	u8 Distribution;
	u8 i;
	u32 PrevSrc;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DistributionSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested fuctionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	memset(DistributionSettingsPtr, 0, sizeof(XRFdc_Distribution_Settings));

	PrevSrc = XRFDC_CLK_DST_TILE_231;
	for (CurrentTile = XRFDC_CLK_DST_TILE_231, Distribution = 0; CurrentTile <= XRFDC_CLK_DST_TILE_224;
	     CurrentTile++) {
		DelayOutSourceLeft = 0;
		DelayOutSourceRight = 0;
		if (CurrentTile < XRFDC_CLK_DST_TILE_227) { /*DAC*/
			Type = XRFDC_DAC_TILE;
			Tile = XRFDC_CLK_DST_TILE_228 - CurrentTile;
			ClockSettingsPtr = &DistributionSettingsPtr->DAC[Tile];
		} else { /*ADC*/
			Type = XRFDC_ADC_TILE;
			Tile = XRFDC_CLK_DST_TILE_224 - CurrentTile;
			ClockSettingsPtr = &DistributionSettingsPtr->ADC[Tile];
		}

		ClockDetReg = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					  XRFDC_CLOCK_DETECT_SRC_MASK);
		if (ClockDetReg == XRFDC_DISABLED) {
			ClockSettingsPtr->SourceTile = CurrentTile;
			ClockSettingsPtr->Delay = 0;
			ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_NONE;
			DistributionSettingsPtr->DistributionStatus[Distribution].Enabled = XRFDC_DISABLED;
			DistributionSettingsPtr->DistributionStatus[Distribution].DistributionSource = CurrentTile;
			DistributionSettingsPtr->DistributionStatus[Distribution].UpperBound = CurrentTile;
			DistributionSettingsPtr->DistributionStatus[Distribution].LowerBound = CurrentTile;
			DistributionSettingsPtr->DistributionStatus[Distribution].MaxDelay = 0;
			DistributionSettingsPtr->DistributionStatus[Distribution].MinDelay = 0;
			DistributionSettingsPtr->DistributionStatus[Distribution].IsDelayBalanced = 1;
			PrevSrc = CurrentTile;
			Distribution++;
		} else {
			for (i = XRFDC_CLK_DST_TILE_231; i <= XRFDC_CLK_DST_TILE_224; i++) {
				if ((ClockDetReg >> (i << 1)) == XRFDC_ENABLED) {
					ClockSettingsPtr->SourceTile = XRFDC_CLK_DST_TILE_224 - i;
					if (i < XRFDC_CLK_DST_TILE_227) {
						SrcType = XRFDC_DAC_TILE;
						SrcTile = XRFDC_CLK_DST_TILE_228 - i;
					} else {
						SrcType = XRFDC_ADC_TILE;
						SrcTile = XRFDC_CLK_DST_TILE_224 - i;
					}
					DistRegSrc =
						XRFdc_ReadReg16(InstancePtr,
								XRFDC_DRP_BASE(SrcType, SrcTile) + XRFDC_HSCOM_ADDR,
								XRFDC_HSCOM_CLK_DSTR_OFFSET);
					DistRegCurrent = XRFdc_ReadReg16(InstancePtr,
									 XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR,
									 XRFDC_HSCOM_CLK_DSTR_OFFSET);
					if ((DistRegCurrent & XRFDC_DIST_CTRL_DIST_SRC_LOCAL) ==
					    XRFDC_DIST_CTRL_DIST_SRC_LOCAL) {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_RX;
					} else if ((DistRegCurrent & XRFDC_DIST_CTRL_DIST_SRC_PLL) ==
						   XRFDC_DIST_CTRL_DIST_SRC_PLL) {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_OUTDIV;
					} else {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_NONE;
					}
					if (DistRegSrc & XRFDC_CLK_DISTR_MUX7_SRC_STH) {
						DelayOutSourceLeft = 2;
					} else if (DistRegSrc & XRFDC_CLK_DISTR_MUX6_SRC_NTH) {
						DelayOutSourceRight = 2;
					}
					if (CurrentTile > i) {
						ClockSettingsPtr->Delay =
							DelayOutSourceRight + ((CurrentTile - i) << 1);
						if ((DistRegCurrent & XRFDC_CLK_DISTR_MUX4A_SRC_INT) !=
						    XRFDC_CLK_DISTR_MUX4A_SRC_INT) {
							ClockSettingsPtr->Delay += 2;
						}
					} else {
						ClockSettingsPtr->Delay = DelayOutSourceLeft + ((CurrentTile - i) << 1);
						if (DistRegCurrent & XRFDC_CLK_DISTR_MUX4A_SRC_INT) {
							ClockSettingsPtr->Delay += 2;
						}
					}
					if (CurrentTile != XRFDC_CLK_DST_TILE_231) {
						if (PrevSrc != i) {
							Distribution++;
							DistributionSettingsPtr->DistributionStatus[Distribution]
								.LowerBound = CurrentTile;
						}
					}
					PrevSrc = i;
					DistributionSettingsPtr->DistributionStatus[Distribution].Enabled =
						XRFDC_ENABLED;
					DistributionSettingsPtr->DistributionStatus[Distribution].DistributionSource =
						i;
					DistributionSettingsPtr->DistributionStatus[Distribution].UpperBound =
						CurrentTile;
					DistributionSettingsPtr->DistributionStatus[Distribution].MaxDelay =
						MAX(DistributionSettingsPtr->DistributionStatus[Distribution].MaxDelay,
						    (ClockSettingsPtr->Delay));
					DistributionSettingsPtr->DistributionStatus[Distribution].MinDelay =
						MIN(DistributionSettingsPtr->DistributionStatus[Distribution].MinDelay,
						    (ClockSettingsPtr->Delay));
					DistributionSettingsPtr->DistributionStatus[Distribution].IsDelayBalanced =
						(DistributionSettingsPtr->DistributionStatus[Distribution].MaxDelay ==
						 DistributionSettingsPtr->DistributionStatus[Distribution].MinDelay) ?
							1 :
							0;
					break;
				}
			}
			(void)XRFdc_GetPLLConfig(InstancePtr, Type, Tile, &(ClockSettingsPtr->PLLSettings));
			ClockSettingsPtr->PLLEnable = ClockSettingsPtr->PLLSettings.Enabled;
			ClockSettingsPtr->DivisionFactor = ClockSettingsPtr->PLLSettings.OutputDivider;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function gets Clock source
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type indicates ADC/DAC.
* @param    Tile_Id indicates Tile number (0-3).
* @param    ClockSourcePtr Pointer to return the clock source
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if tile not enabled.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetClockSource(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 *ClockSourcePtr)
{
	u32 BaseAddr;
	u32 Status;
	u32 PLLEnReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ClockSourcePtr != NULL);

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		*ClockSourcePtr = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1,
					      XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK);
	} else {
		PLLEnReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0);
		if ((PLLEnReg & (XRFDC_PLL_DIVIDER0_BYP_OPDIV_MASK | XRFDC_PLL_DIVIDER0_MODE_MASK)) == XRFDC_DISABLED) {
			*ClockSourcePtr = XRFDC_EXTERNAL_CLK;
		} else if ((PLLEnReg & XRFDC_PLL_DIVIDER0_BYP_PLL_MASK) != 0) {
			*ClockSourcePtr = XRFDC_EXTERNAL_CLK;
		} else {
			*ClockSourcePtr = XRFDC_INTERNAL_PLL_CLK;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function gets PLL lock status
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type indicates ADC/DAC.
* @param    Tile_Id indicates Tile number (0-3).
* @param    LockStatusPtr Pointer to return the PLL lock status
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetPLLLockStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 *LockStatusPtr)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 ClkSrc = 0U;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(LockStatusPtr != NULL);

	/*
	 * Get Tile clock source information
	 */
	if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &ClkSrc) != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Get clock source request %s %u failed in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (ClkSrc == XRFDC_EXTERNAL_CLK) {
		metal_log(METAL_LOG_DEBUG, "\n %s %u uses external clock source in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		*LockStatusPtr = XRFDC_PLL_LOCKED;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		} else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
		}

		ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET, XRFDC_PLL_LOCKED_MASK);
		if (ReadReg != 0U) {
			*LockStatusPtr = XRFDC_PLL_LOCKED;
		} else {
			*LockStatusPtr = XRFDC_PLL_UNLOCKED;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function used for configuring the internal PLL registers
* based on reference clock and sampling rate
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type indicates ADC/DAC.
* @param    Tile_Id indicates Tile number (0-3).
* @param    RefClkFreq Reference Clock Frequency MHz(50MHz - 1.2GHz)
* @param    SamplingRate Sampling Rate in MHz(0.5- 4 GHz)
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
static u32 XRFdc_SetPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double RefClkFreq, double SamplingRate)
{
	u32 BaseAddr;
	u32 Status;
	u32 FeedbackDiv;
	u32 OutputDiv;
	double CalcSamplingRate;
	double PllFreq;
	double SamplingError;
	u32 Best_FeedbackDiv = 0x0U;
	u32 Best_OutputDiv = 0x2U;
	double Best_Error = 0xFFFFFFFFU;
	u32 DivideMode = 0x0U;
	u32 DivideValue = 0x0U;
	u32 PllFreqIndex = 0x0U;
	u32 FbDivIndex = 0x0U;
	u32 RefClkDiv = 0x1;
	u16 ReadReg;
	u32 VCOMin;
	u32 VCOMax;

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
	} else {
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);
	}

	BaseAddr += XRFDC_HSCOM_ADDR;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_REFDIV);
	if (ReadReg & XRFDC_REFCLK_DIV_1_MASK) {
		RefClkDiv = XRFDC_REF_CLK_DIV_1;
	} else {
		switch (ReadReg & XRFDC_REFCLK_DIV_MASK) {
		case XRFDC_REFCLK_DIV_2_MASK:
			RefClkDiv = XRFDC_REF_CLK_DIV_2;
			break;
		case XRFDC_REFCLK_DIV_3_MASK:
			RefClkDiv = XRFDC_REF_CLK_DIV_3;
			break;
		case XRFDC_REFCLK_DIV_4_MASK:
			RefClkDiv = XRFDC_REF_CLK_DIV_4;
			break;
		default:
			/*
				 * IP currently supporting 1 to 4 divider values. This
				 * error condition might change in future based on IP update.
				 */
			metal_log(METAL_LOG_ERROR,
				  "\n Unsupported Reference clock Divider value (%u) for %s %u in %s\r\n",
				  (ReadReg & XRFDC_REFCLK_DIV_MASK), (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
				  __func__);
			return XRFDC_FAILURE;
		}
	}

	RefClkFreq /= RefClkDiv;

	/*
	 * Sweep valid integer values of FeedbackDiv(N) and record a list
	 * of values that fall in the valid VCO range 8.5GHz - 12.8GHz
	 */

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		VCOMin = VCO_RANGE_MIN;
		VCOMax = VCO_RANGE_MAX;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			VCOMin = VCO_RANGE_ADC_MIN;
			VCOMax = VCO_RANGE_ADC_MAX;
		} else {
			VCOMin = VCO_RANGE_DAC_MIN;
			VCOMax = VCO_RANGE_DAC_MAX;
		}
	}

	for (FeedbackDiv = PLL_FPDIV_MIN; FeedbackDiv <= PLL_FPDIV_MAX; FeedbackDiv++) {
		PllFreq = FeedbackDiv * RefClkFreq;

		if ((PllFreq >= VCOMin) && (PllFreq <= VCOMax)) {
			/*
			 * Sweep values of OutputDiv(M) to find the output frequency
			 * that best matches the user requested value
			 */
			if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
				OutputDiv = PLL_DIVIDER_MIN_GEN3;
				CalcSamplingRate = (PllFreq / OutputDiv);

				if (SamplingRate > CalcSamplingRate) {
					SamplingError = SamplingRate - CalcSamplingRate;
				} else {
					SamplingError = CalcSamplingRate - SamplingRate;
				}

				if (Best_Error > SamplingError) {
					Best_FeedbackDiv = FeedbackDiv;
					Best_OutputDiv = OutputDiv;
					Best_Error = SamplingError;
				}
			}
			for (OutputDiv = PLL_DIVIDER_MIN; OutputDiv <= PLL_DIVIDER_MAX; OutputDiv += 2U) {
				CalcSamplingRate = (PllFreq / OutputDiv);

				if (SamplingRate > CalcSamplingRate) {
					SamplingError = SamplingRate - CalcSamplingRate;
				} else {
					SamplingError = CalcSamplingRate - SamplingRate;
				}

				if (Best_Error > SamplingError) {
					Best_FeedbackDiv = FeedbackDiv;
					Best_OutputDiv = OutputDiv;
					Best_Error = SamplingError;
				}
			}

			OutputDiv = 3U;
			CalcSamplingRate = (PllFreq / OutputDiv);

			if (SamplingRate > CalcSamplingRate) {
				SamplingError = SamplingRate - CalcSamplingRate;
			} else {
				SamplingError = CalcSamplingRate - SamplingRate;
			}

			if (Best_Error > SamplingError) {
				Best_FeedbackDiv = FeedbackDiv;
				Best_OutputDiv = OutputDiv;
				Best_Error = SamplingError;
			}
		}

		/*
		 * PLL Static configuration
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_CFG0, 0x80U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_SEED0, 0x111U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_SEED1, 0x11U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VCO1, 0x08U);
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VREG, 0x45U);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VCO0, 0x5800U);

		} else {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VREG, 0x2DU);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VCO0, 0x5F03U);
		}
		/*
		 * Set Feedback divisor value
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_FPDIV, Best_FeedbackDiv - 2U);

		/*
		 * Set Output divisor value
		 */
		if (Best_OutputDiv == 1U) {
			DivideMode = 0x0U;
			/*if divisor is 1 bypass toatally*/
			DivideValue = XRFDC_PLL_DIVIDER0_BYP_OPDIV_MASK;
		} else if (Best_OutputDiv == 2U) {
			DivideMode = 0x1U;
		} else if (Best_OutputDiv == 3U) {
			DivideMode = 0x2U;
			DivideValue = 0x1U;
		} else if (Best_OutputDiv >= 4U) {
			DivideMode = 0x3U;
			DivideValue = ((Best_OutputDiv - 4U) / 2U);
		}

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_MASK,
				((DivideMode << XRFDC_PLL_DIVIDER0_SHIFT) | DivideValue));

		if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
			if (Best_OutputDiv > PLL_DIVIDER_MIN_GEN3) {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_ALT_MASK,
						XRFDC_DISABLED);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_ALT_MASK,
						XRFDC_PLL_DIVIDER0_BYPDIV_MASK);
			}
		}
		/*
		 * Enable fine sweep
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_CRS2, XRFDC_PLL_CRS2_VAL);

		/*
		 * Set default PLL spare inputs LSB
		 */
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x507U);
		} else {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x0D37U);
		}
		/*
		 * Set PLL spare inputs MSB
		 */
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE1, 0x0U);
		} else {
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE1, 0x80U);
		}
		PllFreq = RefClkFreq * Best_FeedbackDiv;

		if (PllFreq < 9400U) {
			PllFreqIndex = 0U;
			FbDivIndex = 2U;
			if (Best_FeedbackDiv < 21U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			}
		} else if (PllFreq < 10070U) {
			PllFreqIndex = 1U;
			FbDivIndex = 2U;
			if (Best_FeedbackDiv < 18U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			}
		} else if (PllFreq < 10690U) {
			PllFreqIndex = 2U;
			FbDivIndex = 3U;
			if (Best_FeedbackDiv < 18U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 25U) {
				FbDivIndex = 1U;
			} else if (Best_FeedbackDiv < 35U) {
				FbDivIndex = 2U;
			}
		} else if (PllFreq < 10990U) {
			PllFreqIndex = 3U;
			FbDivIndex = 3U;
			if (Best_FeedbackDiv < 19U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 27U) {
				FbDivIndex = 1U;
			} else if (Best_FeedbackDiv < 38U) {
				FbDivIndex = 2U;
			}
		} else if (PllFreq < 11430U) {
			PllFreqIndex = 4U;
			FbDivIndex = 3U;
			if (Best_FeedbackDiv < 19U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 27U) {
				FbDivIndex = 1U;
			} else if (Best_FeedbackDiv < 38U) {
				FbDivIndex = 2U;
			}
		} else if (PllFreq < 12040U) {
			PllFreqIndex = 5U;
			FbDivIndex = 3U;
			if (Best_FeedbackDiv < 20U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 28U) {
				FbDivIndex = 1U;
			} else if (Best_FeedbackDiv < 40U) {
				FbDivIndex = 2U;
			}
		} else if (PllFreq < 12530U) {
			PllFreqIndex = 6U;
			FbDivIndex = 3U;
			if (Best_FeedbackDiv < 23U) {
				FbDivIndex = 0U;
			} else if (Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			} else if (Best_FeedbackDiv < 42U) {
				FbDivIndex = 2U;
			}
		} else if (PllFreq < 20000U) {
			PllFreqIndex = 7U;
			FbDivIndex = 2U;
			if (Best_FeedbackDiv < 20U) {
				FbDivIndex = 0U;
				/*
				 * Set PLL spare inputs LSB
				 */
				if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x577);
				} else {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x0D37U);
				}
			} else if (Best_FeedbackDiv < 39U) {
				FbDivIndex = 1U;
			}
		}

		/*
		 * Enable automatic selection of the VCO, this will work with the
		 * IP version 2.0.1 and above and using older version of IP is
		 * not likely to work.
		 */

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_CRS1, XRFDC_PLL_VCO_SEL_AUTO_MASK,
				XRFDC_PLL_VCO_SEL_AUTO_MASK);

		/*
		 * PLL bits for loop filters LSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_LPF0, PllTuningMatrix[PllFreqIndex][FbDivIndex][0]);

		/*
		 * PLL bits for loop filters MSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_LPF1, XRFDC_PLL_LPF1_VAL);

		/*
		 * Set PLL bits for charge pumps
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_CHARGEPUMP,
				 PllTuningMatrix[PllFreqIndex][FbDivIndex][1]);
	}

	CalcSamplingRate = (Best_FeedbackDiv * RefClkFreq) / Best_OutputDiv;
	CalcSamplingRate /= XRFDC_MILLI;

	if (Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate = CalcSamplingRate;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkDivider = RefClkDiv;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = Best_FeedbackDiv;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.OutputDivider = Best_OutputDiv;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate = CalcSamplingRate;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkDivider = RefClkDiv;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = Best_FeedbackDiv;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.OutputDivider = Best_OutputDiv;
	}

	Status = XRFDC_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to get the PLL Configurations.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type represents ADC or DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    PLLSettings pointer to the XRFdc_PLL_Settings structure to get
*           the PLL configurations
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, XRFdc_PLL_Settings *PLLSettings)
{
	u32 Status;
	u32 BaseAddr;
	u16 ReadReg;
	double RefClkFreq;
	double SampleRate;
	u32 FeedbackDivider;
	u8 OutputDivider;
	u32 RefClkDivider;
	u32 Enabled;
	u8 DivideMode;
	u32 PLLFreq;
	u32 PLLFS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(PLLSettings != NULL);

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);
	PLLFreq = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_PLL_FREQ);

	RefClkFreq = ((double)PLLFreq) / 1000;
	PLLFS = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_PLL_FS);
	SampleRate = ((double)PLLFS) / 1000000;
	if (PLLFS == 0) {
		/*This code is here to support the old IPs.*/
		if (Type == XRFDC_ADC_TILE) {
			PLLSettings->Enabled = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.Enabled;
			PLLSettings->FeedbackDivider = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.FeedbackDivider;
			PLLSettings->OutputDivider = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.OutputDivider;
			PLLSettings->RefClkDivider = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkDivider;
			PLLSettings->RefClkFreq = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq;
			PLLSettings->SampleRate = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate;
			Status = XRFDC_SUCCESS;
			goto RETURN_PATH;
		} else {
			PLLSettings->Enabled = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.Enabled;
			PLLSettings->FeedbackDivider = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.FeedbackDivider;
			PLLSettings->OutputDivider = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.OutputDivider;
			PLLSettings->RefClkDivider = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkDivider;
			PLLSettings->RefClkFreq = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq;
			PLLSettings->SampleRate = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate;
			Status = XRFDC_SUCCESS;
			goto RETURN_PATH;
		}
	} else {
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
		} else {
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);
		}

		BaseAddr += XRFDC_HSCOM_ADDR;

		FeedbackDivider = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_PLL_FPDIV, 0x00FF) + 2;

		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_REFDIV);
		if (ReadReg & XRFDC_REFCLK_DIV_1_MASK) {
			RefClkDivider = XRFDC_REF_CLK_DIV_1;
		} else {
			switch (ReadReg & XRFDC_REFCLK_DIV_MASK) {
			case XRFDC_REFCLK_DIV_2_MASK:
				RefClkDivider = XRFDC_REF_CLK_DIV_2;
				break;
			case XRFDC_REFCLK_DIV_3_MASK:
				RefClkDivider = XRFDC_REF_CLK_DIV_3;
				break;
			case XRFDC_REFCLK_DIV_4_MASK:
				RefClkDivider = XRFDC_REF_CLK_DIV_4;
				break;
			default:
				/*
					 * IP currently supporting 1 to 4 divider values. This
					 * error condition might change in future based on IP update.
					 */
				metal_log(METAL_LOG_ERROR,
					  "\n Unsupported Reference clock Divider value (%u) for %s %u in %s\r\n",
					  (ReadReg & XRFDC_REFCLK_DIV_MASK), (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
					  Tile_Id, __func__);
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
		}
		if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &Enabled) != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0);
		DivideMode = (ReadReg & XRFDC_PLL_DIVIDER0_MODE_MASK) >> XRFDC_PLL_DIVIDER0_SHIFT;

		switch (DivideMode) {
		case XRFDC_PLL_OUTDIV_MODE_1:
			OutputDivider = 1;
			break;
		case XRFDC_PLL_OUTDIV_MODE_2:
			OutputDivider = 2;
			break;
		case XRFDC_PLL_OUTDIV_MODE_3:
			OutputDivider = 3;
			break;
		case XRFDC_PLL_OUTDIV_MODE_N:
			OutputDivider = ((ReadReg & XRFDC_PLL_DIVIDER0_VALUE_MASK) + 2) << 1;
			break;
		default:
			metal_log(METAL_LOG_ERROR, "\n Unsupported Output clock Divider value (%u) for %s %u in %s\r\n",
				  DivideMode, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
			break;
		}
		PLLSettings->Enabled = Enabled;
		PLLSettings->FeedbackDivider = FeedbackDivider;
		PLLSettings->OutputDivider = OutputDivider;
		PLLSettings->RefClkDivider = RefClkDivider;
		PLLSettings->RefClkFreq = RefClkFreq;
		PLLSettings->SampleRate = SampleRate;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:

	return Status;
}

/*****************************************************************************/
/**
*
* This function used for dynamically switch between internal PLL and
* external clcok source and configuring the internal PLL
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type indicates ADC/DAC
* @param    Tile_Id indicates Tile number (0-3)
* @param    Source Clock source internal PLL or external clock source
* @param    RefClkFreq Reference Clock Frequency in MHz(102.40625MHz - 1.2GHz)
* @param    SamplingRate Sampling Rate in MHz(0.1- 6.554GHz for DAC and
*           0.5/1.0 - 2.058/4.116GHz for ADC based on the device package).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     This API enables automatic selection of the VCO which will work in
*           IP version 2.0.1 and above. Using older version of IP this API is
*           not likely to work.
*
******************************************************************************/
u32 XRFdc_DynamicPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 Source, double RefClkFreq, double SamplingRate)
{
	u32 ClkSrc = 0U;
	u32 Status;
	u32 BaseAddr;
	u32 PLLEnable = 0x0U;
	u32 InitialPowerUpState;
	double MaxSampleRate;
	double MinSampleRate;
	u32 OpDiv;
	u32 PLLFreq;
	u32 PLLFS;
	u32 DivideMode;
	u32 DivideValue;
	u32 PLLBypVal;
	u32 NetCtrlReg = 0x0U;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((Source != XRFDC_INTERNAL_PLL_CLK) && (Source != XRFDC_EXTERNAL_CLK)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Source value (%u) for %s %u in %s\r\n", Source,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	/*
	 * Get Tile clock source information
	 */
	if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &ClkSrc) != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (XRFdc_GetMaxSampleRate(InstancePtr, Type, Tile_Id, &MaxSampleRate) != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (XRFdc_GetMinSampleRate(InstancePtr, Type, Tile_Id, &MinSampleRate) != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((SamplingRate < MinSampleRate) || (SamplingRate > MaxSampleRate)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid sampling rate value (%lf) for %s %u in %s\r\n", SamplingRate,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);

	if (Source == XRFDC_INTERNAL_PLL_CLK) {
		if ((RefClkFreq < XRFDC_REFFREQ_MIN) || (RefClkFreq > XRFDC_REFFREQ_MAX)) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Input reference clock frequency (%lf MHz) does not respect the specifications for internal PLL usage. Please use a different frequency (%lf - %lf MHz) or bypass the internal PLL for %s %u in %s\r\n",
				RefClkFreq, XRFDC_REFFREQ_MIN, XRFDC_REFFREQ_MAX,
				(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

	PLLFreq = (u32)(RefClkFreq * 1000);
	PLLFS = (u32)(SamplingRate * 1000);
	XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_PLL_FREQ, PLLFreq);
	XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_PLL_FS, PLLFS);
	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if ((Source != XRFDC_INTERNAL_PLL_CLK) && (ClkSrc != XRFDC_INTERNAL_PLL_CLK)) {
			metal_log(METAL_LOG_DEBUG, "\n Requested tile (%s %u) uses external clock source in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			if (Type == XRFDC_ADC_TILE) {
				InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate = (double)(SamplingRate / 1000);
				InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
			} else {
				InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate = (double)(SamplingRate / 1000);
				InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
			}
			Status = XRFDC_SUCCESS;
			goto RETURN_PATH;
		}
	}

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		InitialPowerUpState = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET, XRFDC_PWR_UP_STAT_MASK) >>
				      XRFDC_PWR_UP_STAT_SHIFT;
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
	} else {
		BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
		InitialPowerUpState = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET, XRFDC_PWR_UP_STAT_MASK) >>
				      XRFDC_PWR_UP_STAT_SHIFT;
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
	}

	/*
	 * Stop the ADC or DAC tile by putting tile in reset state if not stopped already
	 */
	if (InitialPowerUpState != XRFDC_DISABLED) {
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_RESTART_STATE_OFFSET,
				XRFDC_PWR_STATE_MASK, (XRFDC_SM_STATE1 << XRFDC_RSR_START_SHIFT) | XRFDC_SM_STATE1);
		/* Trigger restart */
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_RESTART_OFFSET,
			       XRFDC_RESTART_MASK);
		Status |= XRFdc_WaitForState(InstancePtr, Type, Tile_Id, XRFDC_SM_STATE1);
		if (Status != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

	if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
		NetCtrlReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1);
	}

	if (Source == XRFDC_INTERNAL_PLL_CLK) {
		PLLEnable = 0x1;
		/*
		 * Configure the PLL
		 */
		if (XRFdc_SetPLLConfig(InstancePtr, Type, Tile_Id, RefClkFreq, SamplingRate) != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_BYP_PLL_MASK,
					XRFDC_DISABLED);
			if ((NetCtrlReg & XRFDC_CLK_NETWORK_CTRL1_REGS_MASK) != XRFDC_DISABLED) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
						 XRFDC_HSCOM_PWR_STATS_RX_PLL);
			} else {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
						 XRFDC_HSCOM_PWR_STATS_DIST_PLL);
			}
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1,
					XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK, XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
					 XRFDC_HSCOM_PWR_STATS_PLL);
		}
	} else {
		if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
			OpDiv = PLLFreq / PLLFS;
			if (OpDiv != 1U) {
				metal_log(METAL_LOG_ERROR, "\n Can't divide full rate clock for %s %u in %s\r\n",
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			PLLBypVal = XRFDC_DISABLED;
			DivideMode = XRFDC_PLL_OUTDIV_MODE_1;
			DivideValue = XRFDC_DISABLED;

			if ((NetCtrlReg & XRFDC_CLK_NETWORK_CTRL1_REGS_MASK) != XRFDC_DISABLED) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
						 XRFDC_HSCOM_PWR_STATS_DIST_EXT_SRC);
			} else {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
						 XRFDC_HSCOM_PWR_STATS_DIST_EXT);
			}
			XRFdc_ClrSetReg(InstancePtr, XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR,
					XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_MASK,
					((DivideMode << XRFDC_PLL_DIVIDER0_SHIFT) | DivideValue | PLLBypVal));
		} else {
			OpDiv = 0; /*keep backwards compatibility */
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
					 XRFDC_HSCOM_PWR_STATS_EXTERNAL);
		}

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1, XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK,
				XRFDC_DISABLED);
		SamplingRate /= XRFDC_MILLI;

		if (Type == XRFDC_ADC_TILE) {
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate = SamplingRate;
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkDivider = 0x0U;
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = 0x0U;
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.OutputDivider = OpDiv;
		} else {
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate = SamplingRate;
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkDivider = 0x0U;
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = 0x0U;
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.OutputDivider = OpDiv;
		}
	}

	/*
	 * Re-start the ADC or DAC tile if tile was shut down in this function
	 */
	if (InitialPowerUpState != XRFDC_DISABLED) {
		Status = XRFdc_StartUp(InstancePtr, Type, Tile_Id);
		if (Status != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

	if (Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.Enabled = PLLEnable;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.Enabled = PLLEnable;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/** @} */

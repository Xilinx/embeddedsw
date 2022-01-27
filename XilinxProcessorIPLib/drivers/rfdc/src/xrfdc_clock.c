/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_clock.c
* @addtogroup Overview
* @{
*
* Contains the interface functions of the Clock Settings in XRFdc driver.
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
* 9.0   cog    11/25/20 Upversion.
* 10.0  cog    11/26/20 Refactor and split files.
*       cog    11/27/20 Added functionality for 6xdr devices.
*       cog    12/02/20 Supplied FS was being saved rather than actual FS when
*                       setting PLL.
*       cog    02/10/21 Added custom startup API.
*       cog    03/12/21 Allow ADC to divide and redistribute full rate clock.
*       cog    03/12/21 Tweaks for improved calibration performance.
*       cog    05/05/21 Fixed issue where driver was attempting to start ADC 3
*                       for DFE variants.
*       cog    05/05/21 Fixed issue where ADC 0 failed to complete startup if
*                       distributing full rate clock from ADC to all tiles.
*       cog    05/05/21 Rename the MAX/MIN macros to avoid potential conflicts.
*       cog    05/05/21 Some dividers and delays need to be set to run caliration at
*                       high sampling rates.
*       cog    05/18/21 Fixed issue where valid DFE configurations were not being
*                       allowed.
* 11.0  cog    05/31/21 Upversion.
*              06/01/21 MetalLog Updates.
*       cog    07/12/21 Simplified clock distribution user interface.
*       cog    09/21/21 Fixed rounding error in cast.
* 11.1  cog    11/16/21 Upversion.
*       cog    11/17/21 Fixed powerup bit toggle issue.
*       cog    12/07/21 Added clocking configurations for DFE devices.
*       cog    12/23/21 Added output divder value in appropriate error messages.
*       cog    01/06/22 Added error checks to disallow invalid sample rate/reference
*                       clock combinations.
*       cog    01/12/22 Fix compiler warnings.
*       cog    01/24/22 Fix static analysis errors.
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
#define XRFDC_MAX_DLY_INIT 0U
#define XRFDC_MIN_DLY_INIT 0xFFU
#define XRFDC_DLY_UNIT 2U
#define XRFDC_MAX_DISTRS 8U

/***************** Macros (Inline Functions) Definitions *********************/
static u32 XRFdc_CheckClkDistValid(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr);
static u32 XRFdc_SetPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double RefClkFreq, double SamplingRate);
static void XRFdc_DistTile2TypeTile(XRFdc *InstancePtr, u32 DistTile, u32 *Type, u32 *Tile_Id);
static u8 XRFdc_TypeTile2DistTile(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);

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
	u32 PowerStateMaskReg;
	XRFdc_PLL_Settings PLLSettings;

	PLLSource = (SettingsPtr->PLLEnable == XRFDC_ENABLED) ? XRFDC_INTERNAL_PLL_CLK : XRFDC_EXTERNAL_CLK;
	Status = XRFdc_DynamicPLLConfig(InstancePtr, Type, Tile_Id, PLLSource, SettingsPtr->RefClkFreq,
					SettingsPtr->SampleRate);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Could not set up PLL settings for %s %u %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}

	if (PLLSource == XRFDC_EXTERNAL_CLK) {
		(void)XRFdc_GetPLLConfig(InstancePtr, Type, Tile_Id, &PLLSettings);
		SettingsPtr->DivisionFactor = PLLSettings.OutputDivider;
	} else {
		SettingsPtr->DivisionFactor = 1U;
	}

	/* in cases where pll output divder is totally bypassed distribute the RX clock instead */
	if ((PLLSource == XRFDC_EXTERNAL_CLK) && (SettingsPtr->DivisionFactor == 1U) &&
	    (SettingsPtr->DistributedClock == XRFDC_DIST_OUT_OUTDIV)) {
		SettingsPtr->DistributedClock = XRFDC_DIST_OUT_RX;
	}

	DistCtrlReg = 0;
	PLLRefDivReg = 0;
	NetworkCtrlReg = 0;
	if ((SettingsPtr->SourceTile == Tile_Id) && (SettingsPtr->SourceType == Type)) {
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
		if (Type == XRFDC_ADC_TILE) {
			/* This is needed if distributing a full rate clock from ADC 0/1 to ADC 2/3 */
			if ((Tile_Id > XRFDC_TILE_ID1) && (SettingsPtr->SourceTile < XRFDC_TILE_ID2) &&
			    (SettingsPtr->SourceType == XRFDC_ADC_TILE)) {
				DistCtrlReg |= XRFDC_CLK_DISTR_MUX5A_SRC_RX;
			}
		}
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
				if (SettingsPtr->DistributedClock == XRFDC_DIST_OUT_OUTDIV) {
					DistCtrlReg |= XRFDC_DIST_CTRL_DIST_SRC_PLL;
				}
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
	u32 PkgTileId;
	u32 PkgADCEdgeTile;
	u8 IsPLL;
	u16 EFuse;
	u8 TileLayout;

	if (DistributionSettingsPtr->DistributedClock > XRFDC_DIST_OUT_OUTDIV) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Parameter Value for Distribution Out (%u) for %s %u in %s\r\n",
			  DistributionSettingsPtr->DistributedClock,
			  (DistributionSettingsPtr->SourceType == XRFDC_ADC_TILE) ? "ADC" : "DAC",
			  DistributionSettingsPtr->SourceTileId, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((DistributionSettingsPtr->Info.Source < DistributionSettingsPtr->Info.LowerBound) ||
	    (DistributionSettingsPtr->Info.Source > DistributionSettingsPtr->Info.UpperBound)) {
		metal_log(METAL_LOG_ERROR, "\n %s %u does not reside between %s %u and %s %u in %s\r\n",
			  ((DistributionSettingsPtr->SourceType == XRFDC_ADC_TILE) ? "ADC" : "DAC"),
			  DistributionSettingsPtr->SourceTileId,
			  ((DistributionSettingsPtr->EdgeTypes[0] == XRFDC_ADC_TILE) ? "ADC" : "DAC"),
			  DistributionSettingsPtr->EdgeTileIds[0],
			  ((DistributionSettingsPtr->EdgeTypes[1] == XRFDC_ADC_TILE) ? "ADC" : "DAC"),
			  DistributionSettingsPtr->EdgeTileIds[1], __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (DistributionSettingsPtr->Info.UpperBound == DistributionSettingsPtr->Info.LowerBound) {
		if (DistributionSettingsPtr->DistributedClock != XRFDC_DIST_OUT_NONE) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Invalid Parameter Value for Distribution Out (%u) for Single Tile Distribution in %s\r\n",
				DistributionSettingsPtr->DistributedClock, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		if (DistributionSettingsPtr->DistributedClock == XRFDC_DIST_OUT_NONE) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Invalid Parameter Value for Distribution Out (%u) for Multi Tile Distribution in %s\r\n",
				DistributionSettingsPtr->DistributedClock, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

	for (PkgTileId = DistributionSettingsPtr->Info.LowerBound;
	     PkgTileId <= DistributionSettingsPtr->Info.UpperBound; PkgTileId++) {
		XRFdc_DistTile2TypeTile(InstancePtr, PkgTileId, &Type, &Tile);
		Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u not enabled in %s\r\n",
				  ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile, __func__);
			goto RETURN_PATH;
		}
	}

	TileLayout = XRFdc_GetTileLayout(InstancePtr);
	if (TileLayout == XRFDC_4ADC_4DAC_TILES) {
		IsPLL = (DistributionSettingsPtr->SampleRates[DistributionSettingsPtr->SourceType]
							     [DistributionSettingsPtr->SourceTileId] >
			 DistributionSettingsPtr->DistRefClkFreq);
		if ((DistributionSettingsPtr->SourceTileId == XRFDC_TILE_ID0) ||
		    (DistributionSettingsPtr->SourceTileId == XRFDC_TILE_ID3)) {
			if ((DistributionSettingsPtr->DistributedClock == XRFDC_DIST_OUT_OUTDIV) ||
			    ((DistributionSettingsPtr->DistributedClock == XRFDC_DIST_OUT_RX) &&
			     (IsPLL == XRFDC_DISABLED))) {
				Status = XRFDC_FAILURE;
				metal_log(METAL_LOG_ERROR,
					  "\n Distribution of full rate clock from edge tiles not supported in %s\r\n",
					  __func__);
				goto RETURN_PATH;
			}
		}

		if ((DistributionSettingsPtr->SourceType == XRFDC_DAC_TILE) &&
		    (DistributionSettingsPtr->Info.UpperBound > XRFDC_CLK_DST_TILE_226)) {
			if ((DistributionSettingsPtr->DistributedClock == XRFDC_DIST_OUT_OUTDIV) ||
			    (IsPLL == XRFDC_DISABLED)) {
				EFuse = XRFdc_ReadReg16(
					InstancePtr, XRFDC_DRP_BASE(XRFDC_ADC_TILE, XRFDC_BLK_ID1) + XRFDC_HSCOM_ADDR,
					XRFDC_HSCOM_EFUSE_2_OFFSET);
				if (EFuse & XRFDC_PREMIUMCTRL_CLKDIST) {
					Status = XRFDC_FAILURE;
					metal_log(METAL_LOG_ERROR, "\n Invalid Configuration in %s\r\n", __func__);
					goto RETURN_PATH;
				}
			}
		}

		PkgADCEdgeTile = XRFDC_CLK_DST_TILE_227;
	} else {
		PkgADCEdgeTile = XRFDC_CLK_DST_TILE_226;
	}

	if ((DistributionSettingsPtr->SourceType == XRFDC_ADC_TILE) &&
	    (DistributionSettingsPtr->Info.LowerBound < PkgADCEdgeTile)) {
		metal_log(METAL_LOG_ERROR, "\n DAC Cannot Source from ADC in %s\r\n", __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
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
	u32 Type;
	u32 Tile;
	u8 PkgTileId;

	Status = XRFDC_SUCCESS;

	for (PkgTileId = DistributionSettingsPtr->Info.LowerBound;
	     PkgTileId <= DistributionSettingsPtr->Info.UpperBound; PkgTileId++) {
		XRFdc_DistTile2TypeTile(InstancePtr, PkgTileId, &Type, &Tile);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_RESTART_STATE_OFFSET,
				XRFDC_PWR_STATE_MASK, (XRFDC_SM_STATE1 << XRFDC_RSR_START_SHIFT) | XRFDC_SM_STATE15);
		/* Trigger restart */
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_RESTART_OFFSET, XRFDC_RESTART_MASK);
	}
	/*Ensure source Tile reaches state where it is fit to distribute*/
	Status |= XRFdc_WaitForState(InstancePtr, DistributionSettingsPtr->SourceType,
				     DistributionSettingsPtr->SourceTileId, XRFDC_SM_STATE7);

	for (PkgTileId = DistributionSettingsPtr->Info.LowerBound;
	     PkgTileId <= DistributionSettingsPtr->Info.UpperBound; PkgTileId++) {
		XRFdc_DistTile2TypeTile(InstancePtr, PkgTileId, &Type, &Tile);
		Status |= XRFdc_WaitForState(InstancePtr, Type, Tile, XRFDC_SM_STATE15);
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
	u8 Delay;
	s8 ClkDetItr;
	u8 DelayOutSourceLeft;
	u8 DelayOutSourceRight;
	u16 Reg;
	u16 SrcReg;
	u16 ClkDetectMaskOld;
	u16 ClkDetectReg;
	u8 FeedBackForInputRight = 0;
	u8 FeedBackForInputLeft = 0;
	u32 Tile;
	u32 Type;
	XRFdc_Tile_Clock_Settings Settings;
	u8 DACEdgeTile;
	u8 TileLayout;
	u8 PkgTileId;
	u8 FirstTile;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DistributionSettingsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested fuctionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}
	memset(&DistributionSettingsPtr->Info, 0, sizeof(DistributionSettingsPtr->Info));

	DistributionSettingsPtr->Info.Source = XRFdc_TypeTile2DistTile(InstancePtr, DistributionSettingsPtr->SourceType,
								       DistributionSettingsPtr->SourceTileId);
	DistributionSettingsPtr->Info.UpperBound = XRFdc_TypeTile2DistTile(
		InstancePtr, DistributionSettingsPtr->EdgeTypes[0], DistributionSettingsPtr->EdgeTileIds[0]);
	DistributionSettingsPtr->Info.LowerBound = XRFdc_TypeTile2DistTile(
		InstancePtr, DistributionSettingsPtr->EdgeTypes[1], DistributionSettingsPtr->EdgeTileIds[1]);

	if (DistributionSettingsPtr->Info.UpperBound < DistributionSettingsPtr->Info.LowerBound) {
		DistributionSettingsPtr->Info.LowerBound ^= DistributionSettingsPtr->Info.UpperBound;
		DistributionSettingsPtr->Info.UpperBound ^= DistributionSettingsPtr->Info.LowerBound;
		DistributionSettingsPtr->Info.LowerBound ^= DistributionSettingsPtr->Info.UpperBound;
	}
	Status = XRFdc_CheckClkDistValid(InstancePtr, DistributionSettingsPtr);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Distribution in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	TileLayout = XRFdc_GetTileLayout(InstancePtr);

	ClkDetectMaskOld = XRFdc_RDReg(InstancePtr,
				       XRFDC_CTRL_STS_BASE(DistributionSettingsPtr->SourceType,
							   DistributionSettingsPtr->SourceTileId),
				       XRFDC_CLOCK_DETECT_OFFSET, XRFDC_CLOCK_DETECT_SRC_MASK);
	ClkDetectReg = (XRFDC_CLOCK_DETECT_CLK << ((XRFDC_CLK_DST_TILE_224 - DistributionSettingsPtr->Info.Source)
						   << XRFDC_CLOCK_DETECT_DST_SHIFT));

	FirstTile = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_CLK_DST_TILE_228 : XRFDC_CLK_DST_TILE_231;
	for (PkgTileId = FirstTile; PkgTileId < DistributionSettingsPtr->Info.LowerBound; PkgTileId++) {
		XRFdc_ClrSetReg(InstancePtr,
				XRFDC_CTRL_STS_BASE(DistributionSettingsPtr->SourceType,
						    DistributionSettingsPtr->SourceTileId),
				XRFDC_CLOCK_DETECT_OFFSET, ClkDetectMaskOld | ClkDetectReg, XRFDC_DISABLED);
	}
	for (PkgTileId = DistributionSettingsPtr->Info.UpperBound; PkgTileId <= XRFDC_CLK_DST_TILE_224; PkgTileId++) {
		XRFdc_ClrSetReg(InstancePtr,
				XRFDC_CTRL_STS_BASE(DistributionSettingsPtr->SourceType,
						    DistributionSettingsPtr->SourceTileId),
				XRFDC_CLOCK_DETECT_OFFSET, ClkDetectMaskOld | ClkDetectReg, XRFDC_DISABLED);
	}

	for (PkgTileId = DistributionSettingsPtr->Info.LowerBound;
	     PkgTileId <= DistributionSettingsPtr->Info.UpperBound; PkgTileId++) {
		XRFdc_DistTile2TypeTile(InstancePtr, PkgTileId, &Type, &Tile);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_RESTART_STATE_OFFSET,
				XRFDC_PWR_STATE_MASK, (XRFDC_SM_STATE1 << XRFDC_RSR_START_SHIFT) | XRFDC_SM_STATE1);
		/* Trigger restart */
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_RESTART_OFFSET, XRFDC_RESTART_MASK);
		Status |= XRFdc_WaitForState(InstancePtr, Type, Tile, XRFDC_SM_STATE1);
		if (Status != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

	DACEdgeTile = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_CLK_DST_TILE_227 : XRFDC_CLK_DST_TILE_228;
	StatusNonBlocking = XRFDC_SUCCESS;
	DelayLeft = (-DistributionSettingsPtr->Info.LowerBound + DistributionSettingsPtr->Info.Source);
	DelayRight = (DistributionSettingsPtr->Info.UpperBound - DistributionSettingsPtr->Info.Source);
	DelayOutSourceLeft = 0U;
	DelayOutSourceRight = 0U;
	DistributionSettingsPtr->Info.MaxDelay = XRFDC_MAX_DLY_INIT;
	DistributionSettingsPtr->Info.MinDelay = XRFDC_MIN_DLY_INIT;
	DistributionSettingsPtr->Info.IsDelayBalanced = 0U;
	if ((DelayLeft == 0U) && (DelayRight == 0U)) { /*self contained*/
		SrcReg = XRFDC_CLK_DISTR_OFF;
	} else {
		SrcReg = XRFDC_CLK_DISTR_MUX9_SRC_INT;
		if (DelayLeft == 0U) {
			SrcReg |= XRFDC_CLK_DISTR_MUX8_SRC_NTH;
		} else {
			SrcReg |= XRFDC_CLK_DISTR_MUX8_SRC_INT;
		}
		if (((DistributionSettingsPtr->Info.Source == DACEdgeTile) ||
		     (DistributionSettingsPtr->Info.Source == XRFDC_CLK_DST_TILE_224)) &&
		    ((DelayLeft > 1U) || (DelayRight > 0U))) {
			SrcReg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_NTH |
				  XRFDC_CLK_DISTR_MUX7_SRC_INT;
			FeedBackForInputRight = 1U;
			FeedBackForInputLeft = 0U;
			DelayOutSourceRight = XRFDC_DLY_UNIT;
		} else {
			if ((DelayLeft > 1U) || ((DelayLeft == 1U) && (DelayRight == 1U))) {
				SrcReg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH | XRFDC_CLK_DISTR_MUX6_SRC_NTH |
					  XRFDC_CLK_DISTR_MUX7_SRC_INT;
				DelayOutSourceRight = XRFDC_DLY_UNIT;
				FeedBackForInputRight = 0U;
				FeedBackForInputLeft = 1U;
			} else {
				FeedBackForInputRight = (DelayLeft == 0U) ? 0U : 1U;
				FeedBackForInputLeft = 0U;
				if ((DelayRight > 1U) &&
				    (DistributionSettingsPtr->Info.Source != XRFDC_CLK_DST_TILE_229)) {
					FeedBackForInputRight = (DelayLeft == 0U) ? 0U : 1U;
					SrcReg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
					SrcReg |= XRFDC_CLK_DISTR_MUX7_SRC_STH;
					DelayOutSourceLeft = XRFDC_DLY_UNIT;
				} else {
					FeedBackForInputRight = 1U;
					if (DelayLeft == 0U) {
						SrcReg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH;
						SrcReg |= XRFDC_CLK_DISTR_MUX7_SRC_OFF;
					} else {
						SrcReg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
						SrcReg |= XRFDC_CLK_DISTR_MUX7_SRC_INT;
					}
				}
				if (DelayRight == 0U) {
					SrcReg |= XRFDC_CLK_DISTR_MUX6_SRC_OFF;
				} else {
					SrcReg |= XRFDC_CLK_DISTR_MUX6_SRC_INT;
				}
			}
		}
	}

	if (SrcReg == XRFDC_CLK_DISTR_OFF) {
		Settings.Delay = 0U;
	} else if (FeedBackForInputLeft == 0U) {
		Settings.Delay = DelayOutSourceLeft + XRFDC_DLY_UNIT;
	} else {
		Settings.Delay = DelayOutSourceRight + XRFDC_DLY_UNIT;
	}

	DistributionSettingsPtr->Info.MaxDelay = XRFDC_MAX(DistributionSettingsPtr->Info.MaxDelay, (Settings.Delay));
	DistributionSettingsPtr->Info.MinDelay = XRFDC_MIN(DistributionSettingsPtr->Info.MinDelay, (Settings.Delay));

	XRFdc_ClrSetReg(InstancePtr,
			(XRFDC_DRP_BASE(DistributionSettingsPtr->SourceType, DistributionSettingsPtr->SourceTileId) +
			 XRFDC_HSCOM_ADDR),
			XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, SrcReg);
	XRFdc_ClrSetReg(InstancePtr,
			XRFDC_CTRL_STS_BASE(DistributionSettingsPtr->SourceType, DistributionSettingsPtr->SourceTileId),
			XRFDC_CLOCK_DETECT_OFFSET, XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
	Settings.SourceType = DistributionSettingsPtr->SourceType;
	Settings.SourceTile = DistributionSettingsPtr->SourceTileId;
	Settings.DistributedClock = DistributionSettingsPtr->DistributedClock;
	Settings.RefClkFreq = DistributionSettingsPtr->DistRefClkFreq;
	Settings.SampleRate =
		(DistributionSettingsPtr
			 ->SampleRates[DistributionSettingsPtr->SourceType][DistributionSettingsPtr->SourceTileId]);
	Settings.PLLEnable = (Settings.SampleRate > DistributionSettingsPtr->DistRefClkFreq);
	StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, DistributionSettingsPtr->SourceType,
						      DistributionSettingsPtr->SourceTileId, &Settings);
	DistributionSettingsPtr->Info
		.ClkSettings[DistributionSettingsPtr->SourceType][DistributionSettingsPtr->SourceTileId] = Settings;
	if (DistributionSettingsPtr->DistributedClock == XRFDC_DIST_OUT_OUTDIV) {
		Settings.RefClkFreq = Settings.SampleRate;
		Settings.PLLEnable = XRFDC_DISABLED;
	} else {
		Settings.RefClkFreq = DistributionSettingsPtr->DistRefClkFreq;
	}
	Settings.DistributedClock = XRFDC_DIST_OUT_NONE;
	/*Leftmost tile*/
	if (DelayLeft) {
		Settings.Delay = DelayOutSourceLeft + (DelayLeft << 1U);
		DistributionSettingsPtr->Info.MaxDelay =
			XRFDC_MAX(DistributionSettingsPtr->Info.MaxDelay, (Settings.Delay));
		DistributionSettingsPtr->Info.MinDelay =
			XRFDC_MIN(DistributionSettingsPtr->Info.MinDelay, (Settings.Delay));
		Reg = XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX8_SRC_INT | XRFDC_CLK_DISTR_MUX9_SRC_INT;

		if ((DistributionSettingsPtr->Info.Source != DACEdgeTile) && (DelayLeft == 1U) && (DelayRight == 1U)) {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX7_SRC_STH;

		} else {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH | XRFDC_CLK_DISTR_MUX7_SRC_OFF;
		}
		/* setup clk detect register */
		ClkDetectReg =
			(XRFDC_CLOCK_DETECT_BOTH << ((XRFDC_CLK_DST_TILE_224 - DistributionSettingsPtr->Info.Source)
						     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		for (ClkDetItr = DelayLeft - 1; ClkDetItr > 0; ClkDetItr--) {
			ClkDetectReg |=
				(XRFDC_CLOCK_DETECT_DIST
				 << ((XRFDC_CLK_DST_TILE_224 - (DistributionSettingsPtr->Info.Source - ClkDetItr))
				     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		}

		XRFdc_DistTile2TypeTile(InstancePtr, (DistributionSettingsPtr->Info.Source - DelayLeft), &Type, &Tile);
		XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
				XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
				XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
		Settings.SampleRate = (DistributionSettingsPtr->SampleRates[Type][Tile]);
		StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, &Settings);
		DistributionSettingsPtr->Info.ClkSettings[Type][Tile] = Settings;
	}
	/*Rest of tiles left of DistributionSettingsPtr->Info.Source*/
	for (Delay = 1U; Delay < DelayLeft; Delay++) {
		Reg = XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX7_SRC_STH | XRFDC_CLK_DISTR_MUX8_SRC_INT |
		      XRFDC_CLK_DISTR_MUX9_SRC_INT;
		if (FeedBackForInputLeft == 0U) {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH;
		} else {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT;
		}
		Settings.Delay = DelayOutSourceLeft + ((Delay + FeedBackForInputLeft) << 1U);
		DistributionSettingsPtr->Info.MaxDelay =
			XRFDC_MAX(DistributionSettingsPtr->Info.MaxDelay, (Settings.Delay));
		DistributionSettingsPtr->Info.MinDelay =
			XRFDC_MIN(DistributionSettingsPtr->Info.MinDelay, (Settings.Delay));
		FeedBackForInputLeft = !FeedBackForInputLeft;

		/* setup clk detect register */
		ClkDetectReg =
			(XRFDC_CLOCK_DETECT_BOTH << ((XRFDC_CLK_DST_TILE_224 - DistributionSettingsPtr->Info.Source)
						     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		for (ClkDetItr = Delay - 1; ClkDetItr > 0; ClkDetItr--) {
			ClkDetectReg |=
				(XRFDC_CLOCK_DETECT_DIST
				 << ((XRFDC_CLK_DST_TILE_224 - (DistributionSettingsPtr->Info.Source - ClkDetItr))
				     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		}

		XRFdc_DistTile2TypeTile(InstancePtr, (DistributionSettingsPtr->Info.Source - Delay), &Type, &Tile);
		XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
				XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
				XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
		Settings.SampleRate = (DistributionSettingsPtr->SampleRates[Type][Tile]);
		StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, &Settings);
		DistributionSettingsPtr->Info.ClkSettings[Type][Tile] = Settings;
	}

	/*tiles to right*/
	Reg = SrcReg;
	for (Delay = 1U; Delay < DelayRight; Delay++) {
		XRFdc_DistTile2TypeTile(InstancePtr, (DistributionSettingsPtr->Info.Source + Delay), &Type, &Tile);
		Settings.SampleRate = (DistributionSettingsPtr->SampleRates[Type][Tile]);
		if (Type == XRFDC_ADC_TILE) {
			if ((Tile == ((TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_TILE_ID2 : XRFDC_TILE_ID3)) &&
			    (Settings.PLLEnable != XRFDC_ENABLED) && (Settings.RefClkFreq != Settings.SampleRate)) {
				Settings.DistributedClock = XRFDC_DIST_OUT_OUTDIV;
			}
		}
		if (((Reg & XRFDC_CLK_DISTR_MUX4A_SRC_INT) != XRFDC_CLK_DISTR_MUX4A_SRC_INT) ||
		    ((Reg & XRFDC_CLK_DISTR_MUX7_SRC_STH) == XRFDC_CLK_DISTR_MUX7_SRC_STH)) {
			Reg = 0U;
		} else {
			Reg = XRFDC_CLK_DISTR_MUX8_SRC_INT;
		}
		if (Settings.DistributedClock == XRFDC_DIST_OUT_OUTDIV) {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_INT |
			       XRFDC_CLK_DISTR_MUX8_SRC_INT;
			FeedBackForInputRight = 1U;
			Settings.Delay = DelayOutSourceRight + (Delay << 1U);
		} else if (((Delay + DistributionSettingsPtr->Info.Source) == DACEdgeTile) ||
			   (FeedBackForInputRight == 0U)) {
			FeedBackForInputRight = 0U;
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_NTH;
			Settings.Delay = DelayOutSourceRight + (Delay << 1U);
		} else {
			Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_STH | XRFDC_CLK_DISTR_MUX6_SRC_NTH;
			Settings.Delay = DelayOutSourceRight + ((Delay + 1U) << 1U);
		}
		DistributionSettingsPtr->Info.MaxDelay =
			XRFDC_MAX(DistributionSettingsPtr->Info.MaxDelay, (Settings.Delay));
		DistributionSettingsPtr->Info.MinDelay =
			XRFDC_MIN(DistributionSettingsPtr->Info.MinDelay, (Settings.Delay));
		Reg |= XRFDC_CLK_DISTR_MUX7_SRC_OFF | XRFDC_CLK_DISTR_MUX8_SRC_NTH | XRFDC_CLK_DISTR_MUX9_SRC_NTH;

		FeedBackForInputRight = !FeedBackForInputRight;
		/* setup clk detect register */
		ClkDetectReg =
			(XRFDC_CLOCK_DETECT_BOTH << ((XRFDC_CLK_DST_TILE_224 - DistributionSettingsPtr->Info.Source)
						     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		for (ClkDetItr = Delay - 1; ClkDetItr > 0; ClkDetItr--) {
			ClkDetectReg |=
				(XRFDC_CLOCK_DETECT_DIST
				 << ((XRFDC_CLK_DST_TILE_224 - (DistributionSettingsPtr->Info.Source + ClkDetItr))
				     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		}
		XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
				XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
				XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
		StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, &Settings);
		DistributionSettingsPtr->Info.ClkSettings[Type][Tile] = Settings;
		if (Settings.DistributedClock == XRFDC_DIST_OUT_OUTDIV) {
			Settings.RefClkFreq /= Settings.DivisionFactor;
			Settings.DistributedClock = XRFDC_DIST_OUT_NONE;
		}
	}

	/*Rightmost tile*/
	if (DelayRight) {
		if ((Reg & XRFDC_CLK_DISTR_MUX4A_SRC_INT) != XRFDC_CLK_DISTR_MUX4A_SRC_INT) {
			Reg = 0U;
		} else {
			Reg = XRFDC_CLK_DISTR_MUX8_SRC_INT;
		}
		Reg |= XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX7_SRC_OFF |
		       XRFDC_CLK_DISTR_MUX9_SRC_NTH;
		Settings.Delay = DelayOutSourceRight + (DelayRight << 1U);
		DistributionSettingsPtr->Info.MaxDelay =
			XRFDC_MAX(DistributionSettingsPtr->Info.MaxDelay, (Settings.Delay));
		DistributionSettingsPtr->Info.MinDelay =
			XRFDC_MIN(DistributionSettingsPtr->Info.MinDelay, (Settings.Delay));

		/* setup clk detect register */
		ClkDetectReg =
			(XRFDC_CLOCK_DETECT_BOTH << ((XRFDC_CLK_DST_TILE_224 - DistributionSettingsPtr->Info.Source)
						     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		for (ClkDetItr = DelayRight - 1; ClkDetItr > 0; ClkDetItr--) {
			ClkDetectReg |=
				(XRFDC_CLOCK_DETECT_DIST
				 << ((XRFDC_CLK_DST_TILE_224 - (DistributionSettingsPtr->Info.Source + ClkDetItr))
				     << XRFDC_CLOCK_DETECT_DST_SHIFT));
		}

		XRFdc_DistTile2TypeTile(InstancePtr, (DistributionSettingsPtr->Info.Source + DelayRight), &Type, &Tile);
		XRFdc_ClrSetReg(InstancePtr, (XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR),
				XRFDC_HSCOM_CLK_DSTR_OFFSET, XRFDC_HSCOM_CLK_DSTR_MASK, Reg);
		XRFdc_ClrSetReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
				XRFDC_CLOCK_DETECT_MASK, ClkDetectReg);
		Settings.SampleRate = (DistributionSettingsPtr->SampleRates[Type][Tile]);
		StatusNonBlocking |= XRFdc_SetTileClkSettings(InstancePtr, Type, Tile, &Settings);
		DistributionSettingsPtr->Info.ClkSettings[Type][Tile] = Settings;
	}
	DistributionSettingsPtr->Info.IsDelayBalanced =
		(DistributionSettingsPtr->Info.MaxDelay == DistributionSettingsPtr->Info.MinDelay) ? 1U : 0U;

	/*start tiles*/
	if (DistributionSettingsPtr->ShutdownMode == XRFDC_DISABLED) {
		Status = XRFdc_StartUpDist(InstancePtr, DistributionSettingsPtr);
	}
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
u32 XRFdc_GetClkDistribution(XRFdc *InstancePtr, XRFdc_Distribution_System_Settings *DistributionSystemPtr)
{
	u32 Status;
	u32 ClockDetReg;
	u32 DistRegCurrent;
	u32 DistRegSrc = 0U;
	s8 CurrentTile;
	u8 DelayOutSourceLeft;
	u8 DelayOutSourceRight;
	XRFdc_Tile_Clock_Settings *ClockSettingsPtr;
	u32 Type;
	u32 SrcType = XRFDC_CLK_DST_INVALID;
	u32 Tile;
	u32 SrcTile = XRFDC_CLK_DST_INVALID;
	u8 PkgSrcTile;
	u8 Distribution;
	u8 i;
	u8 PrevSrc = XRFDC_CLK_DST_INVALID;
	u8 FirstTile;
	u8 TileLayout;
	XRFdc_Distribution_Settings *DistributionSettingsPtr = NULL;
	XRFdc_PLL_Settings PLLSettings;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DistributionSystemPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested fuctionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}
	memset(DistributionSystemPtr, 0, sizeof(XRFdc_Distribution_System_Settings));
	TileLayout = XRFdc_GetTileLayout(InstancePtr);
	FirstTile = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_CLK_DST_TILE_228 : XRFDC_CLK_DST_TILE_231;

	for (CurrentTile = FirstTile, Distribution = 0; CurrentTile <= XRFDC_CLK_DST_TILE_224; CurrentTile++) {
		XRFdc_DistTile2TypeTile(InstancePtr, CurrentTile, &Type, &Tile);
		Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile);
		if (Status != XRFDC_SUCCESS) {
			continue;
		}
		DelayOutSourceLeft = 0;
		DelayOutSourceRight = 0;

		ClockDetReg = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile), XRFDC_CLOCK_DETECT_OFFSET,
					  XRFDC_CLOCK_DETECT_SRC_MASK);
		if (ClockDetReg != XRFDC_DISABLED) {
			for (i = 0; i <= XRFDC_CLK_DST_TILE_224 - FirstTile; i++) {
				if ((ClockDetReg >> (i << 1)) == XRFDC_ENABLED) {
					PkgSrcTile = XRFDC_CLK_DST_TILE_224 - i;
					if (PrevSrc != PkgSrcTile) {
						if (PrevSrc != XRFDC_CLK_DST_INVALID) {
							Distribution++;
						}
						DistributionSettingsPtr =
							&(DistributionSystemPtr->Distributions[Distribution]);
						XRFdc_DistTile2TypeTile(InstancePtr, PkgSrcTile, &SrcType, &SrcTile);
						DistributionSettingsPtr->Info.LowerBound = CurrentTile;
						DistributionSettingsPtr->SourceTileId = SrcTile;
						DistributionSettingsPtr->SourceType = SrcType;
						DistributionSettingsPtr->Info.MaxDelay = 0U;
						DistributionSettingsPtr->Info.MinDelay = 0xFFU;
						DistributionSettingsPtr->EdgeTileIds[0] = Tile;
						DistributionSettingsPtr->EdgeTypes[0] = Type;
						DistRegSrc = XRFdc_ReadReg16(InstancePtr,
									     XRFDC_DRP_BASE(SrcType, SrcTile) +
										     XRFDC_HSCOM_ADDR,
									     XRFDC_HSCOM_CLK_DSTR_OFFSET);
					}
					ClockSettingsPtr = &(DistributionSettingsPtr->Info.ClkSettings[Type][Tile]);
					DistributionSettingsPtr->EdgeTileIds[1] = Tile;
					DistributionSettingsPtr->EdgeTypes[1] = Type;
					PrevSrc = PkgSrcTile;
					DistRegCurrent = XRFdc_ReadReg16(InstancePtr,
									 XRFDC_DRP_BASE(Type, Tile) + XRFDC_HSCOM_ADDR,
									 XRFDC_HSCOM_CLK_DSTR_OFFSET);
					if (((DistRegCurrent & XRFDC_DIST_CTRL_DIST_SRC_LOCAL) ==
					     XRFDC_DIST_CTRL_DIST_SRC_LOCAL) &&
					    (PkgSrcTile == CurrentTile)) {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_RX;
					} else if ((DistRegCurrent & XRFDC_DIST_CTRL_DIST_SRC_PLL) ==
						   XRFDC_DIST_CTRL_DIST_SRC_PLL) {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_OUTDIV;
					} else {
						ClockSettingsPtr->DistributedClock = XRFDC_DIST_OUT_NONE;
					}

					if (DistRegSrc & XRFDC_CLK_DISTR_MUX7_SRC_STH) {
						DelayOutSourceLeft = XRFDC_DLY_UNIT;
					} else if (DistRegSrc & XRFDC_CLK_DISTR_MUX6_SRC_NTH) {
						DelayOutSourceRight = XRFDC_DLY_UNIT;
					}
					if (CurrentTile > PkgSrcTile) {
						ClockSettingsPtr->Delay =
							DelayOutSourceRight + ((CurrentTile - PkgSrcTile) << 1);
						if ((DistRegCurrent & XRFDC_CLK_DISTR_MUX4A_SRC_INT) ==
						    XRFDC_DISABLED) {
							ClockSettingsPtr->Delay += XRFDC_DLY_UNIT;
						}
					} else if (CurrentTile == PkgSrcTile) {
						if (ClockSettingsPtr->DistributedClock != XRFDC_DIST_OUT_NONE) {
							ClockSettingsPtr->Delay =
								DelayOutSourceLeft + DelayOutSourceRight + 2;
						}
					} else {
						ClockSettingsPtr->Delay =
							DelayOutSourceLeft + ((PkgSrcTile - CurrentTile) << 1);
						if (DistRegCurrent & XRFDC_CLK_DISTR_MUX4A_SRC_INT) {
							ClockSettingsPtr->Delay += XRFDC_DLY_UNIT;
						}
					}

					DistributionSettingsPtr->Info.Source = PkgSrcTile;
					DistributionSettingsPtr->Info.UpperBound = CurrentTile;
					DistributionSettingsPtr->Info.MaxDelay = XRFDC_MAX(
						DistributionSettingsPtr->Info.MaxDelay, (ClockSettingsPtr->Delay));
					DistributionSettingsPtr->Info.MinDelay = XRFDC_MIN(
						DistributionSettingsPtr->Info.MinDelay, (ClockSettingsPtr->Delay));
					DistributionSettingsPtr->Info.IsDelayBalanced =
						(DistributionSettingsPtr->Info.MaxDelay ==
						 DistributionSettingsPtr->Info.MinDelay) ?
							1 :
							0;
					ClockSettingsPtr->SourceType = SrcType;
					ClockSettingsPtr->SourceTile = SrcTile;
					(void)XRFdc_GetPLLConfig(InstancePtr, Type, Tile, &PLLSettings);
					ClockSettingsPtr->PLLEnable = PLLSettings.Enabled;
					ClockSettingsPtr->DivisionFactor =
						((ClockSettingsPtr->PLLEnable == XRFDC_ENABLED) ?
							 1 :
							 PLLSettings.OutputDivider);
					ClockSettingsPtr->RefClkFreq = PLLSettings.RefClkFreq;
					ClockSettingsPtr->SampleRate = PLLSettings.SampleRate * XRFDC_MILLI;
					DistributionSettingsPtr->SampleRates[Type][Tile] = ClockSettingsPtr->SampleRate;
					if (PkgSrcTile == CurrentTile) {
						DistributionSettingsPtr->DistributedClock =
							ClockSettingsPtr->DistributedClock;
						DistributionSettingsPtr->DistRefClkFreq = ClockSettingsPtr->RefClkFreq;
					}
					break;
				}
			}
		} else {
			metal_log(METAL_LOG_WARNING,
				  "\n Distribution system misconfiguration detected for %s %u in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile, __func__);
		}
	}
	for (Distribution = Distribution + 1U; Distribution < XRFDC_MAX_DISTRS; Distribution++) {
		DistributionSystemPtr->Distributions[Distribution].SourceTileId = XRFDC_CLK_DST_INVALID;
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
	/* Store Sampling Frequency in kHz */
	XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_PLL_FS,
		       (u32)(CalcSamplingRate * XRFDC_MILLI));
	/* Convert to GHz */
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
	u32 Enabled = XRFDC_DISABLED;
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

	RefClkFreq = ((double)PLLFreq) / XRFDC_MILLI;
	PLLFS = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_PLL_FS);
	SampleRate = ((double)PLLFS) / XRFDC_MICRO;
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
	double MaxSampleRate = 0.0;
	double MinSampleRate = 0.0;
	u32 OpDiv;
	u32 PLLFreq;
	u32 PLLFS;
	u32 DivideMode;
	u32 PrimaryDivideValue;
	u32 SecondaryDivideValue = 0x0U;
	u32 PLLBypVal;
	u32 NetCtrlReg = 0x0U;
	u32 NorthClk = 0x0U;
	u32 FGDelay = 0x0U;
	u32 ADCEdgeTileId;
	u32 TileLayout;

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

	PLLFreq = (u32)((RefClkFreq + 0.0005) * XRFDC_MILLI);
	PLLFS = (u32)((SamplingRate + 0.0005) * XRFDC_MILLI);
	OpDiv = (u32)((RefClkFreq / SamplingRate) + 0.5);
	if ((Source == XRFDC_EXTERNAL_CLK) && (PLLFreq != PLLFS)) {
		if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
			metal_log(
				METAL_LOG_ERROR,
				"\n Sampling rate value (%lf) must match the reference frequency (%lf) for %s %u in %s\r\n",
				SamplingRate, RefClkFreq, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		} else if ((PLLFreq % PLLFS) != 0U) {
			metal_log(
				METAL_LOG_ERROR,
				"\n The reference frequency (%lf) must be an integer multiple of the Sampling rate (%lf) for %s %u in %s\r\n",
				RefClkFreq, SamplingRate, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
	}

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

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		if ((Source != XRFDC_INTERNAL_PLL_CLK) && (ClkSrc != XRFDC_INTERNAL_PLL_CLK)) {
			metal_log(METAL_LOG_DEBUG, "\n Requested tile (%s %u) uses external clock source in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
			if (Type == XRFDC_ADC_TILE) {
				InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate =
					(double)(SamplingRate / XRFDC_MILLI);
				InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
			} else {
				InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate =
					(double)(SamplingRate / XRFDC_MILLI);
				InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
			}
			Status = XRFDC_SUCCESS;
			goto RETURN_PATH;
		}
	} else {
		BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;
		NetCtrlReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1);
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
	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);
	InitialPowerUpState = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET, XRFDC_PWR_UP_STAT_MASK) >>
			      XRFDC_PWR_UP_STAT_SHIFT;
	BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;

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
		if ((Type == XRFDC_ADC_TILE) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_DISABLED)) {
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID0),
					XRFDC_ADC_DAC_MC_CFG0_OFFSET,
					(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
					XRFDC_RX_PR_MC_CFG0_PSNK_MASK);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID1),
					XRFDC_ADC_DAC_MC_CFG0_OFFSET,
					(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
					XRFDC_RX_PR_MC_CFG0_PSNK_MASK);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID2),
					XRFDC_ADC_DAC_MC_CFG0_OFFSET,
					(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
					XRFDC_RX_PR_MC_CFG0_PSNK_MASK);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID3),
					XRFDC_ADC_DAC_MC_CFG0_OFFSET,
					(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
					XRFDC_RX_PR_MC_CFG0_PSNK_MASK);
		}
	} else {
		if (InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) {
			TileLayout = XRFdc_GetTileLayout(InstancePtr);
			ADCEdgeTileId = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_TILE_ID2 : XRFDC_TILE_ID3;
			switch (OpDiv) {
			case 1U:
				/*This is a special case where we want to totally bypass the entire block.*/
				PLLBypVal = XRFDC_DISABLED;
				DivideMode = XRFDC_PLL_OUTDIV_MODE_1;
				PrimaryDivideValue = XRFDC_DISABLED;
				SecondaryDivideValue = XRFDC_RX_PR_MC_CFG0_PSNK_MASK;
				break;
			case 2U:
				/*dividers used depend on configuration*/
				if ((Type == XRFDC_ADC_TILE) && (Tile_Id < ADCEdgeTileId) &&
				    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_DISABLED)) {
					NorthClk = XRFdc_RDReg(
						InstancePtr, XRFDC_ADC_TILE_DRP_ADDR((Tile_Id + 1U)) + XRFDC_HSCOM_ADDR,
						XRFDC_HSCOM_CLK_DSTR_OFFSET,
						(XRFDC_CLK_DISTR_MUX6_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_NTH));

					SecondaryDivideValue = XRFdc_RDReg(
						InstancePtr,
						XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, (Tile_Id + 1U), XRFDC_BLK_ID0),
						XRFDC_ADC_DAC_MC_CFG0_OFFSET,
						(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK));
				}
				if ((NorthClk != XRFDC_CLK_DISTR_MUX6_SRC_OFF) &&
				    (SecondaryDivideValue == XRFDC_RX_PR_MC_CFG0_IDIV_MASK)) {
					PLLBypVal = XRFDC_DISABLED;
					DivideMode = XRFDC_PLL_OUTDIV_MODE_1;
					PrimaryDivideValue = XRFDC_DISABLED;
				} else {
					PLLBypVal = XRFDC_PLL_DIVIDER0_BYP_PLL_MASK;
					DivideMode = XRFDC_PLL_OUTDIV_MODE_2;
					PrimaryDivideValue = XRFDC_DISABLED;
					SecondaryDivideValue = XRFDC_RX_PR_MC_CFG0_PSNK_MASK;
				}
				break;
			case 4U:
				PLLBypVal = XRFDC_PLL_DIVIDER0_BYP_PLL_MASK;
				if ((Type == XRFDC_ADC_TILE) && (Tile_Id == ADCEdgeTileId) &&
				    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_DISABLED)) {
					DivideMode = XRFDC_PLL_OUTDIV_MODE_2;
					PrimaryDivideValue = XRFDC_DISABLED;
					SecondaryDivideValue = XRFDC_RX_PR_MC_CFG0_IDIV_MASK;
				} else {
					metal_log(METAL_LOG_ERROR, "\n Invalid divider value (%u) for %s %u in %s\r\n",
						  OpDiv, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
					Status = XRFDC_FAILURE;
					goto RETURN_PATH;
				}

				break;
			default:
				metal_log(METAL_LOG_ERROR, "\n Invalid divider value (%u) for %s %u in %s\r\n", OpDiv,
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			if (OpDiv == 1) {
				if ((NetCtrlReg & XRFDC_CLK_NETWORK_CTRL1_REGS_MASK) != XRFDC_DISABLED) {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
							 XRFDC_HSCOM_PWR_STATS_DIST_EXT_SRC);
				} else {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
							 XRFDC_HSCOM_PWR_STATS_DIST_EXT);
				}

			} else {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
						 XRFDC_HSCOM_PWR_STATS_DIST_EXT);
				if ((NetCtrlReg & XRFDC_CLK_NETWORK_CTRL1_REGS_MASK) != XRFDC_DISABLED) {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
							 XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV_SRC);
				} else {
					XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_HSCOM_PWR_STATE_OFFSET,
							 XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV);
				}
			}
			if ((Type == XRFDC_ADC_TILE) &&
			    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_DISABLED)) {
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID0),
						XRFDC_ADC_DAC_MC_CFG0_OFFSET,
						(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
						SecondaryDivideValue);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_ADC_DAC_MC_CFG0_OFFSET,
						(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
						SecondaryDivideValue);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_ADC_DAC_MC_CFG0_OFFSET,
						(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
						SecondaryDivideValue);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, XRFDC_BLK_ID3),
						XRFDC_ADC_DAC_MC_CFG0_OFFSET,
						(XRFDC_RX_PR_MC_CFG0_IDIV_MASK | XRFDC_RX_PR_MC_CFG0_PSNK_MASK),
						SecondaryDivideValue);
			}
			XRFdc_ClrSetReg(InstancePtr, XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR,
					XRFDC_PLL_DIVIDER0, XRFDC_PLL_DIVIDER0_MASK,
					((DivideMode << XRFDC_PLL_DIVIDER0_SHIFT) | PrimaryDivideValue | PLLBypVal));
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
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_PLL_FS, PLLFS);
	}
	XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_PLL_FREQ, PLLFreq);

	if ((InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) && (Type == XRFDC_ADC_TILE)) {
		BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		if (InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate >
		    XRFDC_CAL_DIV_CUTOFF_FREQ(XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id))) {
			FGDelay = (u32)(XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CAL_TMR_MULT_OFFSET) *
					XRFDC_CAL_AXICLK_MULT);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_DIV_BYP_OFFSET, XRFDC_CAL_DIV_BYP_MASK,
					XRFDC_DISABLED);
			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CAL_DLY_OFFSET, FGDelay);
		} else {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_DIV_BYP_OFFSET, XRFDC_CAL_DIV_BYP_MASK,
					XRFDC_CAL_DIV_BYP_MASK);
			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CAL_DLY_OFFSET, 0U);
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
static void XRFdc_DistTile2TypeTile(XRFdc *InstancePtr, u32 DistTile, u32 *Type, u32 *Tile_Id)
{
	u8 DACEdgeTile;
	u8 TileLayout;

	TileLayout = XRFdc_GetTileLayout(InstancePtr);
	DACEdgeTile = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_CLK_DST_TILE_227 : XRFDC_CLK_DST_TILE_228;
	if (DistTile > DACEdgeTile) { /*ADC*/
		*Type = XRFDC_ADC_TILE;
		*Tile_Id = XRFDC_CLK_DST_TILE_224 - DistTile;
	} else { /*DAC*/
		*Type = XRFDC_DAC_TILE;
		*Tile_Id = DACEdgeTile - DistTile;
	}
}
static u8 XRFdc_TypeTile2DistTile(XRFdc *InstancePtr, u32 Type, u32 Tile_Id)
{
	u8 DACEdgeTile;
	u8 TileLayout;
	u8 DistTile;
	TileLayout = XRFdc_GetTileLayout(InstancePtr);
	DACEdgeTile = (TileLayout == XRFDC_3ADC_2DAC_TILES) ? XRFDC_CLK_DST_TILE_227 : XRFDC_CLK_DST_TILE_228;
	if (Type == XRFDC_ADC_TILE) { /*ADC*/
		DistTile = XRFDC_CLK_DST_TILE_224 - Tile_Id;
	} else { /*DAC*/
		DistTile = DACEdgeTile - Tile_Id;
	}
	return DistTile;
}
/** @} */

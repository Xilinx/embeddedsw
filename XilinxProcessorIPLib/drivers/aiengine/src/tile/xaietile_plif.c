/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_plif.c
* @{
*
* This file contains routines for PL interface configuration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/10/2018  Use the mask write API
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Jubaer  03/07/2019  Add PL if downsizer disable API
* 1.5  Tejus   09/05/2019  Fix assertion issues
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiegbl_reginit.h"
#include "xaietile_plif.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_RegPlUpsz UpszCfg;
extern XAieGbl_RegPlDwsz DwszCfg;
extern XAieGbl_RegPlDwszEn DwszEn;
extern XAieGbl_RegPlDwszBypass DwszBypass;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to configure the width of the selected stream in the upsizer
* or downsizer.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Pl2Me - PLTOME or METOPL streams (1-PLTOME,0-ME2PL).
* @param	StreamId - Stream index value, ranging from 0-5 for METOPL and
*		0-7 for PLTOME
* @param	Width - Supported widths are 32, 64 and 128.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntfStrmWidCfg(XAieGbl_Tile *TileInstPtr, u8 Pl2Me, u8 StreamId,
								u8 Width)
{
	u64 RegAddr;
	u32 RegOff;
	u32 FldVal;
        u32 FldMask;
	u8 Idx;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

	XAie_AssertNonvoid((Width == XAIETILE_PLIF_STRM_WIDTH32) ||
				(Width == XAIETILE_PLIF_STRM_WIDTH64) ||
				(Width == XAIETILE_PLIF_STRM_WIDTH128));

	/*
	 * Supported streams for 128-bit width are 0,2,4,6 for ME2PL/upsizer
	 * and 0,2,4 for PL2ME/downsizer
	 */
	if(Width == XAIETILE_PLIF_STRM_WIDTH128) {
		XAie_AssertNonvoid((StreamId %
					XAIETILE_PLIF_STRM_WIDTH128_IDDIV) == 0U);
	}

	if(Pl2Me == XAIETILE_PLIF_PLTOAIE_STRMS_ENABLE) {
		XAie_AssertNonvoid(StreamId < XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS);
		RegOff = DwszCfg.RegOff;
		if(Width == XAIETILE_PLIF_STRM_WIDTH128) {
			Idx = StreamId / XAIETILE_PLIF_STRM_WIDTH128_IDDIV;
			FldVal = XAie_SetField(1U, DwszCfg.Wid128[Idx].Lsb,
						DwszCfg.Wid128[Idx].Mask);
                        FldMask = DwszCfg.Wid128[Idx].Mask;
		} else {
			Idx = StreamId;
			FldVal = XAie_SetField((Width >>
                                        XAIETILE_PLIF_STRM_WIDTH64_SHIFT),
                                        DwszCfg.Wid3264[Idx].Lsb,
					DwszCfg.Wid3264[Idx].Mask);
                        FldMask = DwszCfg.Wid3264[Idx].Mask;
		}
	} else {
		XAie_AssertNonvoid(StreamId < XAIEGBL_TILE_PLIF_AIE2PL_MAX_STRMS);
		RegOff = UpszCfg.RegOff;
		if(Width == XAIETILE_PLIF_STRM_WIDTH128) {
			Idx = StreamId / XAIETILE_PLIF_STRM_WIDTH128_IDDIV;
			FldVal = XAie_SetField(1U, UpszCfg.Wid128[Idx].Lsb,
						UpszCfg.Wid128[Idx].Mask);
                        FldMask = UpszCfg.Wid128[Idx].Mask;
		} else {
			Idx = StreamId;
			FldVal = XAie_SetField((Width >>
                                        XAIETILE_PLIF_STRM_WIDTH64_SHIFT),
                                        UpszCfg.Wid3264[Idx].Lsb,
					UpszCfg.Wid3264[Idx].Mask);
                        FldMask = UpszCfg.Wid3264[Idx].Mask;
		}
	}

	/* Read the upsizer or downsizer config register */
	RegAddr = TileInstPtr->TileAddr + RegOff;

	/* Write to the upsizer or downsizer configuration */
	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	/* If PL2ME streams, then disable the corresponding stream */
	if(Pl2Me == XAIETILE_PLIF_PLTOAIE_STRMS_ENABLE) {
		RegAddr = TileInstPtr->TileAddr + DwszEn.RegOff;

		XAieGbl_MaskWrite32(RegAddr, DwszEn.En[StreamId].Mask, 0);
	}
}

/*****************************************************************************/
/**
*
* This API is used to enable the selected stream in the PL2ME interface.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	StreamId - Stream index value, ranging from 0-7.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntfDownszrEnable(XAieGbl_Tile *TileInstPtr, u8 StreamId)
{
	u64 RegAddr;
	u32 FldVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(StreamId < XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS);

	RegAddr = TileInstPtr->TileAddr + DwszEn.RegOff;

	/* Enable the selected stream in the PL2ME/downsizer config */
	FldVal = XAie_SetField(1U, DwszEn.En[StreamId].Lsb,
			DwszEn.En[StreamId].Mask);

	XAieGbl_MaskWrite32(RegAddr, DwszEn.En[StreamId].Mask, FldVal);
}

/*****************************************************************************/
/**
*
* This API is used to disable the selected stream in the PL2ME interface.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	StreamId - Stream index value, ranging from 0-7.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntfDownszrDisable(XAieGbl_Tile *TileInstPtr, u8 StreamId)
{
	u64 RegAddr;
	u32 FldVal;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(StreamId < XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	RegAddr = TileInstPtr->TileAddr + DwszEn.RegOff;

	/* Disable the selected stream in the PL2ME/downsizer config */
	FldVal = XAie_SetField(0U, DwszEn.En[StreamId].Lsb,
			DwszEn.En[StreamId].Mask);

	XAieGbl_MaskWrite32(RegAddr, DwszEn.En[StreamId].Mask, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets the bypass of the selected stream in the PL2ME interface.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	StreamId - Stream index value, one from 0, 1, 2, 4, 5, or 6.
* @param	Enable - 0 for disable and 1 for enable
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlIntfDownszrSetBypass(XAieGbl_Tile *TileInstPtr, u8 StreamId,
		u8 Enable)
{
	u64 RegAddr;
	u32 FldVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(StreamId < XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS);
	XAie_AssertNonvoid(StreamId != 3 && StreamId != 7);
	XAie_AssertNonvoid(Enable == 0 || Enable == 1);

	RegAddr = TileInstPtr->TileAddr + DwszBypass.RegOff;

	/* StreamId 4-6 are mapped to bits 3-5 */
	if (StreamId > 3) {
		StreamId--;
	}

	FldVal = XAie_SetField(Enable, DwszBypass.Bypass[StreamId].Lsb,
			DwszBypass.Bypass[StreamId].Mask);

	XAieGbl_MaskWrite32(RegAddr, DwszBypass.Bypass[StreamId].Mask, FldVal);
}

/** @} */


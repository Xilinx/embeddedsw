/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiepm_clock.c
* @{
*
* This file contains routines for AIE clock gating
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date       Changes
* ----- ------  --------   -----------------------------------------------------
* 1.0   Dishita 03/03/2020 Initial Creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaiepm_clock.h"

/***************************** Macro Definitions *****************************/
#define CLOCK_GATE_VAL		0

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
* This is an internal API to clock gate entire column if no tile in that col
* is used.
*
* @param	TilePtr - Pointer to the shim tile of the col to be gated.
*
* @return	XAIE_SUCCESS on success
*
* @note		None
*
*******************************************************************************/
static u8 _XAiePm_ClockGateColumn(XAieGbl_Tile *TilePtr){

	u64 RegAddr = TilePtr->TileAddr;
	XAieGbl_Write32(RegAddr + XAIEGBL_PL_TILCLOCTRL, CLOCK_GATE_VAL);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/*
*
* This is an internal API to clock gate the next tile in column.
*
* @param	AieInst - Pointer to the Aie instance.
* @param	row - row number of the tile to be gated
* @param	col - col number of the tile to be gated
*
* @return	XAIE_SUCCESS on success
*
* @note		None
*
*******************************************************************************/
static u8 _XAiePm_ClockGateTile(XAieGbl *AieInst, u8 col, u8 row){

	XAieGbl_Tile *TilePtr;
	u32 NumRows;
	u64 RegAddr;

	NumRows = AieInst->Config->NumRows;
	TilePtr = AieInst->Tiles;
	TilePtr += col * (NumRows + 1U) + (row - 1U);
	RegAddr = TilePtr->TileAddr;
	XAieGbl_MaskWrite32(RegAddr + XAIEGBL_CORE_TILCLOCTRL,
				XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK,
								CLOCK_GATE_VAL);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/*
* This is an API to request tiles to use.
*
* @param	AieInst - Pointer to the Aie instance.
* @param	Loc - location of tiles in row, col.
* @param	NumTiles - total number of tiles requested.
*
* @return	XAIE_SUCCESS on success
*
* @note		This API assumes that all tiles' clock is enabled (not gated)
*		before calling this API.This API is not responsible if any
*		attempt to access any clock gated tile is made by the caller.
*
*******************************************************************************/
u8 XAiePm_RequestTiles(XAieGbl *AieInst, u32 NumTiles, XAie_LocType *Loc)
{
	XAieGbl_Tile *TilePtr;
	u32 NumCols, NumRows;
	u8 status = XAIE_SUCCESS;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertVoid(Loc != XAIE_NULL);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_AssertVoid(NumTiles <= NumCols * NumRows);

	/* mark all the input tiles as used using IsUsed field of its tileptr */
	for(u16 i = 0; i < NumTiles; i++) {
		XAie_AssertNonvoid(Loc[i].Row <= NumRows);
		XAie_AssertNonvoid(Loc[i].Col < NumCols);
		/*
		 * Since shim tile is always enabled, using IsUsed field of shim
		 * tile tileptr to mark the status of that col i.e. 1 if any
		 * tile in that col is used.
		 */
		TilePtr = AieInst->Tiles;
		TilePtr += Loc[i].Col * (NumRows + 1U);
		TilePtr->IsUsed = 1U;

		/* set the IsUsed field of tileptr for tile passed as input */
		TilePtr = AieInst->Tiles;
		TilePtr	+= Loc[i].Col * (NumRows + 1U) + Loc[i].Row;
		TilePtr->IsUsed = 1U;
	}

	/* gate all unused tiles */
	for(u8 j = 0; j < NumCols; j++){

		/* gate entire col if entire col is unused */
		TilePtr = AieInst->Tiles;
		TilePtr += j * (NumRows + 1);
		if(TilePtr->IsUsed != 1){
			status = _XAiePm_ClockGateColumn(TilePtr);
		}

		/* gate unused tile per col */
		else{
			for(u8 k = NumRows; k > 0; k--){
				TilePtr = AieInst->Tiles;
				TilePtr += j * (NumRows + 1U) + k;
				/*
				 * If a top tile in a col is in use, all the
				 * tiles below that tile in that col must not be
				 * gated.
				 */
				if(TilePtr->IsUsed == 1U)
					break;
				else{
					status = _XAiePm_ClockGateTile(AieInst, j, k);
				}
			}
		}
	}
	return status;
}

/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_aie.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs for AIE
* device generation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_AIE_H
#define XAIE_LITE_AIE_H

/***************************** Include Files *********************************/
/************************** Constant Definitions *****************************/
#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			0x20000000000
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			9
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			50
#endif

#define XAIE_COL_SHIFT			23
#define XAIE_ROW_SHIFT			18
#define XAIE_SHIM_ROW			0
#define XAIE_MEM_TILE_ROW_START		0
#define XAIE_MEM_TILE_NUM_ROWS		0
#define XAIE_AIE_TILE_ROW_START		1
#define XAIE_AIE_TILE_NUM_ROWS		8

#define UPDT_NEXT_NOC_TILE_LOC(Loc)	\
	({if ((Loc).Col <= 1) \
		(Loc).Col = 2; \
	else \
		(Loc).Col += ((Loc).Col % 2) * 2 + 1;})

#include "xaie_lite_regdef_aie.h"
#include "xaie_lite_regops_aie.h"

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
/*****************************************************************************/
/**
*
* This API checks if an AI engine tile is in use.
*
* @param	DevInst: Device Instance.
* @param	Loc: Tile location.
*
* @return	XAIE_ENABLE if a tile is in use, otherwise XAIE_DISABLE.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LPmIsTileRequested(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	(void) DevInst;
	(void) Loc.Col;
	(void) Loc.Row;

	/* TODO: Implement lite API to scan AIE array and update bitmap */
	return XAIE_ENABLE;
}

/*****************************************************************************/
/**
*
* This is API returns the tile type for a given device instance and tile
* location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LGetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 ColType;

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR_RETURN("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		ColType = Loc.Col % 4U;
		if((ColType == 0U) || (ColType == 1U)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}
		return XAIEGBL_TILE_TYPE_SHIMNOC;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR_RETURN("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This API maps L2 status bit to its L1 switch.
*
* @param	DevInst: Device Instance.
* @param	Index: Set bit position in L2 status.
* @param	L2Col: Location of L2 column.
* @param	L1Col: Mapped value of L1 column.
* @param	Switch: Broadcast switch.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
static inline void _XAie_MapL2MaskToL1(XAie_DevInst *DevInst, u32 Index,
			u8 L2Col, u8 *L1Col, XAie_BroadcastSw *Switch)
{
	if (L2Col + 3 >=  DevInst->NumCols) {
	        *L1Col = L2Col + (Index % 6) / 2;
	        *Switch = (Index % 6) % 2;
	} else if ((L2Col) % 2 == 0) {
	        /* Set bit position could be 0 - 5 */
	        *L1Col = L2Col - (2 - (Index % 6) / 2);
	        *Switch = (Index % 6) % 2;
	} else {
	        /* Set bit position could be 0 - 1 */
	        *L1Col = L2Col;
	        *Switch= Index;
	}
}

#endif		/* end of protection macro */

/** @} */

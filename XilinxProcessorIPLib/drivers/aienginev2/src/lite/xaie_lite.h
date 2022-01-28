/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs.
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
#ifndef XAIE_LITE_H
#define XAIE_LITE_H

#ifdef XAIE_FEATURE_LITE

#include "xaiegbl_defs.h"

#define __FORCE_INLINE__			__attribute__((always_inline))

#if XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE
#include "xaie_lite_aie.h"
#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIEML
#include "xaie_lite_aieml.h"
#else
#include <xaie_custom_device.h>
#endif

#define XAie_LDeclareDevInst(DevInst, _BaseAddr, _StartCol, _NumCols) \
	XAie_DevInst DevInst = { \
		.BaseAddr = (_BaseAddr), \
		.StartCol = (_StartCol), \
		.NumCols = (_NumCols), \
	}

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
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
	XAIE_ERROR_RETURN((Loc.Col >= XAIE_NUM_COLS), XAIEGBL_TILE_TYPE_MAX,
				"Invalid column: %d\n", Loc.Col);

	if(Loc.Row == 0U) {
		return _XAie_LGetShimTTypefromLoc(DevInst, Loc);
	} else if(Loc.Row >= XAIE_MEM_TILE_ROW_START &&
			(Loc.Row < (XAIE_MEM_TILE_ROW_START +
				    XAIE_MEM_TILE_NUM_ROWS))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= XAIE_AIE_TILE_ROW_START &&
			(Loc.Row < (XAIE_AIE_TILE_ROW_START +
				    XAIE_AIE_TILE_NUM_ROWS))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR_RETURN(1U, XAIEGBL_TILE_TYPE_MAX, "Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

#endif /* XAIE_FEATURE_LITE */

#endif /* XAIE_LITE_H */

/** @} */

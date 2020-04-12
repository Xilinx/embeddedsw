/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_shim.c
* @{
*
* This file contains routines for Shim tile
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    10/12/2018  Initial creation
* 1.1  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaietile_shim.h"

/***************************** Constant Definitions **************************/
/***************************** Macro Definitions *****************************/
/************************** Variable Definitions *****************************/
extern XAieGbl_RegShimColumnReset ShimColumnReset;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This API asserts / deasserts the colum reset to given time
*
* @param	TileInstPtr - Pointer to the Tile instance. Should be shim tile.
* @param	Reset - 0 for deassert. 1 for assert.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_ShimColumnReset(XAieGbl_Tile *TileInstPtr, u8 Reset)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + ShimColumnReset.RegOff, !!Reset);

	return XAIE_SUCCESS;
}

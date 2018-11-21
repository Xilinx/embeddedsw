/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmetile_shim.c
* @{
*
* This file contains routines for Shim tile
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Hyun    10/12/2018  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmegbl.h"
#include "xmegbl_defs.h"
#include "xmegbl_reginit.h"
#include "xmetile_shim.h"

/***************************** Constant Definitions **************************/
/***************************** Macro Definitions *****************************/
/************************** Variable Definitions *****************************/
extern XMeGbl_RegShimColumnReset ShimColumnReset;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This API asserts / deasserts the colum reset to given time
*
* @param	TileInstPtr - Pointer to the Tile instance. Should be shim tile.
* @param	Reset - 0 for deassert. 1 for assert.
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTile_ShimColumnReset(XMeGbl_Tile *TileInstPtr, u8 Reset)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType != XMEGBL_TILE_TYPE_METILE);

	XMeGbl_Write32(TileInstPtr->TileAddr + ShimColumnReset.RegOff, !!Reset);

	return XME_SUCCESS;
}

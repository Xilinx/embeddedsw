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
* @file xmetile_mem.c
* @{
*
* This file contains routines for the ME Tile memory control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/18/2018  Initial creation to fix CR#1000217
* 1.1  Naresh  07/11/2018  Updated copyright info and addressed CR#1006589
* </pre>
*
******************************************************************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmegbl_reginit.h"
#include "xmetile_mem.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API writes a 32-bit value to the specified data memory location for
* the selected tile.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmOffset - Data memory offset to write to.
* @param	DmVal - 32-bit Value to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeTile_DmWriteWord(XMeGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal)
{
	u64 DmAddr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

	/* Write to the Data memory location */
	DmAddr = TileInstPtr->TileAddr + XMEGBL_TILE_DATAMEM_BASE + DmOffset;

	XMeGbl_Write32(DmAddr, DmVal);
}

/*****************************************************************************/
/**
*
* This API reads a 32-bit value from the specified data memory location for
* the selected tile.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	DmOffset - Data memory offset to write to.
*
* @return	32-bit Value.
*
* @note		None.
*
*******************************************************************************/
u32 XMeTile_DmReadWord(XMeGbl_Tile *TileInstPtr, u32 DmOffset)
{
	u64 DmAddr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

	/* Read from the Data memory location */
	DmAddr = TileInstPtr->TileAddr + XMEGBL_TILE_DATAMEM_BASE + DmOffset;

	return(XMeGbl_Read32(DmAddr));
}

/** @} */


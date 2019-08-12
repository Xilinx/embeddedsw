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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaietile_mem.c
* @{
*
* This file contains routines for the AIE Tile memory control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/18/2018  Initial creation to fix CR#1000217
* 1.1  Naresh  07/11/2018  Updated copyright info and addressed CR#1006589
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Hyun    06/27/2019  Add XAieTile_MemReadTimer()
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiegbl_reginit.h"
#include "xaietile_mem.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_RegTimer TimerReg[];

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
void XAieTile_DmWriteWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal)
{
	u64 DmAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

	/* Write to the Data memory location */
	DmAddr = TileInstPtr->TileAddr + XAIEGBL_TILE_DATAMEM_BASE + DmOffset;

	XAieGbl_Write32(DmAddr, DmVal);
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
u32 XAieTile_DmReadWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset)
{
	u64 DmAddr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

	/* Read from the Data memory location */
	DmAddr = TileInstPtr->TileAddr + XAIEGBL_TILE_DATAMEM_BASE + DmOffset;

	return(XAieGbl_Read32(DmAddr));
}

/*****************************************************************************/
/**
*
* This API returns the current value of the Memory module 64-bit timer.
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	64-bit timer value.
*
* @note		None.
*
*******************************************************************************/
u64 XAieTile_MemReadTimer(XAieGbl_Tile *TileInstPtr)
{
	u32 CurValHigh;
	u32 CurValLow;
	u64 CurVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	CurValLow = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_MEM].LowOff);
	CurValHigh = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_MEM].HighOff);
	CurVal = ((u64)CurValHigh << 0x20U) | CurValLow;

	return CurVal;
}

/** @} */


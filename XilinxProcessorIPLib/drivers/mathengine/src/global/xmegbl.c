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
* @file xmegbl.c
* @{
*
* This file contains the global initialization functions for the Tile.
* This is applicable for both the ME tiles and Shim tiles.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  04/12/2018  Code changed to fix CR#999685
* 1.2  Naresh  05/23/2018  Updated code to fix CR#999693
* 1.3  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
extern XMeGbl_Config XMeGbl_ConfigTable[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the global initialization function for all the tiles of the ME array
* and also for the Shim tiles. The initialization involves programming the Tile
* instance data structure with the required parameters of the tile, like base
* addresses for Core module/Memory module/NoC module/Pl module, Stream switch
* configuration, Lock configuration etc.
*
* @param	InstancePtr - Global ME instance structure.
* @param	ConfigPtr - Global ME configuration pointer.
*
* @return	void.
*
* @note		None.
*
******************************************************************************/
void XMeGbl_CfgInitialize(XMeGbl *InstancePtr, XMeGbl_Tile *TileInstPtr,
                                                XMeGbl_Config *ConfigPtr)
{
	u16 RowIdx;
	u16 ColIdx;
	u64 TileAddr;

	XMeGbl_Tile *TilePtr;

	XMe_AssertNonvoid(InstancePtr != XME_NULL);
	XMe_AssertNonvoid(ConfigPtr != XME_NULL);

	if(InstancePtr->IsReady != XME_COMPONENT_IS_READY) {
		InstancePtr->IsReady = 0U;
		InstancePtr->Config = ConfigPtr;
		InstancePtr->IsReady = XME_COMPONENT_IS_READY;
		XMeLib_InitDev();

		/* Row index starts with 1 as row-0 is for shim */
		for(RowIdx = 1; RowIdx <= ConfigPtr->NumRows; RowIdx++) {

			for(ColIdx=0; ColIdx < ConfigPtr->NumCols; ColIdx++) {

				TilePtr = (XMeGbl_Tile *)((u64)TileInstPtr +
                                        ((ColIdx * (ConfigPtr->NumRows + 1)) *
                                        sizeof(XMeGbl_Tile)) +
                                        (RowIdx * sizeof(XMeGbl_Tile)));

				TilePtr->RowId = RowIdx; /* Row index */
				TilePtr->ColId = ColIdx; /* Column index */

				/*
				 * Tile address format:
				 * --------------------------------------------
				 * |                7 bits  5 bits   18 bits  |
				 * --------------------------------------------
				 * | Array offset | Column | Row | Tile addr  |
				 * --------------------------------------------
				 */
				TileAddr = (u64)(((u64)ConfigPtr->ArrOffset <<
						XMEGBL_TILE_ADDR_ARR_SHIFT) |
					(ColIdx << XMEGBL_TILE_ADDR_COL_SHIFT) |
					(RowIdx << XMEGBL_TILE_ADDR_ROW_SHIFT));

				TilePtr->TileAddr = TileAddr;

				/* Set memory module base address for tile */
				TilePtr->MemModAddr = TileAddr +
						XMEGBL_TILE_ADDR_MEMMODOFF;

				/* Set core module base address for tile */
				TilePtr->CoreModAddr = TileAddr +
						XMEGBL_TILE_ADDR_COREMODOFF;

				TilePtr->NocModAddr = 0U;
				TilePtr->PlModAddr = 0U;

				/* Set locks base address in memory module */
				TilePtr->LockAddr = TilePtr->MemModAddr+
						XMEGBL_TILE_ADDR_MEMLOCKOFF;

				/* Set Stream SW base address in core module */
				TilePtr->StrmSwAddr =
						TilePtr->CoreModAddr +
						XMEGBL_TILE_ADDR_CORESTRMOFF;

				TilePtr->TileType = XMEGBL_TILE_TYPE_METILE;

				TilePtr->IsReady = XME_COMPONENT_IS_READY;
				XMeLib_InitTile(TilePtr);

				XMe_print("Tile addr:%016lx, Row idx:%d, "
					"Col idx:%d, Memmodaddr:%016lx, "
					"Coremodaddr:%016lx\n",TileAddr,RowIdx,
					ColIdx, TilePtr->MemModAddr,
					TilePtr->CoreModAddr);
			}
		}

		/* Initialize the Shim tiles here */
		for(ColIdx=0; ColIdx < ConfigPtr->NumCols; ColIdx++) {

                        TilePtr = (XMeGbl_Tile *)((u64)TileInstPtr +
                                        ((ColIdx * (ConfigPtr->NumRows + 1)) *
                                        sizeof(XMeGbl_Tile)));

			TilePtr->RowId = 0U; /* Row index */
			TilePtr->ColId = ColIdx; /* Column index */

			TileAddr = (u64)(((u64)ConfigPtr->ArrOffset <<
					XMEGBL_TILE_ADDR_ARR_SHIFT) |
					(ColIdx << XMEGBL_TILE_ADDR_COL_SHIFT));

			TilePtr->TileAddr = TileAddr;
			TilePtr->MemModAddr = 0U;
			TilePtr->CoreModAddr = 0U;

			/* Set Noc module base address for tile */
			TilePtr->NocModAddr = TileAddr +
						XMEGBL_TILE_ADDR_NOCMODOFF;
			/* Set PL module base address for tile */
			TilePtr->PlModAddr = TileAddr +
						XMEGBL_TILE_ADDR_PLMODOFF;

			/* Set locks base address in NoC module */
			TilePtr->LockAddr = TilePtr->NocModAddr +
						XMEGBL_TILE_ADDR_NOCLOCKOFF;

			/* Set Stream SW base address in PL module */
			TilePtr->StrmSwAddr = TilePtr->PlModAddr +
						XMEGBL_TILE_ADDR_PLSTRMOFF;

			TilePtr->TileType = XMEGBL_TILE_TYPE_SHIMNOC;

			TilePtr->IsReady = XME_COMPONENT_IS_READY;

			XMe_print("Tile addr:%016lx, Row idx:%d, Col idx:%d, "
				"Nocmodaddr:%016lx, Plmodaddr:%016lx\n",
				TileAddr, 0U, ColIdx, TilePtr->NocModAddr,
				TilePtr->PlModAddr);
		}
	}
}

/*****************************************************************************/
/**
*
* This is the routine to initialize the HW configuration.
*
* @param	CfgPtr: Pointer to the HW configuration data structure.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeGbl_HwInit(XMeGbl_HwCfg *CfgPtr)
{
        XMeGbl_ConfigTable->NumRows = CfgPtr->NumRows;
	XMeGbl_ConfigTable->NumCols = CfgPtr->NumCols;
	XMeGbl_ConfigTable->ArrOffset = CfgPtr->ArrayOff;
}

/** @} */

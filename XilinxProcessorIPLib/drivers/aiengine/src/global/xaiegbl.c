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
* @file xaiegbl.c
* @{
*
* This file contains the global initialization functions for the Tile.
* This is applicable for both the AIE tiles and Shim tiles.
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
* 1.4  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.5  Jubaer  05/24/2019  Add PL type on TileType attribute
* 1.6  Nishad  07/31/2019  Add support for RPU baremetal
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/
/**
 * XAIE_BASE_ARRAY_ADDR_OFFSET macro defines the AI Engine's base address offset
 * value. This value is left-shift by 30-bits to obtain the complete physical
 * address of AIE.
 *
 * FIXME: The Makefile used to compile AIE application for ARM Cortex-R5,
 * by default, adds 'ARMR5' compiler flag. Since the XSA file generated using
 * the latest tool hides the remapped address of AIE for R5, this compiler flag
 * can be leveraged to hardcode the value of XAIE_BASE_ARRAY_ADDR_OFFSET.
 * When AIE adress is available in RPU address map, the aiengine.tcl file for
 * BareMetal and XAieIO_Init API for Linux flow must be modified to remove the
 * hardcoded value.
 * For BareMetal applications, the XAIE_BASE_ARRAY_ADDR_OFFSET value needs to be
 * parsed by the TCL script from the XSA file. On the other hand, for the Linux
 * application, this value needs to be parsed from the device tree.
 *
 */
#ifdef ARMR5
#define XAIE_BASE_ARRAY_ADDR_OFFSET	0x1
#endif

/************************** Variable Definitions *****************************/
extern XAieGbl_Config XAieGbl_ConfigTable[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the global initialization function for all the tiles of the AIE array
* and also for the Shim tiles. The initialization involves programming the Tile
* instance data structure with the required parameters of the tile, like base
* addresses for Core module/Memory module/NoC module/Pl module, Stream switch
* configuration, Lock configuration etc.
*
* @param	InstancePtr - Global AIE instance structure.
* @param	ConfigPtr - Global AIE configuration pointer.
*
* @return	void.
*
* @note		None.
*
******************************************************************************/
void XAieGbl_CfgInitialize(XAieGbl *InstancePtr, XAieGbl_Tile *TileInstPtr,
                                                XAieGbl_Config *ConfigPtr)
{
	u16 RowIdx;
	u16 ColIdx;
	u64 TileAddr;

	XAieGbl_Tile *TilePtr;

	XAie_AssertNonvoid(InstancePtr != XAIE_NULL);
	XAie_AssertNonvoid(ConfigPtr != XAIE_NULL);

	if(InstancePtr->IsReady != XAIE_COMPONENT_IS_READY) {
		InstancePtr->IsReady = 0U;
		InstancePtr->Config = ConfigPtr;
		InstancePtr->IsReady = XAIE_COMPONENT_IS_READY;
		XAieLib_InitDev();

#ifdef XAIE_BASE_ARRAY_ADDR_OFFSET
ConfigPtr->ArrOffset = XAIE_BASE_ARRAY_ADDR_OFFSET;
#endif

		/* Row index starts with 1 as row-0 is for shim */
		for(RowIdx = 1; RowIdx <= ConfigPtr->NumRows; RowIdx++) {

			for(ColIdx=0; ColIdx < ConfigPtr->NumCols; ColIdx++) {

				TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
                                        ((ColIdx * (ConfigPtr->NumRows + 1)) *
                                        sizeof(XAieGbl_Tile)) +
                                        (RowIdx * sizeof(XAieGbl_Tile)));

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
						XAIEGBL_TILE_ADDR_ARR_SHIFT) |
					(ColIdx << XAIEGBL_TILE_ADDR_COL_SHIFT) |
					(RowIdx << XAIEGBL_TILE_ADDR_ROW_SHIFT));

				TilePtr->TileAddr = TileAddr;

				/* Set memory module base address for tile */
				TilePtr->MemModAddr = TileAddr +
						XAIEGBL_TILE_ADDR_MEMMODOFF;

				/* Set core module base address for tile */
				TilePtr->CoreModAddr = TileAddr +
						XAIEGBL_TILE_ADDR_COREMODOFF;

				TilePtr->NocModAddr = 0U;
				TilePtr->PlModAddr = 0U;

				/* Set locks base address in memory module */
				TilePtr->LockAddr = TilePtr->MemModAddr+
						XAIEGBL_TILE_ADDR_MEMLOCKOFF;

				/* Set Stream SW base address in core module */
				TilePtr->StrmSwAddr =
						TilePtr->CoreModAddr +
						XAIEGBL_TILE_ADDR_CORESTRMOFF;

				TilePtr->TileType = XAIEGBL_TILE_TYPE_AIETILE;

				TilePtr->IsReady = XAIE_COMPONENT_IS_READY;
				XAieLib_InitTile(TilePtr);

				XAie_print("Tile addr:%016lx, Row idx:%d, "
					"Col idx:%d, Memmodaddr:%016lx, "
					"Coremodaddr:%016lx\n",TileAddr,RowIdx,
					ColIdx, TilePtr->MemModAddr,
					TilePtr->CoreModAddr);
			}
		}

		/* Initialize the Shim tiles here */
		for(ColIdx=0; ColIdx < ConfigPtr->NumCols; ColIdx++) {

                        TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
                                        ((ColIdx * (ConfigPtr->NumRows + 1)) *
                                        sizeof(XAieGbl_Tile)));

			TilePtr->RowId = 0U; /* Row index */
			TilePtr->ColId = ColIdx; /* Column index */

			TileAddr = (u64)(((u64)ConfigPtr->ArrOffset <<
					XAIEGBL_TILE_ADDR_ARR_SHIFT) |
					(ColIdx << XAIEGBL_TILE_ADDR_COL_SHIFT));

			TilePtr->TileAddr = TileAddr;
			TilePtr->MemModAddr = 0U;
			TilePtr->CoreModAddr = 0U;

			/* Set Noc module base address for tile */
			TilePtr->NocModAddr = TileAddr +
						XAIEGBL_TILE_ADDR_NOCMODOFF;
			/* Set PL module base address for tile */
			TilePtr->PlModAddr = TileAddr +
						XAIEGBL_TILE_ADDR_PLMODOFF;

			/* Set locks base address in NoC module */
			TilePtr->LockAddr = TilePtr->NocModAddr +
						XAIEGBL_TILE_ADDR_NOCLOCKOFF;

			/* Set Stream SW base address in PL module */
			TilePtr->StrmSwAddr = TilePtr->PlModAddr +
						XAIEGBL_TILE_ADDR_PLSTRMOFF;

			switch (ColIdx % 4) {
			case 0:
			case 1:
				TilePtr->TileType = XAIEGBL_TILE_TYPE_SHIMPL;
				break;
			default:
				TilePtr->TileType = XAIEGBL_TILE_TYPE_SHIMNOC;
				break;
			}

			TilePtr->IsReady = XAIE_COMPONENT_IS_READY;

			XAie_print("Tile addr:%016lx, Row idx:%d, Col idx:%d, "
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
void XAieGbl_HwInit(XAieGbl_HwCfg *CfgPtr)
{
        XAieGbl_ConfigTable->NumRows = CfgPtr->NumRows;
	XAieGbl_ConfigTable->NumCols = CfgPtr->NumCols;
	XAieGbl_ConfigTable->ArrOffset = CfgPtr->ArrayOff;
}

/** @} */

/******************************************************************************
* Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_pl.h
* @{
*
*  Header file for PL module APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who       Date        Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Jubaer  01/29/2019  Initial creation
* 1.1  Jubaer  03/07/2019  Add Shim reset API
* 1.2  Hyun    06/27/2019  Add XAieTile_PlReadTimer()
* 1.3  Wendy   02/26/2020  Add get/set 1st level IRQ events
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_PL_H
#define XAIETILE_PL_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_PL_BLOCK_SWITCHA			0U
#define XAIETILE_PL_BLOCK_SWITCHB			1U

#define XAIETILE_PL_INTERN_EVENT16			16U
#define XAIETILE_PL_INTERN_EVENT17			17U
#define XAIETILE_PL_INTERN_EVENT18			18U
#define XAIETILE_PL_INTERN_EVENT19			18U

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u32 XAieTile_PlIntcL1Mask(XAieGbl_Tile *TileInstPtr, u8 SwitchAB);
void XAieTile_PlIntcL1Enable(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
void XAieTile_PlIntcL1Disable(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
u32 XAieTile_PlIntcL1StatusGet(XAieGbl_Tile *TileInstPtr, u8 SwitchAB);
void XAieTile_PlIntcL1StatusClr(XAieGbl_Tile *TileInstPtr, u32 Status, u8 SwitchAB);
u32 XAieTile_PlIntcL1IrqNoGet(XAieGbl_Tile *TileInstPtr, u8 SwitchAB);
void XAieTile_PlIntcL1IrqNoSet(XAieGbl_Tile *TileInstPtr, u32 irqNum, u8 SwitchAB);
u32 XAieTile_PlIntcL1IrqEventGet(XAieGbl_Tile *TileInstPtr, u8 IrqEvent, u8 SwitchAB);
u32 XAieTile_PlIntcL1IrqEventSet(XAieGbl_Tile *TileInstPtr, u8 IrqEvent, u8 Event, u8 SwitchAB);
u32 XAieTile_PlIntcL1BlockNorthVal(XAieGbl_Tile *TileInstPtr, u8 SwitchAB);
void XAieTile_PlIntcL1BlockNorthSet(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
void XAieTile_PlIntcL1BlockNorthClr(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
u8 XAieTile_PlShimResetEnable(XAieGbl_Tile *TileInstPtr, u8 Reset);
u64 XAieTile_PlReadTimer(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */

/** @} */

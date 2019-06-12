/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/******************************************************************************/
/**
 * @file xaietile_noc.h
 * @{
 *
 *  Header file for NoC module
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	Who	Date		Changes
 * ----- ------  -------- -----------------------------------------------------
 * 1.0	Nishad	01/29/2019	Initial creation
 * </pre>
 *
 ******************************************************************************/
#ifndef XAIETILE_NOC_H
#define XAIETILE_NOC_H

/****************************** Include Files *********************************/
/****************************** Constant Definitions **************************/
/****************************** Type Definitions ******************************/
/****************************** Macro Definitions *****************************/
/*************************** Function Prototypes  *****************************/

u16 XAieTile_NoCIntcL2StatusGet(XAieGbl_Tile *TileInstPtr);
void XAieTile_NoCIntcL2StatusClear(XAieGbl_Tile *TileInstPtr, u16 Mask);
u16 XAieTile_NoCIntcL2Mask(XAieGbl_Tile *TileInstPtr);
void XAieTile_NoCIntcL2Enable(XAieGbl_Tile *TileInstPtr, u16 InterruptBitMask);
void XAieTile_NoCIntcL2Disable(XAieGbl_Tile *TileInstPtr, u16 InterruptBitMask);
u8 XAieTile_NoCIntcL2IntrGet(XAieGbl_Tile *TileInstPtr);
void XAieTile_NoCIntcL2IntrSet(XAieGbl_Tile *TileInstPtr, u8 NoCInterrupt);

#endif		/* end of protection macro */

/** @} */

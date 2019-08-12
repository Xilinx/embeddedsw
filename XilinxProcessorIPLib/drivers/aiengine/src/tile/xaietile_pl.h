/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_PL_H
#define XAIETILE_PL_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_PL_BLOCK_SWITCHA			0U
#define XAIETILE_PL_BLOCK_SWITCHB			1U

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
u32 XAieTile_PlIntcL1BlockNorthVal(XAieGbl_Tile *TileInstPtr, u8 SwitchAB);
void XAieTile_PlIntcL1BlockNorthSet(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
void XAieTile_PlIntcL1BlockNorthClr(XAieGbl_Tile *TileInstPtr, u32 Mask, u8 SwitchAB);
u8 XAieTile_PlShimResetEnable(XAieGbl_Tile *TileInstPtr, u8 Reset);
u64 XAieTile_PlReadTimer(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */

/** @} */

/*******************************************************************************
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

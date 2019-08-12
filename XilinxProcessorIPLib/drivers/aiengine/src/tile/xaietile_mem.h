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
* @file xaietile_mem.h
* @{
*
*  Header file for Tile memory control functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/18/2018  Initial creation to fix CR#1000217
* 1.1  Naresh  07/11/2018  Updated copyright info and addressed CR#1006589
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Hyun    06/27/2018  Add XAieTile_MemReadTimer()
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_MEM_H
#define XAIETILE_MEM_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XAieTile_DmWriteWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal);
u32 XAieTile_DmReadWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset);
u64 XAieTile_MemReadTimer(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */
/** @} */


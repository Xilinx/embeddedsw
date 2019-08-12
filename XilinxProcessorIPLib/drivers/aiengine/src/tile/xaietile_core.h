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
* @file xaietile_core.h
* @{
*
*  Header file for core control and wait functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  05/07/2018  Fixed CR#1000933
* 1.2  Naresh  07/11/2018  Updated copyright info and addressed CR#1006573
* 1.3  Naresh  08/13/2018  Updated prototype for wait done API and also added
*                          prototype for core read status done API
* 1.4  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_CORE_H
#define XAIETILE_CORE_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_CORE_STATUS_DONE                1U
#define XAIETILE_CORE_STATUS_DISABLE             0U

#define XAIETILE_CORE_STATUS_DEF_WAIT_USECS      500U

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XAieTile_CoreControl(XAieGbl_Tile *TileInstPtr, u8 Enable, u8 Reset);
u8 XAieTile_CoreWaitStatus(XAieGbl_Tile *TileInstPtr, u32 TimeOut, u32 Status);
u8 XAieTile_CoreWaitCycles(XAieGbl_Tile *TileInstPtr, u32 CycleCnt);
u8 XAieTile_CoreReadStatusDone(XAieGbl_Tile *TileInstPtr);
u64 XAieTile_CoreReadTimer(XAieGbl_Tile *TileInstPtr);

#endif		/* end of protection macro */
/** @} */


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
* @file xaietile_perfcnt.h
* @{
*
*  Header file for Performance Counter control
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    10/02/2018  Initial creation
* 1.1  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_PERFCNT_H
#define XAIETILE_PERFCNT_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_PERFCNT_EVENT_INVALID		0xFFFFU

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u8 XAieTileCore_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XAieTileCore_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter);
u32 XAieTileCore_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XAieTileCore_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

u8 XAieTilePl_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XAieTilePl_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter);
u32 XAieTilePl_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XAieTilePl_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

u8 XAieTileMem_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XAieTileMem_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter);
u32 XAieTileMem_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XAieTileMem_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

#endif		/* end of protection macro */

/** @} */


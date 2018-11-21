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
* @file xmetile_perfcnt.h
* @{
*
*  Header file for Performance Counter control
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Hyun    10/02/2018  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XMETILE_PERFCNT_H
#define XMETILE_PERFCNT_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XMETILE_PERFCNT_EVENT_INVALID		0xFFFFU

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u8 XMeTileCore_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XMeTileCore_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter);
u32 XMeTileCore_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XMeTileCore_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

u8 XMeTilePl_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XMeTilePl_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter);
u32 XMeTilePl_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XMeTilePl_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

u8 XMeTileMem_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent);
u32 XMeTileMem_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter);
u32 XMeTileMem_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 CounterVal);
u32 XMeTileMem_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter, u32 EventVal);

#endif		/* end of protection macro */

/** @} */


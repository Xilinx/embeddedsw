/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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


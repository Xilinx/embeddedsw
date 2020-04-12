/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_timer.h
* @{
*
*  Header file for timer
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Wendy   01/02/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_TIMER_H
#define XAIETILE_TIMER_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XAieTile_CoreSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr, u32 LowEventValue, u32 HighEventValue);
void XAieTile_CoreResetTimer(XAieGbl_Tile *TileInstPtr);
u32 XAieTile_CoreSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event, u8 Reset);
void XAieTile_MemSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr, u32 LowEventValue, u32 HighEventValue);
void XAieTile_MemResetTimer(XAieGbl_Tile *TileInstPtr);
u32 XAieTile_MemSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event, u8 Reset);
void XAieTile_PlSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr, u32 LowEventValue, u32 HighEventValue);
void XAieTile_PlResetTimer(XAieGbl_Tile *TileInstPtr);
u32 XAieTile_PlSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event, u8 Reset);

#endif		/* end of protection macro */

/** @} */

/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_timer.c
* @{
*
* This file contains routines for timer configuration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   01/02/2020  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaietile_timer.h"

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_RegTimer TimerReg[];

/************************** Internal Function Definitions ********************/
/*****************************************************************************/
/**
*
* This API sets the timer trigger events value. Timer low event will generate
* if the timer low reaches the specified low event value. Timer high event
* will generate if the timer high reaches the specified high event value.
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	LowEventValue - Value to set for the timer to trigger timer low
*                               event.
* @param	HighEventValue - Value to set for the timer to trigger timer
*                                high event.
* @param	ModuleType - Type of the module
*
* @return	None.
*
* @note		Used only within this file.
*
*******************************************************************************/
static void XAieTile_SetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr,
					  u32 LowEventValue, u32 HighEventValue,
					  u32 ModuleType)
{
	u64 RegAddr;

	/* Set up Timer low event value */
	RegAddr = TileInstPtr->TileAddr +
		  TimerReg[ModuleType].TrigEventLowValOff;
	XAieGbl_Write32(RegAddr, LowEventValue);
	/* Set up Timer high event value */
	RegAddr = TileInstPtr->TileAddr +
		  TimerReg[ModuleType].TrigEventHighValOff;
	XAieGbl_Write32(RegAddr, HighEventValue);
}

/*****************************************************************************/
/**
*
* This API resets the timer
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	ModuleType - Type of the module
*
* @return	None.
*
* @note		Used only within this file.
*
*******************************************************************************/
static void XAieTile_ResetTimer(XAieGbl_Tile *TileInstPtr, u32 ModuleType)
{
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = TileInstPtr->TileAddr + TimerReg[ModuleType].CtrlOff;
	Mask = TimerReg[ModuleType].CtrlReset.Mask;
	RegVal = XAie_SetField(XAIE_RESETENABLE,
			       TimerReg[ModuleType].CtrlReset.Lsb, Mask);
	XAieGbl_MaskWrite32(RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API sets the timer reset event. The timer will reset when the event
* is raised.
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	Event - Reset event ID (0 - 127)
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
* @param	ModuleType - Type of the module
*
* @return	XAIE_SUCCESS
*
* @note		Used only within this file.
*
*******************************************************************************/
static u32 XAieTile_SetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event,
					u8 Reset, u32 ModuleType)
{
	u64 RegAddr;
	u32 RegVal, Mask, Lsb;

	XAie_AssertNonvoid(Reset == XAIE_RESETENABLE ||
			   Reset == XAIE_RESETDISABLE);
	Mask = TimerReg[ModuleType].CtrlResetEvent.Mask;
	Lsb = TimerReg[ModuleType].CtrlResetEvent.Lsb;
	RegVal = XAie_SetField(Event, Lsb, Mask);
	XAie_AssertNonvoid((RegVal >> Lsb) == Event);

	if (Reset == XAIE_RESETENABLE) {
		Mask = TimerReg[ModuleType].CtrlReset.Mask;
		Lsb = TimerReg[ModuleType].CtrlReset.Lsb;
		RegVal |= XAie_SetField(XAIE_RESETENABLE, Lsb, Mask);
	}
	RegAddr = TileInstPtr->TileAddr + TimerReg[ModuleType].CtrlOff;
	XAieGbl_Write32(RegAddr, RegVal);
	return XAIE_SUCCESS;
}

/************************** Function Definitions ********************/
/*****************************************************************************/
/**
*
* This API sets the Core timer trigger events value.
* Timer low event will generate if the timer reaches the low 32bit of the event
* value. Timer high event will generate if the timer reaches the high 32bit
* of the event value.
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	LowEventValue - Timer value to trigger timer low events.
* @param	HighEventValue - Timer value to trigger timer high events.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr,
				       u32 LowEventValue, u32 HighEventValue)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_SetTimerTrigEventVal(TileInstPtr, LowEventValue,
				      HighEventValue,
				      XAIETILE_TIMER_MODULE_CORE);
}

/*****************************************************************************/
/**
*
* This API resets the Core timer
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreResetTimer(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_ResetTimer(TileInstPtr, XAIETILE_TIMER_MODULE_CORE);
}

/*****************************************************************************/
/**
*
* This API sets the Core timer reset event. The timer will reset when the event
* is raised.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Reset event ID (0 - 127)
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event,
				    u8 Reset)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieTile_SetTimerResetEvent(TileInstPtr, Event, Reset,
					   XAIETILE_TIMER_MODULE_CORE);
}

/*****************************************************************************/
/**
*
* This API sets the Mem timer trigger events value.
* Timer low event will generate if the timer low reaches the specified low
* event value. Timer high event will generate if the timer high reaches the
* specified high event value.
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	LowEventValue - Timer value to trigger timer low events.
* @param	HighEventValue - Timer value to trigger timer high events.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr,
				      u32 LowEventValue, u32 HighEventValue)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_SetTimerTrigEventVal(TileInstPtr, LowEventValue,
				      HighEventValue,
				      XAIETILE_TIMER_MODULE_MEM);
}

/*****************************************************************************/
/**
*
* This API resets the Mem timer
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemResetTimer(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_ResetTimer(TileInstPtr, XAIETILE_TIMER_MODULE_MEM);
}

/*****************************************************************************/
/**
*
* This API sets the Mem timer reset event. The timer will reset when the event
* is raised.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Reset event ID (0 - 127)
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event,
				   u8 Reset)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieTile_SetTimerResetEvent(TileInstPtr, Event, Reset,
					   XAIETILE_TIMER_MODULE_MEM);
}

/*****************************************************************************/
/**
*
* This API sets the Pl timer trigger events value.
* Timer low event will generate if the timer low reaches the specified low
* event value. Timer high event will generate if the timer high reaches the
* specified high event value.
*
* @param	TileInstPtr - Pointer to the tile instance.
* @param	LowEventValue - Timer value to trigger timer low events.
* @param	HighEventValue - Timer value to trigger timer high events.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlSetTimerTrigEventVal(XAieGbl_Tile *TileInstPtr,
				     u32 LowEventValue, u32 HighEventValue)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_SetTimerTrigEventVal(TileInstPtr, LowEventValue,
				      HighEventValue,
				      XAIETILE_TIMER_MODULE_PL);
}

/*****************************************************************************/
/**
*
* This API resets the Pl timer
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlResetTimer(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieTile_ResetTimer(TileInstPtr, XAIETILE_TIMER_MODULE_PL);
}

/*****************************************************************************/
/**
*
* This API sets the Pl timer reset event. The timer will reset when the event
* is raised.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Reset event ID (0 - 127)
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
*
* @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlSetTimerResetEvent(XAieGbl_Tile *TileInstPtr, u32 Event,
				  u8 Reset)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	return XAieTile_SetTimerResetEvent(TileInstPtr, Event, Reset,
					   XAIETILE_TIMER_MODULE_PL);
}
/** @} */

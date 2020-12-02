/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_timer.c
* @{
*
* This file contains routines for AIE timers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 04/06/2020  Initial creation
* 1.1   Dishita 06/10/2020  Add XAie_WaitCycles API
* 1.2   Dishita 06/17/2020  Resolve compiler warning
* 1.3   Tejus   06/10/2020  Switch to new io backend apis.
* 1.4   Nishad  09/14/2020  Add check for invalid XAie_Reset value in
*			    XAie_SetTimerResetEvent() API.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaie_timer.h"
#include "xaiegbl.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_TIMER_32BIT_SHIFT		32U
#define XAIE_WAIT_CYCLE_MAX_VAL		0xFFFFFFFFFFFF

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API sets the timer trigger events value. Timer low event will be
* generated if the timer low reaches the specified low event value. Timer high
* event will be generated if the timer high reaches the specified high event
* value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*
* @param	LowEventValue: Value to set for the timer to trigger timer low
*                              event.
* @param	HighEventValue: Value to set for the timer to trigger timer
*                               high event.
*
* @return	XAIE_OK on success.
* 		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
*******************************************************************************/
AieRC XAie_SetTimerTrigEventVal(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u32 LowEventValue, u32 HighEventValue)
{
	u64 RegAddr;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	/* Set up Timer low event value */
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row ,Loc.Col) +
		TimerMod->TrigEventLowValOff;
	XAie_Write32(DevInst, RegAddr, LowEventValue);

	/* Set up Timer high event value */
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row ,Loc.Col) +
		TimerMod->TrigEventHighValOff;
	XAie_Write32(DevInst, RegAddr, HighEventValue);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API resets the timer
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*			 For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			 For Pl or Shim tile - XAIE_PL_MOD,
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid

* @note
*
*******************************************************************************/
AieRC XAie_ResetTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u32 RegVal, Mask;
	u64 RegAddr;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->CtrlOff;
	Mask = TimerMod->CtrlReset.Mask;
	RegVal = XAie_SetField(XAIE_RESETENABLE, TimerMod->CtrlReset.Lsb, Mask);
	XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the timer reset event. The timer will reset when the event
* is raised.
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*                         For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                         For Pl or Shim tile - XAIE_PL_MOD,
* @param	Event - Reset event.
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
* 		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
*******************************************************************************/
AieRC XAie_SetTimerResetEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event,
		XAie_Reset Reset)
{
	u32 RegVal;
	u64 RegAddr;
	u8 TileType, IntEvent, RC;
	const XAie_TimerMod *TimerMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if (Reset > XAIE_RESETENABLE) {
		XAIE_ERROR("Invalid reset value\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	/* check if the event passed as input is corresponding to the module */
	if(Event < EvntMod->EventMin || Event > EvntMod->EventMax) {
		XAIE_ERROR("Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	Event -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntEvent = EvntMod->XAie_EventNumber[Event];

	/*checking for valid true event number */
	if(IntEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	RegVal = XAie_SetField(IntEvent, TimerMod->CtrlResetEvent.Lsb,
			TimerMod->CtrlResetEvent.Mask);

	RegVal |= XAie_SetField(Reset, TimerMod->CtrlReset.Lsb,
			TimerMod->CtrlReset.Mask);

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->CtrlOff;
	XAie_Write32(DevInst, RegAddr, RegVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the current value of the module's 64-bit timer.
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*                         For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                         For Pl or Shim tile - XAIE_PL_MOD,
* @param	TimerVal - Pointer to store Timer Value.
*
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note		None.
*
********************************************************************************/
AieRC XAie_ReadTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u64 *TimerVal)
{
	u32 CurValHigh, CurValLow;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) || (TimerVal == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance or TimerVal\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
        if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid Module\n");
                return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	/* Read the timer high and low values before wait */
	CurValLow = XAie_Read32(DevInst,
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			TimerMod->LowOff);
	CurValHigh = XAie_Read32(DevInst,
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			TimerMod->HighOff);

	*TimerVal = ((u64)CurValHigh << XAIE_TIMER_32BIT_SHIFT) | CurValLow;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API implements a blocking wait function until the specified clock cyles
* are elapsed in the given module's 64-bit counter.
*
* @param        DevInst - Device Instance.
* @param        Loc - Location of tile.
* @param        Module - Module of the tile
*                        For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                        For Pl or Shim tile - XAIE_PL_MOD,
* @param        CycleCnt - No. of timer clock cycles to elapse.
*
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note         CycleCnt has an upper limit of 0xFFFFFFFFFFFF or 300 trillion
*		cycles to prevent overflow.
*
******************************************************************************/
AieRC XAie_WaitCycles(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u64 CycleCnt)
{

	u64 StartVal, EndVal, CurVal = 0U;
	u32 CurHigh, CurLow;
	u8 TileType;
	AieRC RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(CycleCnt > XAIE_WAIT_CYCLE_MAX_VAL) {
		XAIE_ERROR("CycleCnt above max value\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	} else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	/* Read the timer high and low values before wait */
	CurLow = XAie_Read32(DevInst,
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			TimerMod->LowOff);
	CurHigh = XAie_Read32(DevInst,
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			TimerMod->HighOff);

	StartVal = ((u64)CurHigh << XAIE_TIMER_32BIT_SHIFT) | CurLow;
	EndVal = StartVal + CycleCnt;

	while(CurVal < EndVal) {
		/* Read the timer high and low values */
		CurLow = XAie_Read32(DevInst,
				_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
				TimerMod->LowOff);
		CurHigh = XAie_Read32(DevInst,
				_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
				TimerMod->HighOff);
		CurVal = ((u64)CurHigh << XAIE_TIMER_32BIT_SHIFT) | CurLow;
	}

	return XAIE_OK;
}

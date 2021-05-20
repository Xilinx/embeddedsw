/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core.c
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Reorder functions
* 1.3   Tejus   03/20/2020  Make internal functions static
* 1.4   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.5   Tejus   06/01/2020  Add core debug halt apis.
* 1.6   Tejus   06/01/2020  Add api to read core done bit.
* 1.7   Tejus   06/05/2020  Change core enable/disable api to mask write.
* 1.8   Tejus   06/05/2020  Add api to reset/unreset aie cores.
* 1.9   Tejus   06/05/2020  Add null check for DevInst in core status apis.
* 2.0   Tejus   06/10/2020  Switch to new io backend apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_core.h"
#include "xaie_events.h"

/************************** Constant Definitions *****************************/
#define XAIETILE_CORE_STATUS_DEF_WAIT_USECS 500U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core status for a
* range of AIE Tiles for a given Mask and Value. API comes out of the wait loop
* when the status changes to the Mask, Value pair or the timeout elapses,
* whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us.
* @param	Mask: Mask for the core status register.
* @param	Value: Value for the core status register.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_CoreWaitStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut, u32 Mask, u32 Value)
{

	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	u8 TileType;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	/* TimeOut passed by the user is per Core */
	if(TimeOut == 0) {
		/* Set timeout to default value */
		TimeOut = XAIETILE_CORE_STATUS_DEF_WAIT_USECS;
	}


	RegAddr = CoreMod->CoreSts->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	if(XAie_MaskPoll(DevInst, RegAddr, Mask, Value, TimeOut) !=
			XAIE_OK) {
		XAIE_DBG("Status poll time out\n");
		return XAIE_CORE_STATUS_TIMEOUT;
	}

	return XAIE_OK;

}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to disable the core.
* Any gracefulness required in enabling/disabling the core are required to be
* handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tiles
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreDisable(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u32 Mask, Value;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	Mask = CoreMod->CoreCtrl->CtrlEn.Mask;
	Value = 0U << CoreMod->CoreCtrl->CtrlEn.Lsb;
	RegAddr = CoreMod->CoreCtrl->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to enable the core.
* Any gracefulness required in enabling/disabling the core are required to be
* handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;

	return CoreMod->Enable(DevInst, Loc, CoreMod);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to reset the core.
* Any gracefulness required in reset/unreset of the core are required to be
* handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreReset(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u32 Mask, Value;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Mask = CoreMod->CoreCtrl->CtrlRst.Mask;
	Value = 1U << CoreMod->CoreCtrl->CtrlRst.Lsb;
	RegAddr = CoreMod->CoreCtrl->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to unreset the core.
* Any gracefulness required in reset/unreset of the core are required to be
* handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreUnreset(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u32 Mask, Value;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}


	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Mask = CoreMod->CoreCtrl->CtrlRst.Mask;
	Value = 0U << CoreMod->CoreCtrl->CtrlRst.Lsb;
	RegAddr = CoreMod->CoreCtrl->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* done state for a AIE tile. API comes out of the loop when core status
* changes to done or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc, u32 TimeOut)
{
	u8 TileType;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;

	/* TimeOut passed by the user is per Core */
	if(TimeOut == 0) {
		/* Set timeout to default value */
		TimeOut = XAIETILE_CORE_STATUS_DEF_WAIT_USECS;
	}

	return CoreMod->WaitForDone(DevInst, Loc, TimeOut, CoreMod);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* disable state for a AIE tile. API comes out of the loop when core status
* changes to disable or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreWaitForDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut)
{
	const XAie_CoreMod *CoreMod;
	u32 Mask;
	u32 Value;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Mask = CoreMod->CoreSts->En.Mask;
	Value = 0U << CoreMod->CoreSts->En.Lsb;
	return _XAie_CoreWaitStatus(DevInst, Loc, TimeOut, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API writes to the Core debug control register to control the debug halt
* bit.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	Enable: Enable/Disable the debug halt bit(1- Enable, 0-Disable).
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_CoreDebugCtrlHalt(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 Enable)
{
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	RegAddr = CoreMod->CoreDebug->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_MaskWrite32(DevInst, RegAddr,
			CoreMod->CoreDebug->DebugHalt.Mask, Enable);
}

/*****************************************************************************/
/*
*
* This API writes to the Core debug control register to enable the debug halt
* bit.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreDebugHalt(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return _XAie_CoreDebugCtrlHalt(DevInst, Loc, XAIE_ENABLE);
}

/*****************************************************************************/
/*
*
* This API writes to the Core debug control register to disable the debug halt
* bit.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreDebugUnhalt(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return _XAie_CoreDebugCtrlHalt(DevInst, Loc, XAIE_DISABLE);
}

/*****************************************************************************/
/*
*
* This API reads the Done bit value in the core status register.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	DoneBit: Pointer to store the value of Done bit. Returns 1 if
*		Done bit is set, 0 otherwise.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreReadDoneBit(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *DoneBit)
{
	const XAie_CoreMod *CoreMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (DoneBit == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	return CoreMod->ReadDoneBit(DevInst, Loc, DoneBit, CoreMod);
}

/*****************************************************************************/
/*
*
* This API configures the debug control register of aie core
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
* @param	Event0: Event to halt the aie core
* @param	Event1: Event to halt the aie core
* @param	SingleStepEvent: Event to single step aie core
* @param	ResumeCoreEvent: Event to resume aie core execution after
*		debug halt
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreConfigDebugControl1(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_Events Event0, XAie_Events Event1,
		XAie_Events SingleStepEvent, XAie_Events ResumeCoreEvent)
{
	u8 TileType, MEvent1, MEvent0, MSStepEvent, MResumeCoreEvent;
	u32 RegVal;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_CORE_MOD];

	if((Event0 < EvntMod->EventMin || Event0 > EvntMod->EventMax) ||
			(Event1 < EvntMod->EventMin ||
			 Event1 > EvntMod->EventMax) ||
			(SingleStepEvent < EvntMod->EventMin ||
			 SingleStepEvent > EvntMod->EventMax) ||
			(ResumeCoreEvent < EvntMod->EventMin ||
			 ResumeCoreEvent > EvntMod->EventMax)) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event0 -= EvntMod->EventMin;
	Event1 -= EvntMod->EventMin;
	SingleStepEvent -= EvntMod->EventMin;
	ResumeCoreEvent -= EvntMod->EventMin;

	MEvent0 = EvntMod->XAie_EventNumber[Event0];
	MEvent1 = EvntMod->XAie_EventNumber[Event1];
	MSStepEvent = EvntMod->XAie_EventNumber[SingleStepEvent];
	MResumeCoreEvent = EvntMod->XAie_EventNumber[ResumeCoreEvent];

	if((MEvent0 == XAIE_EVENT_INVALID) ||
			(MEvent1 == XAIE_EVENT_INVALID) ||
			(MSStepEvent == XAIE_EVENT_INVALID) ||
			(MResumeCoreEvent == XAIE_EVENT_INVALID)) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegVal = XAie_SetField(MEvent0,
			CoreMod->CoreDebug->DebugHaltCoreEvent0.Lsb,
			CoreMod->CoreDebug->DebugHaltCoreEvent0.Mask) |
		XAie_SetField(MEvent1,
				CoreMod->CoreDebug->DebugHaltCoreEvent1.Lsb,
				CoreMod->CoreDebug->DebugHaltCoreEvent1.Mask) |
		XAie_SetField(MSStepEvent,
				CoreMod->CoreDebug->DebugSStepCoreEvent.Lsb,
				CoreMod->CoreDebug->DebugSStepCoreEvent.Mask) |
		XAie_SetField(MResumeCoreEvent,
				CoreMod->CoreDebug->DebugResumeCoreEvent.Lsb,
				CoreMod->CoreDebug->DebugResumeCoreEvent.Mask);

	RegAddr = CoreMod->CoreDebug->DebugCtrl1Offset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/*
*
* This API clears the configuration in debug control1 register to hardware
* reset values.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreClearDebugControl1(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	RegAddr = CoreMod->CoreDebug->DebugCtrl1Offset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_Write32(DevInst, RegAddr, 0U);
}

/*****************************************************************************/
/*
*
* This API configures the enable events register with enable event. This
* configuration will be used to enable the core when a particular event occurs.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
* @param	Event: Event to enable the core.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreConfigureEnableEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_Events Event)
{
	u8 TileType, MappedEvent;
	u32 Mask, Value;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_CORE_MOD];

	if(Event < EvntMod->EventMin || Event > EvntMod->EventMax) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event -= EvntMod->EventMin;
	MappedEvent = EvntMod->XAie_EventNumber[Event];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Mask = CoreMod->CoreEvent->EnableEvent.Mask |
		CoreMod->CoreEvent->DisableEventOccurred.Mask |
		CoreMod->CoreEvent->EnableEventOccurred.Mask;
	Value = MappedEvent << CoreMod->CoreEvent->EnableEvent.Lsb;

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API configures the enable events register with disable event. This
* configuration will be used to check if the core has triggered the disable
* event.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	return CoreMod->ConfigureDone(DevInst, Loc, CoreMod);
}

/*****************************************************************************/
/*
*
* This API clears event occurred status.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_ClearCoreDisableEventOccurred(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	u8 TileType;
	u32 Mask, Value;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	RegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Mask = CoreMod->CoreEvent->DisableEventOccurred.Mask;
	Value = 1U << CoreMod->CoreEvent->DisableEventOccurred.Lsb;

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
}

/** @} */

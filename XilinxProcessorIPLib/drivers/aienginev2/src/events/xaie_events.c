/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_events.c
* @{
*
* This file contains routines for AIE events module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  07/01/2020  Initial creation
* 1.1   Nishad  07/12/2020  Add APIs to configure event broadcast, PC event,
*			    and group event registers.
* 1.2   Nishad  07/14/2020  Add APIs to reset individual stream switch port
*			    event selection ID and combo event.
* 1.6   Nishad  07/23/2020  Add API to block brodcast signals using bitmap.
* 1.7   Dishita 09/16/2020  Add APIs to convert physical event to logical event
*			    and vice versa.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_EVENTS_ENABLE

/***************************** Macro Definitions *****************************/
#define XAIE_EVENT_PC_RESET		0xFFFF

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to trigger an event the given module
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	Event: Event to be triggered
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventGenerate(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType, MappedEvent;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

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

	RegOffset = EvntMod->GenEventRegOff;
	FldMask = EvntMod->GenEvent.Mask;
	FldVal = XAie_SetField(MappedEvent, EvntMod->GenEvent.Lsb, FldMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This internal API configures combo events for a given module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	ComboId: Combo index.
* @param	Op: Logical operation between Event1 and Event2 to trigger combo
*		    event.
* @param	Event1: When, ComboId == XAIE_EVENT_COMBO0 Event1 coressponds to
*			Event A, ComboId == XAIE_EVENT_COMBO1 Event1 coressponds
*			to Event C, ComboId == XAIE_EVENT_COMBO2 Event1
*			coressponds to XAIE_EVENT_COMBO0.
* @param	Event2: When, ComboId == XAIE_EVENT_COMBO0 Event2 coressponds to
*			Event B, ComboId == XAIE_EVENT_COMBO1 Event2 coressponds
*			to Event D, ComboId == XAIE_EVENT_COMBO2 Event2
*			coressponds to XAIE_EVENT_COMBO1.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_EventComboControl(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_EventComboId ComboId,
		XAie_EventComboOps Op, XAie_Events Event1, XAie_Events Event2)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask, Event1Mask, Event2Mask;
	u8 TileType, Event1Lsb, Event2Lsb, MappedEvent1, MappedEvent2;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	RegOffset = EvntMod->ComboCtrlRegOff;
	FldMask = EvntMod->ComboConfigMask << (ComboId * EvntMod->ComboConfigOff);
	FldVal = XAie_SetField(Op, ComboId * EvntMod->ComboConfigOff, FldMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	RC = XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Skip combo input event register config for XAIE_COMBO2 combo ID */
	if(ComboId == XAIE_EVENT_COMBO2)
		return XAIE_OK;

	if(Event1 < EvntMod->EventMin || Event1 > EvntMod->EventMax ||
		Event2 < EvntMod->EventMin || Event2 > EvntMod->EventMax)
	{
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event1 -= EvntMod->EventMin;
	Event2 -= EvntMod->EventMin;
	MappedEvent1 = EvntMod->XAie_EventNumber[Event1];
	MappedEvent2 = EvntMod->XAie_EventNumber[Event2];
	if(MappedEvent1 == XAIE_EVENT_INVALID ||
			MappedEvent2 == XAIE_EVENT_INVALID)
	{
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = EvntMod->ComboInputRegOff;
	Event1Lsb = (ComboId * 2U) * EvntMod->ComboEventOff;
	Event2Lsb = (ComboId * 2U + 1) * EvntMod->ComboEventOff;
	Event1Mask = EvntMod->ComboEventMask << Event1Lsb;
	Event2Mask = EvntMod->ComboEventMask << Event2Lsb;
	FldVal = XAie_SetField(MappedEvent1, Event1Lsb, Event1Mask) |
		 XAie_SetField(MappedEvent2, Event2Lsb, Event2Mask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_MaskWrite32(DevInst, RegAddr, Event1Mask | Event2Mask,
			FldVal);
}

/*****************************************************************************/
/**
*
* This API configures combo events for a given module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	ComboId: Combo index.
* @param	Op: Logical operation between Event1 and Event2 to trigger combo
*		    event.
* @param	Event1: When, ComboId == XAIE_EVENT_COMBO0 Event1 coressponds to
*			Event A, ComboId == XAIE_EVENT_COMBO1 Event1 coressponds
*			to Event C, ComboId == XAIE_EVENT_COMBO2 Event1
*			coressponds to XAIE_EVENT_COMBO0.
* @param	Event2: When, ComboId == XAIE_EVENT_COMBO0 Event2 coressponds to
*			Event B, ComboId == XAIE_EVENT_COMBO1 Event2 coressponds
*			to Event D, ComboId == XAIE_EVENT_COMBO2 Event2
*			coressponds to XAIE_EVENT_COMBO1.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventComboConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_EventComboId ComboId,
		XAie_EventComboOps Op, XAie_Events Event1, XAie_Events Event2)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventComboControl(DevInst, Loc, Module, ComboId, Op,
			Event1, Event2);
}

/*****************************************************************************/
/**
*
* This API returns the combo base event based on the tile location
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD.
* @param	Event: Base event of tile
*
* @return	XAIE_OK on success, error code on failure
*
* @note		None
******************************************************************************/
AieRC XAie_EventGetComboEventBase(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events *Event)
{
	AieRC RC;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Event == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[Module].ComboEventBase;
	} else {
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[0U].ComboEventBase;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This API resets individual combo config and it's corresponding events for a
* given module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	ComboId: Combo index.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventComboReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_EventComboId ComboId)
{
	u8 TileType;
	XAie_Events Event;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckModule(DevInst, Loc, Module) != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_CORE_MOD) {
		Event = XAIE_EVENT_NONE_CORE;
	} else if(Module == XAIE_PL_MOD) {
		Event = XAIE_EVENT_NONE_PL;
	} else {
		/* Memory module */
		if(TileType == XAIEGBL_TILE_TYPE_MEMTILE)
			Event = XAIE_EVENT_NONE_MEM_TILE;
		else
			Event = XAIE_EVENT_NONE_MEM;
	}

	return _XAie_EventComboControl(DevInst, Loc, Module, ComboId,
			XAIE_EVENT_COMBO_E1_AND_E2, Event, Event);
}

/*****************************************************************************/
/**
*
* This internal API configures the stream switch event selection register for
* any given tile type. Any of the Master or Slave stream switch ports can be
* programmed at given selection index. Events corresponding to the port could be
* monitored at the given selection ID through event status registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	SelectId: Selection index at which given port's event are
*			  captured
* @param	PortIntf: Stream switch port interface.
*			  for Slave port - XAIE_STRMSW_SLAVE,
*			  for Mater port - XAIE_STRMSW_MASTER.
* @param	Port: Stream switch port type.
* @param	PortNum: Stream switch port number.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_EventSelectStrmPortConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 SelectId, XAie_StrmPortIntf PortIntf,
		StrmSwPortType Port, u8 PortNum)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, PortIdMask, PortMstrSlvMask;
	u8 TileType, SelectRegOffId, PortIdx, PortIdLsb, PortMstrSlvLsb;
	const XAie_StrmMod *StrmMod;
	const XAie_EvntMod *EvntMod;

	if(PortIntf > XAIE_STRMSW_MASTER) {
		XAIE_ERROR("Invalid stream switch interface\n");
		return XAIE_INVALID_ARGS;
	}

	if(Port >= SS_PORT_TYPE_MAX) {
		XAIE_ERROR("Invalid stream switch ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_CORE_MOD];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0];

	if(SelectId >= EvntMod->NumStrmPortSelectIds) {
		XAIE_ERROR("Invalid selection ID\n");
		return XAIE_INVALID_ARGS;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	if(PortIntf == XAIE_STRMSW_SLAVE)
		RC = _XAie_GetSlaveIdx(StrmMod, Port, PortNum, &PortIdx);
	else
		RC = _XAie_GetMstrIdx(StrmMod, Port, PortNum, &PortIdx);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to compute port index\n");
		return RC;
	}

	SelectRegOffId = SelectId / EvntMod->StrmPortSelectIdsPerReg;
	RegOffset = EvntMod->BaseStrmPortSelectRegOff + SelectRegOffId * 4U;
	PortIdLsb = EvntMod->PortIdOff *
			(SelectId % EvntMod->StrmPortSelectIdsPerReg);
	PortIdMask = EvntMod->PortIdMask << PortIdLsb;
	PortMstrSlvLsb = EvntMod->PortMstrSlvOff + 8U *
				(SelectId % EvntMod->StrmPortSelectIdsPerReg);
	PortMstrSlvMask = EvntMod->PortMstrSlvMask << (8U *
				(SelectId % EvntMod->StrmPortSelectIdsPerReg));
	FldVal = XAie_SetField(PortIdx, PortIdLsb, PortIdMask) |
		 XAie_SetField(PortIntf, PortMstrSlvLsb, PortMstrSlvMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_MaskWrite32(DevInst, RegAddr, PortIdMask | PortMstrSlvMask,
			FldVal);
}

/*****************************************************************************/
/**
*
* This API configures the stream switch event selection register for any given
* tile type. Any of the Master or Slave stream switch ports can be programmed at
* given selection index. Events corresponding to the port could be monitored at
* the given selection ID through event status registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	SelectId: Selection index at which given port's event are
*			  captured
* @param	PortIntf: Stream switch port interface.
*			  for Slave port - XAIE_STRMSW_SLAVE,
*			  for Mater port - XAIE_STRMSW_MASTER.
* @param	Port: Stream switch port type.
* @param	PortNum: Stream switch port number.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventSelectStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 SelectId, XAie_StrmPortIntf PortIntf, StrmSwPortType Port,
		u8 PortNum)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventSelectStrmPortConfig(DevInst, Loc, SelectId, PortIntf,
			Port, PortNum);
}

/*****************************************************************************/
/**
*
* This API resets individual stream switch event selection ID any given tile
* type.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	SelectId: Selection index at which given port's event are
*			  captured
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventSelectStrmPortReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 SelectId)
{
	u8 TileType;
	StrmSwPortType Port;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE)
		Port = CORE;
	else if(TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
		TileType == XAIEGBL_TILE_TYPE_SHIMNOC)
		Port = CTRL;
	else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE)
		Port = DMA;
	else {
		XAIE_ERROR("Failed to reset event select strm port. Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventSelectStrmPortConfig(DevInst, Loc, SelectId,
			XAIE_STRMSW_SLAVE, Port, 0U);
}

/*****************************************************************************/
/**
*
* This API returns the port idle base event based on the tile location
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD.
* @param	Event: Base event of tile
*
* @return	XAIE_OK on success, error code on failure
*
* @note		None
******************************************************************************/
AieRC XAie_EventGetIdlePortEventBase(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events *Event)
{
	AieRC RC;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Event == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return RC;
	}

	if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_MEM_MOD) {
			return XAIE_INVALID_ARGS;
		}
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[Module].PortIdleEventBase;
	} else {
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[0U].PortIdleEventBase;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This API maps an event to the broadcast ID in the given module. To reset, set
* the value of Event param to XAIE_EVENT_NONE.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	BroadcastId: Broadcast index.
* @param	Event: Event to broadcast.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only
*
******************************************************************************/
static AieRC _XAie_EventBroadcastConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 BroadcastId, XAie_Events Event)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType, MappedEvent;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(BroadcastId >= EvntMod->NumBroadcastIds) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

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

	RegOffset = EvntMod->BaseBroadcastRegOff + BroadcastId * 4U;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, MappedEvent);
}

/*****************************************************************************/
/**
*
* This API maps an event to the broadcast ID in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	BroadcastId: Broadcast index.
* @param	Event: Event to broadcast.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventBroadcast(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 BroadcastId, XAie_Events Event)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventBroadcastConfig(DevInst, Loc, Module, BroadcastId,
			Event);
}

/*****************************************************************************/
/**
*
* This API resets broadcast register corresponding to ID in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	BroadcastId: Broadcast index.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventBroadcastReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 BroadcastId)
{
	u8 TileType;
	XAie_Events Event;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckModule(DevInst, Loc, Module) != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_CORE_MOD) {
		Event = XAIE_EVENT_NONE_CORE;
	} else if(Module == XAIE_PL_MOD) {
		Event = XAIE_EVENT_NONE_PL;
	} else {
		/* Memory module */
		if(TileType == XAIEGBL_TILE_TYPE_MEMTILE)
			Event = XAIE_EVENT_NONE_MEM_TILE;
		else
			Event = XAIE_EVENT_NONE_MEM;
	}

	return _XAie_EventBroadcastConfig(DevInst, Loc, Module, BroadcastId,
			Event);
}

/*****************************************************************************/
/**
*
* This API blocks event broadcasts in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	Switch: Event switch in the given module.
*			for AIE Tile switch value is XAIE_EVENT_SWITCH_A,
*			for Shim tile and Mem tile switch value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	BroadcastId: Broadcast index.
* @parma	Dir: Direction to block events on given broadcast index. Values
*		     could be OR'ed to block multiple directions. For example,
*		     to block event broadcast in West and East directions set
*		     Dir as,
*		     XAIE_EVENT_BROADCAST_WEST | XAIE_EVENT_BROADCAST_EAST.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventBroadcastBlockDir(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_BroadcastSw Switch, u8 BroadcastId,
		u8 Dir)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Dir & ~XAIE_EVENT_BROADCAST_ALL) {
		XAIE_ERROR("Invalid broadcast direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(BroadcastId >= EvntMod->NumBroadcastIds ||
					Switch > EvntMod->NumSwitches) {
		XAIE_ERROR("Invalid broadcast ID or switch value\n");
		return XAIE_INVALID_ARGS;
	}

	for(u8 DirShift = 0; DirShift < 4; DirShift++) {
		if(!(Dir & 1U << DirShift))
			continue;

		RegOffset = EvntMod->BaseBroadcastSwBlockRegOff +
			    DirShift * EvntMod->BroadcastSwBlockOff +
			    Switch * EvntMod->BroadcastSwOff;
		RegAddr   = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			    RegOffset;
		RC = XAie_Write32(DevInst, RegAddr, XAIE_ENABLE << BroadcastId);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API blocks event broadcasts in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	Switch: Event switch in the given module.
*			for AIE Tile switch value is XAIE_EVENT_SWITCH_A,
*			for Shim tile and Mem tile switch value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	ChannelBitMap: Bitmap to block broadcast channels.
* @parma	Dir: Direction to block events on given broadcast index. Values
*		     could be OR'ed to block multiple directions. For example,
*		     to block event broadcast in West and East directions set
*		     Dir as,
*		     XAIE_EVENT_BROADCAST_WEST | XAIE_EVENT_BROADCAST_EAST.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventBroadcastBlockMapDir(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_BroadcastSw Switch,
		u32 ChannelBitMap, u8 Dir)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Dir & ~XAIE_EVENT_BROADCAST_ALL) {
		XAIE_ERROR("Invalid broadcast direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(ChannelBitMap >= (XAIE_ENABLE << EvntMod->NumBroadcastIds) ||
					Switch > EvntMod->NumSwitches) {
		XAIE_ERROR("Invalid broadcast bitmap or switch value\n");
		return XAIE_INVALID_ARGS;
	}

	for(u8 DirShift = 0; DirShift < 4; DirShift++) {
		if(!(Dir & 1U << DirShift))
			continue;

		RegOffset = EvntMod->BaseBroadcastSwBlockRegOff +
			    DirShift * EvntMod->BroadcastSwBlockOff +
			    Switch * EvntMod->BroadcastSwOff;
		RegAddr   = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			    RegOffset;
		RC = XAie_Write32(DevInst, RegAddr, ChannelBitMap);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API unblocks event broadcasts in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	Switch: Event switch in the given module.
*			for AIE Tile switch value is XAIE_EVENT_SWITCH_A,
*			for Shim tile and Mem tile switch value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	BroadcastId: Broadcast index.
* @parma	Dir: Direction to unblock events on given broadcast index.
*		     Values could be OR'ed to unblock multiple directions. For
*		     example, to unblock event broadcast in West and East
*		     directions set Dir as,
*		     XAIE_EVENT_BROADCAST_WEST | XAIE_EVENT_BROADCAST_EAST.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventBroadcastUnblockDir(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_BroadcastSw Switch, u8 BroadcastId,
		u8 Dir)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Dir & ~XAIE_EVENT_BROADCAST_ALL) {
		XAIE_ERROR("Invalid broadcast direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(BroadcastId >= EvntMod->NumBroadcastIds ||
					Switch > EvntMod->NumSwitches) {
		XAIE_ERROR("Invalid broadcast ID or switch value\n");
		return XAIE_INVALID_ARGS;
	}

	for(u8 DirShift = 0; DirShift < 4; DirShift++) {
		if(!(Dir & 1U << DirShift))
			continue;

		RegOffset = EvntMod->BaseBroadcastSwUnblockRegOff +
			    DirShift * EvntMod->BroadcastSwUnblockOff +
			    Switch * EvntMod->BroadcastSwOff;
		RegAddr   = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			    RegOffset;
		RC = XAie_Write32(DevInst, RegAddr, XAIE_ENABLE << BroadcastId);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables, disables or resets events in a group event in the given
* module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	GroupEvent: Group event ID.
* @param	GroupBitMap: Bit mask.
* @param	Reset: XAIE_RESETENABLE or XAIE_RESETDISABLE to reset or unreset
*		       PC event register

* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_EventGroupConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events GroupEvent, u32 GroupBitMap,
		u8 Reset)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	for(u32 Index = 0; Index < EvntMod->NumGroupEvents; Index++) {
		if(GroupEvent == EvntMod->Group[Index].GroupEvent) {
			RegOffset = EvntMod->BaseGroupEventRegOff +
					EvntMod->Group[Index].GroupOff * 4U;
			RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
					RegOffset;

			if(Reset == XAIE_RESETENABLE)
				FldVal = EvntMod->Group[Index].ResetValue;
			else
				FldVal = XAie_SetField(GroupBitMap, 0U,
					 EvntMod->Group[Index].GroupMask);

			RC = XAie_Write32(DevInst, RegAddr, FldVal);
			if(RC != XAIE_OK) {
				return RC;
			}

			return XAIE_OK;
		}
	}

	XAIE_ERROR("Invalid group event ID\n");
	return XAIE_INVALID_ARGS;
}

/*****************************************************************************/
/**
*
* This API enables or disables events in a group event in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	GroupEvent: Group event ID.
* @param	GroupBitMap: Bit mask.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventGroupControl(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events GroupEvent, u32 GroupBitMap)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventGroupConfig(DevInst, Loc, Module, GroupEvent,
			GroupBitMap, XAIE_RESETDISABLE);

}

/*****************************************************************************/
/**
*
* This API resets group event register in the given module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	GroupEvent: Group event ID.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventGroupReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events GroupEvent)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventGroupConfig(DevInst, Loc, Module, GroupEvent, 0U,
			XAIE_RESETENABLE);
}

/*****************************************************************************/
/**
*
* This API enables, disables or resets the program counter event in the given
* module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	PCEventId: PC Event index.
* @param	PCAddr: PC event on this instruction address.
* @param	Valid: XAIE_ENABLE or XAIE_DISABLE to enable or disable PC
*		       event.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_EventPCConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PCEventId, u16 PCAddr, u8 Valid)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_CORE_MOD];

	if(PCEventId >= EvntMod->NumPCEvents) {
		XAIE_ERROR("Invalid PC event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = EvntMod->BasePCEventRegOff + PCEventId * 4U;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	if(Valid == XAIE_DISABLE) {
		FldVal = XAie_SetField(Valid, EvntMod->PCValid.Lsb,
				EvntMod->PCValid.Mask);

		if(PCAddr == XAIE_EVENT_PC_RESET) {
			FldVal |= XAie_SetField(0U, EvntMod->PCAddr.Lsb,
						EvntMod->PCAddr.Mask);
			RC = XAie_Write32(DevInst, RegAddr, FldVal);

			return RC;
		}

		RC = XAie_MaskWrite32(DevInst, RegAddr, EvntMod->PCValid.Mask,
				FldVal);
		if(RC != XAIE_OK) {
			return RC;
		}
	} else {
		FldVal = XAie_SetField(PCAddr, EvntMod->PCAddr.Lsb,
				EvntMod->PCAddr.Mask) |
			 XAie_SetField(Valid, EvntMod->PCValid.Lsb,
				EvntMod->PCValid.Mask);
		RC = XAie_Write32(DevInst, RegAddr, FldVal);

		return RC;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets up program counter event in the given module
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	PCEventId: PC Event index.
* @param	PCAddr: PC event on this instruction address.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventPCEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 PCEventId,
		u16 PCAddr)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventPCConfig(DevInst, Loc, PCEventId, PCAddr,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API disables program counter event in the given module
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	PCEventId: PC Event index.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventPCDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 PCEventId)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventPCConfig(DevInst, Loc, PCEventId, 0U, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API resets program counter event register in the given module. Valid and
* PC address bit fields are set to zero.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	PCEventId: PC Event index.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventPCReset(XAie_DevInst *DevInst, XAie_LocType Loc, u8 PCEventId)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_EventPCConfig(DevInst, Loc, PCEventId,
					XAIE_EVENT_PC_RESET, XAIE_DISABLE);
}
/*****************************************************************************/
/**
* This API is used to convert XAie_Events enum to hardware event id.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the tile
* @param        Module: Module of tile.
* @param        Event: Event to be converted
* @param        HwEvent: Pointer to store physical event id
*
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_EventLogicalToPhysicalConv(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event, u8 *HwEvent)
{
	AieRC RC;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
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
	*HwEvent = EvntMod->XAie_EventNumber[Event];

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API is used to convert hardware event id to XAie_Events enum.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the tile
* @param        Module: Module of tile.
* @param        PhysicalEvent: Physical event id to be converted
* @param        EnumEvent: Pointer to store converted event
*
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_EventPhysicalToLogicalConv(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 HwEvent, XAie_Events *EnumEvent)
{
	AieRC RC;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	for(u32 i = EvntMod->EventMin; i <= EvntMod->EventMax; i++) {
		if(EvntMod->XAie_EventNumber[i - EvntMod->EventMin] == HwEvent) {
			*EnumEvent = i;
			return XAIE_OK;
		}
	}
	XAIE_ERROR("Could not convert Physical event:%u to Logical event.\n", HwEvent);

	return XAIE_INVALID_ARGS;
}

/*****************************************************************************/
/**
*
* This API returns the status of event.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD.
*			for Mem tile - XAIE_MEM_MOD.
* @param	Events: List of XAie_Events.
* @param	Status: Buffer to return status of event.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None
******************************************************************************/
AieRC XAie_EventReadStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Events, u8 *Status)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOff, RegVal;
	u8 TileType, PhyEvent;
	const XAie_EvntMod *EvntMod;

	if((Status == XAIE_NULL) || (DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY) ) {
		XAIE_ERROR("Invalid device instance or buffer pointer\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	RC = XAie_EventLogicalToPhysicalConv(DevInst, Loc, Module, Events,
								&PhyEvent);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOff = EvntMod->BaseStatusRegOff + (PhyEvent / 32U) * 4U;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	*Status =  (RegVal >> (PhyEvent % 32U)) & 1U;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the user base event based on the tile location
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD.
* @param	Event: Base event of tile
*
* @return	XAIE_OK, error code on failure
*
* @note		None
******************************************************************************/
AieRC XAie_EventGetUserEventBase(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events *Event)
{
	AieRC RC;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Event == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[Module].UserEventBase;
	} else {
		*Event = DevInst->DevProp.DevMod[TileType].EvntMod[0U].UserEventBase;
	}

	return RC;
}

#endif /* XAIE_FEATURE_EVENTS_ENABLE */
/** @} */

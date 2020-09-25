/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt.c
* @{
*
* This file contains routines for AIE interrupt module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   07/21/2020  Initial creation
* 1.1   Nishad   07/23/2020  Add APIs to configure second level interrupt
*			     controller.
* 1.2   Nishad   07/23/2020  Add API to initialize error broadcast network.
* 1.3   Nishad   08/13/2020  Block error broadcasts from AIE array to shim
			     while setting up error network.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_clock.h"
#include "xaie_helper.h"
#include "xaie_interrupt.h"
#include "xaie_npi.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API enables/disables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE to enable or disable.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_IntrCtrlL1Config(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId, u8 Enable)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || IntrId >= L1IntrMod->NumIntrIds) {
		XAIE_ERROR("Invalid module type or interrupt ID\n");
		return XAIE_INVALID_ARGS;
	}

	if(Enable == XAIE_ENABLE)
		RegOffset = L1IntrMod->BaseEnableRegOff;
	else
		RegOffset = L1IntrMod->BaseDisableRegOff;

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset +
					Switch * L1IntrMod->SwOff;

	XAie_Write32(DevInst, RegAddr, XAIE_ENABLE << IntrId);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId)
{
	return _XAie_IntrCtrlL1Config(DevInst, Loc, Switch, IntrId, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API disables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId)
{
	return _XAie_IntrCtrlL1Config(DevInst, Loc, Switch, IntrId, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API sets broadcast ID on which the interrupt from first level interrupt
* controller shall be driven to.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	BroadcastId: Broadcast index on which the interrupt shall be
*			     driven.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 BroadcastId)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || BroadcastId >= L1IntrMod->NumBroadcastIds) {
		XAIE_ERROR("Invalid module type or broadcast ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseIrqRegOff + Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, BroadcastId);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API maps an event which interrupts the first level interrupt controller
* at the given IRQ event index.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IrqEventId: IRQ event index. Value 0 causes IRQ16,
*			    value 1 Causes IRQ17, and so on.
* @param	Event: Event ID to interrupt first level interrupt controller.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Event(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IrqEventId, XAie_Events Event)
{
	u64 RegAddr;
	u32 RegOffset, EventMask, FldVal;
	u8 TileType, EventLsb, MappedEvent;
	const XAie_L1IntrMod *L1IntrMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];

	if(L1IntrMod == NULL || IrqEventId >= L1IntrMod->NumIrqEvents) {
		XAIE_ERROR("Invalid module type or IRQ event ID\n");
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

	RegOffset = L1IntrMod->BaseIrqEventRegOff + Switch * L1IntrMod->SwOff;
	EventLsb = IrqEventId * L1IntrMod->IrqEventOff;
	EventMask = L1IntrMod->BaseIrqEventMask << EventLsb;
	FldVal = XAie_SetField(MappedEvent, EventLsb, EventMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;
	XAie_MaskWrite32(DevInst, RegAddr, EventMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API blocks broadcast signals from AIE array at the first level interrupt
* controller. Unlike the tile switch, switches in the PL module have the ability
* to mask incoming signals from the AIE Tile after they have been passed to the
* first level interrupt handler. This prevents pollution of the broadcast
* network in case of interrupt usage.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	ChannelBitMap: Bitmap to block broadcast channels.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1BroadcastBlock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastBlockRegOff +
						Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, ChannelBitMap);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API unblocks broadcast signals from AIE array at the first level
* interrupt controller. Unlike the tile switch, switches in the PL module have
* the ability to mask incoming signals from the AIE Tile after they have been
* passed to the first level interrupt handler. This prevents pollution of the
* broadcast network in case of interrupt usage.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	ChannelBitMap: Bitmap to unblock broadcast channels.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Each broadcast line is allocated and utilized for channeling
*		particular types of events. Hence this API enforces unblocking
*		broadcast line per ID basis rather than a bitmap.

******************************************************************************/
AieRC XAie_IntrCtrlL1BroadcastUnblock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastUnblockRegOff +
						Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, ChannelBitMap);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables/disables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt Bitmap.
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE to enable or disable.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_IntrCtrlL2Config(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap, u8 Enable)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L2IntrMod *L2IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L2IntrMod = DevInst->DevProp.DevMod[TileType].L2IntrMod;

	if(ChannelBitMap >= (XAIE_ENABLE << L2IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid interrupt bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	if(Enable == XAIE_ENABLE)
		RegOffset = L2IntrMod->EnableRegOff;
	else
		RegOffset = L2IntrMod->DisableRegOff;

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, ChannelBitMap);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt bit map.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap)
{
	return _XAie_IntrCtrlL2Config(DevInst, Loc, ChannelBitMap, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API disables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt bit map.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap)
{
	return _XAie_IntrCtrlL2Config(DevInst, Loc, ChannelBitMap, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API sets NoC interrupt ID to which the interrupt from second level
* interrupt controller shall be driven to.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	NoCIrqId: NoC IRQ index on which the interrupt shall be
*			  driven.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 NoCIrqId)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L2IntrMod *L2IntrMod;
	XAie_NpiProtRegReq ProtRegReq = {0};

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L2IntrMod = DevInst->DevProp.DevMod[TileType].L2IntrMod;

	if(L2IntrMod == NULL || NoCIrqId >= L2IntrMod->NumNoCIntr) {
		XAIE_ERROR("Invalid module type or broadcast ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L2IntrMod->IrqRegOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	ProtRegReq.Enable = XAIE_ENABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	XAie_Write32(DevInst, RegAddr, NoCIrqId);

	ProtRegReq.Enable = XAIE_DISABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API computes first level IRQ broadcast ID.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
*
* @return	IrqId: IRQ broadcast ID.
*
* @note		IRQ ID for each switch block starts from 0, every block on the
*		left will increase by 1 until it reaches the first Shim NoC
*		column. The IRQ ID restarts from 0 on the switch A of the second
*		Shim NoC column. For the SHIM PL columns after the second Shim
*		NoC, if there is no Shim NoC further right, the column will use
*		the Shim NoC on the left. That is,
*		For column from 0 to 43: the first IRQ broadcast event ID
*		pattern is: 0 1 2 3 4 5 0 1, 0 1 2 3 4 5 0 1
*		For column 44 to 49: 0 1 2 3 4 5 0 1 2 3 4 5
*
*		Internal Only.
******************************************************************************/
static inline u8 _XAie_IntrCtrlL1IrqId(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch)
{
	u8 IrqId = (((Loc.Col % 4) % 3) * 2) + Switch;

	if(Loc.Col + 3 > DevInst->NumCols)
		IrqId += 2;

	return IrqId;
}

/*****************************************************************************/
/**
* This API enables default group error events which are marked as fatal errors.
* It also channels them on broadcast line #0.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
static AieRC _XAie_GroupErrorInit(XAie_DevInst *DevInst)
{
	AieRC RC;
	u32 GroupErrorEnableMask;
	u8 ReservedStart, ReservedEnd, AieRowStart, AieRowEnd;
	XAie_LocType Loc;

	ReservedStart = DevInst->ReservedRowStart;
	ReservedEnd = DevInst->ReservedRowStart + DevInst->ReservedNumRows;
	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = AieRowStart; Row < AieRowEnd; Row++) {
			Loc = XAie_TileLoc(Col, Row);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_MEM_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_EVENT_GROUP_ERRORS_MEM,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group errors in memory module\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_MEM);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for memory module\n");
				return RC;
			}

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_CORE_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_CORE_MOD,
					XAIE_EVENT_GROUP_ERRORS_0_CORE,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group error in core module\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_CORE_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_0_CORE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for core module\n");
				return RC;
			}
		}

		for(u8 MemRow = ReservedStart; MemRow < ReservedEnd; MemRow++) {
			Loc = XAie_TileLoc(Col, MemRow);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_MEM_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_EVENT_GROUP_ERRORS_MEM,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group error\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_MEM);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast\n");
				return RC;
			}
		}

		/*
		 * Shim tile only needs to setup error notification with first
		 * level interrupt controller.
		 */
		Loc = XAie_TileLoc(Col, DevInst->ShimRow);
		GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst, Loc,
								XAIE_PL_MOD);
		RC = XAie_EventGroupControl(DevInst, Loc, XAIE_PL_MOD,
					XAIE_EVENT_GROUP_ERRORS_PL,
					GroupErrorEnableMask);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure group error in shim tile\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1Event(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_BROADCAST_ID,
				XAIE_EVENT_GROUP_ERRORS_PL);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to setup L1 internal error interrupt in shim tile\n");
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network to deliver error events as interrupts in
* NPI. When error occurs, interrupt is raised on NPI interrupt line #5.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API assumes the whole AIE as a single partition and the
*		following broadcast channels to be available. To avoid conflicts,
*		it is the user's responsibility to make sure none of the below
*		channels are being used.
*			* Broadcast channel #0 in AIE array tiles.
*			* Switch A L1 IRQ 16.
*			* For shim tiles from column 0 to 43 broadcast lines
*			  used follows the pattern as: 0 1 2 3 4 5 0 1.
*			* For shim tiles from column 44 to 49 broadcast lines
*			  used follows the pattern as: 0 1 2 3 4 5.
*			* NPI interrupt line #5.
*		Currently, this API only supports Linux UIO, CDO, and debug
*		backends.
******************************************************************************/
AieRC XAie_ErrorHandlingInit(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 TileType, L1BroadcastId, ReservedStart, ReservedEnd, AieRowStart,
	   AieRowEnd, BroadcastDirSwA, BroadcastDirSwB;
	XAie_LocType Loc;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	ReservedStart = DevInst->ReservedRowStart;
	ReservedEnd = DevInst->ReservedRowStart + DevInst->ReservedNumRows;
	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		/* Setup error broadcasts to SOUTH from memory and core module */
		for(u8 Row = AieRowStart; Row < AieRowEnd; Row++) {
			Loc = XAie_TileLoc(Col, Row);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID,
				   XAIE_EVENT_BROADCAST_NORTH |
				   XAIE_EVENT_BROADCAST_WEST  |
				   XAIE_EVENT_BROADCAST_EAST);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in memory module\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID,
				   XAIE_EVENT_BROADCAST_NORTH |
				   XAIE_EVENT_BROADCAST_WEST  |
				   XAIE_EVENT_BROADCAST_EAST);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in core module\n");
				return RC;
			}
		}

		/* Setup error broadcasts to SOUTH */
		for(u8 MemRow = ReservedStart; MemRow < ReservedEnd; MemRow++) {
			Loc = XAie_TileLoc(Col, MemRow);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
					  XAIE_EVENT_BROADCAST_EAST |
					  XAIE_EVENT_BROADCAST_WEST;
			BroadcastDirSwB = BroadcastDirSwA;

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwA);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in switch A\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in switch B\n");
				return RC;
			}
		}

		/*
		 * Compute the broadcast line number on which L1 generates
		 * error interrupts
		 */
		Loc = XAie_TileLoc(Col, DevInst->ShimRow);
		L1BroadcastId = _XAie_IntrCtrlL1IrqId(DevInst, Loc,
					XAIE_EVENT_SWITCH_A);

		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				L1BroadcastId);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_B,
				L1BroadcastId + 1);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		/* Enable shim tile internal errors */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_SHIM_INTR_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_B,
				XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		if(TileType == XAIEGBL_TILE_TYPE_MAX) {
			XAIE_ERROR("Invalid tile type\n");
			return XAIE_INVALID_TILE;
		}

		if(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
					  XAIE_EVENT_BROADCAST_SOUTH |
					  XAIE_EVENT_BROADCAST_WEST;
			BroadcastDirSwB = XAIE_EVENT_BROADCAST_ALL;

			RC = XAie_IntrCtrlL2Enable(DevInst, Loc,
					XAIE_ERROR_L2_ENABLE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to enable interrupts to L2\n");
				return RC;
			}

			RC = XAie_IntrCtrlL2IrqSet(DevInst, Loc,
					XAIE_ERROR_NPI_INTR_ID);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure L2 IRQ line\n");
				return RC;
			}
		} else {
			if(Col + 3 >= DevInst->NumCols) {
				BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_EAST;
				BroadcastDirSwB = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_EAST;
			} else {

				BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_WEST;
				BroadcastDirSwB = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_WEST;
			}
		}

		RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_PL_MOD,
				XAIE_EVENT_SWITCH_A, L1BroadcastId,
				BroadcastDirSwA);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block broadcasts in shim tile switch A\n");
			return RC;
		}

		RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_PL_MOD,
				XAIE_EVENT_SWITCH_B, L1BroadcastId + 1,
				BroadcastDirSwB);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block broadcasts in shim tile switch B\n");
			return RC;
		}

		/* Block direct broadcasts to shim row from AIE array */
		RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
				XAIE_EVENT_SWITCH_A, XAIE_ERROR_BROADCAST_MASK);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
				XAIE_EVENT_SWITCH_B, XAIE_ERROR_BROADCAST_MASK);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
			return RC;
		}
	}

	/* Enable NPI interrupt to PS GIC */
	RC = _XAie_NpiIrqEnable(DevInst, XAIE_ERROR_NPI_INTR_ID,
				XAIE_ERROR_NPI_INTR_ID);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable NPI interrupt\n");
		return RC;
	}

	RC =  _XAie_GroupErrorInit(DevInst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to initialize group errors\n");
		return RC;
	}

	return XAIE_OK;
}

/** @} */

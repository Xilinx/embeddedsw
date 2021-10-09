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
#include <stdlib.h>

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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_Write32(DevInst, RegAddr, XAIE_ENABLE << IntrId);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_Write32(DevInst, RegAddr, BroadcastId);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_MaskWrite32(DevInst, RegAddr, EventMask, FldVal);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
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
	AieRC RC;
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	RC = XAie_Write32(DevInst, RegAddr, NoCIrqId);
	if(RC != XAIE_OK) {
		return RC;
	}

	ProtRegReq.Enable = XAIE_DISABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	return XAIE_OK;
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
	u8 MemTileStart, MemTileEnd, AieRowStart, AieRowEnd;
	XAie_LocType Loc;

	MemTileStart = DevInst->MemTileRowStart;
	MemTileEnd = DevInst->MemTileRowStart + DevInst->MemTileNumRows;
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

		for(u8 MemRow = MemTileStart; MemRow < MemTileEnd; MemRow++) {
			Loc = XAie_TileLoc(Col, MemRow);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_MEM_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_EVENT_GROUP_ERRORS_MEM_TILE,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group error in mem tile\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_MEM_TILE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for mem tile\n");
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
* This API finds the location on next NoC tile with respect to the current
* shim tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Current shim tile
* @param	NextLoc: Pointer to return location of NoC tile
*
* @return	XAIE_OK on success, error code if no NoC tile is found.
*
* @note		This function is used internally only.
******************************************************************************/
static AieRC _XAie_FindNextNoCTile(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_LocType *NextLoc)
{
	while (++Loc.Col < DevInst->NumCols) {
		u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			NextLoc->Col = Loc.Col;
			NextLoc->Row = Loc.Row;
			return XAIE_OK;
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
 *
* This API reserves broadcast resources for errors interrupt.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This function is used internally only.
******************************************************************************/
static AieRC XAie_ErrorHandlingReserveRsc(XAie_DevInst *DevInst)
{
	XAie_UserRsc *RscsBc, *ShimRscsBc;
	const XAie_L1IntrMod *L1IntrMod;
	u32 UserRscNum = 0, ShimUserRscNum;
	AieRC RC;

	/* Reserved for error broadcast channel */
	for(u32 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC ||
			i == XAIEGBL_TILE_TYPE_SHIMPL) {
			UserRscNum += DevInst->NumCols;
		} else {
			UserRscNum += (DevInst->DevProp.DevMod[i].NumModules) *
				_XAie_GetNumRows(DevInst, i) * DevInst->NumCols;
		}
	}

	RscsBc = (XAie_UserRsc *)malloc(UserRscNum * sizeof(*RscsBc));
	if(RscsBc == NULL) {
		XAIE_ERROR("Memory allocation failed for interrupt resource\n");
		return XAIE_ERR;
	}

	RC = XAie_RequestSpecificBroadcastChannel(DevInst,
		XAIE_ERROR_BROADCAST_ID, &UserRscNum, RscsBc, 1U);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to request error BC for partition.\n");
		free(RscsBc);
		return RC;
	}

	/* Request channels for shim tiles from resource manager */
	ShimUserRscNum = DevInst->NumCols;
	/*
	 * Allocate another memory for SHIM BC resources, in case reservation
	 * failed, it can release the previously requested error broadcast
	 * resource.
	 */
	ShimRscsBc = (XAie_UserRsc *)malloc(ShimUserRscNum *
			sizeof(*ShimRscsBc));
	if (ShimRscsBc == NULL) {
		XAIE_ERROR("Memory allocation failed for interrupt resource\n");
		XAie_ReleaseBroadcastChannel(DevInst, UserRscNum, RscsBc);
		free(RscsBc);
		return XAIE_ERR;
	}

	for(u32 i = 0; i < ShimUserRscNum; i++) {
		ShimRscsBc[i].Loc = XAie_TileLoc(i, 0);
		ShimRscsBc[i].Mod = XAIE_PL_MOD;
		ShimRscsBc[i].RscType = XAIE_BCAST_CHANNEL_RSC;
	}

	L1IntrMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMPL].L1IntrMod;
	for(u32 i = 1; i < L1IntrMod->MaxErrorBcIdsRvd; i++) {
		RC = XAie_RequestSpecificBroadcastChannel(DevInst,
			i, &ShimUserRscNum, ShimRscsBc, 0U);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to request SHIM error BC %u.\n", i);
			for (u32 j = 1; j < i; j++) {
				for (u32 k = 0; k < ShimUserRscNum; k++) {
					ShimRscsBc[i].RscId = j;
				}
				XAie_ReleaseBroadcastChannel(DevInst,
					ShimUserRscNum, ShimRscsBc);
			}
			XAie_ReleaseBroadcastChannel(DevInst, UserRscNum,
					RscsBc);
			break;
		}
	}

	free(ShimRscsBc);
	free(RscsBc);
	return RC;
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
*			* NPI interrupt line #5.
*		Currently, this API only supports Linux UIO, CDO, and debug
*		backends.
******************************************************************************/
AieRC XAie_ErrorHandlingInit(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 TileType, L1BroadcastIdSwA, L1BroadcastIdSwB, MemTileStart,
	   MemTileEnd, AieRowStart, AieRowEnd, BroadcastDirSwA, BroadcastDirSwB;
	XAie_LocType Loc;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	RC = XAie_ErrorHandlingReserveRsc(DevInst);
	if (RC != XAIE_OK) {
		return RC;
	}

	MemTileStart = DevInst->MemTileRowStart;
	MemTileEnd = DevInst->MemTileRowStart + DevInst->MemTileNumRows;
	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(Loc.Col = 0; Loc.Col < DevInst->NumCols; Loc.Col++) {
		/* Setup error broadcasts to SOUTH from memory and core module */
		BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
				  XAIE_EVENT_BROADCAST_EAST |
				  XAIE_EVENT_BROADCAST_WEST;
		BroadcastDirSwB = BroadcastDirSwA;

		for(Loc.Row = AieRowStart; Loc.Row < AieRowEnd; Loc.Row++) {
			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwA);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in core module\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in memory module\n");
				return RC;
			}
		}

		/* Setup error broadcasts to SOUTH from mem tile */
		for(Loc.Row = MemTileStart; Loc.Row < MemTileEnd; Loc.Row++) {
			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
				continue;

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwA);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in mem tile switch A\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in mem tile switch B\n");
				return RC;
			}
		}

		/*
		 * Setup broadcast from array and PL module to the closest
		 * available L2 interrupt controller.
		 */
		Loc.Row = DevInst->ShimRow;

		/*
		 * Block direct broadcast from AIE array to the
		 * broadcast network in shim tiles.
		 */
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

		/*
		 * Enable broadcast network from AIE array to generate
		 * interrupts in L1 interrupt controller.
		 */
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

		/*
		 * Enable shim tile's internal error interrupts to L1
		 * interrupt controller in switch A.
		 */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_SHIM_INTR_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		/*
		 * Compute the broadcast line number on which L1 interrupt
		 * controller must generate error interrupts.
		 */
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
		if (L1IntrMod == NULL) {
			XAIE_ERROR("Invalid module type\n");
			return XAIE_INVALID_ARGS;
		}

		L1BroadcastIdSwA = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc,
					XAIE_EVENT_SWITCH_A);

		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				L1BroadcastIdSwA);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		L1BroadcastIdSwB = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc,
					XAIE_EVENT_SWITCH_B);
		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_B,
				L1BroadcastIdSwB);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		/*
		 * Interrupts within the shim tile's broadcast network must be
		 * routed to the closest L2 interrupt controller. While doing
		 * so, such interrupts need to be blocked from broadcasting
		 * beyond the L2 interrupt controller tile.
		 */
		if(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			XAie_LocType NextLoc;

			BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
					  XAIE_EVENT_BROADCAST_SOUTH |
					  XAIE_EVENT_BROADCAST_WEST;
			BroadcastDirSwB = XAIE_EVENT_BROADCAST_ALL;

			/*
			 * Create bitmask to block broadcast from previous
			 * columns based on the location NoC tile.
			 */
			RC = _XAie_FindNextNoCTile(DevInst, Loc, &NextLoc);
			if (RC != XAIE_OK) {
				L1BroadcastIdSwB = XAIE_ERROR_L2_ENABLE;
			} else {
				L1BroadcastIdSwB = (1U <<
						(L1BroadcastIdSwB + 1U)) - 1U;
			}

			RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
					XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
					L1BroadcastIdSwB, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in shim tile switch B\n");
				return RC;
			}

			RC = XAie_IntrCtrlL2Enable(DevInst, Loc,
					XAIE_ERROR_L2_ENABLE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to enable interrupts to L2\n");
				return RC;
			}
		} else {
			XAie_LocType NextLoc;

			RC = _XAie_FindNextNoCTile(DevInst, Loc, &NextLoc);
			if (RC != XAIE_OK) {
				BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_EAST;
				BroadcastDirSwB = BroadcastDirSwA;
			} else {
				BroadcastDirSwA = XAIE_EVENT_BROADCAST_NORTH |
						  XAIE_EVENT_BROADCAST_SOUTH |
						  XAIE_EVENT_BROADCAST_WEST;
				BroadcastDirSwB = BroadcastDirSwA;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
					XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
					L1BroadcastIdSwB, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in shim tile switch B\n");
				return RC;
			}
		}

		RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_PL_MOD,
				XAIE_EVENT_SWITCH_A, L1BroadcastIdSwA,
				BroadcastDirSwA);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block broadcasts in shim tile switch A\n");
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

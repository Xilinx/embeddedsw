/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc_events.c
* @{
*
* This file contains routines for events resource management
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Tejus   03/22/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API returns the user event enum given a resource id from the event map.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Mod: Module type
* @param	RscId: Specific resource to be requested
*
* @return	Event enum on success. XAIE_EVENT_ENUM_MAX on failure..
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_Events _XAie_GetUserEventfromRscId(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Mod, u8 RscId)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType =  _XAie_GetTileTypefromLoc(DevInst, Loc);

	if(Mod == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

	return EvntMod->UserEventMap->Event + RscId -
		EvntMod->UserEventMap->RscId;
}

/*****************************************************************************/
/**
* This API returns the resource id for a given map and event enum.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Mod: Module type
* @param	Event: Event enum
*
* @return	Resource ID on success and XAIE_USER_EVENT_MAX on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static u8 _XAie_GetRscIdfromUserEvents(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Mod, XAie_Events Event)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType =  _XAie_GetTileTypefromLoc(DevInst, Loc);

	if(Mod == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

	return Event - EvntMod->UserEventMap->Event;
}

/*****************************************************************************/
/**
* This API returns the PC event enum given a resource id from the event map.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Mod: Module type
* @param	RscId: Specific resource to be requested
*
* @return	Event enum on success. XAIE_EVENT_ENUM_MAX on failure..
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_Events _XAie_GetPCEventfromRscId(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Mod, u8 RscId)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType =  _XAie_GetTileTypefromLoc(DevInst, Loc);
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

	return EvntMod->PCEventMap->Event + RscId -
		EvntMod->PCEventMap->RscId;
}

/*****************************************************************************/
/**
* This API returns the resource id for a given map and PC event enum.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Mod: Module type
* @param	Event: Event enum
*
* @return	Resource ID on success and XAIE_USER_EVENT_MAX on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static u8 _XAie_GetRscIdfromPCEvents(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Mod, XAie_Events Event)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType =  _XAie_GetTileTypefromLoc(DevInst, Loc);
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

	return Event - EvntMod->PCEventMap->Event;
}

/*****************************************************************************/
/**
* This API checks the validity of an event enum for a given tile type and module
* type.
*
* @param	DevInst: Device Instance
* @param	TileType: Tile type
* @param	Mod: Module type
* @param	Event: Event enum
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_CheckEventValidity(XAie_DevInst *DevInst, u8 TileType,
		XAie_ModuleType Mod, XAie_Events Event)
{
	u8 MappedEvent;
	const XAie_EvntMod *EvntMod;

	if(Mod == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

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

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request user events in pool. The API grants user
* events based on availibility and marks that resource status in
* relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;
	u32 UserRscIndex = 0U;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_RequestRsc(DevInst, NumReq, RscReq, Rscs,
			XAIE_USER_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map rsc id returned to user event enum */
	for(u32 i = 0U; i < NumReq; i++) {
		for(u32 j = 0U; j < RscReq[i].NumRscPerTile; j++) {
			Rscs[UserRscIndex].RscId =
				_XAie_GetUserEventfromRscId(DevInst,
						RscReq[i].Loc, RscReq[i].Mod,
						Rscs[UserRscIndex].RscId);
			UserRscIndex++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to release particular user event..
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Releasing a particular resource, frees that resource from
* 		static as well as runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_ReleaseUserEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_USER_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetRscIdfromUserEvents(DevInst,
				Rscs[i].Loc, Rscs[i].Mod, Rscs[i].RscId);
	}

	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_USER_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated user event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreeUserEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_USER_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetRscIdfromUserEvents(DevInst,
				Rscs[i].Loc, Rscs[i].Mod, Rscs[i].RscId);
	}

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_USER_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request particular user event that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_USER_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* Check validity of the user events passed by the user */
	for(u32 i = 0U; i < NumReq; i++) {
		RC = _XAie_CheckEventValidity(DevInst,
				_XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc),
				RscReq[i].Mod, RscReq[i].RscId);
		if(RC != XAIE_OK)
			return RC;
	}

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < NumReq; i++) {
		RscReq[i].RscId = _XAie_GetRscIdfromUserEvents(DevInst,
				RscReq[i].Loc, RscReq[i].Mod, RscReq[i].RscId);
	}

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_USER_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request PC events in pool. The API grants PC
* events based on availibility and marks that resource status in
* relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;
	u32 UserRscIndex = 0U;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_RequestRsc(DevInst, NumReq, RscReq, Rscs,
			XAIE_PC_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map rsc id returned to user event enum */
	for(u32 i = 0U; i < NumReq; i++) {
		for(u32 j = 0U; j < RscReq[i].NumRscPerTile; j++) {
			Rscs[UserRscIndex].RscId =
				_XAie_GetPCEventfromRscId(DevInst,
						RscReq[i].Loc, RscReq[i].Mod,
						Rscs[UserRscIndex].RscId);
			UserRscIndex++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to release particular PC event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Releasing a particular resource, frees that resource from
* 		static as well as runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_ReleasePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_PC_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetRscIdfromPCEvents(DevInst,
				Rscs[i].Loc, Rscs[i].Mod, Rscs[i].RscId);
	}

	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_PC_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated PC event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_PC_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetRscIdfromPCEvents(DevInst,
				Rscs[i].Loc, Rscs[i].Mod, Rscs[i].RscId);
	}

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_PC_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request particular PC event that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_PC_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* Check validity of the user events passed by the user */
	for(u32 i = 0U; i < NumReq; i++) {
		RC = _XAie_CheckEventValidity(DevInst,
				_XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc),
				RscReq[i].Mod, RscReq[i].RscId);
		if(RC != XAIE_OK)
			return RC;
	}

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < NumReq; i++) {
		RscReq[i].RscId = _XAie_GetRscIdfromPCEvents(DevInst,
				RscReq[i].Loc, RscReq[i].Mod, RscReq[i].RscId);
	}

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_PC_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request stream switch event ports select in pool.
* The API grants stream switch event ports based on availibility and marks that
* resource status in relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestSSEventPortSelect(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestRsc(DevInst, NumReq, RscReq, Rscs,
			XAIE_SS_EVENT_PORTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to release particular stream switch event port
* selection in the resource manager.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Releasing a particular resource, frees that resource from
* 		static as well as runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_ReleaseSSEventPortSelect(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_SS_EVENT_PORTS_RSC);
	if(RC != XAIE_OK)
		return RC;


	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_SS_EVENT_PORTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated stream switch
* event port selection.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreeSSEventPortSelect(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_SS_EVENT_PORTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_SS_EVENT_PORTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request particular stream switch event port
* selection that was allocated statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedSSEventPortSelect(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_SS_EVENT_PORTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_SS_EVENT_PORTS_RSC);
}

/*****************************************************************************/
/**
* This API returns the resource id for a given group event, location and module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Mod: Module type
* @param	Event: Event enum
*
* @return	Resource ID on success and XAIE_USER_EVENT_MAX on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static u8 _XAie_GetRscIdfromGroupEvents(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Mod, XAie_Events Event)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType =  _XAie_GetTileTypefromLoc(DevInst, Loc);

	if(Mod == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];

	for(u8 i = 0U; i < EvntMod->NumGroupEvents; i++) {
		if(EvntMod->Group[i].GroupEvent == Event)
			return EvntMod->Group[i].GroupOff;
	}

	return 0xFF;
}

/*****************************************************************************/
/**
* This API shall be used to request particular Group event that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedGroupEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_GROUP_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* Check validity of the user events passed by the user */
	for(u32 i = 0U; i < NumReq; i++) {
		RC = _XAie_CheckEventValidity(DevInst,
				_XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc),
				RscReq[i].Mod, RscReq[i].RscId);
		if(RC != XAIE_OK)
			return RC;
	}

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < NumReq; i++) {
		RscReq[i].RscId = _XAie_GetRscIdfromGroupEvents(DevInst,
				RscReq[i].Loc, RscReq[i].Mod, RscReq[i].RscId);
	}

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_GROUP_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated Group Event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreeGroupEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_GROUP_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map RscId from event enums to bit resources */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetRscIdfromGroupEvents(DevInst,
				Rscs[i].Loc, Rscs[i].Mod, Rscs[i].RscId);
	}

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_GROUP_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request PC Range events in pool. The API grants PC
* events based on availibility and marks that resource status in
* relevant bitmap. For every PC Range requested, two PC events are marked as
* allocated. UserRscNum should always be double the NumReq.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestPCRangeEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	/*
	 * UserRscNum > 2U * NumReq as two PC Events are allocated for every
	 * PCRange request.
	 */
	if((NumReq * 2U) > UserRscNum) {
		XAIE_ERROR("Insufficient memory to return allocated rscs\n");
		return XAIE_INVALID_ARGS;
	}

	/* PC Range events are always in a pairs of two */
	RC = _XAie_RscMgr_RequestRscContiguous(DevInst, NumReq, RscReq, Rscs,
			XAIE_PC_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	/* map rsc id returned to user event enum */
	for(u32 i = 0U; i < UserRscNum; i++) {
		Rscs[i].RscId = _XAie_GetPCEventfromRscId(DevInst, Rscs[i].Loc,
				Rscs[i].Mod, Rscs[i].RscId);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request combo events in pool. The API grants user
* events based on availibility and marks that resource status in
* relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestRscContiguous(DevInst, NumReq, RscReq, Rscs,
			XAIE_COMBO_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to release particular Combo event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Releasing a particular resource, frees that resource from
* 		static as well as runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_ReleaseComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_COMBO_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_COMBO_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated combo event.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreeComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_COMBO_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_COMBO_EVENTS_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request particular user event that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_COMBO_EVENTS_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_COMBO_EVENTS_RSC);
}

/** @} */

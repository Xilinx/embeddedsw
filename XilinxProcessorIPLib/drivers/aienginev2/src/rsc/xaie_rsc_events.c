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
* 1.0   Tejus   03/16/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_events.h"
#include "xaie_io.h"
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_NUM_USER_EVENTS	4U
#define XAIE_USER_EVENT_MAX	XAIE_NUM_USER_EVENTS

#define XAIE_USER_EVENT_TYPE	0U
/**************************** Type Definitions *******************************/
/* struct to capture RscId to events mapping*/
typedef struct {
	u8 RscId;
	XAie_Events Event;
} XAie_EventMap;

/************************** Constant Definitions *****************************/
/* const mapping of user event for core module */
static const XAie_EventMap CoreModUserEventStart =
{
	.RscId = 0,
	.Event = XAIE_EVENT_USER_EVENT_0_CORE,
};

/* const mapping of user event for mem module */
static const XAie_EventMap MemModUserEventStart =
{
	.RscId = 0,
	.Event = XAIE_EVENT_USER_EVENT_0_MEM,
};

/* const mapping of user event for pl module */
static const XAie_EventMap PlModUserEventStart =
{
	.RscId = 0,
	.Event = XAIE_EVENT_USER_EVENT_0_PL,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API returns the user event enum given a resource id from the event map.
*
* @param	Mod: Module type
* @param	RscId: Specific resource to be requested
*
* @return	Event enum on success. XAIE_EVENT_ENUM_MAX on failure..
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_Events _XAie_GetEventfromRscId(XAie_ModuleType Mod, u8 RscId)
{
	switch(Mod) {
		case XAIE_MEM_MOD:
			return MemModUserEventStart.Event + RscId -
				MemModUserEventStart.RscId;
		case XAIE_CORE_MOD:
			return CoreModUserEventStart.Event + RscId -
				CoreModUserEventStart.RscId;
		case XAIE_PL_MOD:
			return PlModUserEventStart.Event + RscId -
				PlModUserEventStart.RscId;
		default:
			XAIE_ERROR("Invalid Mod type\n");
			break;
	}

	return  XAIE_EVENT_ENUM_MAX;
}

/*****************************************************************************/
/**
* This API returns the resource id for a given map and event enum.
*
* @param	Mod: Module type
* @param	Event: Event enum
*
* @return	Resource ID on success and XAIE_USER_EVENT_MAX on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static u8 _XAie_GetRscIdfromUserEvents(XAie_ModuleType Mod, XAie_Events Event)
{
	switch(Mod) {
		case XAIE_MEM_MOD:
			return Event - MemModUserEventStart.Event;
		case XAIE_CORE_MOD:
			return Event - CoreModUserEventStart.Event;
		case XAIE_PL_MOD:
			return Event - PlModUserEventStart.Event;
		default:
			XAIE_ERROR("Invalid Mod type\n");
			break;
	}

	return  XAIE_USER_EVENT_MAX;
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
* This API initializes user events resource bitmap for all tile types.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_UserEventsRscInit(XAie_DevInst *DevInst)
{
	for(u8 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {

		u8 NumModules;
		u32 BitmapSize, BitmapNumRows;

		/* Memory already allocated for XAIEGBL_TILE_TYPE_SHIMPL */
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;
		NumModules = DevInst->DevProp.DevMod[i].NumModules;
		BitmapNumRows = _XAie_GetNumRows(DevInst, i);
		/*
		 * One bitmap is allocated for each tiletype. The size of each
		 * bitmap is calculated based on number of rows and cols in the
		 * patition for that tiletype and the number of counters per
		 * tile. The allocation includes static and runtime bitmap
		 * and hence the multiplying factor 2U.
		 * Static bitmap is used to mark the resources allocated during
		 * compile time and runtime bitmap will be updated as per
		 * runtime request for a resource.
		 */
		BitmapSize = _XAie_NearestRoundUp((NumModules *
					XAIE_NUM_USER_EVENTS * BitmapNumRows *
					DevInst->NumCols * 2U), 8 * sizeof(u32));
		BitmapSize /= 8 * sizeof(u32);
		DevInst->RscMapping[i].UserEventsBitmap = (u32 *)calloc(
				BitmapSize, sizeof(u32));
		if(DevInst->RscMapping[i].UserEventsBitmap == NULL) {
			/* Clear previous bitmap alocation */
			for(u8 k = 0; k < i; k++) {
				if(k == XAIEGBL_TILE_TYPE_SHIMNOC)
					continue;
				free(DevInst->RscMapping[k].UserEventsBitmap);
			}

			XAIE_ERROR("Unable to allocate memory for user events"
				       " bitmap\n");

			return XAIE_ERR;
		}
	}

	DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMNOC].UserEventsBitmap =
		DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMPL].UserEventsBitmap;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API deallocates memory for user events resource bitmap for all tiletype.
*
* @param	DevInst: Device Instance
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_UserEventsRscFinish(XAie_DevInst *DevInst)
{
	for(u32 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;
		free(DevInst->RscMapping[i].UserEventsBitmap);
	}
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
* 		granted resource. The resource id returned by the API will have
*		user events enum defined in xaie_events.h
*
*******************************************************************************/
AieRC XAie_RequestUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;
	u8 TileType;
	u32 UserRscIndex = 0, TotalReq = 0;

	if((DevInst == XAIE_NULL) || (Rscs == NULL) || (RscReq == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of UserRscNum */
	for(u32 j = 0; j < NumReq; j++)
		TotalReq += RscReq[j].NumRscPerTile;
	if(UserRscNum != TotalReq) {
		XAIE_ERROR("Invalid UserRscNum: %d\n", UserRscNum);
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles passed to this API */
	for(u32 j = 0; j < NumReq; j++) {
		if(RscReq[j].Loc.Row >= DevInst->NumRows ||
			RscReq[j].Loc.Col >= DevInst->NumCols) {
			XAIE_ERROR("Invalid Loc Col:%d Row:%d\n",
					RscReq[j].Loc.Col, RscReq[j].Loc.Row);
			return XAIE_INVALID_ARGS;
		}
	}

	for(u32 i = 0; i < NumReq; i++) {

		u32 StaticBitmapOffset, BitmapOffset = 0;
		u32 StartBit, MaxRscVal;
		XAie_BackendTilesRsc TilesRsc;

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_TILE;
		}

		MaxRscVal = XAIE_NUM_USER_EVENTS;
		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
					MaxRscVal);
		StartBit = _XAie_GetStartBit(DevInst, RscReq[i].Loc,
				MaxRscVal) + BitmapOffset;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].UserEventsBitmap;
		TilesRsc.RscType = XAIE_USER_EVENTS_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.Loc = RscReq[i].Loc;
		TilesRsc.Mod = RscReq[i].Mod;
		TilesRsc.NumRscPerTile = RscReq[i].NumRscPerTile;
		TilesRsc.StartBit = StartBit;
		TilesRsc.StaticBitmapOffset = StaticBitmapOffset;
		TilesRsc.Rscs = &Rscs[UserRscIndex];

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreeUserEvents(DevInst, UserRscIndex, Rscs);
			XAIE_ERROR("Unable to request user events\n");
			return XAIE_INVALID_ARGS;
		}

		for(u32 j = 0; j < RscReq[i].NumRscPerTile; j++) {
			Rscs[UserRscIndex].Loc = RscReq[i].Loc;
			Rscs[UserRscIndex].Mod = RscReq[i].Mod;
			Rscs[UserRscIndex].RscType = XAIE_USER_EVENTS_RSC;
			/* Map to events enum from RscId */
			Rscs[UserRscIndex].RscId =
				_XAie_GetEventfromRscId(TilesRsc.Mod,
						Rscs[UserRscIndex].RscId);
			UserRscIndex++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to release particular user event.
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
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Rscs == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles passed to this API */
	RC = _XAie_CheckLocsValidity(DevInst, UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	for(u32 i = 0; i < UserRscNum; i++) {

		u32 StaticBitmapOffset, BitmapOffset = 0;
		u32 StartBit, MaxRscVal;
		XAie_BackendTilesRsc TilesRsc;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, Rscs[i].Loc, Rscs[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		MaxRscVal = XAIE_NUM_USER_EVENTS;
		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
					MaxRscVal);
		StartBit = _XAie_GetStartBit(DevInst, Rscs[i].Loc,
				MaxRscVal) + BitmapOffset;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].UserEventsBitmap;
		TilesRsc.RscType = XAIE_USER_EVENTS_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.StartBit = StartBit;
		TilesRsc.StaticBitmapOffset = StaticBitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = _XAie_GetRscIdfromUserEvents(TilesRsc.Mod,
				Rscs[i].RscId);

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_RELEASE_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Unable to release user events\n");
			return XAIE_INVALID_ARGS;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated user events..
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
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Rscs == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles passed to this API */
	RC = _XAie_CheckLocsValidity(DevInst, UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	for(u32 i = 0; i < UserRscNum; i++) {

		u32 BitmapOffset = 0;
		u32 StartBit, MaxRscVal;
		XAie_BackendTilesRsc TilesRsc;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, Rscs[i].Loc, Rscs[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		MaxRscVal = XAIE_NUM_USER_EVENTS;
		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
					MaxRscVal);
		StartBit = _XAie_GetStartBit(DevInst, Rscs[i].Loc,
				MaxRscVal) + BitmapOffset;

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].UserEventsBitmap;
		TilesRsc.StartBit = StartBit;
		TilesRsc.RscType = XAIE_USER_EVENTS_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = _XAie_GetRscIdfromUserEvents(TilesRsc.Mod,
				Rscs[i].RscId);

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_FREE_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Unable to release user events\n");
			return XAIE_INVALID_ARGS;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request a particular user event that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	RscId: Specific resource to be requested
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
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
*		The resource requested by the user should use enum defined in
*		xaie_events.h
*
*******************************************************************************/
AieRC XAie_RequestAllocatedUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;
	u8 TileType;
	u32 UserRscIndex = 0;

	if((DevInst == XAIE_NULL) || (RscReq == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles passed to this API */
	RC = _XAie_CheckLocsValidity(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	for(u32 i = 0; i < NumReq; i++) {

		u32 StaticBitmapOffset, BitmapOffset = 0;
		u32 RscBit, MaxRscVal;
		XAie_BackendTilesRsc TilesRsc;

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			RC = XAie_FreeUserEvents(DevInst, UserRscIndex, RscReq);
			return XAIE_INVALID_TILE;
		}

		/* Check if the event passed by user is valid */
		RC = _XAie_CheckEventValidity(DevInst, TileType, RscReq[i].Mod,
				RscReq[i].RscId);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreeUserEvents(DevInst, UserRscIndex, RscReq);
			return XAIE_INVALID_ARGS;
		}

		MaxRscVal = XAIE_NUM_USER_EVENTS;
		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
					MaxRscVal);
		RscBit = _XAie_GetStartBit(DevInst, RscReq[i].Loc,
				MaxRscVal) + BitmapOffset;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].UserEventsBitmap;
		TilesRsc.RscType = XAIE_USER_EVENTS_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.Loc = RscReq[i].Loc;
		TilesRsc.Mod = RscReq[i].Mod;
		TilesRsc.StartBit = RscBit;
		TilesRsc.StaticBitmapOffset = StaticBitmapOffset;
		TilesRsc.Rscs = &RscReq[UserRscIndex];
		TilesRsc.RscId = _XAie_GetRscIdfromUserEvents(TilesRsc.Mod,
				RscReq[i].RscId);

		RC = XAie_RunOp(DevInst,
				XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreeUserEvents(DevInst, UserRscIndex, RscReq);
			XAIE_ERROR("Unable to request user events\n");
			return XAIE_INVALID_ARGS;
		}

		UserRscIndex++;
	}

	return XAIE_OK;
}

/** @} */

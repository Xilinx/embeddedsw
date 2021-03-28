/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc.c
* @{
*
* This file contains routines for AIE resource manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 01/11/2021  Initial creation
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
#define XAIE_TRACE_CTRL_RSCS_PER_MOD	1U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API returns the total number of Resources available for a given tile
* type and resource type.
*
* @param	DevInst: Device Instance
* @param	TileType: Tile type
* @param	RscType: Resource type.
*
* @return	Number of resources.
*
* @note		Internal only.
*
*******************************************************************************/
u32 _XAie_GetTotalNumRscs(XAie_DevInst *DevInst, u8 TileType,
		XAie_RscType RscType)
{
	u8 NumMods;
	u32 NumRscs = 0U;

	/*
	 * TODO: Replace the below case statement with a data structure that
	 * can be indexed using tile type and resource type.
	 */
	NumMods = DevInst->DevProp.DevMod[TileType].NumModules;
	switch(RscType) {
	case XAIE_PERFCNT_RSC:
	{
		const XAie_PerfMod *PerfMod;

		for(u8 i = 0U; i < NumMods; i++) {
			PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[i];
			NumRscs += PerfMod->MaxCounterVal;
		}
		return NumRscs;
	}
	case XAIE_USER_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		for(u8 i = 0U; i < NumMods; i++) {
			EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[i];
			NumRscs += EventMod->NumUserEvents;
		}
		return NumRscs;
	}
	case XAIE_PC_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		for(u8 i = 0U; i < NumMods; i++) {
			EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[i];
			NumRscs += EventMod->NumPCEvents;
		}
		return NumRscs;
	}
	case XAIE_TRACE_CTRL_RSC:
	{
		for(u8 i = 0U; i < NumMods; i++) {
			NumRscs += XAIE_TRACE_CTRL_RSCS_PER_MOD;
		}
		return NumRscs;
	}
	case XAIE_SS_EVENT_PORTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		for(u8 i = 0U; i < NumMods; i++) {
			EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[i];
			NumRscs += EventMod->NumStrmPortSelectIds;
		}
		return NumRscs;
	}
	case XAIE_GROUP_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		for(u8 i = 0U; i < NumMods; i++) {
			EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[i];
			NumRscs += EventMod->NumGroupEvents;
		}
		return NumRscs;
	}
	default:
		return 0U;
	}
}

/*****************************************************************************/
/**
* This API deallocates memory for all resource bitmaps.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrFinish(XAie_DevInst *DevInst)
{
	for(u8 i = 0U; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		XAie_ResourceManager *RscMap = &DevInst->RscMapping[i];

		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;

		for(u8 RscType = 0U; RscType < XAIE_MAX_RSC; RscType++) {
			free(RscMap->Bitmaps[RscType]);
		}
		free(RscMap->Bitmaps);
	}

	free(DevInst->RscMapping);
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API initializes all resource bitmaps.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrInit(XAie_DevInst *DevInst)
{
	u8 i;

	DevInst->RscMapping = (XAie_ResourceManager *)malloc(
			sizeof(*(DevInst->RscMapping)) * XAIEGBL_TILE_TYPE_MAX);
	if(DevInst->RscMapping == NULL) {
		XAIE_ERROR("Unable to allocate memory for bitmaps\n");
		return XAIE_ERR;
	}

	for(i = 0U; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		u32 NumRows;
		XAie_ResourceManager *RscMap;
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;

		NumRows = _XAie_GetNumRows(DevInst, i);
		RscMap = &DevInst->RscMapping[i];
		RscMap->Bitmaps = (u32 **)malloc(sizeof(u32*) * XAIE_MAX_RSC);
		if(RscMap == XAIE_NULL) {
			XAIE_ERROR("Memory allocation failed for tile "
					"type:%d\n", i);
			for(u8 k = 0U; k < i; k++) {
				free(DevInst->RscMapping[k].Bitmaps);
				goto error;
			}
		}

		for(u8 RscType = 0U; RscType < XAIE_MAX_RSC; RscType++) {
			u32 BitmapSize, NumRscs;

			NumRscs = _XAie_GetTotalNumRscs(DevInst, i, RscType);
			/*
			 * One bitmap is allocated for each tiletype. The size
			 * of each bitmap is calculated based on number of rows
			 * and cols in the patition for that tiletype and the
			 * number of counters per tile. The allocation includes
			 * static and runtime bitmap and hence the multiplying
			 * factor 2U. Static bitmap is used to mark the
			 * resources allocated during compile time and runtime
			 * bitmap will be updated as per runtime request for a
			 * resource.
			 */
			BitmapSize = _XAie_NearestRoundUp(NumRows * NumRscs *
					DevInst->NumCols * 2U, 8 * sizeof(u32));
			BitmapSize /= 8 * sizeof(u32);
			XAIE_DBG("TileType: %u, RscType: %u, BitmapSize: 0x%x\n",
					i, RscType, BitmapSize);
			RscMap->Bitmaps[RscType] = (u32 *)calloc(BitmapSize,
					sizeof(u32));
			if(RscMap->Bitmaps[RscType] == XAIE_NULL) {
				XAIE_ERROR("Calloc failed for TileType: %u, "
						"RscType: %u\n", i, RscType);
				for(u8 k = 0U; k < RscType; k++) {
					free(RscMap->Bitmaps[k]);
				}
				goto freeRscMaps;
			}
		}
	}

	DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMNOC] =
		DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMPL];

	return XAIE_OK;

freeRscMaps:
	for(u8 k = 0U; k < i; k++) {
		free(DevInst->RscMapping[k].Bitmaps);
	}

error:
	free(DevInst->RscMapping);

	XAIE_ERROR("Memory allocation for resource manager bitmaps failed\n");
	return XAIE_ERR;

}

/*****************************************************************************/
/**
* This API checks validity for the given list of tiles.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of tiles to be validated
* @param	RscReq: Pointer to request containing locs to be validated
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_CheckLocsValidity(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	for(u32 j = 0; j < NumReq; j++) {
		if(RscReq[j].Loc.Row >= DevInst->NumRows ||
			RscReq[j].Loc.Col >= DevInst->NumCols) {
			XAIE_ERROR("Invalid Loc Col:%d Row:%d\n",
					RscReq[j].Loc.Col, RscReq[j].Loc.Row);
			return XAIE_INVALID_ARGS;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API calculates index for the start bit in a resource bitmap.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of tile
* @param	MaxRscVal: Number of resource per tile
*
* @return	Start bit index
*
* @note		Internal only.
*
*******************************************************************************/
u32 _XAie_GetStartBit(XAie_DevInst *DevInst, XAie_LocType Loc, u32 MaxRscVal)
{
	u8 TileType;
	u32 StartRow, BitmapNumRows;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	StartRow = _XAie_GetStartRow(DevInst, TileType);
	BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

	return _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows, Loc,
		MaxRscVal, StartRow);
}

/*****************************************************************************/
/**
* This API checks the validity of all the arugments passed to the resource
* manager request APIs.
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
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrRequestApi_CheckArgs(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	u32 TotalReq = 0;

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

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API checks the validity of all the arugments passed to the resource
* manager APIs.
*
* @param	DevInst: Device Instance
* @param	RscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	RscType: Resource type
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrRscApi_CheckArgs(XAie_DevInst *DevInst, u32 RscNum,
		XAie_UserRsc *Rscs, XAie_RscType RscType)
{
	AieRC RC;

	if((DevInst == XAIE_NULL) || (Rscs == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles passed to this API */
	RC = _XAie_CheckLocsValidity(DevInst, RscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	for(u32 i = 0U; i < RscNum; i++) {
		RC = _XAie_CheckModule(DevInst, Rscs[i].Loc, Rscs[i].Mod);
		if(RC != XAIE_OK) {
			return RC;
		}
		if(Rscs[i].RscType != RscType) {
			XAIE_ERROR("Resource type mismatch\n");
			return XAIE_ERR;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API checks the validity of modules in all resources requested by the
* user.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgr_CheckModforReqs(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq)
{
	AieRC RC;

	/* check for module and tiletype combination */
	for(u32 i = 0U; i < NumReq; i++) {
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API gets PerfMod from device instance, TileType and Module
*
* @param	DevInst: Device Instance
* @param	TileType: Type of tile
* @param	Mod: Module - MEM, CORE or PL.
*
* @return	Pointer to performance counter module
*
* @note		Internal only.
*
*******************************************************************************/
static const XAie_PerfMod *_XAie_GetPerfMod(XAie_DevInst *DevInst, u8 TileType,
		XAie_ModuleType Mod)
{
	if(Mod == XAIE_PL_MOD)
		return &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	else
		return &DevInst->DevProp.DevMod[TileType].PerfMod[Mod];
}

/*****************************************************************************/
/**
* This API gets EventMod from device instance, TileType and Module
*
* @param	DevInst: Device Instance
* @param	TileType: Type of tile
* @param	Mod: Module - MEM, CORE or PL.
*
* @return	Pointer to performance counter module
*
* @note		Internal only.
*
*******************************************************************************/
static const XAie_EvntMod *_XAie_GetEventMod(XAie_DevInst *DevInst, u8 TileType,
		XAie_ModuleType Mod)
{
	if(Mod == XAIE_PL_MOD)
		return &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		return &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];
}

/*****************************************************************************/
/**
* This API returns the max resource value for a give location, resource type and
* module.
*
* @param	DevInst: Device Instance
* @param	RscType: Resource type
* @param	TileType: Type of tile
* @param	Mod: Module - MEM, CORE or PL.
*
* @return	Max number of resources.
*
* @note		Internal only.
*
*******************************************************************************/
u32 _XAie_RscMgr_GetMaxRscVal(XAie_DevInst *DevInst, XAie_RscType RscType,
		XAie_LocType Loc, XAie_ModuleType Mod)
{
	switch(RscType) {
	case XAIE_PERFCNT_RSC:
	{
		const XAie_PerfMod *PerfMod;
		u8 TileType;
		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		PerfMod = _XAie_GetPerfMod(DevInst, TileType, Mod);
		return PerfMod->MaxCounterVal;
	}
	case XAIE_USER_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumUserEvents;
	}
	case XAIE_PC_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumPCEvents;
	}
	case XAIE_TRACE_CTRL_RSC:
	{
		return XAIE_TRACE_CTRL_RSCS_PER_MOD;
	}
	case XAIE_SS_EVENT_PORTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumStrmPortSelectIds;
	}
	case XAIE_GROUP_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumGroupEvents;
	}
	default:
		return 0U;
	}
}

/*****************************************************************************/
/**
* This API returns the bitmap offsets and max resource values for a given
* resource type, location and module.
*
* @param	DevInst: Device Instance
* @param	RscType: Resource type
* @param	TileType: Type of tile
* @param	Mod: Module - MEM, CORE or PL.
* @param	Offsets: Pointers to internal data structure to store the
*		offsets
*
* @return	Max number of resources.
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_RscMgr_GetBitmapOffsets(XAie_DevInst *DevInst, XAie_RscType RscType,
		XAie_LocType Loc, XAie_ModuleType Mod,
		XAie_BitmapOffsets *Offsets)
{
	u32 BitmapOffset = 0U, StartBit, StaticBitmapOffset;
	u32 MaxRscVal;
	u8 TileType;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	MaxRscVal = _XAie_RscMgr_GetMaxRscVal(DevInst, RscType, Loc, Mod);
	if(Mod == XAIE_CORE_MOD)
		BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				_XAie_RscMgr_GetMaxRscVal(DevInst, RscType, Loc,
					XAIE_MEM_MOD));

	StartBit = _XAie_GetStartBit(DevInst, Loc, MaxRscVal) + BitmapOffset;
	StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
		_XAie_GetNumRows(DevInst, TileType);

	Offsets->StaticBitmapOffset = StaticBitmapOffset;
	Offsets->BitmapOffset = BitmapOffset;
	Offsets->StartBit = StartBit;
	Offsets->MaxRscVal = MaxRscVal;
}

/*****************************************************************************/
/**
* This API shall be used to request a resource from the backend based on the
* resource type.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	RscType: Resource type
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource. Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgr_RequestRsc(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, XAie_UserRsc *Rscs,
		XAie_RscType RscType)
{
	AieRC RC;
	u32 UserRscIndex = 0U;

	for(u32 i = 0U; i < NumReq; i++) {
		XAie_BackendTilesRsc TilesRsc;
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		_XAie_RscMgr_GetBitmapOffsets(DevInst, RscType,
				RscReq[i].Loc, RscReq[i].Mod, &Offsets);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].Bitmaps[RscType];
		TilesRsc.RscType = RscType;
		TilesRsc.MaxRscVal = Offsets.MaxRscVal;
		TilesRsc.BitmapOffset = Offsets.BitmapOffset;
		TilesRsc.StartBit = Offsets.StartBit;
		TilesRsc.StaticBitmapOffset = Offsets.StaticBitmapOffset;
		TilesRsc.Loc = RscReq[i].Loc;
		TilesRsc.Mod = RscReq[i].Mod;
		TilesRsc.NumRscPerTile = RscReq[i].NumRscPerTile;
		TilesRsc.Rscs = &Rscs[UserRscIndex];

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			_XAie_RscMgr_FreeRscs(DevInst, UserRscIndex, Rscs,
					RscType);
			XAIE_ERROR("Unable to request resources. RscType: %d\n",
					RscType);
			return XAIE_INVALID_ARGS;
		}
		for(u32 j = 0U; j < RscReq[i].NumRscPerTile; j++) {
			Rscs[UserRscIndex].Loc = RscReq[i].Loc;
			Rscs[UserRscIndex].Mod = RscReq[i].Mod;
			Rscs[UserRscIndex].RscType = RscType;
			UserRscIndex++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated resource.
*
* @param	DevInst: Device Instance
* @param	RscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	RscType: Resource type
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically. Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgr_FreeRscs(XAie_DevInst *DevInst, u32 RscNum,
		XAie_UserRsc *Rscs, XAie_RscType RscType)
{
	for(u32 i =0U; i < RscNum; i++) {
		XAie_BackendTilesRsc TilesRsc;
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		_XAie_RscMgr_GetBitmapOffsets(DevInst, RscType,
				Rscs[i].Loc, Rscs[i].Mod, &Offsets);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].Bitmaps[RscType];
		TilesRsc.RscType = RscType;
		TilesRsc.MaxRscVal = Offsets.MaxRscVal;
		TilesRsc.BitmapOffset = Offsets.BitmapOffset;
		TilesRsc.StartBit = Offsets.StartBit;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = Rscs[i].RscId;
		/*
		 * NOTE: No need to check the return value from run op function
		 * as free resource is always successful.
		 */
		XAie_RunOp(DevInst, XAIE_BACKEND_OP_FREE_RESOURCE,
				(void *)&TilesRsc);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to free a particular resource.
*
* @param	DevInst: Device Instance
* @param	RscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	RscType: Resource type
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically. Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgr_ReleaseRscs(XAie_DevInst *DevInst, u32 RscNum,
		XAie_UserRsc *Rscs, XAie_RscType RscType)
{
	for(u32 i =0U; i < RscNum; i++) {
		XAie_BackendTilesRsc TilesRsc;
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		_XAie_RscMgr_GetBitmapOffsets(DevInst, RscType,
				Rscs[i].Loc, Rscs[i].Mod, &Offsets);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].Bitmaps[RscType];
		TilesRsc.RscType = RscType;
		TilesRsc.MaxRscVal = Offsets.MaxRscVal;
		TilesRsc.BitmapOffset = Offsets.BitmapOffset;
		TilesRsc.StartBit = Offsets.StartBit;
		TilesRsc.StaticBitmapOffset = Offsets.StaticBitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = Rscs[i].RscId;
		/*
		 * NOTE: No need to check the return value from run op function
		 * as free resource is always successful.
		 */
		XAie_RunOp(DevInst, XAIE_BACKEND_OP_RELEASE_RESOURCE,
				(void *)&TilesRsc);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request particular resource that was allocated
* statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	RscType: Resource type
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC _XAie_RscMgr_RequestAllocatedRsc(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *Rscs, XAie_RscType RscType)
{
	AieRC RC;
	u32 UserRscIndex = 0U;

	for(u32 i =0U; i < NumReq; i++) {
		XAie_BackendTilesRsc TilesRsc;
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		_XAie_RscMgr_GetBitmapOffsets(DevInst, RscType,
				Rscs[i].Loc, Rscs[i].Mod, &Offsets);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].Bitmaps[RscType];
		TilesRsc.RscType = RscType;
		TilesRsc.MaxRscVal = Offsets.MaxRscVal;
		TilesRsc.BitmapOffset = Offsets.BitmapOffset;
		TilesRsc.StartBit = Offsets.StartBit + Rscs[i].RscId;
		TilesRsc.StaticBitmapOffset = Offsets.StaticBitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = Rscs[i].RscId;

		RC = XAie_RunOp(DevInst,
				XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			_XAie_RscMgr_FreeRscs(DevInst, UserRscIndex, Rscs,
					RscType);
			XAIE_ERROR("Unable to request resources. RscType: %d\n",
					RscType);
			return XAIE_INVALID_ARGS;
		}

		UserRscIndex++;
	}

	return XAIE_OK;
}

/** @} */

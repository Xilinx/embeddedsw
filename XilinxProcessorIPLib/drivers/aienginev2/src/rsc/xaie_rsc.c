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
#include <stdio.h>
#include <stdlib.h>

#include "xaie_feature_config.h"
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_RSC_ENABLE
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_TRACE_CTRL_RSCS_PER_MOD	1U
#define XAIE_COMBO_EVENTS_PER_MOD	4U

#define XAIE_RSC_HEADER_TILE_TYPE_SHIFT	0U
#define XAIE_RSC_HEADER_TILE_TYPE_MASK	0xF
#define XAIE_RSC_HEADER_MOD_TYPE_SHIFT	4U
#define XAIE_RSC_HEADER_MOD_TYPE_MASK	0xF
#define XAIE_RSC_HEADER_RSC_TYPE_SHIFT	8U
#define XAIE_RSC_HEADER_RSC_TYPE_MASK	0xFF
#define XAIE_RSC_HEADER_SIZE_SHIFT	16U
#define XAIE_RSC_HEADER_SIZE_MASK	0xFFFFFFFF

/*
 * This typedef defines a resource bitmaps meta data header
 */
typedef struct XAieRscMetaHeader {
	u64 Stat; /* statistics information of the bitmaps, such as number of
		   * bitmaps */
	u64 BitmapOff; /* offset to the start of the binary of the first bitmap
			* element */
} XAieRscMetaHeader;

/*
 * This typedef defines a resource bitmap element
 */
typedef struct XAieRscBitmap {
	u64 Header; /* bitmap header, it contains the following information:
		     * tile type, module type, resource type, and the bitmap
		     * length. */
	u64 Bitmap[0]; /* the pointer of bitmap of the resource */
} XAieRscBitmap;

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
	case XAIE_COMBO_EVENTS_RSC:
	{
		for(u8 i = 0U; i < NumMods; i++) {
			NumRscs += XAIE_COMBO_EVENTS_PER_MOD;
		}
		return NumRscs;
	}
	case XAIE_BCAST_CHANNEL_RSC:
	{
		for(u8 i = 0U; i < NumMods; i++) {
			NumRscs += XAIE_NUM_BROADCAST_CHANNELS;
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
			}
			free(DevInst->RscMapping);
			return XAIE_ERR;
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
				for(u8 k = 0U; k < i; k++) {
					free(DevInst->RscMapping[k].Bitmaps);
				}
				free(DevInst->RscMapping);
				return XAIE_ERR;
			}
		}
	}

	DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMNOC] =
		DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMPL];

	return XAIE_OK;
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	StartRow = _XAie_GetStartRow(DevInst, TileType);
	BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

	return _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows, Loc,
		MaxRscVal, StartRow);
}

/*****************************************************************************/
/**
* This API populates UserRsc with rsc id and marks the rsc allocation in the
* broadcast channel bitmap.
*
* @param        DevInst: Device Instance
* @param        UserRscNum: Pointer for size of UserRsc array
* @param        Rscs: UserRsc return to user
* @param        ChannelIndex: Index of the common channel for broadcast
*
* @return       None.
*
* @note         Internal only.

*******************************************************************************/
void _XAie_MarkChannelBitmapAndRscId(XAie_DevInst *DevInst,
		u32 UserRscNum, XAie_UserRsc *Rscs, u32 ChannelIndex)
{
	u8 TileType;
	u32 *Bitmap;
	XAie_BitmapOffsets Offsets;

	for(u32 i = 0; i < UserRscNum; i++) {
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Rscs[i].Loc);
		Bitmap = DevInst->RscMapping[TileType].
				Bitmaps[XAIE_BCAST_CHANNEL_RSC];
		_XAie_RscMgr_GetBitmapOffsets(DevInst, XAIE_BCAST_CHANNEL_RSC,
				Rscs[i].Loc, Rscs[i].Mod, &Offsets);

		/* Mark allocation for common channel id in BC channel bitmap */
		_XAie_SetBitInBitmap(Bitmap, ChannelIndex + Offsets.StartBit,
				1U);
		/* Return resource granted to caller by populating RscId */
		Rscs[i].RscId = ChannelIndex;
	}
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
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		PerfMod = _XAie_GetPerfMod(DevInst, TileType, Mod);
		return PerfMod->MaxCounterVal;
	}
	case XAIE_USER_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumUserEvents;
	}
	case XAIE_PC_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumStrmPortSelectIds;
	}
	case XAIE_GROUP_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		EventMod = _XAie_GetEventMod(DevInst, TileType, Mod);
		return EventMod->NumGroupEvents;
	}
	case XAIE_COMBO_EVENTS_RSC:
	{
		return XAIE_COMBO_EVENTS_PER_MOD;
	}
	case XAIE_BCAST_CHANNEL_RSC:
		return XAIE_NUM_BROADCAST_CHANNELS;
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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
		XAie_BackendTilesRsc TilesRsc = {};
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, RscReq[i].Loc);
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
			XAIE_WARN("Unable to request resources. RscType: %d\n",
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

AieRC _XAie_RscMgr_RequestRscContiguous(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, XAie_UserRsc *Rscs,
		XAie_RscType RscType)
{
	AieRC RC;
	u32 UserRscIndex = 0U;

	for(u32 i = 0U; i < NumReq; i++) {
		XAie_BackendTilesRsc TilesRsc = {};
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, RscReq[i].Loc);
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
		TilesRsc.Flags = XAIE_RSC_MGR_CONTIG_FLAG;
		TilesRsc.NumContigRscs = RscReq[i].NumRscPerTile;
		TilesRsc.Rscs = &Rscs[UserRscIndex];

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			_XAie_RscMgr_FreeRscs(DevInst, UserRscIndex, Rscs,
					RscType);
			XAIE_WARN("Unable to request resources. RscType: %d\n",
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
		XAie_BackendTilesRsc TilesRsc = {};
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Rscs[i].Loc);
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
		XAie_BackendTilesRsc TilesRsc = {};
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Rscs[i].Loc);
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
		XAie_BackendTilesRsc TilesRsc = {};
		XAie_BitmapOffsets Offsets;
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Rscs[i].Loc);
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
			XAIE_WARN("Unable to request resources. RscType: %d\n",
					RscType);
			return XAIE_INVALID_ARGS;
		}

		UserRscIndex++;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API returns the 64 bit header for a given TileType, module type, resource
* type and Size of the bitmap in number of 64 bit words.
*
*
* @param	TileType: Tile type
* @param	ModType: Module type
* @param	RscType: Resource Type
* @param	Size: Number of 32 bit words.
*
* @return	64 bit resource header.
*
* @note		Internal only. The size of the bitmap is adjusted if the 32bit
*		size value is odd and divided by two convert it to number of
*		64 bit words.
*
*******************************************************************************/
static inline u64 _XAie_CreateRscHeader(u8 TileType, XAie_ModuleType ModType,
		u8 RscType, u32 Size)
{
	Size = (Size % 2U) ? Size + 1 : Size;
	Size /= 2U;

	return ((TileType & XAIE_RSC_HEADER_TILE_TYPE_MASK) <<
			XAIE_RSC_HEADER_TILE_TYPE_SHIFT) |
		((ModType & XAIE_RSC_HEADER_MOD_TYPE_MASK) <<
		 XAIE_RSC_HEADER_MOD_TYPE_SHIFT) |
		((RscType & XAIE_RSC_HEADER_RSC_TYPE_MASK) <<
		 XAIE_RSC_HEADER_RSC_TYPE_SHIFT) |
		((Size & XAIE_RSC_HEADER_SIZE_MASK) <<
		 XAIE_RSC_HEADER_SIZE_SHIFT);
}

/*****************************************************************************/
/**
* This API returns the number of resources for a give tile type, module type and
* resource type.
*
* @param	DevInst: Device Instance
* @param	TileType: Type of tile
* @param	Mod: Module - MEM, CORE or PL.
* @param	RscType: Resource type
*
* @return	Max number of resources.
*
* @note		Internal only.
*
*******************************************************************************/
static u32 _XAie_GetNumRscs(XAie_DevInst *DevInst, u8 TileType,
		XAie_ModuleType Mod, XAie_RscType RscType)
{
	switch(RscType) {
	case XAIE_PERFCNT_RSC:
	{
		const XAie_PerfMod *PerfMod;

		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Mod];
		return PerfMod->MaxCounterVal;
	}
	case XAIE_USER_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;

		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];
		return EventMod->NumUserEvents;
	}
	case XAIE_PC_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;

		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];
		return EventMod->NumPCEvents;
	}
	case XAIE_TRACE_CTRL_RSC:
	{
		return XAIE_TRACE_CTRL_RSCS_PER_MOD;
	}
	case XAIE_SS_EVENT_PORTS_RSC:
	{
		const XAie_EvntMod *EventMod;

		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];
		return EventMod->NumStrmPortSelectIds;
	}
	case XAIE_GROUP_EVENTS_RSC:
	{
		const XAie_EvntMod *EventMod;

		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Mod];
		return EventMod->NumGroupEvents;
	}
	case XAIE_COMBO_EVENTS_RSC:
	{
		return XAIE_COMBO_EVENTS_PER_MOD;
	}
	case XAIE_BCAST_CHANNEL_RSC:
	{
		return XAIE_NUM_BROADCAST_CHANNELS;
	}
	default:
		return 0U;
	}
}

/*****************************************************************************/
/**
* This API returns module type for a given tile type and module index from the
* tile property.
*
* @param	TileType: Type of tile
* @param	ModIndex: Index of the module in TileMod
*
* @return	Module type.
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_ModuleType _XAie_GetModTypefromModIndex(u8 TileType, u8 ModIndex)
{
	switch(TileType) {
		case XAIEGBL_TILE_TYPE_AIETILE:
			{
				if(ModIndex == 0U)
					return XAIE_MEM_MOD;
				else if(ModIndex == 1U)
					return XAIE_CORE_MOD;

				break;
			}
		case XAIEGBL_TILE_TYPE_SHIMNOC:
		case XAIEGBL_TILE_TYPE_SHIMPL:
			{
				if(ModIndex == 0U)
					return XAIE_PL_MOD;

				break;
			}
		case XAIEGBL_TILE_TYPE_MEMTILE:
			{
				if(ModIndex == 0U)
					return XAIE_MEM_MOD;

				break;
			}
		default:
			XAIE_ERROR("Invalid tile type\n");
			break;
		}

	return XAIE_PL_MOD + 1U;
}

/*****************************************************************************/
/**
* This API is used to dump the total allocated resources to a file.
*
* @param	DevInst: Device Instance
* @param	File: Path of the file which will contain the information of
*		      all the allocated resources.
*
* @return	XAIE_OK on success and error code on failure.
*
*******************************************************************************/
AieRC XAie_SaveAllocatedRscsToFile(XAie_DevInst *DevInst, const char *File)
{
	FILE *F;
	size_t Ret;
	u64 NumRscsInFile = 0U;
	const u64 FirstRscsOffset = 0x10;

	if((DevInst == XAIE_NULL) || (File == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid pointer\n");
		return XAIE_INVALID_ARGS;
	}

	F = fopen(File, "w");
	if (F == NULL) {
		XAIE_ERROR("Not able to open file to dump resources.\n");
		return XAIE_INVALID_ARGS;
	}

	/* Write NumRscs and first offset value to file */
	Ret = fwrite(&NumRscsInFile, sizeof(NumRscsInFile), 1U, F);
	if(Ret != 1U) {
		fclose(F);
		XAIE_ERROR("Failed to write resource bitmaps to file\n");
		return XAIE_ERR;
	}
	Ret = fwrite(&FirstRscsOffset, sizeof(FirstRscsOffset), 1U, F);
	if(Ret != 1U) {
		fclose(F);
		XAIE_ERROR("Failed to write resource bitmaps to file\n");
		return XAIE_ERR;
	}
	for(u8 i = 0U; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		u32 NumRows;
		XAie_ResourceManager *RscMap;

		if (i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;

		NumRows = _XAie_GetNumRows(DevInst, i);
		RscMap = &DevInst->RscMapping[i];
		for(u8 j = 0U; j < XAIE_MAX_RSC; j++) {
			u32 *Bitmap;
			u8 NumMods;

			Bitmap = RscMap->Bitmaps[j];
			NumMods = DevInst->DevProp.DevMod[i].NumModules;
			for(u8 k = 0U; k < NumMods; k++) {
				u32 NumRscs;
				u32 BitmapSize;
				u64 RscHeader;
				XAie_ModuleType Mod;
				u32 BitmapOffset = 0U;

				NumRscs = _XAie_GetNumRscs(DevInst, i, k, j);
				/* Save only runtime bitmap */
				BitmapSize = _XAie_NearestRoundUp(NumRows *
						NumRscs * DevInst->NumCols,
						8 * sizeof(u32));
				BitmapSize /= 8U * sizeof(u32);
				if(BitmapSize == 0U)
					continue;

				Mod = _XAie_GetModTypefromModIndex(i, k);
				RscHeader = _XAie_CreateRscHeader(i, Mod, j,
						BitmapSize);
				XAIE_DBG("RSC HEADER: 0x%lx\n", RscHeader);
				Ret = fwrite(&RscHeader, sizeof(RscHeader), 1U,
					       F);
				if(Ret != 1U) {
					fclose(F);
					XAIE_ERROR("Failed to write resource bitmaps to file\n");
					return XAIE_ERR;
				}

				if(Mod == XAIE_CORE_MOD) {
					u32 MemModRscs;

					MemModRscs = _XAie_GetNumRscs(DevInst,
							i, XAIE_MEM_MOD, j);
					BitmapOffset =
						_XAie_GetCoreBitmapOffset(
								DevInst,
								MemModRscs);
					BitmapOffset = (BitmapOffset/8) /
						sizeof(u32);
				}

				Bitmap += BitmapOffset;
				for(u32 Word = 0U; Word < BitmapSize; Word += 2) {
					u64 Payload;

					/*
					 * if bitmap size is odd number of 32
					 * bit values, zero pad additional 32
					 * bits to for a 64 bit payload.
					 */
					if(Word != BitmapSize - 1U)
						Payload = Bitmap[Word] |
							((u64)Bitmap[Word + 1] << 32);
					else
						Payload = Bitmap[Word];

					Ret = fwrite(&Payload, sizeof(Payload),
								1U, F);
					if(Ret != 1U) {
						free(DevInst->RscMapping);
						XAIE_ERROR("Memory allocation for resource manager bitmaps failed\n");
						return XAIE_ERR;
					}
				}

				NumRscsInFile++;
			}
		}
	}

	/* Update Number of Rscs entries */
	rewind(F);
	Ret = fwrite(&NumRscsInFile, sizeof(NumRscsInFile), 1U, F);
	if(Ret != 1U) {
		fclose(F);
		XAIE_ERROR("Failed to write resource bitmaps to file\n");
		return XAIE_ERR;
	}

	fclose(F);
	return XAIE_OK;

}

/*****************************************************************************/
/**
* This API is used to apply resource meta data to resource static bitmaps
*
* @param	DevInst: Device Instance
* @param	MetaData: pointer to the AI engine resource meta data memory
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		This function should be called before calling any resource
*		requesting functions.
*
*******************************************************************************/
AieRC XAie_LoadStaticRscfromMem(XAie_DevInst *DevInst, const char *MetaData)
{
	const XAieRscMetaHeader *Header = (XAieRscMetaHeader *)MetaData;
	const XAieRscBitmap *Bitmap;
	u64 NumBitmaps, BitmapsOffset;

	if((DevInst == XAIE_NULL) || (Header == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments for loading static resources\n");
		return XAIE_INVALID_ARGS;
	}

	/*
	 * For now, the stat field of the header only contains the number of
	 * bitmaps.
	 */
	NumBitmaps = Header->Stat;
	BitmapsOffset = Header->BitmapOff;
	if(!NumBitmaps || BitmapsOffset < sizeof(*Header)) {
		XAIE_ERROR("failed to get static resources, invalid header.\n");
		return XAIE_INVALID_ARGS;
	}

	Bitmap = (const XAieRscBitmap *)(MetaData + BitmapsOffset);
	for(u32 i = 0; i < NumBitmaps; i++) {
		u64 RscHeader = Bitmap->Header;
		u32 RscLen, ModType, RscType, ModId;
		u32 NumRows, NumRscs;
		u32 BitmapSize, Bitmap64Size;
		u32 BitmapOffset = 0U;
		u8 TileType;
		const u64 *Bits64;
		u32 *Bits32;
		XAie_ResourceManager *RscMap;


		TileType = (RscHeader >> XAIE_RSC_HEADER_TILE_TYPE_SHIFT) &
			XAIE_RSC_HEADER_TILE_TYPE_MASK;
		ModType = (RscHeader >> XAIE_RSC_HEADER_MOD_TYPE_SHIFT) &
			XAIE_RSC_HEADER_MOD_TYPE_MASK;
		RscType = (RscHeader >> XAIE_RSC_HEADER_RSC_TYPE_SHIFT) &
			XAIE_RSC_HEADER_RSC_TYPE_MASK;
		RscLen =  (RscHeader >> XAIE_RSC_HEADER_SIZE_SHIFT) &
			XAIE_RSC_HEADER_SIZE_MASK;

		if(!RscLen) {
			XAIE_ERROR("invalid static bitmap[%u], length is 0.\n",
				i);
			return XAIE_INVALID_ARGS;
		}
		if(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			XAIE_ERROR("invalid tile type SHIMNOC in rsc"
				"metadata.\n");
			return XAIE_INVALID_ARGS;
		}

		RscMap = &DevInst->RscMapping[TileType];
		Bits32 = RscMap->Bitmaps[RscType];
		/*
		 * If Module type is PL, it the module field of _XAie_GetNumRscs
		 * needs to be 0 as SHIM only have single module for resources.
		 */
		ModId = ModType;
		if(ModType == XAIE_PL_MOD) {
			ModId = 0;
		}
		NumRscs = _XAie_GetNumRscs(DevInst, TileType, ModId, RscType);
		NumRows = _XAie_GetNumRows(DevInst, TileType);
		BitmapSize = NumRows * NumRscs * DevInst->NumCols;
		Bitmap64Size = _XAie_NearestRoundUp(BitmapSize, 8 * sizeof(u64))
			/ (8U * sizeof(u64));
		if((u64)(Bitmap64Size) != RscLen) {
			XAIE_ERROR("Rsc %u of Tile type %u, Mod %u, "
				"Invalid bitmap size, expect %u, actual %lu.\n",
				RscType, TileType, ModType,
				Bitmap64Size, RscLen);
			return XAIE_INVALID_ARGS;
		}

		if(ModType == (u32)XAIE_CORE_MOD) {
			u32 MemModRscs;

			MemModRscs = _XAie_GetNumRscs(DevInst, TileType,
				XAIE_MEM_MOD, RscType);
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
								MemModRscs);
		}

		/* Locate static bitmap */
		BitmapOffset += BitmapSize;
		Bits64 = Bitmap->Bitmap;
		/*
		 * Cannot just copy, as the static bitmap may not start as
		 * 32bit aligned.
		 */
		for(u32 j = 0; j < RscLen; j++) {
			u64 lBits64Val = Bits64[j];

			for(u32 k = 0; k < 8 * sizeof(u64); k++) {
				u32 Pos = k + j * 8 * sizeof(u64);

				if (Pos >= BitmapSize) {
					break;
				}
				if (lBits64Val & 1LU) {
					_XAie_SetBitInBitmap(Bits32,
						Pos + BitmapOffset, 1);
				}
				lBits64Val >>= 1;
			}
		}

		Bitmap = (const XAieRscBitmap *)((const char *)Bitmap +
				sizeof(RscHeader) + RscLen * sizeof(u64));
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This helper API is used to get resource statistics information.
*
* @param	DevInst: Device Instance
* @param	NumRscStat: Number of resource statistics requests
* @param	RscStats: Resuorce statistics requests. Each element contains
*		the resource type, module type and tile type.
* @param	RscStatType: resource statistics type it supports:
*			* XAIE_BACKEND_RSC_STAT_STATIC
*			* XAIE_BACKEND_RSC_STAT_USED
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		This function is internal to this file only.
*
*******************************************************************************/
static AieRC _XAie_GetRscsStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats,
		XAie_BackendRscStatType RscStatType)
{
	XAie_BackendRscStat BRscStats = {0};

	if((DevInst == XAIE_NULL) || (NumRscStat == 0) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY) ||
		(RscStats == XAIE_NULL)) {
		XAIE_ERROR("Invalid arguments for requesting resource statistics\n");
		return XAIE_INVALID_ARGS;
	}

	for (u32 i = 0; i < NumRscStat; i++) {
		if (_XAie_CheckModule(DevInst, RscStats[i].Loc,
			(XAie_ModuleType)(RscStats[i].Mod)) != XAIE_OK) {
			XAIE_ERROR("Invalid tile(%u, %u) for requesting resource statistics\n",
				RscStats[i].Loc.Col, RscStats[i].Loc.Row);
			return XAIE_INVALID_ARGS;
		}
	}

	BRscStats.NumRscStats = NumRscStat;
	BRscStats.RscStatType = RscStatType;
	BRscStats.RscStats = RscStats;
	return XAie_RunOp(DevInst, XAIE_BACKEND_OP_GET_RSC_STAT,
			(void *)&BRscStats);
}

/*****************************************************************************/
/**
* This API is used to request the number of statically allocated resources of a
* resource type of a module of a tile.
*
* @param	DevInst: Device Instance
* @param	NumRscStat: Number of resource statistics requests
* @param	RscStats: Resuorce statistics requests
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		None
*
*******************************************************************************/
AieRC XAie_GetStaticRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats)
{
	return _XAie_GetRscsStat(DevInst, NumRscStat, RscStats,
			XAIE_BACKEND_RSC_STAT_STATIC);
}

/*****************************************************************************/
/**
* This API is used to request the number of available resources of a resource
* type of a module of a tile.
*
* @param	DevInst: Device Instance
* @param	NumRscStat: Number of resource statistics requests
* @param	RscStats: Resuorce statistics requests
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		None
*
*******************************************************************************/
AieRC XAie_GetAvailRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats)
{
	return _XAie_GetRscsStat(DevInst, NumRscStat, RscStats,
			XAIE_BACKEND_RSC_STAT_AVAIL);
}

#endif /* XAIE_FEATURE_RSC_ENABLE */

/** @} */

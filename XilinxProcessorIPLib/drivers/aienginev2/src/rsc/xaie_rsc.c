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
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Rounds the BitmapSize up to the nearest multiple of n.
*
* @param        Val: value to be aligned
* @param        Aligned: multiple to align to
*
* @return       Aligned result
*
* @note         Internal API only.
*
******************************************************************************/
static inline u32 _XAie_NearestRoundUp(u32 Val, u32 Aligned)
{
	return ((Val + Aligned - 1) & Aligned);
}

/*****************************************************************************/
/**
* This API initializes performance counter resource bitmap for all tiletype.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_PerfCntRscInit(XAie_DevInst *DevInst)
{
	u8 NumModules;
	u32 BitmapNumRows;
	const XAie_PerfMod *PerfMod;
	XAie_ResourceManager *RscMapping;

	for(u32 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {

		u32 NumCounters = 0;
		/* Memory already allocated for XAIEGBL_TILE_TYPE_SHIMPL */
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;
		NumModules = DevInst->DevProp.DevMod[i].NumModules;
		for(u32 j = 0; j < NumModules; j++) {
			PerfMod = &DevInst->DevProp.DevMod[i].PerfMod[j];
			NumCounters += PerfMod->MaxCounterVal;
		}

		RscMapping = &DevInst->DevProp.DevMod[i].RscMapping;
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
		RscMapping->PerfCntBitmap = (u32*)calloc(_XAie_NearestRoundUp
			((NumCounters * BitmapNumRows * DevInst->NumCols * 2U),
			(8U * sizeof(u32))) / (8U * sizeof(u32)), sizeof(u32));
		if(RscMapping->PerfCntBitmap == NULL) {
			/* Clear previous bitmap alocation */
			for(u32 k = 0; k < i; k++) {
				if(k == XAIEGBL_TILE_TYPE_SHIMNOC)
					continue;
				RscMapping = &DevInst->DevProp.DevMod[k].
					RscMapping;
				free(RscMapping->PerfCntBitmap);
			}
			XAIE_ERROR("Unable to allocate memory for PerfCnt bitmap\n");

			return XAIE_ERR;
		}
	}

	DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].RscMapping =
		DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMPL].RscMapping;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API deallocates performance counter resource bitmap for all tiletype.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_PerfCntRscFinish(XAie_DevInst *DevInst)
{
	XAie_ResourceManager *RscMapping;

	for(u32 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;
		RscMapping = &DevInst->DevProp.DevMod[i].RscMapping;
		free(RscMapping->PerfCntBitmap);
	}

	return XAIE_OK;
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
	return _XAie_PerfCntRscFinish(DevInst);
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
	return _XAie_PerfCntRscInit(DevInst);
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
static AieRC _XAie_CheckLocsValidity(XAie_DevInst *DevInst, u32 NumReq,
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
* This API calculates core module offset in bitmap.
*
* @param	DevInst: Device Instance
* @param	MaxRscVal: Number of resource per tile.
*
* @return	Index for start of core module bitmap.
*
* @note		Internal only.
*
*******************************************************************************/
static inline u32 _XAie_GetCoreBitmapOffset(XAie_DevInst *DevInst,
		u32 MaxRscVal)
{
	return MaxRscVal * DevInst->AieTileNumRows * DevInst->NumCols * 2U;
}

/*****************************************************************************/
/**
* This API calculates resource index in bitmap based on location and number of
* resource per tile.
*
* @param	BitmapNumRows: Number of rows based on tiletype
* @param	Loc: Location of AIE tile
* @param	MaxRscVal: Number of resource per tile.
* @param	StartRow: Start row for tiletype
*
* @return	Index for the resource start bit in bitmap
*
* @note		Internal only.
*
*******************************************************************************/
static inline u32 _XAie_GetTileRscStartBitPosFromLoc(u32 BitmapNumRows,
		XAie_LocType Loc, u32 MaxRscVal, u32 StartRow)
{
	return (Loc.Col * BitmapNumRows + (Loc.Row - StartRow) * MaxRscVal);
}

/*****************************************************************************/
/**
* This API finds free resource after checking static and runtime allocated
* resource status in bitmap.
*
* @param	Bitmap: Bitmap of the resource
* @param	StaticBitmapOffset: Offset for static bitmap
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	MaxRscVal: Number of resource per tile
* @param	Index: Pointer to store free resource found
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_FindAvailableRsc(u32 *Bitmap, u32 StaticBitmapOffset,
		u32 StartBit, u32 MaxRscVal, u32 *Index)
{
	for(u32 i = StartBit; i < StartBit + MaxRscVal; i++) {
		if(!((CheckBit(Bitmap, i)) |
				CheckBit(Bitmap, (i + StaticBitmapOffset)))) {
			*Index = i - StartBit;
			return XAIE_OK;
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API grants resource based on availibility for the given location and
* marks that rsc as in use in the relevant bitmap.
*
* @param	Bitmap: Bitmap of the resource
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	StaticBitmapOffset: Offset for static bitmap
* @param	NumRscPerTile: Number of resource requested per tile
* @param	MaxRscVal: Maximum number of resource per tile
* @param	RscArrPerTile: Pointer to store available resource
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_RequestRsc(u32 *Bitmap, u32 StartBit,
		u32 StaticBitmapOffset, u32 NumRscPerTile, u32 MaxRscVal,
		u32 *RscArrPerTile)
{
	AieRC RC;

	/* Check for the requested resource in the bitmap locally */
	for(u32 i = 0; i < NumRscPerTile; i++) {
		u32 Index;
		RC = _XAie_FindAvailableRsc(Bitmap, StaticBitmapOffset,
				StartBit, MaxRscVal, &Index);
		if(RC != XAIE_OK) {
			/* Clear bitmap if any resource request failed */
			for(u32 j = 0; j < i; j++)
				_XAie_ClrBitInBitmap(Bitmap, RscArrPerTile[j]
				+ StartBit, 1U);
			XAIE_ERROR("Unable to find free resource\n");

			return XAIE_ERR;
		}

		/* Set the bit as allocated if the request was successful*/
		_XAie_SetBitInBitmap(Bitmap, Index + StartBit, 1U);
		RscArrPerTile[i] = Index;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request performance counter in pool. The API grants
* performance counter based on availibility and marks that resource status in
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
AieRC XAie_RequestPerfcnt(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {

	AieRC RC;
	u8 TileType;
	u32 UserRscIndex = 0, TotalReq = 0;
	const XAie_PerfMod *PerfMod;
	XAie_ResourceManager *RscMapping;

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
		u32 RscArrPerTile[RscReq[i].NumRscPerTile];
		u32 StaticBitmapOffset, BitmapOffset = 0, BitmapNumRows;
		u32 StartBit, StartRow;

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}
		PerfMod = _XAie_GetPerfMod(DevInst, TileType, RscReq[i].Mod);
		RscMapping = &DevInst->DevProp.DevMod[TileType].RscMapping;
		StartRow = _XAie_GetStartRow(DevInst, TileType);
		BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);

		StaticBitmapOffset = PerfMod->MaxCounterVal * BitmapNumRows *
				DevInst->NumCols;
		StartBit = _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows,
			RscReq[i].Loc, PerfMod->MaxCounterVal, StartRow) +
			BitmapOffset;

		/* For each tile request no. of resource as per user request */
		RC = _XAie_RequestRsc(RscMapping->PerfCntBitmap,
			StartBit, StaticBitmapOffset, RscReq[i].NumRscPerTile,
			PerfMod->MaxCounterVal, RscArrPerTile);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreePerfcnt(DevInst, UserRscIndex, Rscs);
			XAIE_ERROR("Unable to request performance counters\n");
			return XAIE_INVALID_ARGS;
		}

		/* Return resource granted to caller by populating UserRsc */
		for(u32 j = 0; j < RscReq[i].NumRscPerTile; j++) {
			Rscs[UserRscIndex].RscId = RscArrPerTile[j];
			Rscs[UserRscIndex].Loc = RscReq[i].Loc;
			Rscs[UserRscIndex].Mod = RscReq[i].Mod;
			Rscs[UserRscIndex].RscType = XAIE_PERFCNT_RSC;
			UserRscIndex++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to release particular performance counter.
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
AieRC XAie_ReleasePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {

	AieRC RC;
	u8 TileType;
	u32 BitmapOffset = 0, BitmapNumRows;
	u32 StaticBitmapOffset, StartBit, StartRow;
	const XAie_PerfMod *PerfMod;
	XAie_ResourceManager *RscMapping;

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

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, Rscs[i].Loc, Rscs[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		PerfMod = _XAie_GetPerfMod(DevInst, TileType, Rscs[i].Mod);
		RscMapping = &DevInst->DevProp.DevMod[TileType].RscMapping;
		StartRow = _XAie_GetStartRow(DevInst, TileType);
		BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		StaticBitmapOffset = PerfMod->MaxCounterVal * BitmapNumRows *
				DevInst->NumCols;
		StartBit = _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows,
			Rscs[i].Loc, PerfMod->MaxCounterVal, StartRow) +
			BitmapOffset;

		/* Clear resource from run-time bitmap */
		_XAie_ClrBitInBitmap(RscMapping->PerfCntBitmap, Rscs[i].RscId +
			StartBit, 1U);
		/* Clear resource from static bitmap */
		_XAie_ClrBitInBitmap(RscMapping->PerfCntBitmap, Rscs[i].RscId +
			StartBit + StaticBitmapOffset, 1U);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated performance
* counter.
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
AieRC XAie_FreePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {

	AieRC RC;
	u8 TileType;
	u32 BitmapOffset = 0, BitmapNumRows, StartBit, StartRow;
	const XAie_PerfMod *PerfMod;
	XAie_ResourceManager *RscMapping;

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

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, Rscs[i].Loc, Rscs[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		PerfMod = _XAie_GetPerfMod(DevInst, TileType, Rscs[i].Mod);
		RscMapping = &DevInst->DevProp.DevMod[TileType].RscMapping;
		StartRow = _XAie_GetStartRow(DevInst, TileType);
		BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		StartBit = _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows,
			Rscs[i].Loc, PerfMod->MaxCounterVal, StartRow) +
			BitmapOffset;

		/* Clear resource from run-time bitmap */
		_XAie_ClrBitInBitmap(RscMapping->PerfCntBitmap, Rscs[i].RscId +
			StartBit, 1U);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request particular performance counter that was
* allocated statically.
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
*
*******************************************************************************/
AieRC XAie_RequestAllocatedPerfcnt(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq) {

	AieRC RC;
	u8 TileType;
	u32 UserRscIndex = 0;
	const XAie_PerfMod *PerfMod;
	XAie_ResourceManager *RscMapping;

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
		u32 StaticBitmapOffset, BitmapOffset = 0, BitmapNumRows;
		u32 RscBit, StartRow;

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		PerfMod = _XAie_GetPerfMod(DevInst, TileType, RscReq[i].Mod);
		RscMapping = &DevInst->DevProp.DevMod[TileType].RscMapping;
		StartRow = _XAie_GetStartRow(DevInst, TileType);
		BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);

		StaticBitmapOffset = PerfMod->MaxCounterVal * BitmapNumRows *
				DevInst->NumCols;
		RscBit = _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows,
			RscReq[i].Loc, PerfMod->MaxCounterVal, StartRow) +
			BitmapOffset + RscReq[i].RscId;
		/* Check if rsc is not allocated statically */
		if(!(CheckBit(RscMapping->PerfCntBitmap,
				(RscBit + StaticBitmapOffset)))) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreePerfcnt(DevInst, UserRscIndex, RscReq);
			XAIE_ERROR("Resource not allocated statically\n");
			return XAIE_INVALID_ARGS;
		}
		/* Mark the resource granted in the runtime bitmap */
		_XAie_SetBitInBitmap(RscMapping->PerfCntBitmap, RscBit, 1U);
		UserRscIndex++;
	}

	return XAIE_OK;
}

/** @} */

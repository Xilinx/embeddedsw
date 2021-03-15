/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc_perfcnt.c
* @{
*
* This file contains routines for performance counter resource management
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

#include "xaie_io.h"
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
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
AieRC _XAie_PerfCntRscInit(XAie_DevInst *DevInst)
{
	u8 NumModules;
	u32 BitmapNumRows;
	const XAie_PerfMod *PerfMod;

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
		DevInst->RscMapping[i].PerfCntBitmap = (u32 *)calloc
			(_XAie_NearestRoundUp((NumCounters * BitmapNumRows *
			DevInst->NumCols * 2U),(8U * sizeof(u32))) /
			(8U * sizeof(u32)), sizeof(u32));
		if(DevInst->RscMapping[i].PerfCntBitmap == NULL) {
			/* Clear previous bitmap alocation */
			for(u32 k = 0; k < i; k++) {
				if(k == XAIEGBL_TILE_TYPE_SHIMNOC)
					continue;
				free(DevInst->RscMapping[k].PerfCntBitmap);
			}
			XAIE_ERROR("Unable to allocate memory for PerfCnt bitmap\n");

			return XAIE_ERR;
		}
	}

	DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMNOC].PerfCntBitmap =
		DevInst->RscMapping[XAIEGBL_TILE_TYPE_SHIMPL].PerfCntBitmap;

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
AieRC _XAie_PerfCntRscFinish(XAie_DevInst *DevInst)
{
	for(u32 i = 0; i < XAIEGBL_TILE_TYPE_MAX; i++) {
		if(i == XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;
		free(DevInst->RscMapping[i].PerfCntBitmap);
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
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;
	u8 TileType;
	u32 UserRscIndex = 0, TotalReq = 0;
	const XAie_PerfMod *PerfMod;
	XAie_BackendTilesRsc TilesRsc;

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

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_TILE;
		}
		PerfMod = _XAie_GetPerfMod(DevInst, TileType, RscReq[i].Mod);
		MaxRscVal = PerfMod->MaxCounterVal;
		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
			(&DevInst->DevProp.DevMod[TileType].
			PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		StartBit = _XAie_GetStartBit(DevInst, RscReq[i].Loc,
			MaxRscVal) + BitmapOffset;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].PerfCntBitmap;
		TilesRsc.RscType = XAIE_PERFCNT_RSC;
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
			RC = XAie_FreePerfcnt(DevInst, UserRscIndex, Rscs);
			XAIE_ERROR("Unable to request performance counters\n");
			return XAIE_INVALID_ARGS;
		}
		for(u32 j = 0; j < RscReq[i].NumRscPerTile; j++) {
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
	u32 BitmapOffset = 0;
	u32 StaticBitmapOffset, StartBit, MaxRscVal;
	const XAie_PerfMod *PerfMod;
	XAie_BackendTilesRsc TilesRsc;

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
		MaxRscVal = PerfMod->MaxCounterVal;
		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
			(&DevInst->DevProp.DevMod[TileType].
			PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		StartBit = _XAie_GetStartBit(DevInst, Rscs[i].Loc, MaxRscVal) +
			BitmapOffset;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].PerfCntBitmap;
		TilesRsc.RscType = XAIE_PERFCNT_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.StartBit = StartBit;
		TilesRsc.StaticBitmapOffset = StaticBitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = Rscs[i].RscId;

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_RELEASE_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Unable to release performance counter\n");
			return XAIE_INVALID_ARGS;
		}
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
	u32 BitmapOffset = 0, StartBit, MaxRscVal;
	const XAie_PerfMod *PerfMod;
	XAie_BackendTilesRsc TilesRsc;

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
		MaxRscVal = PerfMod->MaxCounterVal;
		if(Rscs[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		StartBit = _XAie_GetStartBit(DevInst, Rscs[i].Loc, MaxRscVal) +
			BitmapOffset;

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].PerfCntBitmap;
		TilesRsc.StartBit = StartBit;
		TilesRsc.RscType = XAIE_PERFCNT_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.Loc = Rscs[i].Loc;
		TilesRsc.Mod = Rscs[i].Mod;
		TilesRsc.RscId = Rscs[i].RscId;

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_FREE_RESOURCE,
				(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Unable to release performance counter\n");
			return XAIE_INVALID_ARGS;
		}
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
	XAie_BackendTilesRsc TilesRsc;

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

		TileType = _XAie_GetTileTypefromLoc(DevInst, RscReq[i].Loc);
		/* check for module and tiletype combination */
		RC = _XAie_CheckModule(DevInst, RscReq[i].Loc, RscReq[i].Mod);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Invalid Module\n");
			return XAIE_INVALID_ARGS;
		}

		PerfMod = _XAie_GetPerfMod(DevInst, TileType, RscReq[i].Mod);
		MaxRscVal = PerfMod->MaxCounterVal;
		if(RscReq[i].Mod == XAIE_CORE_MOD)
			BitmapOffset = _XAie_GetCoreBitmapOffset(DevInst,
				(&DevInst->DevProp.DevMod[TileType].
				PerfMod[XAIE_MEM_MOD])->MaxCounterVal);
		RscBit = _XAie_GetStartBit(DevInst, RscReq[i].Loc, MaxRscVal) +
			BitmapOffset + RscReq[i].RscId;
		StaticBitmapOffset = MaxRscVal * DevInst->NumCols *
			_XAie_GetNumRows(DevInst, TileType);

		TilesRsc.Bitmap = DevInst->RscMapping[TileType].PerfCntBitmap;
		TilesRsc.RscType = XAIE_PERFCNT_RSC;
		TilesRsc.MaxRscVal = MaxRscVal;
		TilesRsc.BitmapOffset = BitmapOffset;
		TilesRsc.Loc = RscReq[i].Loc;
		TilesRsc.Mod = RscReq[i].Mod;
		TilesRsc.StartBit = RscBit;
		TilesRsc.StaticBitmapOffset = StaticBitmapOffset;
		TilesRsc.RscId = RscReq[i].RscId;
		TilesRsc.Rscs = &RscReq[UserRscIndex];

		RC = XAie_RunOp(DevInst,
			XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE,
			(void *)&TilesRsc);
		if(RC != XAIE_OK) {
			/* Clear resource marking for all previous requests */
			RC = XAie_FreePerfcnt(DevInst, UserRscIndex, RscReq);
			XAIE_ERROR("Resource not allocated statically\n");
			return XAIE_INVALID_ARGS;
		}

		UserRscIndex++;
	}

	return XAIE_OK;
}

/** @} */

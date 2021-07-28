/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks.c
* @{
*
* This file contains routines for AIE locks.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* 1.1   Tejus   04/13/2020  Remove use of range in apis
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_locks.h"
#include "xaiegbl_defs.h"

#ifdef XAIE_FEATURE_LOCK_ENABLE
/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to acquire the specified lock with or without value. Lock
* acquired without value if LockVal is (XAIE_LOCK_WITH_NO_VALUE). Lock without
* value can be acquired for AIE only. This API can be blocking or non-blocking
* based on the TimeOut value. If the TimeOut is 0us, the API behaves in a
* non-blocking fashion and returns immediately after the first lock acquire
* request. If TimeOut > 0, then the API is a blocking call and will issue lock
* acquire request until the acquire is successful or it times out, whichever
* occurs first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the acquire request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Acquired, else XAIE_LOCK_RESULT_FAILED.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_LockAcquire(XAie_DevInst *DevInst, XAie_LocType Loc, XAie_Lock Lock,
		u32 TimeOut)
{
	u8  TileType;
	const XAie_LockMod *LockMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;

	if(Lock.LockId > LockMod->NumLocks) {
		XAIE_ERROR("Invalid Lock Id\n");
		return XAIE_INVALID_LOCK_ID;
	}

	if((Lock.LockVal > LockMod->LockValUpperBound) ||
			(Lock.LockVal < LockMod->LockValLowerBound)) {
		XAIE_ERROR("Lock value out of range\n");
		return XAIE_INVALID_LOCK_VALUE;
	}

	return LockMod->Acquire(DevInst, LockMod, Loc, Lock, TimeOut);
}

/*****************************************************************************/
/**
*
* This API is used to release the specified lock with or without value. Lock is
* released without value if LockVal is (XAIE_LOCK_WITH_NO_VALUE). Lock without
* value can be released for AIE only. This API can be blocking or non-blocking
* based on the TimeOut value. If the TimeOut is 0us, the API behaves in a
* non-blocking fashion and returns immediately after the first lock release
* request. If TimeOut > 0, then the API is a blocking call and will issue lock
* release request until the release is successful or it times out, whichever
* occurs first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the release request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Release, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	None.
*
******************************************************************************/
AieRC XAie_LockRelease(XAie_DevInst *DevInst, XAie_LocType Loc, XAie_Lock Lock,
		u32 TimeOut)
{
	u8  TileType;
	const XAie_LockMod *LockMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;

	if(Lock.LockId > LockMod->NumLocks) {
		XAIE_ERROR("Invalid Lock Id\n");
		return XAIE_INVALID_LOCK_ID;
	}

	if((Lock.LockVal > LockMod->LockValUpperBound) ||
			(Lock.LockVal < LockMod->LockValLowerBound)) {
		XAIE_ERROR("Lock value out of range\n");
		return XAIE_INVALID_LOCK_VALUE;
	}

	return LockMod->Release(DevInst, LockMod, Loc, Lock, TimeOut);
}

/*****************************************************************************/
/**
*
* This API is used to initalize a lock with given value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
*
* @return	XAIE_OK if Lock Release, else error code.
*
* @note 	None.
*
******************************************************************************/
AieRC XAie_LockSetValue(XAie_DevInst *DevInst, XAie_LocType Loc, XAie_Lock Lock)
{
	u8  TileType;
	const XAie_LockMod *LockMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;

	if(Lock.LockId > LockMod->NumLocks) {
		XAIE_ERROR("Invalid Lock Id\n");
		return XAIE_INVALID_LOCK_ID;
	}

	return LockMod->SetValue(DevInst, LockMod, Loc, Lock);
}

#endif /* XAIE_FEATURE_LOCK_ENABLE */
/** @} */

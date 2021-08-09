/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks_aieml.c
* @{
*
* This file contains routines for AIEML locks.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* 1.1   Tejus   03/23/2020  fix check of return value from mask poll for acquire
*			    api
* 1.2   Tejus   06/10/2020  Switch to new io backend.
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
#define XAIEML_LOCK_VALUE_MASK		0x7F
#define XAIEML_LOCK_VALUE_SHIFT		0x2

#define XAIEML_LOCK_RESULT_SUCCESS	1U
#define XAIEML_LOCK_RESULT_LSB		0x0
#define XAIEML_LOCK_RESULT_MASK		0x1

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to release the specified lock with or without value. This
* API can be blocking or non-blocking based on the TimeOut value. If the
* TimeOut is 0us, the API behaves in a non-blocking fashion and returns
* immediately after the first lock release request. If TimeOut > 0, then the
* API is a blocking call and will issue lock release request until the release
* is successful or it times out, whichever occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the release request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Release, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIEML. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAieMl_LockRelease(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut)
{
	u64 RegAddr;
	u32 RegOff;

	RegOff = LockMod->BaseAddr + (Lock.LockId * LockMod->LockIdOff) +
		((Lock.LockVal & XAIEML_LOCK_VALUE_MASK) <<
		 XAIEML_LOCK_VALUE_SHIFT);

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if(XAie_MaskPoll(DevInst, RegAddr, XAIEML_LOCK_RESULT_MASK,
				(XAIEML_LOCK_RESULT_SUCCESS <<
				 XAIEML_LOCK_RESULT_LSB), TimeOut) != XAIE_OK) {

		return XAIE_LOCK_RESULT_FAILED;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to acquire the specified lock and value. This API can be
* blocking or non-blocking based on the TimeOut value. If the TimeOut is 0us,
* the API behaves in a non-blocking fashion and returns immediately after the
* first lock acquire request. If TimeOut > 0, then the API is a blocking call
* and will issue lock acquire request until the acquire is successful or it
* times out, whichever occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the acquire request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Acquired, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIEML. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAieMl_LockAcquire(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut)
{
	u64 RegAddr;
	u32 RegOff;

	RegOff = LockMod->BaseAddr + (Lock.LockId * LockMod->LockIdOff) +
		(LockMod->RelAcqOff) + ((Lock.LockVal &
					XAIEML_LOCK_VALUE_MASK) <<
				XAIEML_LOCK_VALUE_SHIFT);

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if(XAie_MaskPoll(DevInst, RegAddr, XAIEML_LOCK_RESULT_MASK,
				(XAIEML_LOCK_RESULT_SUCCESS <<
				 XAIEML_LOCK_RESULT_LSB), TimeOut) != XAIE_OK) {

		return XAIE_LOCK_RESULT_FAILED;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to initalize a lock with given value.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
*
* @return	XAIE_OK if Lock Release, else error code.
*
* @note 	Internal only.
*
******************************************************************************/
AieRC _XAieMl_LockSetValue(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock)
{
	u64 RegAddr;
	u32 RegVal;

	RegAddr = LockMod->LockSetValBase +
		LockMod->LockSetValOff * Lock.LockId +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RegVal = XAie_SetField(Lock.LockVal, LockMod->LockInit->Lsb,
			LockMod->LockInit->Mask);

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

#endif /* XAIE_FEATURE_LOCK_ENABLE */
/** @} */

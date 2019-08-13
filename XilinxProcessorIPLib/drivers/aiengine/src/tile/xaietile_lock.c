/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaietile_lock.c
* @{
*
* This file contains routine for lock acquire and release functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  04/19/2018  Updated code to fix CR#1000292
* 1.2  Naresh  07/11/2018  Updated copyright info
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Hyun    01/08/2019  Use the poll function
* 1.5  Nishad  03/20/2019  Fix usage of uninitialized variable in
* 			   XAieTile_LockAcquire and XAieTile_LockRelease
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiegbl_reginit.h"
#include "xaietile_lock.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_RegLocks TileLockRegs[];
extern XAieGbl_RegLocks ShimLockRegs[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to acquire the specified lock with or without value. Lock
* acquired without value if LockVal==0xFF. This API can be blocking or non-
* blocking based on the TimeOut value. If TimeOut==0, then API behaves in a
* non-blocking fashion and returns immediately after the first read request.
* If TimeOut>0, then API becomes blocking and will issue the read request until
* the acquire is successful or time out happens, whichever occurs first.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	LockId - Lock value index, ranging from 0-15.
* @param	LockVal - Lock value used for acquire. If set to 0xFF, lock
*		acquired with no value.
* @param	TimeOut - Time-out value for which the read request needs to
*		be repeated. Value to be specified in usecs.
*
* @return	1 if acquire successful, else 0.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_LockAcquire(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
								u32 TimeOut)
{
	u64 RegAddr;
	u8 AcqDone = XAIETILE_LOCK_ACQ_FAILED;
	u8 Lsb;
	u32 Mask, Value;
	XAieGbl_RegLocks *RegPtr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(LockId < XAIEGBL_TILE_LOCK_NUM_MAX);
	XAie_AssertNonvoid(LockVal == XAIETILE_LOCK_ACQ_VAL0 ||
				LockVal == XAIETILE_LOCK_ACQ_VAL1 ||
				LockVal == XAIETILE_LOCK_ACQ_VALINVALID);

	if(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE) {
                RegPtr = &TileLockRegs[LockId];
        } else {
                RegPtr = &ShimLockRegs[LockId];
        }

	if(LockVal == XAIETILE_LOCK_ACQ_VAL0) {
		/* Acquire lock with value 0 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqV0Off;
		Lsb = RegPtr->AcqV0.Lsb;
		Mask = RegPtr->AcqV0.Mask;

	} else if(LockVal == XAIETILE_LOCK_ACQ_VAL1) {
		/* Acquire lock with value 1 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqV1Off;
		Lsb = RegPtr->AcqV1.Lsb;
		Mask = RegPtr->AcqV1.Mask;
	} else {
		/* LockVal == XAIETILE_LOCK_ACQ_VALINVALID */
		/* Acquire lock with no value register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqNvOff;
		Lsb = RegPtr->AcqNv.Lsb;
		Mask = RegPtr->AcqNv.Mask;
	}

	Value = XAIETILE_LOCK_ACQ_SUCCESS << Lsb;

	if (XAieGbl_MaskPoll(RegAddr, Mask, Value, TimeOut) == XAIE_SUCCESS) {
		AcqDone = XAIETILE_LOCK_ACQ_SUCCESS;
	}

	return AcqDone;
}

/*****************************************************************************/
/**
*
* This API is used to release the specified lock with or without value. Lock
* released without value if LockVal==0xFF. This API can be blocking or non-
* blocking based on the TimeOut value. If TimeOut==0, then API behaves in a
* non-blocking fashion and returns immediately after the first read request.
* If TimeOut>0, then API becomes blocking and will issue the read request until
* the release is successful or time out happens, whichever occurs first.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	LockId - Lock value index, ranging from 0-15.
* @param	LockVal - Lock value used for release. If set to 0xFF, lock
*		released with no value.
* @param	TimeOut - Time-out value for which the read request needs to
*		be repeated. Value to be specified in usecs.
*
* @return	1 if release successful, else 0.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_LockRelease(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
								u32 TimeOut)
{
	u64 RegAddr;
	u8 RelDone = XAIETILE_LOCK_REL_FAILED;
	u8 Lsb;
	u32 Mask, Value;
        XAieGbl_RegLocks *RegPtr;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(LockId < XAIEGBL_TILE_LOCK_NUM_MAX);
	XAie_AssertNonvoid(LockVal == XAIETILE_LOCK_REL_VAL0 ||
				LockVal == XAIETILE_LOCK_REL_VAL1 ||
				LockVal == XAIETILE_LOCK_REL_VALINVALID);

       	if(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE) {
                RegPtr = &TileLockRegs[LockId];
        } else {
                RegPtr = &ShimLockRegs[LockId];
        }

	if(LockVal == XAIETILE_LOCK_REL_VAL0) {
		/* Release lock with value 0 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelV0Off;
		Lsb = RegPtr->RelV0.Lsb;
		Mask = RegPtr->RelV0.Mask;

	} else if(LockVal == XAIETILE_LOCK_REL_VAL1) {
		/* Release lock with value 1 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelV1Off;
		Lsb = RegPtr->RelV1.Lsb;
		Mask = RegPtr->RelV1.Mask;
	} else {
		/* LockVal == XAIETILE_LOCK_REL_VALINVALID */
		/* Release lock with no value register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelNvOff;
		Lsb = RegPtr->RelNv.Lsb;
		Mask = RegPtr->RelNv.Mask;

	}

	Value = XAIETILE_LOCK_REL_SUCCESS << Lsb;

	if (XAieGbl_MaskPoll(RegAddr, Mask, Value, TimeOut) == XAIE_SUCCESS) {
		RelDone = XAIETILE_LOCK_REL_SUCCESS;
	}

	return RelDone;
}

/** @} */


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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmetile_lock.c
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
* </pre>
*
******************************************************************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmegbl_reginit.h"
#include "xmetile_lock.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XMeGbl_RegLocks TileLockRegs[];
extern XMeGbl_RegLocks ShimLockRegs[];

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
u8 XMeTile_LockAcquire(XMeGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
								u32 TimeOut)
{
	u64 RegAddr;
	u32 RegVal;
	u32 LoopCnt = 0U;
	u8 AcqDone = XMETILE_LOCK_ACQ_FAILED;
	u8 AcqDoneLsb;
	u32 AcqDoneMask;
	XMeGbl_RegLocks *RegPtr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(LockId < XMEGBL_TILE_LOCK_NUM_MAX);

	if(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
                RegPtr = &TileLockRegs[LockId];
        } else {
                RegPtr = &ShimLockRegs[LockId];
        }
		
	if(LockVal == XMETILE_LOCK_ACQ_VALINVALID) {
		/* Acquire lock with no value register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqNvOff;
		AcqDoneLsb = RegPtr->AcqNv.Lsb;
		AcqDoneMask = RegPtr->AcqNv.Mask;

	} else if(LockVal == XMETILE_LOCK_ACQ_VAL0) {
		/* Acquire lock with value 0 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqV0Off;
		AcqDoneLsb = RegPtr->AcqV0.Lsb;
		AcqDoneMask = RegPtr->AcqV0.Mask;

	} else if(LockVal == XMETILE_LOCK_ACQ_VAL1) {
		/* Acquire lock with value 1 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->AcqV1Off;
		AcqDoneLsb = RegPtr->AcqV1.Lsb;
		AcqDoneMask = RegPtr->AcqV1.Mask;
	}

	/* Loop count rounded off */
	LoopCnt = (TimeOut + XMETILE_LOCK_ACQ_MIN_WAIT_USECS - 1U) /
					XMETILE_LOCK_ACQ_MIN_WAIT_USECS;

	while(LoopCnt >= 0U) {
		/* Read request to Lock#_Acquire_NV/V0/V1 */
		RegVal = XMeGbl_Read32(RegAddr);
		AcqDone = XMe_GetField(RegVal, AcqDoneLsb, AcqDoneMask);

		if((AcqDone == XMETILE_LOCK_ACQ_SUCCESS) ||
                                (LoopCnt == 0U)) {
			break;
		}
		LoopCnt--;

                XMe_usleep(XMETILE_LOCK_ACQ_MIN_WAIT_USECS);
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
u8 XMeTile_LockRelease(XMeGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
								u32 TimeOut)
{
	u64 RegAddr;
	u32 RegVal;
	u32 LoopCnt = 0U;
	u8 RelDone = XMETILE_LOCK_REL_FAILED;
	u8 RelDoneLsb;
	u32 RelDoneMask;
        XMeGbl_RegLocks *RegPtr;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(LockId < XMEGBL_TILE_LOCK_NUM_MAX);

       	if(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE) {
                RegPtr = &TileLockRegs[LockId];
        } else {
                RegPtr = &ShimLockRegs[LockId];
        }

	if(LockVal == XMETILE_LOCK_REL_VALINVALID) {
		/* Release lock with no value register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelNvOff;
		RelDoneLsb = RegPtr->RelNv.Lsb;
		RelDoneMask = RegPtr->RelNv.Mask;

	} else if(LockVal == XMETILE_LOCK_REL_VAL0) {
		/* Release lock with value 0 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelV0Off;
		RelDoneLsb = RegPtr->RelV0.Lsb;
		RelDoneMask = RegPtr->RelV0.Mask;

	} else if(LockVal == XMETILE_LOCK_REL_VAL1) {
		/* Release lock with value 1 register address */
		RegAddr = TileInstPtr->TileAddr + RegPtr->RelV1Off;
		RelDoneLsb = RegPtr->RelV1.Lsb;
		RelDoneMask = RegPtr->RelV1.Mask;
	}

	/* Loop count rounded off */
	LoopCnt = (TimeOut + XMETILE_LOCK_ACQ_MIN_WAIT_USECS - 1U) /
					XMETILE_LOCK_REL_MIN_WAIT_USECS;

	while(LoopCnt >= 0U) {
		/* Read request to Lock#_Release_NV/V0/V1 */
		RegVal = XMeGbl_Read32(RegAddr);
		RelDone = XMe_GetField(RegVal, RelDoneLsb, RelDoneMask);

		if((RelDone == XMETILE_LOCK_REL_SUCCESS) ||
                                (LoopCnt == 0U)) {
			break;
		}
		LoopCnt--;

		XMe_usleep(XMETILE_LOCK_REL_MIN_WAIT_USECS);
	}

	return RelDone;
}

/** @} */


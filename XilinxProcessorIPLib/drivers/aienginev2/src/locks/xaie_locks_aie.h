/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks_aie.h
* @{
*
* This file contains function prototypes for AIE locks. This header file is
* not exposed to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIELOCKS_AIE_H
#define XAIELOCKS_AIE_H
/***************************** Include Files *********************************/
#include "xaie_locks.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes  *****************************/
AieRC _XAie_LockAcquire(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut);
AieRC _XAie_LockRelease(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut);

#endif
/** @} */

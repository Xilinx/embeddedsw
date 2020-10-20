/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks.h
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* 1.1   Tejus   03/23/2020  Include xaiegbl header file
* </pre>
*
******************************************************************************/
#ifndef XAIELOCKS_H
#define XAIELOCKS_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_LockAcquire(XAie_DevInst *DevInst, XAie_LocType Loc, XAie_Lock Lock,
		u32 TimeOut);
AieRC XAie_LockRelease(XAie_DevInst *DevInst, XAie_LocType Loc, XAie_Lock Lock,
		u32 TimeOut);

#endif		/* end of protection macro */

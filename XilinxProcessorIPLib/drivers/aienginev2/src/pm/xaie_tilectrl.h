/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_tilectrl.h
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   05/27/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_TILECTRL_H
#define XAIE_TILECTRL_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
/**************************** Type Definitions *******************************/
/***************************** Macro Definitions *****************************/

#define XAIE_ISOLATE_EAST_MASK	(1U << 3)
#define XAIE_ISOLATE_NORTH_MASK	(1U << 2)
#define XAIE_ISOLATE_WEST_MASK	(1U << 1)
#define XAIE_ISOLATE_SOUTH_MASK	(1U << 0)
#define XAIE_ISOLATE_ALL_MASK	((1U << 4) - 1)

/************************** Function Prototypes  *****************************/
AieRC _XAie_TileCtrlSetIsolation(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 Dir);

#endif		/* end of protection macro */

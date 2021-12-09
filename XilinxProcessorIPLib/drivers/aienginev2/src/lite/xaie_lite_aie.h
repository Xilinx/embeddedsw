/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_aie.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs for AIE
* device generation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_AIE_H
#define XAIE_LITE_AIE_H

/***************************** Include Files *********************************/
/************************** Constant Definitions *****************************/
#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			0x20000000000
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			9
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			50
#endif

#define XAIE_COL_SHIFT			23
#define XAIE_ROW_SHIFT			18
#define XAIE_SHIM_ROW			0
#define XAIE_MEM_TILE_ROW_START		0
#define XAIE_MEM_TILE_NUM_ROWS		0
#define XAIE_AIE_TILE_ROW_START		1
#define XAIE_AIE_TILE_NUM_ROWS		8

#define UPDT_NEXT_NOC_TILE_LOC(Loc)	\
	({if ((Loc).Col <= 1) \
		(Loc).Col = 2; \
	else \
		(Loc).Col += ((Loc).Col % 2) * 2 + 1;})

#include "xaie_lite_regdef_aie.h"
#include "xaie_lite_regops_aie.h"

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */

/** @} */

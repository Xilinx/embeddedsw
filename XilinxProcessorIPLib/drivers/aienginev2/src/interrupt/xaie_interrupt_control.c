/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_control.c
* @{
*
* This file implements routines for enabling/disabling AIE interrupts.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  08/20/2021  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_interrupt.h"
#include "xaie_lite.h"
#include "xaie_lite_io.h"

#if defined(XAIE_FEATURE_INTR_CTRL_ENABLE) && defined(XAIE_FEATURE_LITE)

/************************** Constant Definitions *****************************/
#define XAIE_ERROR_L2_DISABLE			0x3FU

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API returns the status of second-level interrupt controller.
*
* @param	Loc: Location of AIE tile.
*
* @return	Status: Status second-level interrupt controller.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline u32 _XAie_IntrCtrlL2Status(XAie_LocType Loc)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_STATUS;
	return _XAie_LRead32(RegAddr);
}

/*****************************************************************************/
/**
*
* This API clears the status of interrupts in the second-level interrupt
* controller.
*
* @param	Loc: Location of AIE tile.
* @param	ChannelBitMap: Bitmap of channels to be acknowledged. Writing a
*				value of 1 to the register field clears the
*				corresponding interrupt channel.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_IntrCtrlL2Ack(XAie_LocType Loc, u32 ChannelBitMap)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_STATUS;
	_XAie_LWrite32(RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API enables/disables interrupts to second level interrupt controller.
*
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt Bitmap.
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE to enable or disable.
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_IntrCtrlL2Config(XAie_LocType Loc, u32 ChannelBitMap,
		u8 Enable)
{
	u64 RegAddr;
	u32 RegOffset;

	if(Enable == XAIE_ENABLE)
		RegOffset = XAIE_NOC_MOD_INTR_L2_ENABLE;
	else
		RegOffset = XAIE_NOC_MOD_INTR_L2_DISABLE;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOffset;
	_XAie_LWrite32(RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API disables all second-level interrupt controllers reporting errors.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAie_DisableErrorInterrupts()
{
	XAie_LocType Loc = XAie_TileLoc(0, XAIE_SHIM_ROW);

	for (UPDT_NEXT_NOC_TILE_LOC(Loc); Loc.Col < XAIE_NUM_COLS;
			UPDT_NEXT_NOC_TILE_LOC(Loc)) {
		u32 Status;

		Status = _XAie_IntrCtrlL2Status(Loc);

		/* Only disable L2s that are reporting errors. */
		if (Status) {
			_XAie_IntrCtrlL2Config(Loc, XAIE_ERROR_L2_DISABLE,
					XAIE_DISABLE);
			_XAie_IntrCtrlL2Ack(Loc, Status);
		}
	}
}

#endif /* XAIE_FEATURE_INTR_CTRL_ENABLE */

/** @} */

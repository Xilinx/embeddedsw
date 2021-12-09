/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_backtrack.c
* @{
*
* This file implements routines for backtracking an AIE error interrupt to its
* source.
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
#include "xaie_clock.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_interrupt.h"
#include "xaie_lite.h"
#include "xaie_lite_io.h"

#if defined(XAIE_FEATURE_INTR_BTRK_ENABLE) && defined(XAIE_FEATURE_LITE)

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API enables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	ChannelBitMap: Interrupt Bitmap.
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
static inline void _XAie_LIntrCtrlL2Enable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u32 ChannelBitMap)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_ENABLE;
	_XAie_LPartWrite32(DevInst, RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API return the bitmap value of second level interrupts channels enabled.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
*
* @return	Channel bitmap.
*
* @note		Internal Only.
*
******************************************************************************/
static inline u32 _XAie_LIntrCtrlL2Mask(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_MASK;
	return _XAie_LPartRead32(DevInst, RegAddr);
}

/*****************************************************************************/
/**
*
* This API returns the status of first-level interrupt controller.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Switch: Broadcast switch.
*
* @return	Status first-level interrupt controller.
*
* @note		Internal only.
*
******************************************************************************/
static inline u32 _XAie_LIntrCtrlL1Status(XAie_DevInst *DevInst,
			XAie_LocType Loc, XAie_BroadcastSw Switch)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_PL_MOD_INTR_L1_STATUS + Switch *
				XAIE_PL_MOD_INTR_L1_SW_REGOFF;
	return _XAie_LPartRead32(DevInst, RegAddr);
}

/*****************************************************************************/
/**
*
* This API clears the status of first-level interrupt controller.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Switch: Broadcast switch.
* @param	ChannelBitMap: Bitmap of channel statues to be cleared.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
static inline void _XAie_LIntrCtrlL1Ack(XAie_DevInst *DevInst,
			XAie_LocType Loc, XAie_BroadcastSw Switch,
			u32 ChannelBitMap)
{
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_PL_MOD_INTR_L1_STATUS + Switch *
				XAIE_PL_MOD_INTR_L1_SW_REGOFF;
	_XAie_LPartWrite32(DevInst, RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API returns the status an event.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	Event: Physical event ID.
*
* @return	True is event was asserted, otherwise false.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LEventReadStatus(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module, u8 Event)
{
	u64 RegAddr;
	u32 RegOff, RegVal;

	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		RegOff = XAIE_MEM_TILE_BASE_EVENT_STATUS;
	} else if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_CORE_MOD) {
			RegOff = XAIE_CORE_MOD_BASE_EVENT_STATUS;
		} else {
			RegOff = XAIE_MEM_MOD_BASE_EVENT_STATUS;
		}
	} else {
		RegOff = XAIE_PL_MOD_BASE_EVENT_STATUS;
	}

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOff +
			(Event / 32U) * 4U;
	RegVal = _XAie_LPartRead32(DevInst, RegAddr);
	return XAie_GetField(RegVal, (Event % 32U), 1U << (Event % 32U));
}

/*****************************************************************************/
/**
*
* This API clears the event status.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	Event: Physical event ID.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
static inline void _XAie_LEventClearStatus(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module, u8 Event)
{
	u64 RegAddr;
	u32 RegOff, RegVal;

	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		RegOff = XAIE_MEM_TILE_BASE_EVENT_STATUS;
	} else if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_CORE_MOD) {
			RegOff = XAIE_CORE_MOD_BASE_EVENT_STATUS;
		} else {
			RegOff = XAIE_MEM_MOD_BASE_EVENT_STATUS;
		}
	} else {
		RegOff = XAIE_PL_MOD_BASE_EVENT_STATUS;
	}

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOff +
			(Event / 32U) * 4U;
	RegVal = XAie_SetField(1U, (Event % 32U), (1U << (Event % 32U)));
	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API returns the event ID of event being broadcast on a broadcast channel.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	BroadcastId: Broadcast channel ID.
*
* @return	Event: Physical event ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u32 _XAie_LReadArrayErrorBroadcastEvent(XAie_DevInst *DevInst,
		 XAie_LocType Loc, XAie_ModuleType Module, u8 BroadcastId)
{
	u64 RegAddr;
	u32 RegOff;

	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		RegOff = XAIE_MEM_TILE_BASE_EVENT_BROADCAST;
	} else {
		if (Module == XAIE_CORE_MOD) {
			RegOff = XAIE_CORE_MOD_BASE_EVENT_BROADCAST;
		} else {
			RegOff = XAIE_MEM_MOD_BASE_EVENT_BROADCAST;
		}
	}

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOff +
			BroadcastId * 4U;
	return _XAie_LPartRead32(DevInst, RegAddr);
}

/*****************************************************************************/
/**
*
* This API returns the event ID of shim event being broadcast on L1 IRQ channel.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	IrqId: L1 IRQ channel ID.
*
* @return	Event: Physical event ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u32 _XAie_LReadShimErrorBroadcastEvent(XAie_DevInst *DevInst,
		 XAie_LocType Loc, u8 IrqId)
{
	u64 RegAddr;
	u32 RegValue, Lsb, Mask;

	Lsb = IrqId * 8U;
	Mask = XAIE_EVENT_MASK << Lsb;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_PL_MOD_INTR_L1_IRQ_EVENTA;
	RegValue = _XAie_LPartRead32(DevInst, RegAddr);
	return XAie_GetField(RegValue, Lsb, Mask);
}

/*****************************************************************************/
/**
*
* This is a wrapper API returns the event ID of event being broadcast.
* For AIE array tiles, it return the event on the event broadcast channels. For
* Shim tiles, it return the event on L1 IRQ channels.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	BroadcastId: Broadcast channel ID.
*
* @return	Event: Physical event ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u32 _XAie_ReadErrorBroadcastEvent(XAie_DevInst *DevInst,
		 XAie_LocType Loc, XAie_ModuleType Module, u8 BroadcastId)
{
	if (Loc.Row == XAIE_SHIM_ROW) {
		return _XAie_LReadShimErrorBroadcastEvent(DevInst, Loc,
				BroadcastId);
	} else {
		return _XAie_LReadArrayErrorBroadcastEvent(DevInst, Loc, Module,
				BroadcastId);
	}
}

/*****************************************************************************/
/**
*
* This API maps a given group error event index to its physical event ID.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	GroupErrorIndex: Index of event in group error.
*
* @return	Physical event ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LMapGroupErrorsToEventId(XAie_DevInst *DevInst,
			XAie_LocType Loc, XAie_ModuleType Module,
			u8 GroupErrorIndex)
{
	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		return XAIE_MEM_TILE_EVENT_GROUP_ERROR0 + GroupErrorIndex;
	} else if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_CORE_MOD) {
			return XAIE_CORE_MOD_EVENT_GROUP_ERROR0 +
					GroupErrorIndex;
		} else {
			return XAIE_MEM_MOD_EVENT_GROUP_ERROR0 +
					GroupErrorIndex;
		}
	} else {
		return XAIE_PL_MOD_EVENT_GROUP_ERROR0 + GroupErrorIndex;
	}
}

/*****************************************************************************/
/**
*
* This API returns a bitmap of group error enabled.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
*
* @return	Bitmap of enabled group errors.
*
* @note		Internal only.
*
******************************************************************************/
static inline u32 _XAie_LReadGroupErrors(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	u64 RegAddr;
	u32 RegOff;

	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		RegOff = XAIE_MEM_TILE_GROUP_ERROR0_ENABLE;
	} else if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_CORE_MOD) {
			RegOff = XAIE_CORE_MOD_GROUP_ERROR0_ENABLE;
		} else {
			RegOff = XAIE_MEM_MOD_GROUP_ERROR0_ENABLE;
		}
	} else {
		RegOff = XAIE_PL_MOD_GROUP_ERROR0_ENABLE;
	}

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOff;
	return _XAie_LPartRead32(DevInst, RegAddr);
}

/*****************************************************************************/
/**
*
* This API applies the bitmap of events to be enabled/disabled in a group error.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	EventMap: Bitmap of events to be enabled/disabled from the
*			  group.
*
* @return	Bitmap of enabled group errors.
*
* @note		Internal only.
*
******************************************************************************/
static inline void _XAie_LGroupErrorControl(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module, u32 EventMap)
{
	u64 RegAddr;
	u32 RegOff;

	u8 TType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
		RegOff = XAIE_MEM_TILE_GROUP_ERROR0_ENABLE;
	} else if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
		if (Module == XAIE_CORE_MOD) {
			RegOff = XAIE_CORE_MOD_GROUP_ERROR0_ENABLE;
		} else {
			RegOff = XAIE_MEM_MOD_GROUP_ERROR0_ENABLE;
		}
	} else {
		RegOff = XAIE_PL_MOD_GROUP_ERROR0_ENABLE;
	}

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + RegOff;
	_XAie_LPartWrite32(DevInst, RegAddr, EventMap);
}

/*****************************************************************************/
/**
*
* This API backtracks the source of error interrupt within a tile.
*
* @param	DevInst: Device Instance.
* @param	MData: Error metadata.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_LBacktrackTile(XAie_DevInst *DevInst,
		XAie_ErrorMetaData *MData, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	XAie_ErrorPayload *Buffer = MData->Payload;
	u32 *Count = &(MData->ErrorCount);
	u32 Size = MData->ArraySize - (*Count);
	u32 Value, Index, ErrorsMap;
	u8 GroupEvent, Event;

	/* Read event being broadcast on error channel */
	GroupEvent = _XAie_ReadErrorBroadcastEvent(DevInst, Loc, Module,
			XAIE_ERROR_BROADCAST_ID);

	if (!_XAie_LEventReadStatus(DevInst, Loc, Module, GroupEvent))
		return XAIE_OK;

	ErrorsMap = Value = _XAie_LReadGroupErrors(DevInst, Loc, Module);

	for_each_set_bit(Index, Value, 32) {
		Event = _XAie_LMapGroupErrorsToEventId(DevInst, Loc, Module,
				Index);

		if (!_XAie_LEventReadStatus(DevInst, Loc, Module, Event))
			continue;

		XAIE_DBG("%d: Error event %d asserted in module %d at (%d, %d)\n",
				*Count, Event, Module, Loc.Col, Loc.Row);

		ErrorsMap &= ~(1U << Index);

		Buffer[(*Count)].Loc = Loc;
		Buffer[(*Count)].Module = Module;
		Buffer[(*Count)].EventId = Event;
		(*Count)++;

		if (--Size == 0U) {
			_XAie_LGroupErrorControl(DevInst, Loc, Module,
					ErrorsMap);
			MData->NextTile = Loc;
			MData->NextModule = Module;
			MData->IsNextInfoValid = 1;
			return XAIE_INSUFFICIENT_BUFFER_SIZE;
		}
	}

	/* Disable backtracked error event broadcast */
	_XAie_LGroupErrorControl(DevInst, Loc, Module, ErrorsMap);

	/*
	 * Clear group error event status only if all active errors were
	 * backtracked and disabled.
	 */
	_XAie_LEventClearStatus(DevInst, Loc, Module, GroupEvent);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API backtracks the source of error interrupt within a column.
*
* @param	DevInst: Device Instance.
* @param	MData: Error metadata.
* @param	Loc: Location of AIE tile.
* @param	Switch: Broadcast switch.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_LBacktrackIntrCtrlL1(XAie_DevInst *DevInst,
		XAie_ErrorMetaData *MData, XAie_LocType Loc,
		XAie_BroadcastSw Switch)
{
	AieRC RC;
	u32 Status;

	Status = _XAie_LIntrCtrlL1Status(DevInst, Loc, Switch);

	/* Backtrack shim's internal events */
	if ((Status & XAIE_ERROR_SHIM_INTR_MASK) || (MData->IsNextInfoValid &&
					MData->NextModule == XAIE_PL_MOD &&
					MData->NextTile.Row == Loc.Row)) {
		_XAie_LIntrCtrlL1Ack(DevInst, Loc, Switch,
				XAIE_ERROR_SHIM_INTR_MASK);

		RC = _XAie_LBacktrackTile(DevInst, MData, Loc, XAIE_PL_MOD);
		if (RC == XAIE_INSUFFICIENT_BUFFER_SIZE)
			return RC;
	}

	/* Backtrack array tile's internal events. */
	if (!((Status & XAIE_ERROR_BROADCAST_MASK) || (MData->IsNextInfoValid &&
		MData->NextModule != XAIE_PL_MOD &&
		MData->NextTile.Row >= Loc.Row)))
		return XAIE_OK;

	_XAie_LIntrCtrlL1Ack(DevInst, Loc, Switch, XAIE_ERROR_BROADCAST_MASK);

	for (Loc.Row = XAIE_MEM_TILE_ROW_START;
	     Loc.Row < (XAIE_MEM_TILE_ROW_START + XAIE_MEM_TILE_NUM_ROWS);
	     Loc.Row++)
	{
		if (_XAie_LPmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
			continue;

		RC = _XAie_LBacktrackTile(DevInst, MData, Loc, XAIE_MEM_MOD);
		if (RC == XAIE_INSUFFICIENT_BUFFER_SIZE)
			return RC;

		/*
		 * TODO: In SystemC model, incoming broadcast status bit
		 *	 reported is off by a bit position. For switch A, skip
		 *	 backtracking above array tiles when this bug is fixed.
		 */

		_XAie_LEventClearStatus(DevInst, Loc, XAIE_MEM_MOD,
					 XAIE_MEM_TILE_EVENT_BROADCAST0);
	}

	for (Loc.Row = XAIE_AIE_TILE_ROW_START;
	     Loc.Row < (XAIE_AIE_TILE_ROW_START + XAIE_AIE_TILE_NUM_ROWS);
	     Loc.Row++)
	{
		XAie_ModuleType Module;
		u8 Event;

		if (Switch == XAIE_EVENT_SWITCH_A) {
			Module = XAIE_CORE_MOD;
			Event = XAIE_CORE_MOD_EVENT_BROADCAST0;
		} else {
			Module = XAIE_MEM_MOD;
			Event = XAIE_MEM_MOD_EVENT_BROADCAST0;
		}

		if (_XAie_LPmIsTileRequested(DevInst, Loc) == XAIE_DISABLE)
			continue;

		RC = _XAie_LBacktrackTile(DevInst, MData, Loc, Module);
		if (RC == XAIE_INSUFFICIENT_BUFFER_SIZE)
			return RC;

		/*
		 * Skip backtracking above array tiles if no broadcast signal
		 * was received and the last backtrack operation was successful.
		 */
		if (!(_XAie_LEventReadStatus(DevInst, Loc, Module, Event) ||
			(MData->IsNextInfoValid &&
			 MData->NextModule == Module &&
			 MData->NextTile.Row > Loc.Row))) {
			return XAIE_OK;
		}

		_XAie_LEventClearStatus(DevInst, Loc, Module, Event);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API backtracks the source of error interrupts. While doing so, any active
* error can be backtracked only once. Disabled L2 channels are re-enabled upon
* successful backtrack. Mdata.ErrorCount captures the total number of valid
* error payloads returned.
* If more number of errors are backtracked than it can fit in the allocated
* memory, XAIE_INSUFFICIENT_BUFFER_SIZE error code is returned. In such a
* scenario, Mdata.IsNextInfoValid flag is set and Mdata.ErrorCount holds the
* number of errors valid error payloads returned. Remaining active errors will
* be backtracked upon successive invocations of this API.
*
* @param	DevInst: Device Instance. Passing valid partition device
*			instance will only backtrack errors in the given
*			partition.
* @param	MData: Error metadata.
*
* @return	XAIE_OK on success, XAIE_INSUFFICIENT_BUFFER_SIZE code on
*		failure.
*
* @note		Before invoking this API,
*		- AIE error interrupts needs to be disabled by calling
*		  XAie_BacktrackErrorInterrupts().
*		- Error metadata instance needs to initialized using
*		  XAie_ErrorMetadataInit() helper.
*		- If more than one buffers are used to backtrack the same
*		  partition, error metadata needs to be preserve. To override
*		  error payload buffer only, use
*		  XAie_ErrorMetadataOverrideBuffer() helper macro.
*
******************************************************************************/
AieRC XAie_BacktrackErrorInterrupts(XAie_DevInst *DevInst,
		XAie_ErrorMetaData *MData)
{
	XAIE_ERROR_RETURN((DevInst == NULL || DevInst->NumCols > XAIE_NUM_COLS),
			XAIE_INVALID_ARGS,
			"Error interrupt backtracking failed, invalid partition instance\n");

	XAIE_ERROR_RETURN(MData->Payload == NULL || ArraySize == NULL,
			XAIE_INVALID_ARGS,
			"Invalid error payload buffer or size\n");

	XAie_LocType L2 = XAie_TileLoc(0, XAIE_SHIM_ROW);
	XAie_LocType L1 = XAie_TileLoc(0, XAIE_SHIM_ROW);

	/* Reset the total error count from previous backtrack. */
	MData->ErrorCount = 0U;

	for (L2 = XAie_LPartGetNextNocTile(DevInst, L2);
	     L2.Col < DevInst->NumCols;
	     L2 = XAie_LPartGetNextNocTile(DevInst, L2)) {
		AieRC RC;
		XAie_BroadcastSw Switch;
		u32 Enable = 0, Mask, Index;

		Mask = _XAie_LIntrCtrlL2Mask(DevInst, L2);

		/* Only backtrack disabled L2 channels. */
		if (Mask == XAIE_ERROR_L2_ENABLE)
			continue;

		Mask = (~Mask) & XAIE_ERROR_L2_ENABLE;

		for_each_set_bit(Index, Mask, 32) {
			_XAie_MapL2MaskToL1(DevInst, Index, L2.Col, &L1.Col,
					&Switch);

			RC = _XAie_LBacktrackIntrCtrlL1(DevInst, MData, L1,
					Switch);
			if (RC == XAIE_INSUFFICIENT_BUFFER_SIZE) {
				_XAie_LIntrCtrlL2Enable(DevInst, L2, Enable);
				return RC;
			}

			Enable |= BIT(Index);
		}

		_XAie_LIntrCtrlL2Enable(DevInst, L2, Enable);
	}

	/* Invalidate next info upon successful backtrack. */
	MData->IsNextInfoValid = 0;

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_INTR_BTRK_ENABLE */

/** @} */

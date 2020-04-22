/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_event.c
* @{
*
* This file contains routines for Event handling
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Hyun    10/02/2018  Initial creation
* 1.1   Hyun    10/10/2018  Use the mask write API
* 1.2   Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3   Hyun    12/13/2018  Add the core event API
* 1.4   Jubaer  02/14/2019  Add get Event Broadcast API
* 1.5   Jubaer  02/26/2019  Add group Event API
* 1.6   Jubaer  03/01/2019  Add Combo Event API
* 1.7   Jubaer  04/17/2019  Add Stream Switch Event Port Selection API
* 1.8   Tejus   09/05/2019  Fix compiler warnings
* 1.9   Dishita 11/1/2019   Fix coverity warnings
* 1.9   Wendy   02/24/2020  Add events notification
* 2.0   Dishita 03/29/2020  Add support for clock gating
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaielib.h"
#include "xaietile_error.h"
#include "xaietile_event.h"
#include "xaietile_noc.h"
#include "xaietile_pl.h"
#include "xaielib_npi.h"

/***************************** Constant Definitions **************************/
#define XAIETILE_EVENT_MODULE_CORE	0x0U
#define XAIETILE_EVENT_MODULE_PL		0x1U
#define XAIETILE_EVENT_MODULE_MEM	0x2U

#define XAIETILE_EVENT_1ST_IRQ_IDS_MASK	0x3FU /**< 1st level interrupt output IRQ
						   ids mask. 1st level interrupt
						   controller has its own irq IDs
						   to the 2nd level interrupt
						   controller.
						   For column 0 to 43, the pattern is:
						   0 1 2 3 4 5 0 1, 0 1 2 3 4 5 0 1
						   for column 44 to 49, the pattern is:
						   0 1 2 3 4 5 0 1 2 3 4 5
						*/
#define XAIETILE_EVENT_NPI_INTERRUPT	0x1U /** < Events notification NPI
					           interrupt ID */

/***************************** Macro Definitions *****************************/
/************************** Variable Definitions *****************************/
extern XAieGbl_RegEventGenerate EventGenerate[];
extern XAieGbl_RegEventBroadcast EventBroadcast[];
extern XAieGbl_RegEventBroadcastSet EventBroadcastSet[];
extern XAieGbl_RegEventBroadcastClear EventBroadcastClear[];
extern XAieGbl_RegEventBroadcastValue EventBroadcastValue[];
extern XAieGbl_RegTraceCtrls TraceCtrl[];
extern XAieGbl_RegTraceEvent TraceEvent[];
extern XAieGbl_RegCorePCEvent CorePCEvents[];
extern XAieGbl_Config XAieGbl_ConfigTable[];
extern XAieGbl_GroupEvents GroupEvents[];
extern XAieGbl_ComboEventInput ComboEventInput[];
extern XAieGbl_ComboEventControl ComboEventControl[];
extern XAieGbl_RegStrmSwEventPortSelect StrmSwEventPortSelect[];

/************************** Function Definitions *****************************/

/*
 * Event broadcast block value
 */

/*****************************************************************************/
/**
*
* This API returns the current event broadcast block value for memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileMem_EventBroadcastBlockValue(XAieGbl_Tile *TileInstPtr, u8 Dir)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XAIETILE_EVENT_MODULE_MEM].RegOff[Dir]);
}

/*****************************************************************************/
/**
*
* This API returns the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XAIETILE_EVENT_BLOCK_SWITCHA or XAIETILE_EVENT_BLOCK_SWITCHB.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTilePl_EventBroadcastBlockValue(XAieGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Dir < 4);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XAIETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U]);
}

/*****************************************************************************/
/**
*
* This API returns the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileCore_EventBroadcastBlockValue(XAieGbl_Tile *TileInstPtr, u8 Dir)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XAIETILE_EVENT_MODULE_CORE].RegOff[Dir]);
}

/*
 * Event broadcast block clear
 */

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_EventBroadcastBlockClear(XAieGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastClear[XAIETILE_EVENT_MODULE_MEM].RegOff[Dir],
			Mask);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XAIETILE_EVENT_BLOCK_SWITCHA or XAIETILE_EVENT_BLOCK_SWITCHB.
* @param	Mask - Mask with bits to clear
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_EventBroadcastBlockClear(XAieGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB,
		u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastClear[XAIETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U],
			Mask);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventBroadcastBlockClear(XAieGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
		EventBroadcastClear[XAIETILE_EVENT_MODULE_CORE].RegOff[Dir],
		Mask);
	return XAIE_SUCCESS;
}

/*
 * Event broadcast block set
 */

/*****************************************************************************/
/**
*
* This API sets the current event broadcast block value for memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_EventBroadcastBlockSet(XAieGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XAIETILE_EVENT_MODULE_MEM].RegOff[Dir],
			Mask);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XAIETILE_EVENT_BLOCK_SWITCHA or XAIETILE_EVENT_BLOCK_SWITCHB.
* @param	Mask - Mask with bits to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_EventBroadcastBlockSet(XAieGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB,
		u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XAIETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U],
			Mask);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
* XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventBroadcastBlockSet(XAieGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Dir < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XAIETILE_EVENT_MODULE_CORE].RegOff[Dir],
			Mask);
	return XAIE_SUCCESS;
}

/*
 * Event broadcast
 */

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XAIETILE_EVENT_MEM_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_EventBroadcast(XAieGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(BroadcastId < 16);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XAIETILE_EVENT_MODULE_MEM].RegOff +
			BroadcastId * 0x4U, Event);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XAIETILE_EVENT_PL_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_EventBroadcast(XAieGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(BroadcastId < 16);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XAIETILE_EVENT_MODULE_PL].RegOff +
			BroadcastId * 0x4U, Event);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventBroadcast(XAieGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(BroadcastId < 16);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XAIETILE_EVENT_MODULE_CORE].RegOff +
			BroadcastId * 0x4U, Event);
	return XAIE_SUCCESS;
}

/*
 * Event Generate
 */

/*****************************************************************************/
/**
*
* This API sets the memory module to generate an internal event of @Event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_MEM_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_EventGenerate(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XAIETILE_EVENT_MODULE_MEM].RegOff,
			Event);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the PL module to generate an internal event of @Event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_PL_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_EventGenerate(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XAIETILE_EVENT_MODULE_PL].RegOff,
			Event);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the Core module to generate an internal event of @Event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventGenerate(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XAIETILE_EVENT_MODULE_CORE].RegOff,
			Event);
	return XAIE_SUCCESS;
}

/*
 * Event trace control
 */

/*****************************************************************************/
/**
*
* This internal API sets the trace control of given module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XAIETILE_EVENT_MODULE_CORE, XAIETILE_EVENT_MODULE_PL, or
* XAIETILE_EVENT_MODULE_MEM.
* @param	Mode - Trace mode: Should be XAIETILE_EVENT_MODE_EVENT_TIME,
* XAIETILE_EVENT_MODE_EVENT_PC, or XAIETILE_EVENT_MODE_EXECUTION.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XAIE_SUCCESS on success
*
* @note		Only for internal use. If XAIETILE_EVENT_TRACING_INVALID_VAL is
* given for any argument, the setting argument is skipped.
*
*******************************************************************************/
static u8 _XAieTile_EventTraceControl(XAieGbl_Tile *TileInstPtr, u8 ModId,
		u8 Mode, u8 StartEvent, u8 StopEvent, u8 Id, u8 Packet)
{
	u64 RegAddr;
	u32 FldVal = 0;
	u32 FldMask = 0;

	if (Mode != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Mode.Mask;
		FldVal |= XAie_SetField(Mode, TraceCtrl[ModId].Mode.Lsb,
				TraceCtrl[ModId].Mode.Mask);
	}
	if (StartEvent != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Start.Mask;
		FldVal |= XAie_SetField(StartEvent, TraceCtrl[ModId].Start.Lsb,
				TraceCtrl[ModId].Start.Mask);
	}
	if (StopEvent != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Stop.Mask;
		FldVal |= XAie_SetField(StopEvent, TraceCtrl[ModId].Stop.Lsb,
				TraceCtrl[ModId].Stop.Mask);
	}

	if (Mode != XAIETILE_EVENT_TRACING_INVALID_VAL ||
			StartEvent != XAIETILE_EVENT_TRACING_INVALID_VAL ||
			StopEvent != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		RegAddr = TileInstPtr->TileAddr + TraceCtrl[ModId].RegOff[0];
		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	FldVal = 0;
	FldMask = 0;

	if (Id != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Id.Mask;
		FldVal |= XAie_SetField(Id, TraceCtrl[ModId].Id.Lsb,
				TraceCtrl[ModId].Id.Mask);
	}

	if (Packet != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Packet.Mask;
		FldVal |= XAie_SetField(Packet, TraceCtrl[ModId].Packet.Lsb,
				TraceCtrl[ModId].Packet.Mask);
	}

	if (Id != XAIETILE_EVENT_TRACING_INVALID_VAL ||
			Packet != XAIETILE_EVENT_TRACING_INVALID_VAL) {
		RegAddr = TileInstPtr->TileAddr + TraceCtrl[ModId].RegOff[1];
		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the trace control of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XAIE_SUCCESS on success
*
* @note		If XAIETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XAieTileMem_EventTraceControl(XAieGbl_Tile *TileInstPtr, u8 StartEvent,
		u8 StopEvent, u8 Id, u8 Packet)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return _XAieTile_EventTraceControl(TileInstPtr,
			XAIETILE_EVENT_MODULE_MEM,
			XAIETILE_EVENT_TRACING_INVALID_VAL,
			StartEvent, StopEvent, Id, Packet);
}

/*****************************************************************************/
/**
*
* This API sets the trace control of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XAIE_SUCCESS on success
*
* @note		If XAIETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XAieTilePl_EventTraceControl(XAieGbl_Tile *TileInstPtr, u8 StartEvent,
		u8 StopEvent, u8 Id, u8 Packet)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);

	return _XAieTile_EventTraceControl(TileInstPtr,
			XAIETILE_EVENT_MODULE_PL,
			XAIETILE_EVENT_TRACING_INVALID_VAL,
			StartEvent, StopEvent, Id, Packet);
}

/*****************************************************************************/
/**
*
* This API sets the trace control of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mode - Trace mode: Should be XAIETILE_EVENT_MODE_EVENT_TIME,
* XAIETILE_EVENT_MODE_EVENT_PC, or XAIETILE_EVENT_MODE_EXECUTION.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XAIE_SUCCESS on success
*
* @note		If XAIETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XAieTileCore_EventTraceControl(XAieGbl_Tile *TileInstPtr, u8 Mode,
		u8 StartEvent, u8 StopEvent, u8 Id, u8 Packet)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return _XAieTile_EventTraceControl(TileInstPtr,
			XAIETILE_EVENT_MODULE_CORE, Mode,
			StartEvent, StopEvent, Id, Packet);
}

/*
 * Event trace
 */

/*****************************************************************************/
/**
*
* This internal API sets the event trace of given module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XAIETILE_EVENT_MODULE_CORE, XAIETILE_EVENT_MODULE_PL, or
* XAIETILE_EVENT_MODULE_MEM.
* @param	Event - Event ID
* @param	Idx - Trace ID. 0 to 7.
*
* @return	XAIE_SUCCESS on success
*
* @note		Only for internal use.
*
*******************************************************************************/
static u8 _XAieTile_EventTraceEventId(XAieGbl_Tile *TileInstPtr, u8 ModeId,
		u8 Event, u8 Idx)
{
	u8 RegPart = (Idx < 4) ? 0 : 1;
	u64 RegAddr;
	u32 FldVal;

	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[RegPart];
	FldVal = XAie_SetField(Event, TraceEvent[ModeId].Event[Idx].Lsb,
			TraceEvent[ModeId].Event[Idx].Mask);
	XAieGbl_MaskWrite32(RegAddr, TraceEvent[ModeId].Event[Idx].Mask, FldVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event trace of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID
* @param	Idx - Trace ID. 0 to 7.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_EventTraceEventWriteId(XAieGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

	return _XAieTile_EventTraceEventId(TileInstPtr,
			XAIETILE_EVENT_MODULE_MEM, Event, Idx);
}

/*****************************************************************************/
/**
*
* This API sets the event trace of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID
* @param	Idx - Trace ID. 0 to 7.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_EventTraceEventWriteId(XAieGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

	return _XAieTile_EventTraceEventId(TileInstPtr,
			XAIETILE_EVENT_MODULE_PL,Event, Idx);
}

/*****************************************************************************/
/**
*
* This API sets the event trace of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID
* @param	Idx - Trace ID. 0 to 7.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventTraceEventWriteId(XAieGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

	return _XAieTile_EventTraceEventId(TileInstPtr,
			XAIETILE_EVENT_MODULE_CORE, Event, Idx);
}

/*****************************************************************************/
/**
*
* This internal API sets multiple event traces of given module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XAIETILE_EVENT_MODULE_CORE, XAIETILE_EVENT_MODULE_PL, or
* XAIETILE_EVENT_MODULE_MEM.
* @param	TraceEvents - Events to trace.
*
* @return	XAIE_SUCCESS on success
*
* @note		Only for internal use. All 8 event traces should be set.
*
*******************************************************************************/
static u8 _XAieTile_EventTraceEvent(XAieGbl_Tile *TileInstPtr, u8 ModeId,
		XAie_TraceEvents *TraceEvents)
{
	u8 Idx;
	u64 RegAddr;
	u32 RegVal = 0;

	for (Idx = 0; Idx < 4; Idx++) {
		RegVal |= XAie_SetField(TraceEvents->TraceEvent[Idx],
				TraceEvent[ModeId].Event[Idx].Lsb,
				TraceEvent[ModeId].Event[Idx].Mask);
	}
	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[0];
	XAieGbl_Write32(RegAddr, RegVal);

	RegVal = 0;
	for (Idx = 4; Idx < XAIETILE_EVENT_NUM_TRACE_EVENT; Idx++) {
		RegVal |= XAie_SetField(TraceEvents->TraceEvent[Idx],
				TraceEvent[ModeId].Event[Idx].Lsb,
				TraceEvent[ModeId].Event[Idx].Mask);
	}
	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[1];
	XAieGbl_Write32(RegAddr, RegVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets multiple event traces of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Events to trace.
*
* @return	XAIE_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XAieTileMem_EventTraceEventWrite(XAieGbl_Tile *TileInstPtr,
		XAie_TraceEvents *TraceEvents)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TraceEvents != XAIE_NULL);

	return _XAieTile_EventTraceEvent(TileInstPtr, XAIETILE_EVENT_MODULE_MEM,
			TraceEvents);
}

/*****************************************************************************/
/**
*
* This API sets multiple event traces of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Events to trace.
*
* @return	XAIE_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XAieTilePl_EventTraceEventWrite(XAieGbl_Tile *TileInstPtr,
		XAie_TraceEvents *TraceEvents)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(TraceEvents != XAIE_NULL);

	return _XAieTile_EventTraceEvent(TileInstPtr, XAIETILE_EVENT_MODULE_PL,
			TraceEvents);
}

/*****************************************************************************/
/**
*
* This API sets multiple event traces of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Events to trace.
*
* @return	XAIE_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XAieTileCore_EventTraceEventWrite(XAieGbl_Tile *TileInstPtr,
		XAie_TraceEvents *TraceEvents)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TraceEvents != XAIE_NULL);

	return _XAieTile_EventTraceEvent(TileInstPtr, XAIETILE_EVENT_MODULE_CORE,
			TraceEvents);
}

/*****************************************************************************/
/**
*
* This API adds an event to TraceEvents
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Where event trace is queued
* @param	Idx - Trace ID to add an event to.
* @param	Event - Event ID
*
* @return	XAIE_SUCCESS on success
*
* @note		Once all events are added, the TraceEvents can be used with
* XAieTileCore_EventTraceEventWrite to update with a single write.
*
*******************************************************************************/
u8 XAieTile_EventTraceEventAdd(XAieGbl_Tile *TileInstPtr,
		XAie_TraceEvents *TraceEvents, u8 Idx, u8 Event)
{
	(void)TileInstPtr;
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TraceEvents != XAIE_NULL);
	XAie_AssertNonvoid(Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

	TraceEvents->TraceEvent[Idx] = Event;

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API initializes TraceEvents
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Where event trace is queued
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_EventTraceEventInit(XAieGbl_Tile *TileInstPtr,
		XAie_TraceEvents *TraceEvents)
{
	u8 Idx;

	(void)TileInstPtr;
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TraceEvents != XAIE_NULL);

	for (Idx = 0U; Idx < XAIETILE_EVENT_NUM_TRACE_EVENT; Idx++) {
		TraceEvents->TraceEvent[Idx] = 0U;
	}

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the PC event for core module
*
* @param	TileInstPtr - Pointer to the Tile instance with core module.
* @param	PCEvent - which PC event to set. Should be one of
* XAIETILE_EVENT_CORE_PC_EVENT0, XAIETILE_EVENT_CORE_PC_EVENT1,
* XAIETILE_EVENT_CORE_PC_EVENT2, or XAIETILE_EVENT_CORE_PC_EVENT3.
* @param	PCAddr - PC address to generate an event
* @param	Valid - Valid bit. Should be 0 or 1.
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_EventPCEvent(XAieGbl_Tile *TileInstPtr, u8 PCEvent, u16 PCAddr,
		u8 Valid)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(PCEvent <= XAIETILE_EVENT_CORE_PC_EVENT3);
	XAie_AssertNonvoid(Valid == 0 || Valid == 1);

	RegVal = XAie_SetField(PCAddr, CorePCEvents[PCEvent].PCAddr.Lsb,
			CorePCEvents[PCEvent].PCAddr.Mask);
	RegVal |= XAie_SetField(Valid, CorePCEvents[PCEvent].Valid.Lsb,
			CorePCEvents[PCEvent].Valid.Mask);
	XAieGbl_Write32(TileInstPtr->TileAddr + CorePCEvents[PCEvent].RegOff,
			RegVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API gets the event to the given broadcast ID for memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
*
* @return	Event - Event ID. One of XAIETILE_EVENT_MEM_*
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemEventBroadcastGet(XAieGbl_Tile *TileInstPtr, u8 BroadcastId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(BroadcastId < 16);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				EventBroadcast[XAIETILE_EVENT_MODULE_MEM].RegOff
				+ BroadcastId * 0x4U);

	return XAie_GetField(RegVal, XAIEGBL_MEM_EVTBRDCAST0_EVT_LSB,
			     XAIEGBL_MEM_EVTBRDCAST0_EVT_MASK);
}

/*****************************************************************************/
/**
*
* This API gets the event to the given broadcast ID for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
*
* @return	Event - Event ID. One of XAIETILE_EVENT_PL_*
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlEventBroadcastGet(XAieGbl_Tile *TileInstPtr, u8 BroadcastId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(BroadcastId < 16);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				EventBroadcast[XAIETILE_EVENT_MODULE_PL].RegOff
				+ BroadcastId * 0x4U);

	return XAie_GetField(RegVal, XAIEGBL_CORE_EVTBRDCAST0_EVT_LSB,
			     XAIEGBL_CORE_EVTBRDCAST0_EVT_MASK);
}

/*****************************************************************************/
/**
*
* This API gets the event to the given broadcast ID for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
*
* @return	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreEventBroadcastGet(XAieGbl_Tile *TileInstPtr, u8 BroadcastId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(BroadcastId < 16);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcast[XAIETILE_EVENT_MODULE_CORE].RegOff +
			BroadcastId * 0x4U);

	return XAie_GetField(RegVal, XAIEGBL_CORE_EVTBRDCAST0_EVT_LSB,
			     XAIEGBL_CORE_EVTBRDCAST0_EVT_MASK);
}

/*****************************************************************************/
/**
*
* This API returns the event status correspond to the Memory module event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	Current event status
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemEventStatusGet(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_MEM_EVTSTA0 +
			      (Event / XAIEGBL_MEM_EVTSTA0WID) * 0x4U);
}

/*****************************************************************************/
/**
*
* This API returns the event status correspond to the PL module event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	Current event status
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlEventStatusGet(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_PL_EVTSTA0 +
			      (Event / XAIEGBL_PL_EVTSTA0WID) * 0x4U);
}

/*****************************************************************************/
/**
*
* This API returns the event status correspond to the Core Module event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	Current event status
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreEventStatusGet(XAieGbl_Tile *TileInstPtr, u8 Event)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_CORE_EVTSTA0 +
			      (Event / XAIEGBL_CORE_EVTSTA0WID) * 0x4U);
}

/*****************************************************************************/
/**
*
* This API clears the event status correspond to the Memory module event
*
* @param        TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
* @param        Mask - value with to clear the bits.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemEventStatusClear(XAieGbl_Tile *TileInstPtr, u8 Event, u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_EVTSTA0 +
			(Event / XAIEGBL_MEM_EVTSTA0WID) * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API clears the event status correspond to the PL module event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
* @param	Mask - Bits to clear.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlEventStatusClear(XAieGbl_Tile *TileInstPtr, u8 Event, u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_EVTSTA0 +
			(Event / XAIEGBL_PL_EVTSTA0WID) * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API clears the event status correspond to the Core Module event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
* @param	Mask - Bits to clear.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreEventStatusClear(XAieGbl_Tile *TileInstPtr, u8 Event,
				   u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_EVTSTA0 +
			(Event / XAIEGBL_CORE_EVTSTA0WID) * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for given
* Core module Column
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	XAIE_SUCCESS on success
*
* @note		Failure indicates partial column configuration.
*
*******************************************************************************/
u8 XAieGbl_CoreEventBroadcastColumn(XAieGbl_Tile *TileInstPtr, u8 BroadcastId,
				    u8 Event)
{
	u16 NumRows = XAieGbl_ConfigTable->NumRows;
	u16 RowIdx;
	XAieGbl_Tile *TilePtr;
	u8 status;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TileInstPtr->RowId == 0);

	for (RowIdx = 1; RowIdx <= NumRows; RowIdx++) {
		TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
			  (RowIdx * sizeof(XAieGbl_Tile)));

		status = XAieTileCore_EventBroadcast(TilePtr, BroadcastId,
						     Event);
		if (status != XAIE_SUCCESS) {
			return status;
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for memory module
* for given Column
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XAIETILE_EVENT_CORE_*
*
* @return	XAIE_SUCCESS on success
*
* @note		Failure indicates partial column configuration.
*
*******************************************************************************/
u8 XAieGbl_MemEventBroadcastColumn(XAieGbl_Tile *TileInstPtr, u8 BroadcastId,
				   u8 Event)
{
	u16 NumRows = XAieGbl_ConfigTable->NumRows;
	u16 RowIdx;
	XAieGbl_Tile *TilePtr;
	u8 status;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TileInstPtr->RowId == 0);

	for (RowIdx = 1; RowIdx <= NumRows; RowIdx++) {
		TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
			  (RowIdx * sizeof(XAieGbl_Tile)));

		status = XAieTileMem_EventBroadcast(TilePtr, BroadcastId,
						    Event);
		if (status != XAIE_SUCCESS) {
			return status;
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for Core module
* for given Column
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	Dir - Direction. Should be one of
*		XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
*		XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XAIE_SUCCESS on success
*
* @note		Failure indicates partial column configuration.
*
*******************************************************************************/
u8 XAieGbl_CoreEventBroadcastBlockClearColumn(XAieGbl_Tile *TileInstPtr, u8 Dir,
					      u16 Mask)
{
	u16 NumRows = XAieGbl_ConfigTable->NumRows;
	u16 RowIdx;
	XAieGbl_Tile *TilePtr;
	u8 status;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TileInstPtr->RowId == 0);

	for (RowIdx = 1; RowIdx <= NumRows; RowIdx++) {
		TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
			  (RowIdx * sizeof(XAieGbl_Tile)));

		status = XAieTileCore_EventBroadcastBlockClear(TilePtr, Dir,
							       Mask);
		if (status != XAIE_SUCCESS) {
			return status;
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for memory module
* for given Column
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	Dir - Direction. Should be one of
*		XAIETILE_EVENT_BLOCK_SOUTH, XAIETILE_EVENT_BLOCK_WEST,
*		XAIETILE_EVENT_BLOCK_NORTH, or XAIETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XAIE_SUCCESS on success
*
* @note		Failure indicates partial column configuration.
*
*******************************************************************************/
u8 XAieGbl_MemEventBroadcastBlockClearColumn(XAieGbl_Tile *TileInstPtr, u8 Dir,
					     u16 Mask)
{
	u16 NumRows = XAieGbl_ConfigTable->NumRows;
	u16 RowIdx;
	XAieGbl_Tile *TilePtr;
	u8 status;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TileInstPtr->RowId == 0);

	for (RowIdx = 1; RowIdx <= NumRows; RowIdx++) {

		TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
			  (RowIdx * sizeof(XAieGbl_Tile)));

		status = XAieTileMem_EventBroadcastBlockClear(TilePtr, Dir,
							      Mask);
		if (status != XAIE_SUCCESS) {
			return status;
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API set all signals blocks in the given Column.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
*
* @return	XAIE_SUCCESS on success
*
* @note		Failure indicates partial column configuration.
*
*******************************************************************************/
u8 XAieGbl_Column_EventBroadcastBlockAll(XAieGbl_Tile *TileInstPtr)
{
	u16 NumRows = XAieGbl_ConfigTable->NumRows;
	u16 RowIdx;
	XAieGbl_Tile *TilePtr;
	u8 Dir;
	u8 status;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(TileInstPtr->RowId == 0);

	/* Set block for AI Tile */
	for (Dir = 0; Dir < 4; Dir++) {
		for (RowIdx = 1; RowIdx <= NumRows; RowIdx++) {

			TilePtr = (XAieGbl_Tile *)((u64)TileInstPtr +
				(RowIdx * sizeof(XAieGbl_Tile)));

			status = XAieTileCore_EventBroadcastBlockSet(TilePtr,
					Dir, XAIETILE_EVENT_BLOCK_ALL_MASK);
			if (status != XAIE_SUCCESS) {
				return status;
			}

			status = XAieTileMem_EventBroadcastBlockSet(TilePtr,
					Dir, XAIETILE_EVENT_BLOCK_ALL_MASK);
			if (status != XAIE_SUCCESS) {
				return status;
			}
		}
	}

	/* Shim Tile */
	TilePtr = (XAieGbl_Tile *)(u64)TileInstPtr;

	for (Dir = 0; Dir < 4; Dir++) {
		XAieTilePl_EventBroadcastBlockSet(TilePtr, Dir,
					XAIETILE_EVENT_BLOCK_SWITCHA,
					XAIETILE_EVENT_BLOCK_ALL_MASK);
		XAieTilePl_EventBroadcastBlockSet(TilePtr, Dir,
					XAIETILE_EVENT_BLOCK_SWITCHB,
					XAIETILE_EVENT_BLOCK_ALL_MASK);
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is used to read the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for Memory. This will be One of
*			  XAIETILE_GROUP_EVENT_MEM_*
*
* @return	current value of the Grour Event register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemGroupEventGet(XAieGbl_Tile *TileInstPtr, u8 groupId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 8);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_MEM_EVTGRP0ENA + groupId * 0x4U);

	return XAie_GetField(RegVal, XAIETILE_GROUP_EVENT_LSB,
			GroupEvents[XAIETILE_EVENT_MODULE_MEM].Mask[groupId]);
}

/*****************************************************************************/
/**
*
* This API is used to read the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for PL. This will be one of
*			  XAIETILE_GROUP_EVENT_PL_*
*
* @return	current value of the Grour Event register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlGroupEventGet(XAieGbl_Tile *TileInstPtr, u8 groupId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 7);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_PL_EVTGRP0ENA + groupId * 0x4U);

	return XAie_GetField(RegVal, XAIETILE_GROUP_EVENT_LSB,
			GroupEvents[XAIETILE_EVENT_MODULE_PL].Mask[groupId]);
}

/*****************************************************************************/
/**
*
* This API is used to read the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for Core. This will be one of
*			  XAIETILE_GROUP_EVENT_CORE_*
*
* @return	current value of the Grour Event register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreGroupEventGet(XAieGbl_Tile *TileInstPtr, u8 groupId)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 9);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_CORE_EVTGRP0ENA + groupId * 0x4U);

	return XAie_GetField(RegVal, XAIETILE_GROUP_EVENT_LSB,
			GroupEvents[XAIETILE_EVENT_MODULE_CORE].Mask[groupId]);
}

/*****************************************************************************/
/**
*
* This API is used to set the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for Memory. This will be one of
*			  XAIETILE_GROUP_EVENT_MEM_*
* @param	Mask - 32 bit value to mask.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemGroupEventSet(XAieGbl_Tile *TileInstPtr, u8 groupId, u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 8);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_EVTGRP0ENA +
			groupId * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API is used to set the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for PL. This will be one of
*			  XAIETILE_GROUP_EVENT_PL_*
* @param	Mask - 32 bit value to mask.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlGroupEventSet(XAieGbl_Tile *TileInstPtr, u8 groupId, u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 7);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_EVTGRP0ENA +
			groupId * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API is used to set the Group Event register of the corresponding group.
*
* @param	TileInstPtr - Pointer to the Tile instance. This has to be
*		the shim tile of the corresponding column.
* @param	groupId - Group Event ID for Core. This will be one of
*			  XAIETILE_GROUP_EVENT_CORE_*
* @param	Mask - 32 bit value to mask.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreGroupEventSet(XAieGbl_Tile *TileInstPtr, u8 groupId, u32 Mask)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(groupId <= 9);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_EVTGRP0ENA +
			groupId * 0x4U, Mask);
}

/*****************************************************************************/
/**
*
* This API returns the event values of the Memory Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of memory combo event inputs register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemComboEventInputGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTINP);
}

/*****************************************************************************/
/**
*
* This API returns the event values of the PL Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of PL combo event inputs register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlComboEventInputGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTINP);
}

/*****************************************************************************/
/**
*
* This API returns the event values of the Core Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of core combo event inputs register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreComboEventInputGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTINP);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the Memory Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - 32 bit combination of Event numbers.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemComboEventInputSet32(XAieGbl_Tile *TileInstPtr, u32 Event)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTINP,
			Event & XAIEGBL_MEM_COMEVTINPMSK);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the PL Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - 32 bit combination of Event numbers.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlComboEventInputSet32(XAieGbl_Tile *TileInstPtr, u32 Event)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTINP,
			Event & XAIEGBL_PL_COMEVTINPMSK);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the Core Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - 32 bit combination of Event numbers.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreComboEventInputSet32(XAieGbl_Tile *TileInstPtr, u32 Event)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTINP,
			Event & (XAIEGBL_CORE_COMEVTINP_EVTD_MASK |
				 XAIEGBL_CORE_COMEVTINP_EVTC_MASK |
				 XAIEGBL_CORE_COMEVTINP_EVTB_MASK |
				 XAIEGBL_CORE_COMEVTINP_EVTA_MASK));
}

/*****************************************************************************/
/**
*
* This API returns the event values of the Memory Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of memory combo event control register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_MemComboEventControlGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTCTRL);
}

/*****************************************************************************/
/**
*
* This API returns the event values of the PL Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of PL combo event control register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlComboEventControlGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTCTRL);
}

/*****************************************************************************/
/**
*
* This API returns the event values of the Core Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Current value of core combo event control register
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreComboEventControlGet32(XAieGbl_Tile *TileInstPtr)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	return XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTCTRL);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the Memory Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Combo - Combination of event combo.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_MemComboEventControlSet32(XAieGbl_Tile *TileInstPtr, u32 Combo)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTCTRL,
			Combo & XAIEGBL_MEM_COMEVTCTRLMSK);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the PL Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Combo - Combination of event combo.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_PlComboEventControlSet32(XAieGbl_Tile *TileInstPtr, u32 Combo)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTCTRL,
			Combo & XAIEGBL_PL_COMEVTCTRLMSK);
}

/*****************************************************************************/
/**
*
* This API sets the event values of the Core Combo Event Control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Combo - Combination of event combo.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreComboEventControlSet32(XAieGbl_Tile *TileInstPtr, u32 Combo)
{
	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTCTRL,
			Combo & (XAIEGBL_CORE_COMEVTCTRL_COM2_MASK |
				 XAIEGBL_CORE_COMEVTCTRL_COM1_MASK |
				 XAIEGBL_CORE_COMEVTCTRL_COM0_MASK));
}

/*****************************************************************************/
/**
*
* This API sets the values of the memory Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
* To keep an entry unaltered pass 8 bit value.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventA - Event Number. One of XAIETILE_EVENT_MEM_*
* @param	eventB - Event Number. One of XAIETILE_EVENT_MEM_*
* @param	eventC - Event Number. One of XAIETILE_EVENT_MEM_*
* @param	eventD - Event Number. One of XAIETILE_EVENT_MEM_*
*
* @return	None.
*
* @note		Values greater than 0x7F is ignored.
*
*******************************************************************************/
void XAieTile_MemComboEventInputSet(XAieGbl_Tile *TileInstPtr, u8 eventA,
				    u8 eventB, u8 eventC, u8 eventD)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	if (eventD <= 0x7F) {
		Mask |= XAIEGBL_MEM_COMEVTINP_EVTD_MASK;
	}
	if (eventC <= 0x7F) {
		Mask |= XAIEGBL_MEM_COMEVTINP_EVTC_MASK;
	}
	if (eventB <= 0x7F) {
		Mask |= XAIEGBL_MEM_COMEVTINP_EVTB_MASK;
	}
	if (eventA <= 0x7F) {
		Mask |= XAIEGBL_MEM_COMEVTINP_EVTA_MASK;
	}

	EventSet = XAIEGBL_MEM_VALUE_COMEVTINP(eventD, eventC, eventB, eventA);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_MEM_COMEVTINP) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTINP,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API sets the values of the PL Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
* To keep an entry unaltered pass 8 bit value.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventA - Event Number. One of XAIETILE_EVENT_PL_*
* @param	eventB - Event Number. One of XAIETILE_EVENT_PL_*
* @param	eventC - Event Number. One of XAIETILE_EVENT_PL_*
* @param	eventD - Event Number. One of XAIETILE_EVENT_PL_*
*
* @return	None.
*
* @note		Values greater than 0x7F is ignored.
*
*******************************************************************************/
void XAieTile_PlComboEventInputSet(XAieGbl_Tile *TileInstPtr, u8 eventA,
				   u8 eventB, u8 eventC, u8 eventD)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	if (eventD <= 0x7F) {
		Mask |= XAIEGBL_PL_COMEVTINP_EVTD_MASK;
	}
	if (eventC <= 0x7F) {
		Mask |= XAIEGBL_PL_COMEVTINP_EVTC_MASK;
	}
	if (eventB <= 0x7F) {
		Mask |= XAIEGBL_PL_COMEVTINP_EVTB_MASK;
	}
	if (eventA <= 0x7F) {
		Mask |= XAIEGBL_PL_COMEVTINP_EVTA_MASK;
	}

	EventSet = XAIEGBL_PL_VALUE_COMEVTINP(eventD, eventC, eventB, eventA);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_PL_COMEVTINP) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTINP,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API sets the values of the Core Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
* To keep an entry unaltered pass 8 bit value.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventA - Event Number. One of XAIETILE_EVENT_CORE_*
* @param	eventB - Event Number. One of XAIETILE_EVENT_CORE_*
* @param	eventC - Event Number. One of XAIETILE_EVENT_CORE_*
* @param	eventD - Event Number. One of XAIETILE_EVENT_CORE_*
*
* @return	None.
*
* @note		Values greater than 0x7F is ignored.
*
*******************************************************************************/
void XAieTile_CoreComboEventInputSet(XAieGbl_Tile *TileInstPtr, u8 eventA,
				     u8 eventB, u8 eventC, u8 eventD)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	if (eventD <= 0x7F) {
		Mask |= XAIEGBL_CORE_COMEVTINP_EVTD_MASK;
	}
	if (eventC <= 0x7F) {
		Mask |= XAIEGBL_CORE_COMEVTINP_EVTC_MASK;
	}
	if (eventB <= 0x7F) {
		Mask |= XAIEGBL_CORE_COMEVTINP_EVTB_MASK;
	}
	if (eventA <= 0x7F) {
		Mask |= XAIEGBL_CORE_COMEVTINP_EVTA_MASK;
	}

	EventSet = XAIEGBL_CORE_VALUE_COMEVTINP(eventD, eventC, eventB, eventA);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_CORE_COMEVTINP) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTINP,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API sets the values of the Memory Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventIdx - Event Index. One of
		XAIETILE_COMBO_EVENT_A, XAIETILE_COMBO_EVENT_B,
		XAIETILE_COMBO_EVENT_C, XAIETILE_COMBO_EVENT_D.
*
* @return	Event Number.
*
* @note		None.
*
*******************************************************************************/
u8  XAieTile_MemComboEventInputGet(XAieGbl_Tile *TileInstPtr, u8 eventIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(eventIdx <= 3);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTINP);

	return XAie_GetField(RegVal,
		ComboEventInput[XAIETILE_EVENT_MODULE_MEM].Lsb[eventIdx],
		ComboEventInput[XAIETILE_EVENT_MODULE_MEM].Mask[eventIdx]);
}

/*****************************************************************************/
/**
*
* This API sets the values of the PL Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventIdx - Event Index. One of
		XAIETILE_COMBO_EVENT_A, XAIETILE_COMBO_EVENT_B,
		XAIETILE_COMBO_EVENT_C, XAIETILE_COMBO_EVENT_D.
*
* @return	Event Number.
*
* @note		None.
*
*******************************************************************************/
u8  XAieTile_PlComboEventInputGet(XAieGbl_Tile *TileInstPtr, u8 eventIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(eventIdx <= 3);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTINP);

	return XAie_GetField(RegVal,
		ComboEventInput[XAIETILE_EVENT_MODULE_PL].Lsb[eventIdx],
		ComboEventInput[XAIETILE_EVENT_MODULE_PL].Mask[eventIdx]);
}

/*****************************************************************************/
/**
*
* This API sets the values of the Core Combo Event inputs. Bit map:
* eventD (30 : 24), eventC (22 : 16), eventB (14 : 8), eventA (6 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	eventIdx - Event Index. One of
		XAIETILE_COMBO_EVENT_A, XAIETILE_COMBO_EVENT_B,
		XAIETILE_COMBO_EVENT_C, XAIETILE_COMBO_EVENT_D.
*
* @return	Event Number.
*
* @note		None.
*
*******************************************************************************/
u8  XAieTile_CoreComboEventInputGet(XAieGbl_Tile *TileInstPtr, u8 eventIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(eventIdx <= 3);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTINP);

	return XAie_GetField(RegVal,
		ComboEventInput[XAIETILE_EVENT_MODULE_CORE].Lsb[eventIdx],
		ComboEventInput[XAIETILE_EVENT_MODULE_CORE].Mask[eventIdx]);
}

/*****************************************************************************/
/**
*
* This API sets the values of the memory Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
* To keep an entry unaltered pass value greater than 3.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	combo0 - control value for combo0.
* @param	combo1 - control value for combo1.
* @param	combo2 - control value for combo2.
*
* @return	None.
*
* @note		Values greater than 0x3 will be ignored.
*
*******************************************************************************/
void XAieTile_MemComboEventControlSet(XAieGbl_Tile *TileInstPtr, u8 combo0,
				      u8 combo1, u8 combo2)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	if (combo0 <= 0x3) {
		Mask |= XAIEGBL_MEM_COMEVTCTRL_COM0_MASK;
	}
	if (combo1 <= 0x3) {
		Mask |= XAIEGBL_MEM_COMEVTCTRL_COM1_MASK;
	}
	if (combo2 <= 0x3) {
		Mask |= XAIEGBL_MEM_COMEVTCTRL_COM2_MASK;
	}

	EventSet = XAIEGBL_MEM_VALUE_COMEVTCTRL(combo2, combo1, combo0);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_MEM_COMEVTCTRL) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTCTRL,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API sets the values of the PL Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
* To keep an entry unaltered pass value greater than 3.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	combo0 - control value for combo0.
* @param	combo1 - control value for combo1.
* @param	combo2 - control value for combo2.
*
* @return	None.
*
* @note		Values greater than 0x3 will be ignored.
*
*******************************************************************************/
void XAieTile_PlComboEventControlSet(XAieGbl_Tile *TileInstPtr, u8 combo0,
				      u8 combo1, u8 combo2)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);

	if (combo0 <= 0x3) {
		Mask |= XAIEGBL_PL_COMEVTCTRL_COM0_MASK;
	}
	if (combo1 <= 0x3) {
		Mask |= XAIEGBL_PL_COMEVTCTRL_COM1_MASK;
	}
	if (combo2 <= 0x3) {
		Mask |= XAIEGBL_PL_COMEVTCTRL_COM2_MASK;
	}

	EventSet = XAIEGBL_PL_VALUE_COMEVTCTRL(combo2, combo1, combo0);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_PL_COMEVTCTRL) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTCTRL,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API sets the values of the Core Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
* To keep an entry unaltered pass value greater than 3.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	combo0 - control value for combo0.
* @param	combo1 - control value for combo1.
* @param	combo2 - control value for combo2.
*
* @return	None.
*
* @note		Values greater than 0x3 will be ignored.
*
*******************************************************************************/
void XAieTile_CoreComboEventControlSet(XAieGbl_Tile *TileInstPtr, u8 combo0,
				      u8 combo1, u8 combo2)
{
	u32 RegVal, EventSet;
	u32 Mask = 0;

	XAie_AssertVoid(TileInstPtr != XAIE_NULL);
	XAie_AssertVoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);

	if (combo0 <= 0x3) {
		Mask |= XAIEGBL_CORE_COMEVTCTRL_COM0_MASK;
	}
	if (combo1 <= 0x3) {
		Mask |= XAIEGBL_CORE_COMEVTCTRL_COM1_MASK;
	}
	if (combo2 <= 0x3) {
		Mask |= XAIEGBL_CORE_COMEVTCTRL_COM2_MASK;
	}

	EventSet = XAIEGBL_CORE_VALUE_COMEVTCTRL(combo2, combo1, combo0);
	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_CORE_COMEVTCTRL) & (~Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr + XAIEGBL_CORE_COMEVTCTRL,
			(RegVal | (EventSet & Mask)));
}

/*****************************************************************************/
/**
*
* This API returns the values of the memory Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	comboIdx - Combo index. One of
*		XAIETILE_COMBO_0, XAIETILE_COMBO_1, XAIETILE_COMBO_2.
*
* @return	Curent Value of combo control for comboIdx
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_MemComboEventControlGet(XAieGbl_Tile *TileInstPtr, u8 comboIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(comboIdx <= 2);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_MEM_COMEVTCTRL);

	return XAie_GetField(RegVal,
		ComboEventControl[XAIETILE_EVENT_MODULE_MEM].Lsb[comboIdx],
		ComboEventControl[XAIETILE_EVENT_MODULE_MEM].Mask[comboIdx]);
}

/*****************************************************************************/
/**
*
* This API returns the values of the PL Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	comboIdx - Combo index. One of
*		XAIETILE_COMBO_0, XAIETILE_COMBO_1, XAIETILE_COMBO_2.
*
* @return	Curent Value of combo control for comboIdx
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_PlComboEventControlGet(XAieGbl_Tile *TileInstPtr, u8 comboIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(comboIdx <= 2);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + XAIEGBL_PL_COMEVTCTRL);

	return XAie_GetField(RegVal,
		ComboEventControl[XAIETILE_EVENT_MODULE_PL].Lsb[comboIdx],
		ComboEventControl[XAIETILE_EVENT_MODULE_PL].Mask[comboIdx]);
}

/*****************************************************************************/
/**
*
* This API returns the values of the Core Combo Event control. Bit map:
* combo2 (17 : 16), combo1 (9 : 8), combo0 (1 : 0).
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	comboIdx - Combo index. One of
*		XAIETILE_COMBO_0, XAIETILE_COMBO_1, XAIETILE_COMBO_2.
*
* @return	Curent Value of combo control for comboIdx
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreComboEventControlGet(XAieGbl_Tile *TileInstPtr, u8 comboIdx)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(comboIdx <= 2);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				XAIEGBL_CORE_COMEVTCTRL);

	return XAie_GetField(RegVal,
		ComboEventControl[XAIETILE_EVENT_MODULE_CORE].Lsb[comboIdx],
		ComboEventControl[XAIETILE_EVENT_MODULE_CORE].Mask[comboIdx]);
}


/*****************************************************************************/
/**
*
* This API returns the port ID selected on the given PortNo for Core module:
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	PortNo - Port index. One of
*		XAIETILE_STRSW_EVENT_PORT_0,
*		XAIETILE_STRSW_EVENT_PORT_1,
*		XAIETILE_STRSW_EVENT_PORT_2,
*		XAIETILE_STRSW_EVENT_PORT_3,
*		XAIETILE_STRSW_EVENT_PORT_4,
*		XAIETILE_STRSW_EVENT_PORT_5,
*		XAIETILE_STRSW_EVENT_PORT_6,
*		XAIETILE_STRSW_EVENT_PORT_7.
*
* @return	port ID.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreStrmSwEventPortSelectGet(XAieGbl_Tile *TileInstPtr, u8 PortNo)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(PortNo < XAIETILE_STRSW_EVENT_PORT_NUMBER);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				StrmSwEventPortSelect[0].RegAddr +
				(PortNo / 4) * 0x4U);

	return XAie_GetField(RegVal,
			     StrmSwEventPortSelect[0].PortIndex[PortNo].Lsb,
			     StrmSwEventPortSelect[0].PortIndex[PortNo].Mask);
}


/*****************************************************************************/
/**
*
* This API returns the port ID selected on the given PortNo for PL module:
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	PortNo - Port index. One of
*		XAIETILE_STRSW_EVENT_PORT_0,
*		XAIETILE_STRSW_EVENT_PORT_1,
*		XAIETILE_STRSW_EVENT_PORT_2,
*		XAIETILE_STRSW_EVENT_PORT_3.
*		XAIETILE_STRSW_EVENT_PORT_4,
*		XAIETILE_STRSW_EVENT_PORT_5,
*		XAIETILE_STRSW_EVENT_PORT_6,
*		XAIETILE_STRSW_EVENT_PORT_7.
*
* @return	port ID.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_PlStrmSwEventPortSelectGet(XAieGbl_Tile *TileInstPtr, u8 PortNo)
{
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(PortNo < XAIETILE_STRSW_EVENT_PORT_NUMBER);

	RegVal = XAieGbl_Read32(TileInstPtr->TileAddr +
				StrmSwEventPortSelect[1].RegAddr +
				(PortNo / 4) * 0x4U);

	return XAie_GetField(RegVal,
			     StrmSwEventPortSelect[1].PortIndex[PortNo].Lsb,
			     StrmSwEventPortSelect[1].PortIndex[PortNo].Mask);
}


/*****************************************************************************/
/**
*
* This API sets the port ID on the given PortNo for Core module:
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	PortNo -Port index. One of
*		XAIETILE_STRSW_EVENT_PORT_0,
*		XAIETILE_STRSW_EVENT_PORT_1,
*		XAIETILE_STRSW_EVENT_PORT_2,
*		XAIETILE_STRSW_EVENT_PORT_3.
*		XAIETILE_STRSW_EVENT_PORT_4,
*		XAIETILE_STRSW_EVENT_PORT_5,
*		XAIETILE_STRSW_EVENT_PORT_6,
*		XAIETILE_STRSW_EVENT_PORT_7.
* @param	PortType - Type of the port. (master = 1, slave = 0)
* @param	PortID - port ID selected.
*
* @return	XAIE_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreStrmSwEventPortSelectSet(XAieGbl_Tile *TileInstPtr, u8 PortNo,
					 u8 PortType, u8 PortID)
{
	u32 CurrVal, RegVal;
	u32 Mask;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(PortNo < XAIETILE_STRSW_EVENT_PORT_NUMBER);
	XAie_AssertNonvoid(PortType == 0 || PortType == 1);

	Mask = StrmSwEventPortSelect[0].PortIndex[PortNo].Mask |
	       StrmSwEventPortSelect[0].PortMode[PortNo].Mask;


	CurrVal = ~Mask & XAieGbl_Read32(TileInstPtr->TileAddr +
					   StrmSwEventPortSelect[0].RegAddr +
					   (PortNo / 4) * 0x4U);

	RegVal = XAie_SetField(PortID,
			       StrmSwEventPortSelect[0].PortIndex[PortNo].Lsb,
			       StrmSwEventPortSelect[0].PortIndex[PortNo].Mask);
	RegVal |= XAie_SetField(PortType,
				StrmSwEventPortSelect[0].PortMode[PortNo].Lsb,
				StrmSwEventPortSelect[0].PortMode[PortNo].Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			StrmSwEventPortSelect[0].RegAddr +
			(PortNo / 4) * 0x4U, CurrVal | RegVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the port ID on the given PortNo for PL module:
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	PortNo -Port index. One of
*		XAIETILE_STRSW_EVENT_PORT_0,
*		XAIETILE_STRSW_EVENT_PORT_1,
*		XAIETILE_STRSW_EVENT_PORT_2,
*		XAIETILE_STRSW_EVENT_PORT_3.
*		XAIETILE_STRSW_EVENT_PORT_4,
*		XAIETILE_STRSW_EVENT_PORT_5,
*		XAIETILE_STRSW_EVENT_PORT_6,
*		XAIETILE_STRSW_EVENT_PORT_7.
* @param	PortType - Type of the port. (master = 1, slave = 0)
* @param	PortID - port ID selected.
*
* @return	XAIE_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_PlStrmSwEventPortSelectSet(XAieGbl_Tile *TileInstPtr, u8 PortNo,
				       u8 PortType, u8 PortID)
{
	u32 CurrVal, RegVal;
	u32 Mask;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(PortNo < XAIETILE_STRSW_EVENT_PORT_NUMBER);
	XAie_AssertNonvoid(PortType == 0 || PortType == 1);

	Mask = StrmSwEventPortSelect[1].PortIndex[PortNo].Mask |
	       StrmSwEventPortSelect[1].PortMode[PortNo].Mask;

	CurrVal = ~Mask & XAieGbl_Read32(TileInstPtr->TileAddr +
					   StrmSwEventPortSelect[1].RegAddr +
					   (PortNo / 4) * 0x4U);

	RegVal = XAie_SetField(PortID,
			       StrmSwEventPortSelect[1].PortIndex[PortNo].Lsb,
			       StrmSwEventPortSelect[1].PortIndex[PortNo].Mask);
	RegVal |= XAie_SetField(PortType,
				StrmSwEventPortSelect[1].PortMode[PortNo].Lsb,
				StrmSwEventPortSelect[1].PortMode[PortNo].Mask);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			StrmSwEventPortSelect[1].RegAddr +
			(PortNo / 4) * 0x4U, CurrVal | RegVal);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API returns 32-bit register value of the Stream Switch Event Port
* Selection (0/1) register for core module. (batch read)
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SelectID - register index (0 - Selection_0, 1 - Selection_1).
*
* @return	Core stream Switch event port selection register value.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_CoreStrmSwEventPortSelectGet32(XAieGbl_Tile *TileInstPtr,
					    u8 SelectID)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SelectID == 0 || SelectID == 1);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			      StrmSwEventPortSelect[0].RegAddr +
			      SelectID * 0x4U);
}

/*****************************************************************************/
/**
* This API returns 32-bit register value of the Stream Switch Event Port
* Selection (0/1) register for PL module. (batch read)
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SelectID - register index (0 - Selection_0, 1 - Selection_1).
*
* @return	PL stream Switch event port selection register value.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTile_PlStrmSwEventPortSelectGet32(XAieGbl_Tile *TileInstPtr,
					  u8 SelectID)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SelectID == 0 || SelectID == 1);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			      StrmSwEventPortSelect[1].RegAddr +
			      SelectID * 0x4U);
}

/*****************************************************************************/
/**
*
* This API sets the 32-bit register value of the Stream Switch Event Port
* Selection register for core module. (batch write)
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SelectID - register index (0 - Selection_0, 1 - Selection_1).
* @param	RegVal - 32 bit value to be written.
*
* @return	XAIE_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreStrmSwEventPortSelectSet32(XAieGbl_Tile *TileInstPtr,
					   u8 SelectID, u32 RegVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SelectID == 0 || SelectID == 1);

	u32 Mask = XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0ID_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT1ID_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT2ID_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT3ID_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_CORE_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK;

	XAieGbl_Write32(TileInstPtr->TileAddr +
			StrmSwEventPortSelect[0].RegAddr + SelectID * 0x4U,
			RegVal & Mask);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the 32-bit register value of the Stream Switch Event Port
* Selection register for PL module. (batch write)
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	SelectID - register index (0 - Selection_0, 1 - Selection_1).
* @param	RegVal - 32 bit value to be written.
*
* @return	XAIE_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_PlStrmSwEventPortSelectSet32(XAieGbl_Tile *TileInstPtr,
					 u8 SelectID, u32 RegVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType != XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(SelectID == 0 || SelectID == 1);

	u32 Mask = XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0ID_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT1ID_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT2ID_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT3ID_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK |
		   XAIEGBL_PL_STRSWIEVTPORTSEL0_PORT0MSTRSLV_MASK;

	XAieGbl_Write32(TileInstPtr->TileAddr +
			StrmSwEventPortSelect[1].RegAddr + SelectID * 0x4U,
			RegVal & Mask);

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This is API is to check the events raised from a module
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Location of the shim tile
 * @param	Module - Module type, PL, Memory or Core
 *
 * @return	1 - there are broadcast events from tiles above.
 *		0 - there are no broadcast events from tiles above.
 *
 * @note	API will check the events from the events status registers.
 *		If there are events raised, and callback registered, it will
 *		call the callback and will clear the event.
 *		It will also check the broadcast events to see if there are
 *		events generated from the above tiles.
 *
 *		Used only in this file.
 *****************************************************************************/
static u8 XAieTile_EventsModuleCheck(XAieGbl *AieInst, XAie_LocType Loc,
				     u32 Module)
{
	XAieGbl_Tile *TilePtr;
	u8 NumRows, NeedToCheckAbove;
	u32 RegOff, RegVal, BCEventsMask, BCEventsStart;
	u64 RegAddr;
	XAieGbl_EventHandler *EvtHandler;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);

	TilePtr = AieInst->Tiles;
	NumRows = AieInst->Config->NumRows;
	TilePtr += Loc.Col * (NumRows + 1) + Loc.Row;

	RegAddr = TilePtr->TileAddr;
	if (Module == XAIEGBL_MODULE_CORE) {
		RegOff = XAIEGBL_CORE_EVTSTA0;
		EvtHandler = AieInst->CoreEvtHandlers;
		BCEventsStart = XAIETILE_EVENT_CORE_GROUP_BROADCAST;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		RegOff = XAIEGBL_MEM_EVTSTA0;
		EvtHandler = AieInst->MemEvtHandlers;
		BCEventsStart = XAIETILE_EVENT_MEM_GROUP_BROADCAST;
	} else {
		RegOff = XAIEGBL_PL_EVTSTA0;
		EvtHandler = AieInst->ShimEvtHandlers;
		BCEventsStart = XAIETILE_EVENT_SHIM_GROUP_BROADCAST_A;
	}
	BCEventsMask = 0;
	for (u32 i = BCEventsStart;
	     i <= BCEventsStart + XAIETILE_EVENTS_BROADCAST_MAX + 1; i++) {
		BCEventsMask |= 1 << (i % 32);
	}
	RegAddr += (u64)RegOff;
	NeedToCheckAbove = 0;
	/* Scan through the 4 event registers */
	for (u32 i = 0, EvtIdOff = 0; i <= 3; i++, RegAddr += 4) {
		u32 ValTmp = 0;

		RegVal = XAieGbl_Read32(RegAddr);
		if (RegVal == 0) {
			EvtIdOff += 32;
			continue;
		}
		XAie_print("%s: (%u,%u),(%u,%u):%u:StatusReg[%d]: 0x%08x.\n",
			   __func__, Loc.Col, Loc.Row,
			   TilePtr->ColId, TilePtr->RowId, Module, i, RegVal);
		for (u32 j = 0; j < 32; j++, EvtIdOff++) {
			/* Event0 is none, and Event1 is True */
			if (EvtIdOff <= 2) {
				continue;
			}
			if ((RegVal & (1 << j)) == 0) {
				continue;
			}
			if (EvtHandler[EvtIdOff].Cb != XAIE_NULL) {
				/* If callback is registered, call the callback */
				ValTmp |= 1 << j;
				EvtHandler[EvtIdOff].Cb(AieInst, Loc, Module,
						   EvtIdOff,
						   EvtHandler[EvtIdOff].Arg);
			}
		}
		if (i == (BCEventsStart / 32)) {
			/* Check broadcast events to see if there
			 * are events from the above tiles. */
			ValTmp |= BCEventsMask;
			if ((RegVal & BCEventsMask) != 0) {
				NeedToCheckAbove = 1;
			}
		}
		/* Clear events */
		XAieGbl_Write32(RegAddr, ValTmp);
	}
	return NeedToCheckAbove;
}

/*****************************************************************************/
/**
 * This is API is to check the events raised from a shim tile
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Location of the shim tile
 * @param	Intr1stStatus - Status value of the 1st interrupt controller.
 *
 * @return	None.
 *
 * @note	Used only this file.
 *****************************************************************************/
static void XAieTile_EventsShimCheck(XAieGbl *AieInst, XAie_LocType Loc,
				     u32 Intr1stStatus)
{
	XAieGbl_Tile *TilePtr;
	u8 NumRows;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);

	TilePtr = AieInst->Tiles;
	NumRows = AieInst->Config->NumRows;
	TilePtr += Loc.Col * (NumRows + 1);
	if ((Intr1stStatus & (0xFU << 16)) == 0) {
		return;
	}
	XAieTile_EventsModuleCheck(AieInst, Loc, XAIEGBL_MODULE_PL);
}

/*****************************************************************************/
/**
 * This is API is to check the events raised from the tiles in a column
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Col - Column to check
 * @param	SwitchAB - Which subcolumn
 *
 * @return	None.
 *
 * @note	Used only this file.
 *****************************************************************************/
static void XAieTile_CheckColEvents(XAieGbl *AieInst, u8 Col, u8 SwitchAB)
{
	XAieGbl_Tile *TilePtr;
	u8 NumCols, NumRows, i;
	u32 RegVal;
	XAie_LocType Loc;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_AssertVoid(Col < NumCols);

	TilePtr = AieInst->Tiles;
	NumRows = AieInst->Config->NumRows;
	TilePtr += Col * (NumRows + 1);
	RegVal = XAieTile_PlIntcL1StatusGet(TilePtr, SwitchAB);
	if (RegVal == 0) {
		/* Not events from this block */
		return;
	}
	XAie_print("%s, Col %u: Switch %u, Status: 0x%x\n",
		   __func__, Col, SwitchAB, RegVal);
	Loc.Col = Col;
	/* Clear the 1st interrupt controller registers first.
	 * Otherwise, it can loose interrupt. */
	XAieTile_PlIntcL1StatusClr(TilePtr, RegVal, SwitchAB);
	if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
		/* Only Switch A need to check SHIM internal events */
		Loc.Row = 0;
		XAieTile_EventsShimCheck(AieInst, Loc, RegVal);
	}
	if ((RegVal & XAIETILE_EVENTS_BROADCAST_MASK) == 0) {
		/* No events generated for the column */
		return;
	}

	for (i = 0; i < NumRows; i++) {
		u8 Module, NeedToCheckAbove;
		u32 ClockCntrlRegVal;
		u64 RegAddr;

		if (SwitchAB == XAIETILE_PL_BLOCK_SWITCHA) {
			Module = XAIEGBL_MODULE_CORE;
		} else {
			Module = XAIEGBL_MODULE_MEM;
		}
		Loc.Row = i + 1;
		XAie_print("%s, check module, (%d,%d).\n", __func__,
			   Loc.Col, Loc.Row);
		NeedToCheckAbove = XAieTile_EventsModuleCheck(AieInst, Loc,
							      Module);
		if (NeedToCheckAbove == 0) {
			/* If no events generated from the tiles above,
			 * stop scanning from this column.*/
			break;
		}
		TilePtr = AieInst->Tiles;
		TilePtr += Col * (NumRows + 1) + i;
		RegAddr = TilePtr->TileAddr;
		ClockCntrlRegVal = XAieGbl_Read32(RegAddr + XAIEGBL_CORE_TILCLOCTRL);
		/* check if the tile above is gated */
		if(!(ClockCntrlRegVal & XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
			break;
	}
}

/*****************************************************************************/
/**
 * This is the AIE interrupt handler.
 * It will check which events from which tiles rasied the interrupt.
 * It will call the registered interrupt handler for the event.
 *
 * @param	Data - Pointer to the AIE device instance
 *
 * @return	None.
 *
 * @note	For Baremetal, application will need to register this
 *		handler to interrupt controller.
 *		For Linux, the XAieTile_EventsHandlingInitialize() function
 *		will register this handler, user doesn't need to register
 *		this handler.
 *****************************************************************************/
void XAieTile_EventsIsr(void *Data)
{
	u32 NumCols, NumRows;
	XAieGbl_Tile *TilePtr;
	u32 RegVal;
	XAieGbl *AieInst = Data;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	TilePtr = AieInst->Tiles;
	/* Scan 2nd level interrupt controller */
	XAie_print("%s.\n", __func__);
	/* Disable NPI Interrupts first */
	XAieGbl_NPIWrite32(XAIE_NPI_IDR1, (1 << XAIETILE_EVENT_NPI_INTERRUPT));
	/* Scan through 2nd interrup level controller */
	for (u32 c = 0; c < NumCols; c++, TilePtr += NumRows + 1) {
		if (TilePtr->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
			continue;
		}
		/* Check if events are rasied from 2nd level intr handler */
		RegVal = XAieTile_NoCIntcL2StatusGet(TilePtr);
		XAie_print("%s:(%u,%u): 2nd Level 0x%x.\n",
			   __func__, c, 0, RegVal);
		if (RegVal == 0) {
			continue;
		}
		/* Check which columns caused the interrupt.
		 * Please see the function XAieTile_Cal1stIrqNo() to
		 * check how to identify each column.
		 */
		if ((c % 2) == 0) {
			/* SHIM NOC on the left */
			for (u32 i = 0; i < 6; i++) {
				/* Check the 2 no SHIM NOC columns on its left
				 * and itself */
				if ((RegVal & (1 << i)) != 0) {
					u8 Col, SwitchAB;

					Col = c - 2 + (i / 2);
					if ((i % 2) == 0) {
						SwitchAB = XAIETILE_PL_BLOCK_SWITCHA;
					} else {
						SwitchAB = XAIETILE_PL_BLOCK_SWITCHB;
					}
					XAie_print("%s: check column %d,%d\n",
						   __func__, Col, SwitchAB);
					XAieTile_CheckColEvents(AieInst, Col,
								SwitchAB);
				}
			}
		} else {
			/* SHIM NOC on the right */
			for (u32 i = 0; i < 6; i++) {
				/* Check its own column, if it is the last SHIM
				 * NOC on the right, check the no SHIM NOC
				 * columns on its right */
				if ((RegVal & (1 << i)) != 0) {
					u8 Col, SwitchAB;

					Col = c + (i / 2);
					if ((i % 2) == 0) {
						SwitchAB = XAIETILE_PL_BLOCK_SWITCHA;
					} else {
						SwitchAB = XAIETILE_PL_BLOCK_SWITCHB;
					}
					XAie_print("%s: check column %d,%d\n",
						   __func__, Col, SwitchAB);
					XAieTile_CheckColEvents(AieInst, Col,
								SwitchAB);
				}
			}
		}
		XAieTile_NoCIntcL2StatusClear(TilePtr, RegVal);
	}
	XAieGbl_NPIWrite32(XAIE_NPI_ISR, (1 << XAIETILE_EVENT_NPI_INTERRUPT));
	/* Renable NPI Interrupt */
	XAieGbl_NPIWrite32(XAIE_NPI_IER1, (1 << XAIETILE_EVENT_NPI_INTERRUPT));
}

#ifdef __linux__
/*****************************************************************************/
/**
 * This is the AIE libmetal interrupt handler.
 *
 * @param	Data - Pointer to the AIE device instance
 *
 * @return	Always 0 as libmetal IRQ isr requires return.
 *
 *		Used only in this file.
 *****************************************************************************/
static int XAieTile_EventsMetalIsr(void *Data)
{
	XAieTile_EventsIsr(Data);
	return 0;
}
#endif

/*****************************************************************************/
/**
 * This API gets 1st level irq broadcast event id.
 *
 * @param	AieInst - pointer to AIE instance
 * @param	Loc - Shim tile location
 * @param	SwitchAB - Flag to indicate if it's the A or B block.
 *
 * @return	IRQ broadcast event ID.
 *
 * @note	Irq id for each switch block starts from 0, every block on the
 *		left will increase by 1 until it reaches the 1st SHIM NOC column.
 *		the Irq id restarts from 0 on the switch A block on the 2nd
 *		SHIM NOC column. For the SHIM PL columns after the 2nd SHIM NOC,
 *		If there is no SHIM NOC further right, the column will use the
 *		SHIM NOC on the left. That is,
 *		For column from 0 to 43: the 1st irq broadcast event id pattern
 *		is: 0 1 2 3 4 5 0 1, 0 1 2 3 4 5 0 1
 *		For column 44 to 49: 0 1 2 3 4 5 0 1 2 3 4 5
 *
 *		Used only in this file.
 *****************************************************************************/
static u8 XAieTile_Cal1stIrqNo(XAieGbl *AieInst, XAie_LocType Loc, u8 SwitchAB)
{
	u8 Col;
	XAieGbl_Tile *TilePtr;
	u32 NumRows;

	Col = Loc.Col;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);

	TilePtr = AieInst->Tiles;
	NumRows = AieInst->Config->NumRows;
	TilePtr += Col * (NumRows + 1);
	if (TilePtr->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		/* Column without SHIM NOC */
		if (((Col / 4) * 4 + 2) < AieInst->Config->NumCols) {
			/* There is SHIM NOC on the right */
			return (Col % 4) * 2 + SwitchAB;
		} else {
			/* There is no SHIM NOC on the right.
			 * will need to use the SHIN NOC on the left. */
			return (Col % 4) * 2 + SwitchAB + 2;
		}
	} else {
		/* Column with SHIM NOC */
		if ((Col % 4) == 2) {
			/* SHIM NOC on the left */
			return 4 + (u32)SwitchAB;
		} else {
			/* SHIM NOC on the right */
			return (u32)SwitchAB;
		}
	}
}

/*****************************************************************************/
/**
 * This API checks if it can broadcast an event
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Tile location
 * @param	Module - Module type,
 *			 XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_MEM,
 *			 XAIEGBL_MODULE_PL
 *
 * @return	XAIE_SUCCESS for success, XAIE_FAILURE for failure
 *
 * @note	Used only in this file
 *****************************************************************************/
static int _XAieTile_EventBroadcastCheckAvail(XAieGbl *AieInst,
					      XAie_LocType Loc, u8 Module)
{
	XAieGbl_Tile *TilePtr;
	u32 NumCols, NumRows;
	u32 *UsedEvtMask;
	int AvailBit;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertNonvoid(Module == XAIEGBL_MODULE_CORE ||
			   Module == XAIEGBL_MODULE_MEM ||
			   Module == XAIEGBL_MODULE_PL);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_AssertNonvoid(Loc.Row <= NumRows);
	XAie_AssertNonvoid(Loc.Col < NumCols);
	/* Check if there is enough broadcast signal to broadcast event */
	TilePtr = AieInst->Tiles;
	TilePtr += (Loc.Col * (NumRows + 1)) + Loc.Row;
	if (TilePtr->TileType == XAIEGBL_TILE_TYPE_AIETILE &&
	    Module == XAIEGBL_MODULE_PL) {
		XAieLib_IntPrint("%s: failed, Module for SHIM,"
				 "but tile(%u,%u) is not SHIM.\n",
				 __func__, Loc.Col, Loc.Row);
		return XAIE_FAILURE;
	}
	if (TilePtr->TileType != XAIEGBL_TILE_TYPE_AIETILE &&
	    Module != XAIEGBL_MODULE_PL) {
		XAieLib_IntPrint("%s: failed, Module for non SHIM,"
				 "but tile(%u,%u) is SHIM.\n",
				 __func__, Loc.Col, Loc.Row);
		return XAIE_FAILURE;
	}
	/* Verify Tile */
	if (Module == XAIEGBL_MODULE_CORE) {
		UsedEvtMask = &TilePtr->CoreBCUsedMask;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		UsedEvtMask = &TilePtr->MemBCUsedMask;
	} else {
		UsedEvtMask = &TilePtr->PlIntEvtUsedMask;
	}
	/* Check if there is unused broadcast event */
	AvailBit = __builtin_ffs(~(*UsedEvtMask));
	if (Module != XAIEGBL_MODULE_PL) {
		if ((u32)AvailBit > (XAIETILE_EVENTS_BROADCAST_MAX + 1)) {
			XAieLib_IntPrint("%s: failed, (%d, %d) no broadcast.\n",
				   __func__, TilePtr->ColId, TilePtr->RowId);
			return XAIE_FAILURE;
		}
	} else {
		if (AvailBit > 4) {
			XAieLib_IntPrint("%s: failed, (%d, %d) no Int event.\n",
				   __func__, TilePtr->ColId, TilePtr->RowId);
			return XAIE_FAILURE;
		}

	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API checks set up event broadcast or SHIM internal event to broadcast
 * an event.
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Tile location
 * @param	Module - Module type,
 *			 XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_MEM,
 *			 XAIEGBL_MODULE_PL
 * @param	Event - Event ID
 *
 * @return	XAIE_SUCCESS for success, XAIE_FAILURE for failure
 *
 * @note	Before calling this function, caller will need to call
 *		_XAieTile_EventBroadcastCheckAvail() first to check the
 *		availability.
 *
 *		Used only in this file
 *****************************************************************************/
static int _XAieTile_EventSetNotification(XAieGbl *AieInst,
					  XAie_LocType Loc, u8 Module,
					  u8 Event)
{
	XAieGbl_Tile *TilePtr;
	u32 NumCols, NumRows;
	u32 *UsedEvtMask;
	int AvailBit;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertNonvoid(Module == XAIEGBL_MODULE_CORE ||
			   Module == XAIEGBL_MODULE_MEM ||
			   Module == XAIEGBL_MODULE_PL);
	XAie_AssertNonvoid(Event > 1);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_AssertNonvoid(Loc.Row <= NumRows);
	XAie_AssertNonvoid(Loc.Col < NumCols);
	/* Check if there is enough broadcast signal to broadcast event */
	TilePtr = AieInst->Tiles;
	TilePtr += (Loc.Col * (NumRows + 1)) + Loc.Row;
	XAie_AssertNonvoid((TilePtr->TileType == XAIEGBL_TILE_TYPE_AIETILE &&
			   Module != XAIEGBL_MODULE_PL) ||
			   (TilePtr->TileType != XAIEGBL_TILE_TYPE_AIETILE &&
			   Module == XAIEGBL_MODULE_PL));
	if (Module == XAIEGBL_MODULE_CORE) {
		UsedEvtMask = &TilePtr->CoreBCUsedMask;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		UsedEvtMask = &TilePtr->MemBCUsedMask;
	} else {
		UsedEvtMask = &TilePtr->PlIntEvtUsedMask;
	}
	/* Check if there is unused broadcast event */
	AvailBit = __builtin_ffs(~(*UsedEvtMask));
	*UsedEvtMask |= 1 << (AvailBit - 1);
	AvailBit--;
	AvailBit &= 0xFF;
	if (Module == XAIEGBL_MODULE_CORE) {
		XAieLib_print("%s: Core(%u,%u), Event %u.\n", __func__,
			      TilePtr->ColId, TilePtr->RowId, Event);
		XAieTileCore_EventBroadcast(TilePtr, (u8)AvailBit,
					    Event);
	} else if (Module == XAIEGBL_MODULE_MEM) {
		XAieLib_print("%s: Mem(%u,%u), Event %u.\n", __func__,
			      TilePtr->ColId, TilePtr->RowId, Event);
		XAieTileMem_EventBroadcast(TilePtr, (u8)AvailBit,
					   Event);
	} else {
		AvailBit += 16;
		XAieLib_print("%s: Shim(%u,%u), Event %u.\n", __func__,
			      TilePtr->ColId, TilePtr->RowId, Event);
		XAieTile_PlIntcL1IrqEventSet(TilePtr, (u8)AvailBit,
					     Event,
					     XAIETILE_PL_BLOCK_SWITCHA);
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API unregisters for event notification for a module of a tile.
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Tile location
 * @param	Module - Module type,
 *			 XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_MEM,
 *			 XAIEGBL_MODULE_PL, XAIEGBL_MODULE_ALL,
 *			 or any OR combination of CPRE, MEM, and PL
 * @param	Event - Event id, if Event id is XAIETILE_EVENTS_ALL, it will
 *			unregister all the non-error events of the specified
 *			tiles. If Module is XAIEGBL_MODULE_ALL, Event needs to
 *			be XAIETILE_EVENTS_ALL.
 *
 * @return	XAIE_SUCCESS for success, XAIE_FAILURE for failure
 *
 * @note	Used only in this file.
 *****************************************************************************/
static int _XAieTile_EventUnregisterNotification(XAieGbl *AieInst,
						  XAie_LocType Loc, u8 Module,
						  u8 Event)
{
	XAieGbl_Tile *TilePtr;
	u32 NumCols, NumRows, UsedEvtMask, RegVal;
	XAieGbl_EventHandler *Handler;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertNonvoid(Module <= XAIEGBL_MODULE_ALL);
	XAie_AssertNonvoid(Event != 1);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_AssertNonvoid(Loc.Row <= NumRows);
	XAie_AssertNonvoid(Loc.Col < NumCols);
	TilePtr = AieInst->Tiles;
	TilePtr += (Loc.Col * (NumRows + 1)) + Loc.Row;
	if (TilePtr->TileType == XAIEGBL_TILE_TYPE_AIETILE &&
	    Module == XAIEGBL_MODULE_PL) {
		XAieLib_print("INFO: %s: failed, Mudule for SHIM,"
				 "but tile(%u,%u) is not SHIM.\n",
				 __func__, Loc.Col, Loc.Row);
		return XAIE_FAILURE;
	}
	if (TilePtr->TileType != XAIEGBL_TILE_TYPE_AIETILE &&
	    (Module & XAIEGBL_MODULE_PL) == 0) {
		XAieLib_print("INFO: %s: failed, Module for non SHIM,"
				 "but tile(%u,%u) is SHIM.\n",
				 __func__, Loc.Col, Loc.Row);
		return XAIE_FAILURE;
	}
	if ((Module & XAIEGBL_MODULE_CORE)!= 0) {
		UsedEvtMask = TilePtr->CoreBCUsedMask;
		Handler = AieInst->CoreEvtHandlers;
		if ((Event != XAIETILE_EVENTS_ALL) && (Handler[Event].Refs == 0)) {
			return XAIE_SUCCESS;
		}
		for (u8 j = 1; j <= XAIETILE_EVENTS_BROADCAST_MAX;
		     j++) {
			if ((UsedEvtMask & (1 << j)) != 0) {
				RegVal = XAieTile_CoreEventBroadcastGet(TilePtr, j);
				XAie_AssertNonvoid(RegVal <= 127);
				if (RegVal == 0 || Handler[RegVal].Refs == 0) {
					continue;
				}
				if ((RegVal == (u32)Event) ||
				     (Event == XAIETILE_EVENTS_ALL)) {
					XAieTileCore_EventBroadcast(TilePtr, j, 0);
					TilePtr->CoreBCUsedMask &= (~(1U << j));
					Handler[RegVal].Refs--;
					if (Handler[RegVal].Refs == 0) {
						Handler[RegVal].Cb = XAIE_NULL;
						Handler[RegVal].Arg = XAIE_NULL;
					}
					if (Event != XAIETILE_EVENTS_ALL) {
						break;
					}
				}
			}
		}
	}
	if ((Module & XAIEGBL_MODULE_MEM) != 0) {
		UsedEvtMask = TilePtr->MemBCUsedMask;
		Handler = AieInst->MemEvtHandlers;
		if ((Event != XAIETILE_EVENTS_ALL) && (Handler[Event].Refs == 0)) {
			return XAIE_SUCCESS;
		}
		for (u8 j = 0; j <= XAIETILE_EVENTS_BROADCAST_MAX;
		     j++) {
			if ((UsedEvtMask & (1 << j)) != 0) {
				RegVal = XAieTile_MemEventBroadcastGet(TilePtr, j);
				XAie_AssertNonvoid(RegVal <= 127);
				if (RegVal == 0 || Handler[RegVal].Refs == 0) {
					continue;
				}
				if ((RegVal == (u32)Event) ||
				     (Event == XAIETILE_EVENTS_ALL)) {
					XAieTileMem_EventBroadcast(TilePtr, j, 0);
					TilePtr->MemBCUsedMask &= ~(1U << j);
					Handler[RegVal].Refs--;
					if (Handler[RegVal].Refs == 0) {
						Handler[RegVal].Cb = XAIE_NULL;
						Handler[RegVal].Arg = XAIE_NULL;
					}
					if (Event != XAIETILE_EVENTS_ALL) {
						break;
					}
				}
			}
		}
	}
       if ((Module & XAIEGBL_MODULE_PL) !=0) {
		UsedEvtMask = TilePtr->PlIntEvtUsedMask;
		Handler = AieInst->ShimEvtHandlers;
		if ((Event != XAIETILE_EVENTS_ALL) && (Handler[Event].Refs == 0)) {
			return XAIE_SUCCESS;
		}
		for (u8 j = 0; j <= 4; j++) {
			if ((UsedEvtMask & (1 << j)) != 0) {
				RegVal = XAieTile_PlIntcL1IrqEventGet(TilePtr,
								      j + 16,
								      XAIETILE_PL_BLOCK_SWITCHA);
				XAie_AssertNonvoid(RegVal <= 127);
				if (RegVal == 0 || Handler[RegVal].Refs == 0) {
					continue;
				}
				if ((RegVal == (u32)Event) ||
				     (Event == XAIETILE_EVENTS_ALL)) {
					XAieTile_PlIntcL1IrqEventSet(TilePtr,
								     j + 16, 0,
								     XAIETILE_PL_BLOCK_SWITCHA);
					TilePtr->PlIntEvtUsedMask &= ~(1U << j);
					Handler[RegVal].Refs--;
					if (Handler[RegVal].Refs == 0) {
						Handler[RegVal].Cb = XAIE_NULL;
						Handler[RegVal].Arg = XAIE_NULL;
					}
					if (Event != XAIETILE_EVENTS_ALL) {
						break;
					}
				}
			}
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API registers for event notification
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Pointer to tile location array
 * @param	NumTiles - Number of Tiles
 *			   If Loc is NULL and NumTiles is 0, the driver will
 *			   consider it as all the tiles in the AieInst.
 * @param	Module - Module type,
 *			 XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_MEM,
 *			 XAIEGBL_MODULE_PL
 * @param	Event - Event id
 * @param	Cb - Callback, it cannot be NULL
 * @param	Arg - Private data pointer which will pass to callback,
 *		      it can be NULL
 *
 * @return	XAIE_SUCCESS for success, XAIE_FAILURE for failure
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_EventRegisterNotification(XAieGbl *AieInst, XAie_LocType *Loc,
				       u32 NumTiles, u8 Module, u8 Event,
				       XAieTile_EventCallBack Cb, void *Arg)
{
	XAieGbl_EventHandler *Handler;
	XAieGbl_Tile *TilePtr;
	u32 NumCols, NumRows, NumTotalTiles, ClockCntrlRegVal;
	u64 RegAddr;
	int ret;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertNonvoid(Cb != XAIE_NULL);

	if (Event <= 1) {
		XAieLib_IntPrint("%s: failed,"
				 "event(%u), invalid event.\n",
				 __func__, Event);
		return XAIE_FAILURE;
	}
	if (Module == XAIEGBL_MODULE_CORE) {
		Handler = AieInst->CoreEvtHandlers;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		Handler = AieInst->MemEvtHandlers;
	} else if (Module == XAIEGBL_MODULE_PL) {
		Handler = AieInst->ShimEvtHandlers;
	} else {
		XAieLib_IntPrint("%s: failed,"
				 "event(%u), invalid module %u.\n",
				 __func__, Event, Module);
		return XAIE_FAILURE;
	}
	if (Cb == XAIE_NULL) {
		XAieLib_IntPrint("%s: failed,"
				 "event(%u,%u), Callback is NULL.\n",
				 __func__, Module, Event);
		return XAIE_FAILURE;
	}
	if (Handler[Event].Cb != XAIE_NULL) {
		XAieLib_IntPrint("%s: failed,"
				 "event(%u,%u) has been registered with"
				 "a callback. Please unregister first.\n",
				 __func__, Module, Event);
		return XAIE_FAILURE;
	}
	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	NumTotalTiles = NumCols * (NumRows + 1);
	if ((Loc == XAIE_NULL) && (NumTiles != 0) &&
	    (NumTiles != NumTotalTiles)) {
		XAieLib_IntPrint("%s: failed, event(%u,%u): "
				"Invalid Loc pointer and NumTiles combination",
				__func__, Module, Event);
		return XAIE_FAILURE;
	}

	/* Check availability */
	if (Loc != XAIE_NULL) {
		for (u32 i = 0; i < NumTiles; i++) {
			if ((Loc[i].Col >= NumCols) || (Loc[i].Row > NumRows)) {
				XAieLib_IntPrint("%s: failed, invalid Loc[%u](%u,%u).\n",
						 __func__, i, Loc[i].Col, Loc[i].Row);
				return XAIE_FAILURE;
			}
			ret = _XAieTile_EventBroadcastCheckAvail(AieInst, Loc[i],
								 Module);
			if (ret != XAIE_SUCCESS) {
				XAieLib_IntPrint("%s: failed, event(%u,%u): "
						 "check notification failed.\n",
						 __func__, Module, Event);
				return XAIE_FAILURE;
			}
		}
	} else {
		for (u32 c = 0; c < NumCols; c++) {
			XAie_LocType TmpLoc;
			TmpLoc.Col = c;
			TmpLoc.Row = 0;
			ret = _XAieTile_EventBroadcastCheckAvail(AieInst, TmpLoc,
								 Module);
			if (ret != XAIE_SUCCESS) {
				XAieLib_IntPrint("%s: failed, event(%u,%u): "
						 "check notification failed.\n",
						 __func__, Module, Event);
				return XAIE_FAILURE;
			}
			/*
			 * check if any tile in this col is used by checking IsUsed
			 * field of shim TilePtr of this col.
			 */
			TilePtr = AieInst->Tiles;
			TilePtr += c * (NumRows + 1);
			RegAddr = TilePtr->TileAddr;
			ClockCntrlRegVal = XAieGbl_Read32(RegAddr +
							XAIEGBL_PL_TILCLOCTRL);
			if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
				for (u32 r = 1; r < NumRows; r++) {
					XAie_LocType TmpLoc;
					TmpLoc.Col = c;
					TmpLoc.Row = r;
					ret = _XAieTile_EventBroadcastCheckAvail
						(AieInst, TmpLoc, Module);
					if (ret != XAIE_SUCCESS) {
						XAieLib_IntPrint("%s: failed, event(%u,%u): "
								 "check notification failed.\n",
								 __func__, Module, Event);
						return XAIE_FAILURE;
					}
					/* check if the tile above is gated */
					TilePtr = AieInst->Tiles;
					TilePtr += c * (NumRows + 1) + r;
					RegAddr = TilePtr->TileAddr;
					ClockCntrlRegVal = XAieGbl_Read32(
					     RegAddr + XAIEGBL_CORE_TILCLOCTRL);
					if(!(ClockCntrlRegVal &
					     XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
						break;
				}
			}
		}
	}
	if (Handler[Event].Cb == XAIE_NULL) {
		Handler[Event].Cb = Cb;
		Handler[Event].Arg = Arg;
	}

	/* Set up notification */
	if (Loc != XAIE_NULL) {
		for (u32 i = 0; i < NumTiles; i++) {
			ret = _XAieTile_EventSetNotification(AieInst, Loc[i],
							     Module, Event);
			if (ret != XAIE_SUCCESS) {
				XAieLib_IntPrint("%s: failed, event(%u,%u): "
						 "set notification failed.\n",
						 __func__, Module, Event);
				return XAIE_FAILURE;
			}
			Handler[Event].Refs++;
		}
	} else {
		for (u32 c = 0; c < NumCols; c++) {
			XAie_LocType TmpLoc;
			TmpLoc.Col = c;
			TmpLoc.Row = 0;
			ret = _XAieTile_EventSetNotification(AieInst, TmpLoc,
								 Module,Event);
			if (ret != XAIE_SUCCESS) {
				XAieLib_IntPrint("%s: failed, event(%u,%u): "
						 "set notification failed.\n",
						 __func__, Module, Event);
				return XAIE_FAILURE;
			}
			/*
			 * check if any tile in this col is used by checking IsUsed
			 * field of shim TilePtr of this col.
			 */
			TilePtr = AieInst->Tiles;
			TilePtr += c * (NumRows + 1);
			RegAddr = TilePtr->TileAddr;
			ClockCntrlRegVal = XAieGbl_Read32(RegAddr + XAIEGBL_PL_TILCLOCTRL);
			if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
				for (u32 r = 1; r < NumRows; r++) {
					XAie_LocType TmpLoc;
					TmpLoc.Col = c;
					TmpLoc.Row = r;
					ret = _XAieTile_EventSetNotification
						(AieInst, TmpLoc,Module, Event);
					if (ret != XAIE_SUCCESS) {
						XAieLib_IntPrint("%s: failed, event(%u,%u): "
								 "set notification failed.\n",
								 __func__, Module, Event);
						return XAIE_FAILURE;
					}
					TilePtr = AieInst->Tiles;
					TilePtr += c * (NumRows + 1) + r;
					RegAddr = TilePtr->TileAddr;
					ClockCntrlRegVal = XAieGbl_Read32(
					     RegAddr + XAIEGBL_CORE_TILCLOCTRL);
					/* check if the tile above is gated */
					if(!(ClockCntrlRegVal &
						XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
						break;
				}
			}
			Handler[Event].Refs++;
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API unregisters for event notification
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Loc - Tile location array pointer
 * @param	NumTiles - Number of tiles to unregister.
 *			   If Loc is NULL and NumTiles is 0, the driver will
 *			   consider it as all the tiles in the AieInst.
 * @param	Module - Module type,
 *			 XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_MEM,
 *			 XAIEGBL_MODULE_PL, XAIEGBL_MODULE_ALL,
 *			 or any OR combination of CPRE, MEM, and PL
 * @param	Event - Event id, if Event id is XAIETILE_EVENTS_ALL, it will
 *			unregister all the non-error events of the specified
 *			tiles. If Module is XAIEGBL_MODULE_ALL, Event needs to
 *			be XAIETILE_EVENTS_ALL.
 *
 * @return	XAIE_SUCCESS for success, XAIE_FAILURE for failure
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_EventUnregisterNotification(XAieGbl *AieInst,
					 XAie_LocType *Loc, u32 NumTiles,
					 u8 Module, u8 Event)
{
	u32 NumCols, NumRows, NumTotalTiles;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	NumTotalTiles = NumCols * (NumRows + 1);
	if (Event == 1) {
		XAieLib_IntPrint("%s: invalid event %u.\n", __func__, Event);
		return XAIE_FAILURE;
	}
	if (Module > XAIEGBL_MODULE_ALL) {
		XAieLib_IntPrint("%s: invalid module %u.\n", __func__, Module);
		return XAIE_FAILURE;
	}
	if ((Module != XAIEGBL_MODULE_CORE) && (Module != XAIEGBL_MODULE_ALL) &&
	    (Module != XAIEGBL_MODULE_PL) && (Event != XAIETILE_EVENTS_ALL)) {
		XAieLib_IntPrint("%s: failed (%u,%u) if Module is not single module type,"
				 "events need to be XAIETILE_EVENTS_ALL.\n",
				 __func__, Module, Event);
		return XAIE_FAILURE;
	}
	if ((Loc == XAIE_NULL) && (NumTiles != 0) &&
	    (NumTiles != NumTotalTiles)) {
		XAieLib_IntPrint("%s: failed, event(%u,%u): "
				"Invalid Loc pointer and NumTiles combination",
				__func__, Module, Event);
		return XAIE_FAILURE;
	}

	if (Loc != XAIE_NULL) {
		for (u32 i = 0; i < NumTiles; i++) {
			if ((Loc[i].Col >= NumCols) || (Loc[i].Row > NumRows)) {
				XAieLib_IntPrint("%s: failed, invalid Loc[%u](%u,%u).\n",
						 __func__, i, Loc[i].Col, Loc[i].Row);
				return XAIE_FAILURE;
			}
			(void)_XAieTile_EventUnregisterNotification(AieInst, Loc[i],
								    Module, Event);
		}
	}
	else {
		for (u32 c = 0; c < NumCols; c++) {
			XAie_LocType TmpLoc;
			XAieGbl_Tile *TilePtr;
			u32 ClockCntrlRegVal;
			u64 RegAddr;
			TmpLoc.Col = c;
			TmpLoc.Row = 0;
			(void)_XAieTile_EventUnregisterNotification(AieInst,
					TmpLoc, Module, Event);
			/* check if any tile in this col is used */
			TilePtr = AieInst->Tiles;
			TilePtr += c * (NumRows + 1);
			RegAddr = TilePtr->TileAddr;
			ClockCntrlRegVal = XAieGbl_Read32(RegAddr + XAIEGBL_PL_TILCLOCTRL);
			if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
				for (u32 r = 1; r < NumRows; r++) {
					XAie_LocType TmpLoc;
					TmpLoc.Col = c;
					TmpLoc.Row = r;
					(void)_XAieTile_EventUnregisterNotification(
						AieInst, TmpLoc, Module, Event);
					/* check if the tile above is gated */
					TilePtr = AieInst->Tiles;
					TilePtr += c * (NumRows + 1) + r;
					RegAddr = TilePtr->TileAddr;
					ClockCntrlRegVal = XAieGbl_Read32(
						RegAddr + XAIEGBL_CORE_TILCLOCTRL);
					if(!(ClockCntrlRegVal &
						XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
						break;
				}
			}
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API enable events NPI interrupt.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	XAIE_SUCCESS
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_EventsEnableInterrupt(XAieGbl *AieInst)
{
	(void)AieInst;
	XAieLib_InterruptEnable();
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API disable events NPI interrupt.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	XAIE_SUCCESS
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_EventsDisableInterrupt(XAieGbl *AieInst)
{
	(void)AieInst;
	XAieLib_InterruptDisable();
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API initialize events default handlers.
 * By default, the events handlers do nothing except for errors.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	None.
 *
 * @note	None.
 *****************************************************************************/
void XAieTile_EventsSetupDefaultHandlers(XAieGbl *AieInst)
{
	memset(AieInst->CoreEvtHandlers, 0, sizeof(AieInst->CoreEvtHandlers));
	memset(AieInst->MemEvtHandlers, 0, sizeof(AieInst->MemEvtHandlers));
	memset(AieInst->ShimEvtHandlers, 0, sizeof(AieInst->ShimEvtHandlers));
	XAieTile_ErrorsSetupDefaultHandler(AieInst);
#if defined __linux__
	/* Register for libmetal interrupt handler for Linux.
	 * For baremetal the XAieTile_EventsIsr() will need to be registered
	 * with AieInst to the Xilinx interrupt controller
	 */
	XAieGbl_IntrRegisterIsr(XAIETILE_EVENT_NPI_INTERRUPT,
				XAieTile_EventsMetalIsr,
				AieInst);
#endif
}

/*****************************************************************************/
/**
 * This API enable events handling. It will setup the events broadcast network
 * so that if events is raised, interrupt will be raised, and the interrupt
 * will be captured by the AIE driver. When event happens, the AIE driver
 * will call the stored events handler.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_EventsHandlingInitialize(XAieGbl *AieInst)
{
	u32 NumCols, NumRows;
	XAieGbl_Tile *TilePtr;
	u32 RegVal;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	XAie_print("%s: Disabling NPI interrupts.\n", __func__);
	/* Disable NPI interrupt first */
	XAieGbl_NPIWrite32(XAIE_NPI_IDR0, 0xFU);
	XAieGbl_NPIWrite32(XAIE_NPI_IDR1, 0xFU);
	XAieGbl_NPIWrite32(XAIE_NPI_IDR2, 0xFU);
	XAieGbl_NPIWrite32(XAIE_NPI_IDR3, 0xFU);
	/* Setup events broadcast network */
	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	XAie_print("%s: Setup broadcast network.\n", __func__);

	for(u32 col = 0; col < NumCols; col++){
		/* SHIM Tile */
		TilePtr = AieInst->Tiles;
		TilePtr += col * (NumRows + 1);
		u8 Irq1stBc;
		XAie_LocType Loc;
#ifndef __AIESIM__
		u32 ClockCntrlRegVal;
		u64 RegAddr;
#endif

		Loc.Col = TilePtr->ColId;
		Loc.Row = 0;

		/* Setup 1st level interrup controller to capture
		 * events from the tiles above and local shim.
		 */
		Irq1stBc = XAieTile_Cal1stIrqNo(AieInst, Loc,
						XAIETILE_PL_BLOCK_SWITCHA);
		XAie_print("%s: (%u,%u),Switch %u, BC %u.\n",
			   __func__, Loc.Col, Loc.Row, XAIETILE_PL_BLOCK_SWITCHA, Irq1stBc);
		XAieTile_PlIntcL1IrqNoSet(TilePtr, Irq1stBc,
					  XAIETILE_PL_BLOCK_SWITCHA);
		XAieTile_PlIntcL1IrqNoSet(TilePtr, Irq1stBc + 1,
					  XAIETILE_PL_BLOCK_SWITCHB);
		/* Enable 1st level interrupt controller */
		RegVal = XAIETILE_EVENTS_BROADCAST_MASK;
		XAieTile_PlIntcL1Enable(TilePtr, RegVal,
					XAIETILE_PL_BLOCK_SWITCHB);
		RegVal |= XAIETILE_EVENTS_SHIM_INTEVENT_MASK;
		XAieTile_PlIntcL1Enable(TilePtr, RegVal,
					XAIETILE_PL_BLOCK_SWITCHA);
		TilePtr->PlIntEvtUsedMask = 1;
		/* Block broacast event to the north */
		XAieTilePl_EventBroadcastBlockSet(TilePtr,
				XAIETILE_EVENT_BLOCK_NORTH,
				XAIETILE_PL_BLOCK_SWITCHA,
				XAIETILE_BCEVENTS_NOTIFY_MASK);
		XAieTilePl_EventBroadcastBlockSet(TilePtr,
				XAIETILE_EVENT_BLOCK_NORTH,
				XAIETILE_PL_BLOCK_SWITCHB,
				XAIETILE_BCEVENTS_NOTIFY_MASK);
		if (TilePtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			/* SHIM NoC, block events from west/east */
			XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHA,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_EAST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			/* Enable 2nd level interrupt controller */
			RegVal = XAIETILE_EVENT_1ST_IRQ_IDS_MASK;
			XAieTile_NoCIntcL2IntrSet(TilePtr,
						  XAIETILE_EVENT_NPI_INTERRUPT);
			XAieTile_NoCIntcL2Enable(TilePtr, RegVal);
		}
		else {
			/* SHIM PL */
			if (Irq1stBc == 0) {
				/* Block to left */
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHA,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			} else if (Irq1stBc == 4) {
				/* Block to right */
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_EAST,
					XAIETILE_PL_BLOCK_SWITCHA,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_EAST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			} else if ((TilePtr->ColId % 4) == 0) {
				/* SHIM PL needs to broadast to the
				 * SHIM NOC on the left */
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_EAST,
					XAIETILE_PL_BLOCK_SWITCHA,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_EAST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			}
			else {
				/* SHIM PL needs to broadcast to
				 * the SHIM NOC in the right */
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHA,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTilePl_EventBroadcastBlockSet(TilePtr,
					XAIETILE_EVENT_BLOCK_WEST,
					XAIETILE_PL_BLOCK_SWITCHB,
					XAIETILE_BCEVENTS_NOTIFY_MASK);
			}
		}
#ifndef __AIESIM__
		RegAddr = TilePtr->TileAddr;
		ClockCntrlRegVal = XAieGbl_Read32(RegAddr + XAIEGBL_PL_TILCLOCTRL);
		if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
#endif
			for(u32 row = 1; row <= NumRows; row++){
				TilePtr = AieInst->Tiles;
				TilePtr += col * (NumRows + 1) + row;
				/* Non SHIM tile, block broadcast events from west
				 * and east */
				XAieTileCore_EventBroadcastBlockSet(TilePtr,
									XAIETILE_EVENT_BLOCK_WEST,
									XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTileCore_EventBroadcastBlockSet(TilePtr,
									XAIETILE_EVENT_BLOCK_EAST,
									XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTileCore_EventBroadcastBlockSet(TilePtr,
									XAIETILE_EVENT_BLOCK_NORTH,
									XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTileMem_EventBroadcastBlockSet(TilePtr,
								   XAIETILE_EVENT_BLOCK_WEST,
								   XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTileMem_EventBroadcastBlockSet(TilePtr,
								   XAIETILE_EVENT_BLOCK_EAST,
								   XAIETILE_BCEVENTS_NOTIFY_MASK);
				XAieTileMem_EventBroadcastBlockSet(TilePtr,
								   XAIETILE_EVENT_BLOCK_NORTH,
								   XAIETILE_BCEVENTS_NOTIFY_MASK);
				TilePtr->MemBCUsedMask = 1 << XAIETILE_ERROR_BROADCAST;
				TilePtr->CoreBCUsedMask = 1 << XAIETILE_ERROR_BROADCAST;

#ifndef __AIESIM__
				RegAddr = TilePtr->TileAddr;
				ClockCntrlRegVal = XAieGbl_Read32(RegAddr +
					XAIEGBL_CORE_TILCLOCTRL);
				/* check if the tile above this tile is gated */
				if(!(ClockCntrlRegVal & XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
					break;
#endif
			}
#ifndef __AIESIM__
		}
#endif
	}

	/* Initialize errors handling. Setup the errors broadcasting event.
	 * Install the errors default handler. */
	XAie_print("%s: Initialize errors handling.\n", __func__);
	XAieTile_ErrorsHandlingInitialize(AieInst);
	XAieGbl_NPIWrite32(XAIE_NPI_IER1, (1 << XAIETILE_EVENT_NPI_INTERRUPT));
	/* Register for NPI interrupt handler */
	/* For Baremetal, user will need to register interrupt handler
	 * in the application. */
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API waits until all pending events got reported.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	None.
 *
 * @note	None.
 *****************************************************************************/
void XAieTile_EventsWaitForPending(XAieGbl *AieInst)
{
	u32 NumCols, NumRows;
	XAieGbl_Tile *TilePtr;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	/* Poll 2nd level interrupt to see if there are pending interrupts */
	TilePtr = AieInst->Tiles;
	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;
	for (u32 i = 0; i < NumCols; i++) {
		if (TilePtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			u32 RegVal;

			do {
				RegVal = XAieTile_NoCIntcL2StatusGet(TilePtr);
			} while (RegVal != 0);

		}
		TilePtr += NumRows + 1;
	}
}

/** @} */

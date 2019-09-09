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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaietile_event.h"

/***************************** Constant Definitions **************************/
#define XAIETILE_EVENT_MODULE_CORE	0x0U
#define XAIETILE_EVENT_MODULE_PL		0x1U
#define XAIETILE_EVENT_MODULE_MEM	0x2U

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
	XAie_AssertNonvoid(PCEvent >= XAIETILE_EVENT_CORE_PC_EVENT0 &&
			PCEvent <= XAIETILE_EVENT_CORE_PC_EVENT3);
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

/** @} */

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(Dir >= 0 && Dir < 4);

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
	XAie_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

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
	XAie_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

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
	XAie_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

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
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Idx >= 0 && Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

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
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Idx >= 0 && Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

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
	XAie_AssertNonvoid(Idx >= 0 && Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

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
	XAie_AssertNonvoid(Idx >= 0 && Idx < XAIETILE_EVENT_NUM_TRACE_EVENT);

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
	u16 ColumnId = TileInstPtr->ColId;
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
	u16 ColumnId = TileInstPtr->ColId;
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
	u16 ColumnId = TileInstPtr->ColId;
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
	u16 ColumnId = TileInstPtr->ColId;
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
	u16 ColumnId = TileInstPtr->ColId;
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

/** @} */

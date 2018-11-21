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
* @file xmetile_event.c
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmegbl.h"
#include "xmegbl_defs.h"
#include "xmegbl_reginit.h"
#include "xmetile_event.h"

/***************************** Constant Definitions **************************/
#define XMETILE_EVENT_MODULE_CORE	0x0U
#define XMETILE_EVENT_MODULE_PL		0x1U
#define XMETILE_EVENT_MODULE_MEM	0x2U

/***************************** Macro Definitions *****************************/
/************************** Variable Definitions *****************************/
extern XMeGbl_RegEventGenerate EventGenerate[];
extern XMeGbl_RegEventBroadcast EventBroadcast[];
extern XMeGbl_RegEventBroadcastSet EventBroadcastSet[];
extern XMeGbl_RegEventBroadcastClear EventBroadcastClear[];
extern XMeGbl_RegEventBroadcastValue EventBroadcastValue[];
extern XMeGbl_RegTraceCtrls TraceCtrl[];
extern XMeGbl_RegTraceEvent TraceEvent[];

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
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileMem_EventBroadcastBlockValue(XMeGbl_Tile *TileInstPtr, u8 Dir)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XMETILE_EVENT_MODULE_MEM].RegOff[Dir]);
}

/*****************************************************************************/
/**
*
* This API returns the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XMETILE_EVENT_BLOCK_SWITCHA or XMETILE_EVENT_BLOCK_SWITCHB.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XMeTilePl_EventBroadcastBlockValue(XMeGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XMETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U]);
}

/*****************************************************************************/
/**
*
* This API returns the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
*
* @return	Current event broadcast value
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileCore_EventBroadcastBlockValue(XMeGbl_Tile *TileInstPtr, u8 Dir)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
			EventBroadcastValue[XMETILE_EVENT_MODULE_CORE].RegOff[Dir]);
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
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_EventBroadcastBlockClear(XMeGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastClear[XMETILE_EVENT_MODULE_MEM].RegOff[Dir],
			Mask);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XMETILE_EVENT_BLOCK_SWITCHA or XMETILE_EVENT_BLOCK_SWITCHB.
* @param	Mask - Mask with bits to clear
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_EventBroadcastBlockClear(XMeGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB,
		u16 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastClear[XMETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U],
			Mask);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API clears the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to clear
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_EventBroadcastBlockClear(XMeGbl_Tile *TileInstPtr, u8 Dir, u8 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
		EventBroadcastClear[XMETILE_EVENT_MODULE_CORE].RegOff[Dir],
		Mask);
	return XME_SUCCESS;
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
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to set
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_EventBroadcastBlockSet(XMeGbl_Tile *TileInstPtr, u8 Dir, u16 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XMETILE_EVENT_MODULE_MEM].RegOff[Dir],
			Mask);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current event broadcast block value for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	SwitchAB - Flag to indicate if it's the A or B block. Should
* be one of XMETILE_EVENT_BLOCK_SWITCHA or XMETILE_EVENT_BLOCK_SWITCHB.
* @param	Mask - Mask with bits to set
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_EventBroadcastBlockSet(XMeGbl_Tile *TileInstPtr, u8 Dir, u8 SwitchAB,
		u16 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XMETILE_EVENT_MODULE_PL].RegOff[Dir + SwitchAB * 0x4U],
			Mask);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current event broadcast block value for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Dir - Direction. Should be one of
* XMETILE_EVENT_BLOCK_SOUTH, XMETILE_EVENT_BLOCK_WEST,
* XMETILE_EVENT_BLOCK_NORTH, or XMETILE_EVENT_BLOCK_EAST.
* @param	Mask - Mask with bits to set
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_EventBroadcastBlockSet(XMeGbl_Tile *TileInstPtr, u8 Dir, u8 Mask)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Dir >= 0 && Dir < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcastSet[XMETILE_EVENT_MODULE_CORE].RegOff[Dir],
			Mask);
	return XME_SUCCESS;
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
* @param	Event - Event ID. One of XMETILE_EVENT_MEM_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_EventBroadcast(XMeGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XMETILE_EVENT_MODULE_MEM].RegOff +
			BroadcastId * 0x4U, Event);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XMETILE_EVENT_PL_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_EventBroadcast(XMeGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XMETILE_EVENT_MODULE_PL].RegOff +
			BroadcastId * 0x4U, Event);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the event to the given broadcast ID for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	BroadcastId - Broadcast ID. 0 to 15.
* @param	Event - Event ID. One of XMETILE_EVENT_CORE_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_EventBroadcast(XMeGbl_Tile *TileInstPtr, u8 BroadcastId, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(BroadcastId >= 0 && BroadcastId < 16);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventBroadcast[XMETILE_EVENT_MODULE_CORE].RegOff +
			BroadcastId * 0x4U, Event);
	return XME_SUCCESS;
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
* @param	Event - Event ID. One of XMETILE_EVENT_MEM_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_EventGenerate(XMeGbl_Tile *TileInstPtr, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XMETILE_EVENT_MODULE_MEM].RegOff,
			Event);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the PL module to generate an internal event of @Event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XMETILE_EVENT_PL_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_EventGenerate(XMeGbl_Tile *TileInstPtr, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XMETILE_EVENT_MODULE_PL].RegOff,
			Event);
	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the Core module to generate an internal event of @Event
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Event - Event ID. One of XMETILE_EVENT_CORE_*
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_EventGenerate(XMeGbl_Tile *TileInstPtr, u8 Event)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			EventGenerate[XMETILE_EVENT_MODULE_CORE].RegOff,
			Event);
	return XME_SUCCESS;
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
* XMETILE_EVENT_MODULE_CORE, XMETILE_EVENT_MODULE_PL, or
* XMETILE_EVENT_MODULE_MEM.
* @param	Mode - Trace mode: Should be XMETILE_EVENT_MODE_EVENT_TIME,
* XMETILE_EVENT_MODE_EVENT_PC, or XMETILE_EVENT_MODE_EXECUTION.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XME_SUCCESS on success
*
* @note		Only for internal use. If XMETILE_EVENT_TRACING_INVALID_VAL is
* given for any argument, the setting argument is skipped.
*
*******************************************************************************/
static u8 _XMeTile_EventTraceControl(XMeGbl_Tile *TileInstPtr, u8 ModId,
		u8 Mode, u8 StartEvent, u8 StopEvent, u8 Id, u8 Packet)
{
	u64 RegAddr;
	u32 FldVal = 0;
	u32 FldMask = 0;

	if (Mode != XMETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Mode.Mask;
		FldVal |= XMe_SetField(Mode, TraceCtrl[ModId].Mode.Lsb,
				TraceCtrl[ModId].Mode.Mask);
	}
	if (StartEvent != XMETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Start.Mask;
		FldVal |= XMe_SetField(StartEvent, TraceCtrl[ModId].Start.Lsb,
				TraceCtrl[ModId].Start.Mask);
	}
	if (StopEvent != XMETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Stop.Mask;
		FldVal |= XMe_SetField(StopEvent, TraceCtrl[ModId].Stop.Lsb,
				TraceCtrl[ModId].Stop.Mask);
	}

	if (Mode != XMETILE_EVENT_TRACING_INVALID_VAL ||
			StartEvent != XMETILE_EVENT_TRACING_INVALID_VAL ||
			StopEvent != XMETILE_EVENT_TRACING_INVALID_VAL) {
		RegAddr = TileInstPtr->TileAddr + TraceCtrl[ModId].RegOff[0];
		XMeGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	FldVal = 0;
	FldMask = 0;

	if (Id != XMETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Id.Mask;
		FldVal |= XMe_SetField(Id, TraceCtrl[ModId].Id.Lsb,
				TraceCtrl[ModId].Id.Mask);
	}

	if (Packet != XMETILE_EVENT_TRACING_INVALID_VAL) {
		FldMask |= TraceCtrl[ModId].Packet.Mask;
		FldVal |= XMe_SetField(Packet, TraceCtrl[ModId].Packet.Lsb,
				TraceCtrl[ModId].Packet.Mask);
	}

	if (Id != XMETILE_EVENT_TRACING_INVALID_VAL ||
			Packet != XMETILE_EVENT_TRACING_INVALID_VAL) {
		RegAddr = TileInstPtr->TileAddr + TraceCtrl[ModId].RegOff[1];
		XMeGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		If XMETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XMeTileMem_EventTraceControl(XMeGbl_Tile *TileInstPtr, u8 StartEvent,
		u8 StopEvent, u8 Id, u8 Packet)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);

	return _XMeTile_EventTraceControl(TileInstPtr,
			XMETILE_EVENT_MODULE_MEM,
			XMETILE_EVENT_TRACING_INVALID_VAL,
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
* @return	XME_SUCCESS on success
*
* @note		If XMETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XMeTilePl_EventTraceControl(XMeGbl_Tile *TileInstPtr, u8 StartEvent,
		u8 StopEvent, u8 Id, u8 Packet)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);

	return _XMeTile_EventTraceControl(TileInstPtr,
			XMETILE_EVENT_MODULE_PL,
			XMETILE_EVENT_TRACING_INVALID_VAL,
			StartEvent, StopEvent, Id, Packet);
}

/*****************************************************************************/
/**
*
* This API sets the trace control of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Mode - Trace mode: Should be XMETILE_EVENT_MODE_EVENT_TIME,
* XMETILE_EVENT_MODE_EVENT_PC, or XMETILE_EVENT_MODE_EXECUTION.
* @param	StartEvent - Event to start
* @param	StopEvent - Event to stop
* @param	Id - Packet ID
* @param	Packet - Packet type
*
* @return	XME_SUCCESS on success
*
* @note		If XMETILE_EVENT_TRACING_INVALID_VAL is given for any argument,
* the setting argument is skipped.
*
*******************************************************************************/
u8 XMeTileCore_EventTraceControl(XMeGbl_Tile *TileInstPtr, u8 Mode,
		u8 StartEvent, u8 StopEvent, u8 Id, u8 Packet)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);

	return _XMeTile_EventTraceControl(TileInstPtr,
			XMETILE_EVENT_MODULE_CORE, Mode,
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
* XMETILE_EVENT_MODULE_CORE, XMETILE_EVENT_MODULE_PL, or
* XMETILE_EVENT_MODULE_MEM.
* @param	Event - Event ID
* @param	Idx - Trace ID. 0 to 7.
*
* @return	XME_SUCCESS on success
*
* @note		Only for internal use.
*
*******************************************************************************/
static u8 _XMeTile_EventTraceEventId(XMeGbl_Tile *TileInstPtr, u8 ModeId,
		u8 Event, u8 Idx)
{
	u8 RegPart = (Idx < 4) ? 0 : 1;
	u64 RegAddr;
	u32 FldVal;

	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[RegPart];
	FldVal = XMe_SetField(Event, TraceEvent[ModeId].Event[Idx].Lsb,
			TraceEvent[ModeId].Event[Idx].Mask);
	XMeGbl_MaskWrite32(RegAddr, TraceEvent[ModeId].Event[Idx].Mask, FldVal);

	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_EventTraceEventWriteId(XMeGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	u64 RegAddr;
	u32 RegVal;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Idx >= 0 && Idx < XMETILE_EVENT_NUM_TRACE_EVENT);

	return _XMeTile_EventTraceEventId(TileInstPtr,
			XMETILE_EVENT_MODULE_MEM, Event, Idx);
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_EventTraceEventWriteId(XMeGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	u64 RegAddr;
	u32 RegVal;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Idx >= 0 && Idx < XMETILE_EVENT_NUM_TRACE_EVENT);

	return _XMeTile_EventTraceEventId(TileInstPtr,
			XMETILE_EVENT_MODULE_PL,Event, Idx);
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_EventTraceEventWriteId(XMeGbl_Tile *TileInstPtr, u8 Event,
		u8 Idx)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Idx >= 0 && Idx < XMETILE_EVENT_NUM_TRACE_EVENT);

	return _XMeTile_EventTraceEventId(TileInstPtr,
			XMETILE_EVENT_MODULE_CORE, Event, Idx);
}

/*****************************************************************************/
/**
*
* This internal API sets multiple event traces of given module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XMETILE_EVENT_MODULE_CORE, XMETILE_EVENT_MODULE_PL, or
* XMETILE_EVENT_MODULE_MEM.
* @param	TraceEvents - Events to trace.
*
* @return	XME_SUCCESS on success
*
* @note		Only for internal use. All 8 event traces should be set.
*
*******************************************************************************/
static u8 _XMeTile_EventTraceEvent(XMeGbl_Tile *TileInstPtr, u8 ModeId,
		XMe_TraceEvents *TraceEvents)
{
	u8 Idx;
	u64 RegAddr;
	u32 RegVal = 0;

	for (Idx = 0; Idx < 4; Idx++) {
		RegVal |= XMe_SetField(TraceEvents->TraceEvent[Idx],
				TraceEvent[ModeId].Event[Idx].Lsb,
				TraceEvent[ModeId].Event[Idx].Mask);
	}
	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[0];
	XMeGbl_Write32(RegAddr, RegVal);

	RegVal = 0;
	for (Idx = 4; Idx < XMETILE_EVENT_NUM_TRACE_EVENT; Idx++) {
		RegVal |= XMe_SetField(TraceEvents->TraceEvent[Idx],
				TraceEvent[ModeId].Event[Idx].Lsb,
				TraceEvent[ModeId].Event[Idx].Mask);
	}
	RegAddr = TileInstPtr->TileAddr + TraceEvent[ModeId].RegOff[1];
	XMeGbl_Write32(RegAddr, RegVal);

	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets multiple event traces of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Events to trace.
*
* @return	XME_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XMeTileMem_EventTraceEventWrite(XMeGbl_Tile *TileInstPtr,
		XMe_TraceEvents *TraceEvents)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(TraceEvents != XME_NULL);

	return _XMeTile_EventTraceEvent(TileInstPtr, XMETILE_EVENT_MODULE_MEM,
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
* @return	XME_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XMeTilePl_EventTraceEventWrite(XMeGbl_Tile *TileInstPtr,
		XMe_TraceEvents *TraceEvents)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(TraceEvents != XME_NULL);

	return _XMeTile_EventTraceEvent(TileInstPtr, XMETILE_EVENT_MODULE_PL,
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
* @return	XME_SUCCESS on success
*
* @note		All 8 event traces should be set in TraceEvents.
*
*******************************************************************************/
u8 XMeTileCore_EventTraceEventWrite(XMeGbl_Tile *TileInstPtr,
		XMe_TraceEvents *TraceEvents)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(TraceEvents != XME_NULL);

	return _XMeTile_EventTraceEvent(TileInstPtr, XMETILE_EVENT_MODULE_CORE,
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
* @return	XME_SUCCESS on success
*
* @note		Once all events are added, the TraceEvents can be used with
* XMeTileCore_EventTraceEventWrite to update with a single write.
*
*******************************************************************************/
u8 XMeTile_EventTraceEventAdd(XMeGbl_Tile *TileInstPtr,
		XMe_TraceEvents *TraceEvents, u8 Idx, u8 Event)
{
	(void)TileInstPtr;
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TraceEvents != XME_NULL);
	XMe_AssertNonvoid(Idx >= 0 && Idx < XMETILE_EVENT_NUM_TRACE_EVENT);

	TraceEvents->TraceEvent[Idx] = Event;

	return XME_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API initializes TraceEvents
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TraceEvents - Where event trace is queued
*
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTile_EventTraceEventInit(XMeGbl_Tile *TileInstPtr,
		XMe_TraceEvents *TraceEvents)
{
	u8 Idx;

	(void)TileInstPtr;
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TraceEvents != XME_NULL);

	for (Idx = 0U; Idx < XMETILE_EVENT_NUM_TRACE_EVENT; Idx++) {
		TraceEvents->TraceEvent[Idx] = 0U;
	}

	return XME_SUCCESS;
}

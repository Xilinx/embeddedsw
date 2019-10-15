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
* @file xaietile_perfcnt.c
* @{
*
* This file contains routines for Performance Counter configuration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Hyun    10/02/2018  Initial creation
* 1.1   Hyun    10/10/2018  Use the mask write API
* 1.2   Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3   Tejus   10/14/2019  Remove unwanted assertions
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_reginit.h"
#include "xaietile_perfcnt.h"

/***************************** Macro Definitions *****************************/
#define XAIETILE_PERFCNT_MODULE_CORE		0x0
#define XAIETILE_PERFCNT_MODULE_PL		0x1
#define XAIETILE_PERFCNT_MODULE_MEM		0x2

/************************** Variable Definitions *****************************/
extern XAieGbl_RegPerfCtrls PerfCtrl[];
extern XAieGbl_RegPerfCtrlReset PerfCtrlReset[];
extern XAieGbl_RegPerfCounter PerfCounter[];
extern XAieGbl_RegPerfCounterEvent PerfCounterEvent[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is an internal API to set the performance counter control.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XAIETILE_PERFCNT_MODULE_CORE, XAIETILE_PERFCNT_MODULE_PL, or
* XAIETILE_PERFCNT_MODULE_MEM.
* @param	Counter - Counter ID. The max value differs upon @ModID
* @param	StartEvent - Event ID to start
* @param	StopEvent - Event ID to stop
* @param	ResetEvent - Event ID to reset
*
* @return	XAIE_SUCCESS on success
*
* @note		Used only within this file.
*
*******************************************************************************/
static u8 _XAieTile_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 ModId,
		u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	u64 RegAddr;
	u32 FldVal = 0;
	u32 FldMask = 0;
	u64 Addr = TileInstPtr->TileAddr;

	if (StartEvent != XAIETILE_PERFCNT_EVENT_INVALID) {
		FldMask |= PerfCtrl[ModId].Start[Counter].Mask;
		FldVal |= XAie_SetField(StartEvent,
				PerfCtrl[ModId].Start[Counter].Lsb,
				PerfCtrl[ModId].Start[Counter].Mask);
	}

	if (StopEvent != XAIETILE_PERFCNT_EVENT_INVALID) {
		FldMask |= PerfCtrl[ModId].Stop[Counter].Mask;
		FldVal |= XAie_SetField(StopEvent,
				PerfCtrl[ModId].Stop[Counter].Lsb,
				PerfCtrl[ModId].Stop[Counter].Mask);

	}

	if (StartEvent != XAIETILE_PERFCNT_EVENT_INVALID ||
			StopEvent != XAIETILE_PERFCNT_EVENT_INVALID) {
		RegAddr = Addr + PerfCtrl[ModId].RegOff[Counter];
		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	if (ResetEvent != XAIETILE_PERFCNT_EVENT_INVALID) {
		RegAddr = Addr + PerfCtrlReset[ModId].RegOff[Counter];

		FldMask = PerfCtrlReset[ModId].Reset[Counter].Mask;
		FldVal = XAie_SetField(ResetEvent,
				PerfCtrlReset[ModId].Reset[Counter].Lsb,
				PerfCtrlReset[ModId].Reset[Counter].Mask);

		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the performance counter control for Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 or 1.
* @param	StartEvent - Event ID to start
* @param	StopEvent - Event ID to stop
* @param	ResetEvent - Event ID to reset
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileMem_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	return _XAieTile_PerfCounterControl(TileInstPtr,
			XAIETILE_PERFCNT_MODULE_MEM, Counter, StartEvent,
			StopEvent, ResetEvent);
}

/*****************************************************************************/
/**
*
* This API sets the performance counter control for PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 or 1.
* @param	StartEvent - Event ID to start
* @param	StopEvent - Event ID to stop
* @param	ResetEvent - Event ID to reset
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTilePl_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	return _XAieTile_PerfCounterControl(TileInstPtr,
			XAIETILE_PERFCNT_MODULE_PL, Counter,
			StartEvent, StopEvent, ResetEvent);
}

/*****************************************************************************/
/**
*
* This API sets the performance counter control for Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 3.
* @param	StartEvent - Event ID to start
* @param	StopEvent - Event ID to stop
* @param	ResetEvent - Event ID to reset
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XAieTileCore_PerfCounterControl(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 4);

	return _XAieTile_PerfCounterControl(TileInstPtr,
			XAIETILE_PERFCNT_MODULE_CORE, Counter,
			StartEvent, StopEvent, ResetEvent);
}

/*****************************************************************************/
/**
*
* This API gets the current performance counter value of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 2.
*
* @return	Current counter value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileMem_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
		PerfCounter[XAIETILE_PERFCNT_MODULE_MEM].RegOff[Counter]);
}

/*****************************************************************************/
/**
*
* This API gets the current performance counter value of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 2.
*
* @return	Current counter value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTilePl_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
			PerfCounter[XAIETILE_PERFCNT_MODULE_PL].RegOff[Counter]);
}

/*****************************************************************************/
/**
*
* This API gets the current performance counter value of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 3.
*
* @return	Current counter value
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileCore_PerfCounterGet(XAieGbl_Tile *TileInstPtr, u8 Counter)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 4);

	return XAieGbl_Read32(TileInstPtr->TileAddr +
		PerfCounter[XAIETILE_PERFCNT_MODULE_CORE].RegOff[Counter]);
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter value of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 3.
* @param	CounterVal - Counter value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileMem_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			PerfCounter[XAIETILE_PERFCNT_MODULE_MEM].RegOff[Counter],
			CounterVal);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter value of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 2.
* @param	CounterVal - Counter value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTilePl_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XAIEGBL_TILE_TYPE_SHIMPL);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	XAieGbl_Write32(TileInstPtr->TileAddr +
			PerfCounter[XAIETILE_PERFCNT_MODULE_PL].RegOff[Counter],
			CounterVal);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter value of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 4.
* @param	CounterVal - Counter value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileCore_PerfCounterSet(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
		PerfCounter[XAIETILE_PERFCNT_MODULE_CORE].RegOff[Counter],
		CounterVal);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter event value of Memory module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 2.
* @param	EventVal - Event value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileMem_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	XAieGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XAIETILE_PERFCNT_MODULE_MEM].RegOff[Counter],
		EventVal);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter event value of PL module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 2.
* @param	EventVal - Event value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTilePl_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 2);

	XAieGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XAIETILE_PERFCNT_MODULE_PL].RegOff[Counter],
		EventVal);
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API sets the current performance counter event value of Core module
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Counter - Counter ID. 0 to 4.
* @param	EventVal - Event value to set
*
* @return	XAIE_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileCore_PerfCounterEventValue(XAieGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(TileInstPtr->TileType == XAIEGBL_TILE_TYPE_AIETILE);
	XAie_AssertNonvoid(Counter >= 0 && Counter < 4);

	XAieGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XAIETILE_PERFCNT_MODULE_CORE].RegOff[Counter],
		EventVal);
	return XAIE_SUCCESS;
}

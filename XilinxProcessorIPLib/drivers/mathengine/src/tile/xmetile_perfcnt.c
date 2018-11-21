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
* @file xmetile_perfcnt.c
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xmegbl.h"
#include "xmegbl_defs.h"
#include "xmegbl_reginit.h"
#include "xmetile_perfcnt.h"

/***************************** Macro Definitions *****************************/
#define XMETILE_PERFCNT_MODULE_CORE		0x0
#define XMETILE_PERFCNT_MODULE_PL		0x1
#define XMETILE_PERFCNT_MODULE_MEM		0x2

/************************** Variable Definitions *****************************/
extern XMeGbl_RegPerfCtrls PerfCtrl[];
extern XMeGbl_RegPerfCtrlReset PerfCtrlReset[];
extern XMeGbl_RegPerfCounter PerfCounter[];
extern XMeGbl_RegPerfCounterEvent PerfCounterEvent[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is an internal API to set the performance counter control.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	ModId - Module ID. Should be one of
* XMETILE_PERFCNT_MODULE_CORE, XMETILE_PERFCNT_MODULE_PL, or
* XMETILE_PERFCNT_MODULE_MEM.
* @param	Counter - Counter ID. The max value differs upon @ModID
* @param	StartEvent - Event ID to start
* @param	StopEvent - Event ID to stop
* @param	ResetEvent - Event ID to reset
*
* @return	XME_SUCCESS on success
*
* @note		Used only within this file.
*
*******************************************************************************/
static u8 _XMeTile_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 ModId,
		u8 Counter, u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	u64 RegAddr;
	u32 FldVal = 0;
	u32 FldMask = 0;
	u64 Addr = TileInstPtr->TileAddr;

	if (StartEvent != XMETILE_PERFCNT_EVENT_INVALID) {
		FldMask |= PerfCtrl[ModId].Start[Counter].Mask;
		FldVal |= XMe_SetField(StartEvent,
				PerfCtrl[ModId].Start[Counter].Lsb,
				PerfCtrl[ModId].Start[Counter].Mask);
	}

	if (StopEvent != XMETILE_PERFCNT_EVENT_INVALID) {
		FldMask |= PerfCtrl[ModId].Stop[Counter].Mask;
		FldVal |= XMe_SetField(StopEvent,
				PerfCtrl[ModId].Stop[Counter].Lsb,
				PerfCtrl[ModId].Stop[Counter].Mask);

	}

	if (StartEvent != XMETILE_PERFCNT_EVENT_INVALID ||
			StopEvent != XMETILE_PERFCNT_EVENT_INVALID) {
		RegAddr = Addr + PerfCtrl[ModId].RegOff[Counter];
		XMeGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	if (ResetEvent != XMETILE_PERFCNT_EVENT_INVALID) {
		RegAddr = Addr + PerfCtrlReset[ModId].RegOff[Counter];

		FldMask = PerfCtrlReset[ModId].Reset[Counter].Mask;
		FldVal = XMe_SetField(ResetEvent,
				PerfCtrlReset[ModId].Reset[Counter].Lsb,
				PerfCtrlReset[ModId].Reset[Counter].Mask);

		XMeGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileMem_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	return _XMeTile_PerfCounterControl(TileInstPtr,
			XMETILE_PERFCNT_MODULE_MEM, Counter, StartEvent,
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTilePl_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	return _XMeTile_PerfCounterControl(TileInstPtr,
			XMETILE_PERFCNT_MODULE_PL, Counter,
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u8 XMeTileCore_PerfCounterControl(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u16 StartEvent, u16 StopEvent, u16 ResetEvent)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 4);

	return _XMeTile_PerfCounterControl(TileInstPtr,
			XMETILE_PERFCNT_MODULE_CORE, Counter,
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
u32 XMeTileMem_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
		PerfCounter[XMETILE_PERFCNT_MODULE_MEM].RegOff[Counter]);
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
u32 XMeTilePl_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
			PerfCounter[XMETILE_PERFCNT_MODULE_PL].RegOff[Counter]);
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
u32 XMeTileCore_PerfCounterGet(XMeGbl_Tile *TileInstPtr, u8 Counter)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 4);

	return XMeGbl_Read32(TileInstPtr->TileAddr +
		PerfCounter[XMETILE_PERFCNT_MODULE_CORE].RegOff[Counter]);
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileMem_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			PerfCounter[XMETILE_PERFCNT_MODULE_MEM].RegOff[Counter],
			CounterVal);
	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTilePl_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	XMeGbl_Write32(TileInstPtr->TileAddr +
			PerfCounter[XMETILE_PERFCNT_MODULE_PL].RegOff[Counter],
			CounterVal);
	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileCore_PerfCounterSet(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 CounterVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
		PerfCounter[XMETILE_PERFCNT_MODULE_CORE].RegOff[Counter],
		CounterVal);
	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileMem_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	XMeGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XMETILE_PERFCNT_MODULE_MEM].RegOff[Counter],
		EventVal);
	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTilePl_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 2);

	XMeGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XMETILE_PERFCNT_MODULE_PL].RegOff[Counter],
		EventVal);
	return XME_SUCCESS;
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
* @return	XME_SUCCESS on success
*
* @note		None.
*
*******************************************************************************/
u32 XMeTileCore_PerfCounterEventValue(XMeGbl_Tile *TileInstPtr, u8 Counter,
		u32 EventVal)
{
	XMe_AssertNonvoid(TileInstPtr != XME_NULL);
	/*
	 * NOC / PL is actually not tile type. If any of those is set,
	 * treat as Shim which always has a PL module
	 */
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMNOC ||
			TileInstPtr->TileType == XMEGBL_TILE_TYPE_SHIMPL);
	XMe_AssertNonvoid(TileInstPtr->TileType == XMEGBL_TILE_TYPE_METILE);
	XMe_AssertNonvoid(Counter >= 0 && Counter < 4);

	XMeGbl_Write32(TileInstPtr->TileAddr +
		PerfCounterEvent[XMETILE_PERFCNT_MODULE_CORE].RegOff[Counter],
		EventVal);
	return XME_SUCCESS;
}

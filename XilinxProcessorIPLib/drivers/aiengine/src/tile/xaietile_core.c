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
* @file xaietile_core.c
* @{
*
* This file contains routines for the AIE Tile core control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/21/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info and addressed CR#1006573
* 1.2  Naresh  08/13/2018  Updated core wait done API to wait for Core_done
*                          status instead of Core_Enable and also added an
*                          API to read core done status
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Hyun    01/08/2019  Use the poll function
* 1.5  Nishad  03/20/2019  Fix return statement for XAieTile_CoreWaitCycles
* 1.6  Nishad  03/20/2019  Fix the usage of unintialized variable in
* 			   XAieTile_CoreWaitStatus
* 1.7  Hyun    06/27/2019  Use TimerReg
* </pre>
*
******************************************************************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiegbl_reginit.h"
#include "xaietile_core.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_RegCoreCtrl CoreCtrlReg;
extern XAieGbl_RegCoreSts CoreStsReg;
extern XAieGbl_RegTimer TimerReg[];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API writes to the Core_control register to enable and/or reset the AIE
* core of the selected tile. This API bluntly writes to the register and any
* gracefullness required in enabling/disabling and/or resetting/unresetting
* the core are required to be handled by the application layer.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	Enable - Enable/Disable the core (1-Enable,0-Disable).
* @param	Reset - Reset/Unreset the core (1-Reset,0-Unreset).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieTile_CoreControl(XAieGbl_Tile *TileInstPtr, u8 Enable, u8 Reset)
{
	u64 RegAddr;
	u32 RegVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

	/* Write to the Core control register */
	RegAddr = TileInstPtr->TileAddr + CoreCtrlReg.RegOff;

	RegVal = XAie_SetField(Enable, CoreCtrlReg.CtrlEn.Lsb,
					CoreCtrlReg.CtrlEn.Mask) |
		XAie_SetField(Reset, CoreCtrlReg.CtrlRst.Lsb,
					CoreCtrlReg.CtrlRst.Mask);

	XAieGbl_Write32(RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API implements a blocking wait function to check for the core status
* to be disabled or done. API comes out of the wait loop when core status
* changes to done/disable or the timeout elapses, whichever happens first.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	TimeOut - TimeOut in usecs. If set to 0, the default timeout
*               will be set to 500 usecs.
* @param        Status - 1 for Core_done and 0 for Disable
*               Use macros XAIETILE_CORE_STATUS_DONE/XAIETILE_CORE_STATUS_DISABLE
*
* @return	The requested status if wait completes successful, or !Status.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreWaitStatus(XAieGbl_Tile *TileInstPtr, u32 TimeOut, u32 Status)
{
	u64 RegAddr;
	u32 Mask;
	u32 Value;
	u8 Ret;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);
	XAie_AssertNonvoid(Status == XAIETILE_CORE_STATUS_DONE ||
				Status == XAIETILE_CORE_STATUS_DISABLE);

	if(Status == XAIETILE_CORE_STATUS_DONE) {
		Mask = CoreStsReg.Done.Mask;
		Value = XAIETILE_CORE_STATUS_DONE << CoreStsReg.Done.Lsb;
	} else {
		Mask = CoreStsReg.En.Mask;
		Value = XAIETILE_CORE_STATUS_DISABLE << CoreStsReg.En.Lsb;
	}

	/* Get the Core status register address */
	RegAddr = TileInstPtr->TileAddr + CoreStsReg.RegOff;

	if(TimeOut == 0U) {
		/* Set timeout to default value */
		TimeOut = XAIETILE_CORE_STATUS_DEF_WAIT_USECS;
	}

	if (XAieGbl_MaskPoll(RegAddr, Mask, Value, TimeOut) == XAIE_SUCCESS) {
		Ret = Status;
	} else {
		Ret = !Status;
	}

	return Ret;
}

/*****************************************************************************/
/**
*
* This API returns the current value of the Core status done bit.
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	Core_Done status bit.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreReadStatusDone(XAieGbl_Tile *TileInstPtr)
{
        u32 RegVal;
        u8 Done = 0U;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

        /* Read the Core status register */
        RegVal = XAieGbl_Read32(TileInstPtr->TileAddr + CoreStsReg.RegOff);

        Done = XAie_GetField(RegVal, CoreStsReg.Done.Lsb, CoreStsReg.Done.Mask);
        return Done;
}

/*****************************************************************************/
/**
*
* This API implements a blocking wait function until the specified clock cyles
* are elapsed in the Core module 64-bit counter.
*
* @param	TileInstPtr - Pointer to the Tile instance.
* @param	CycleCnt - No. of timer clock cycles to elapse.
*
* @return	XAIE_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
u8 XAieTile_CoreWaitCycles(XAieGbl_Tile *TileInstPtr, u32 CycleCnt)
{
        u64 StartVal;
        u64 EndVal;
        u64 CurVal = 0U;
	u32 StartHigh;
        u32 StartLow;
        u32 CurHigh;
        u32 CurLow;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

        /* Read the timer high and low values before wait */
        StartLow = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_CORE].LowOff);
        StartHigh = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_CORE].HighOff);
        StartVal = ((u64)StartHigh << 0x20U) | StartLow;

        EndVal = StartVal + CycleCnt;

        while(CurVal < EndVal) {
                /* Read the timer high and low values */
                CurLow = XAieGbl_Read32(TileInstPtr->TileAddr +
				TimerReg[XAIETILE_TIMER_MODULE_CORE].LowOff);
                CurHigh = XAieGbl_Read32(TileInstPtr->TileAddr +
				TimerReg[XAIETILE_TIMER_MODULE_CORE].HighOff);
                CurVal = ((u64)CurHigh << 0x20U) | CurLow;
        }

	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API returns the current value of the Core module 64-bit timer.
*
* @param	TileInstPtr - Pointer to the Tile instance.
*
* @return	64-bit timer value.
*
* @note		None.
*
*******************************************************************************/
u64 XAieTile_CoreReadTimer(XAieGbl_Tile *TileInstPtr)
{
	u32 CurValHigh;
	u32 CurValLow;
	u64 CurVal;

	XAie_AssertNonvoid(TileInstPtr != XAIE_NULL);

	/* Read the timer high and low values before wait */
	CurValLow = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_CORE].LowOff);
	CurValHigh = XAieGbl_Read32(TileInstPtr->TileAddr +
			TimerReg[XAIETILE_TIMER_MODULE_CORE].HighOff);
	CurVal = ((u64)CurValHigh << 0x20U) | CurValLow;

	return CurVal;
}

/** @} */


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
* @file xmetile_core.c
* @{
*
* This file contains routines for the ME Tile core control.
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
* </pre>
*
******************************************************************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmegbl_reginit.h"
#include "xmetile_core.h"

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XMeGbl_RegCoreCtrl CoreCtrlReg;
extern XMeGbl_RegCoreSts CoreStsReg;
extern XMeGbl_RegTimer CoreTimerReg;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API writes to the Core_control register to enable and/or reset the ME
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
void XMeTile_CoreControl(XMeGbl_Tile *TileInstPtr, u8 Enable, u8 Reset)
{
	u64 RegAddr;
	u32 RegVal;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

	/* Write to the Core control register */
	RegAddr = TileInstPtr->TileAddr + CoreCtrlReg.RegOff;

	RegVal = XMe_SetField(Enable, CoreCtrlReg.CtrlEn.Lsb,
					CoreCtrlReg.CtrlEn.Mask) |
		XMe_SetField(Reset, CoreCtrlReg.CtrlRst.Lsb,
					CoreCtrlReg.CtrlRst.Mask);

	XMeGbl_Write32(RegAddr, RegVal);
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
*               Use macros XMETILE_CORE_STATUS_DONE/XMETILE_CORE_STATUS_DISABLE
*
* @return	0 if wait done successful and core disabled, else 1.
*
* @note		None.
*
*******************************************************************************/
u8 XMeTile_CoreWaitStatus(XMeGbl_Tile *TileInstPtr, u32 TimeOut, u32 Status)
{
	u64 RegAddr;
	u32 RegVal;
        u32 LoopCnt;
        u32 Lsb;
        u32 Mask;
        u8 State;
        u8 StateExp;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

        if(TimeOut == 0U) {
                /* Set timeout to default value */
                TimeOut = XMETILE_CORE_STATUS_DEF_WAIT_USECS;
        }

        if(Status == XMETILE_CORE_STATUS_DONE) {
                Lsb = CoreStsReg.Done.Lsb;
                Mask = CoreStsReg.Done.Mask;
                StateExp = XMETILE_CORE_STATUS_DONE;
        } else if(Status == XMETILE_CORE_STATUS_DISABLE) {
                Lsb = CoreStsReg.En.Lsb;
                Mask = CoreStsReg.En.Mask;
                StateExp = XMETILE_CORE_STATUS_DISABLE;
        }

        /* Loop count rounded off */
	LoopCnt = (TimeOut + XMETILE_CORE_STATUS_MIN_WAIT_USECS - 1U) /
					XMETILE_CORE_STATUS_MIN_WAIT_USECS;

        /* Get the Core status register address */
        RegAddr = TileInstPtr->TileAddr + CoreStsReg.RegOff;

	while(LoopCnt > 0U) {
                /* Read the Core status register */
                RegVal = XMeGbl_Read32(RegAddr);

                /* Get the core done/disable status */
		State = XMe_GetField(RegVal, Lsb, Mask);

		if(State == StateExp) {
			break;
		}

		XMe_usleep(XMETILE_CORE_STATUS_MIN_WAIT_USECS);
		LoopCnt--;
	}

        return State;
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
u8 XMeTile_CoreReadStatusDone(XMeGbl_Tile *TileInstPtr)
{
        u32 RegVal;
        u8 Done = 0U;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

        /* Read the Core status register */
        RegVal = XMeGbl_Read32(TileInstPtr->TileAddr + CoreStsReg.RegOff);

        Done = XMe_GetField(RegVal, CoreStsReg.Done.Lsb, CoreStsReg.Done.Mask);
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
* @return	None.
*
* @note		None.
*
*******************************************************************************/
u8 XMeTile_CoreWaitCycles(XMeGbl_Tile *TileInstPtr, u32 CycleCnt)
{
        u64 StartVal;
        u64 EndVal;
        u64 CurVal = 0U;
	u32 StartHigh;
        u32 StartLow;
        u32 CurHigh;
        u32 CurLow;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

        /* Read the timer high and low values before wait */
        StartLow = XMeGbl_Read32(TileInstPtr->TileAddr + CoreTimerReg.LowOff);
        StartHigh = XMeGbl_Read32(TileInstPtr->TileAddr + CoreTimerReg.HighOff);
        StartVal = ((u64)StartHigh << 0x20U) | StartLow;

        EndVal = StartVal + CycleCnt;

        while(CurVal < EndVal) {
                /* Read the timer high and low values */
                CurLow = XMeGbl_Read32(TileInstPtr->TileAddr +
                                                CoreTimerReg.LowOff);
                CurHigh = XMeGbl_Read32(TileInstPtr->TileAddr +
                                                CoreTimerReg.HighOff);
                CurVal = ((u64)CurHigh << 0x20U) | CurLow;
        }
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
u64 XMeTile_CoreReadTimer(XMeGbl_Tile *TileInstPtr)
{
        u32 CurValHigh = 0U;
        u32 CurValLow = 0U;
        u64 CurVal = 0U;

	XMe_AssertNonvoid(TileInstPtr != XME_NULL);

        /* Read the timer high and low values before wait */
        CurValLow = XMeGbl_Read32(TileInstPtr->TileAddr + CoreTimerReg.LowOff);
        CurValHigh = XMeGbl_Read32(TileInstPtr->TileAddr +CoreTimerReg.HighOff);
        CurVal = ((u64)CurValHigh << 0x20U) | CurValLow;

        return CurVal;
}

/** @} */


/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file cortexr5_sleep.c
 * @addtogroup xiltimer_v1_0
 * @{
 * @details
 *
 * This file contains the definitions for sleep implementation using cortexr5
 * processor instructions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
*  1.0  adk	 24/11/21 Initial release.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xpm_counter.h"

#ifdef XTIMER_IS_DEFAULT_TIMER
#include "xcortexr5_config.h"
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XCortexr5_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType);

/****************************************************************************/
/**
 * Initialize the cortexr5 sleep timer
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XCortexr5_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function configures the sleep interval using cortexr5 processor
 * instructions
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XCortexr5_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType)
{
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	/* For the CortexR5 PMU cycle counter. As boot code is setting up "D"
	 * bit in PMCR, cycle counter increments on every 64th bit of processor cycle
	 */
	u32 frequency = XGet_CpuFreq()/64;

#if defined (__GNUC__)
	TimeLowVal1 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
	Xpm_ReadCycleCounterVal(TimeLowVal1);
#endif

	tEnd = (u64)TimeLowVal1 + ((u64)(delay) * (frequency/(DelayType)));

	do {
#if defined (__GNUC__)
		TimeLowVal2 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
		Xpm_ReadCycleCounterVal(TimeLowVal2);
#endif
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

/****************************************************************************/
/**
 * Get the time from the PM Read cycle counter.
 *
 * @param	Xtime_Global: Pointer to the 64-bit location which will be
 * 		updated with the current timer value.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global) {
	XTimer *InstancePtr = &TimerInst;

	*Xtime_Global = Xpm_ReadCycleCounterVal();
}
#endif

#ifdef XTIMER_NO_TICK_TIMER
/****************************************************************************/
/**
 * Initialize the cortexr5 Tick Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = NULL;
	InstancePtr->XTimer_TickInterval = NULL;
	InstancePtr->XTickTimer_Stop = NULL;
	InstancePtr->XTickTimer_ClearInterrupt = NULL;
	return XST_SUCCESS;
}
#endif

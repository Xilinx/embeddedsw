/******************************************************************************
*
* Copyright (C) 2014-2017 Xilinx, Inc. All rights reserved.
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
*
* @file microblaze_sleep.c
*
* Contains implementation of microblaze sleep function.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 4.1   hk   04/18/14 Add sleep function.
* 6.0   asa  08/15/16 Updated the sleep/usleep signature. Fix for CR#956899.
* 6.6   srm  10/18/17 Updated sleep routines to support user configurable
*                     implementation. Now sleep routines will use Axi Timer
*                     or machine cycles as specified by the user.
* 6.8   mus  10/04/18 FreeRTOS BSP always use default method for sleep
*                     implementation, It does not have user configurable sleep
*                     implementation support, that is why FreeRTOS tcl does not
*                     export XSLEEP_TIMER_IS_DEFAULT_TIMER macro to
*                     xparameters.h. Modified code to always use default timer
*                     path for FreeRTOS BSP, based on the "FREERTOS_BSP" macro.
*                     It fixes CR#1012363.
*
* </pre>
*
* @note
*
* This file may contain architecture-dependent code.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "microblaze_sleep.h"
#include "bspconfig.h"

/***************** Macros (Inline Functions) Definitions *********************/

#if defined (XSLEEP_TIMER_IS_AXI_TIMER)
#define COUNTS_PER_SECOND (SLEEP_TIMER_FREQUENCY)
#define COUNTS_PER_MSECOND (COUNTS_PER_SECOND / 1000)
#define COUNTS_PER_USECOND (COUNTS_PER_SECOND / 1000000)
#warning "May wait for more than the specified delay"
#elif defined (XSLEEP_TIMER_IS_DEFAULT_TIMER) || defined (FREERTOS_BSP)
#define ITERS_PER_SEC	(XPAR_CPU_CORE_CLOCK_FREQ_HZ / 4)
#define ITERS_PER_MSEC	(ITERS_PER_SEC / 1000)
#define ITERS_PER_USEC	(ITERS_PER_MSEC / 1000)
#pragma message ("For the sleep routines, assembly instructions are used")
#endif

#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER) || defined (FREERTOS_BSP)
static void sleep_common(u32 n, u32 iters)
{
	asm volatile (
			"1:               \n\t"
			"addik %1, %1, -1 \n\t"
			"add   r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"or  r0, r0, r0   \n\t"
			:
			: "r"(iters), "r"(n)
			: "r0", "r7"
	);
}
#endif

/*****************************************************************************/
/**
* @brief    Provides delay for requested duration.
* @param	useconds- time in useconds.
* @return	0
*
* @note		Instruction cache should be enabled for this to work.
*
******************************************************************************/
int usleep_MB(unsigned long useconds)
{
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER) || defined (FREERTOS_BSP)
	sleep_common((u32)useconds, ITERS_PER_USEC);
#elif defined (XSLEEP_TIMER_IS_AXI_TIMER)
	/* Start Axi timer */
	XTime_StartAxiTimer();
	Xil_SleepAxiTimer(useconds, COUNTS_PER_USECOND);
#endif

	return 0;
}

/*****************************************************************************/
/**
* @brief    Provides delay for requested duration.
* @param	seconds- time in useconds.
* @return	0
*
* @note		Instruction cache should be enabled for this to work.
*
******************************************************************************/
unsigned sleep_MB(unsigned int seconds)
{
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER) || defined (FREERTOS_BSP)
	 sleep_common(seconds, ITERS_PER_SEC);
#elif defined (XSLEEP_TIMER_IS_AXI_TIMER)
	/* Start Axi timer */
	XTime_StartAxiTimer();
	Xil_SleepAxiTimer(seconds, COUNTS_PER_SECOND);
#endif

	return 0;
}

/*****************************************************************************/
/**
*
* @brief    Provides delay for requested duration..
*
* @param	MilliSeconds- Delay time in milliseconds.
*
* @return	None.
*
* @note		Instruction cache should be enabled for this to work.
*
******************************************************************************/
void MB_Sleep(u32 MilliSeconds)
{
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER) || defined (FREERTOS_BSP)
	sleep_common(MilliSeconds, ITERS_PER_MSEC);

#elif defined (XSLEEP_TIMER_IS_AXI_TIMER)
	/* Start Axi timer */
	XTime_StartAxiTimer();
	Xil_SleepAxiTimer(MilliSeconds, COUNTS_PER_MSECOND);
#endif
}

/*****************************************************************************/
/**
 *
 * @brief    Provides delay for requested duration by using Axi Timer.
 *
 * @param	 delay - delay time in seconds/milli seconds/micro seconds.
 *           frequency - Number of counts per
 *                       second/milli second/micro second.
 *
 * @return	 None.
 *
 *
  ******************************************************************************/
#if defined (XSLEEP_TIMER_IS_AXI_TIMER)
static void Xil_SleepAxiTimer(u32 delay, u64 frequency)
{
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;

	TimeLowVal1 = Xil_In32((SLEEP_TIMER_BASEADDR) +
			(XSLEEP_TIMER_AXITIMER_TCR_OFFSET));
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) * frequency);
	do
	{
		TimeLowVal2 = Xil_In32((SLEEP_TIMER_BASEADDR) +
				(XSLEEP_TIMER_AXITIMER_TCR_OFFSET));
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
	}while (tCur < tEnd);
}

/*****************************************************************************/
/**
 *
 * @brief   This API starts the Axi Timer only when the timer is not enabled.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note    Instruction cache should be enabled for this to work.
 *
  ******************************************************************************/
  static void XTime_StartAxiTimer()
  {
	u32 ControlStatusReg;

	/*  Checking if the timer is enabled  */
	if(Xil_In32(SLEEP_TIMER_BASEADDR + XSLEEP_TIMER_AXITIMER_TCSR0_OFFSET) &&
	                                   XSLEEP_TIMER_AXITIMER_CSR_ENABLE_TMR_MASK)
	{
		return;
	}
	/*
	 * Read the current register contents such that only the necessary bits
	 * of the register are modified in the following operations
	 */
	ControlStatusReg = Xil_In32(SLEEP_TIMER_BASEADDR +
			                          XSLEEP_TIMER_AXITIMER_TCSR0_OFFSET);
	/*
	 * Remove the reset condition such that the timer counter starts running
	 * with the value loaded from the compare register
	 */
	Xil_Out32((SLEEP_TIMER_BASEADDR + XSLEEP_TIMER_AXITIMER_TCSR0_OFFSET),
			(ControlStatusReg | XSLEEP_TIMER_AXITIMER_CSR_ENABLE_TMR_MASK |
				       XSLEEP_TIMER_AXITIMER_CSR_AUTO_RELOAD_MASK));
  }
#endif

/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file riscv_sleep.c
*
* Contains implementation of RISC-V sleep function.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 9.1   mus   10/24/23  Initial version.
* </pre>
*
* @note
*
* This file may contain architecture-dependent code.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "riscv_sleep.h"
#include "bspconfig.h"

/***************** Macros (Inline Functions) Definitions *********************/

#if defined (XSLEEP_TIMER_IS_AXI_TIMER)
#define COUNTS_PER_SECOND (SLEEP_TIMER_FREQUENCY)
#define COUNTS_PER_MSECOND ((COUNTS_PER_SECOND + 500) / 1000)
#define COUNTS_PER_USECOND ((COUNTS_PER_SECOND + 500000)/ 1000000)
#warning "May wait for more than the specified delay"
#elif defined (XSLEEP_TIMER_IS_DEFAULT_TIMER)
#define ITERS_PER_SEC	Xil_GetRISCVFrequency()
#define ITERS_PER_MSEC	((ITERS_PER_SEC + 500U) / 1000U)
#define ITERS_PER_USEC	((ITERS_PER_MSEC + 500U) / 1000U)
#pragma message ("For the sleep routines, rdtime is used")
#endif
/***************** Variable Definitions *********************/
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER)
/**
 * This variable stores current value of Microblaze frequency.
 * It would be used by sleep routines to generate delay.
 */
static u32 RISCVFreq = XPAR_CPU_CORE_CLOCK_FREQ_HZ;
#endif

/*****************************************************************************/

#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER)
/*****************************************************************************/
/**
* @brief	Sets variable which stores RISC-V frequency value
* @param	Val - Frequency value to be set
* @return	XST_SUCCESS - If frequency updated successfully
* 			XST_INVALID_PARAM - If specified frequency value is not valid
*
* @note		It must be called after runtime change in RISC-V frequency,
* 			failing to do so would result in to incorrect behavior of sleep
* 			routines
*
******************************************************************************/
u32 Xil_SetRISCVFrequency(u32 Val)
{
	if ( Val != 0U) {
		RISCVFreq = Val;
		return XST_SUCCESS;
	}
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* @brief	Returns current RISC-V frequency value
* @return	RISCVFreq - Current RISC-V frequency value
*
******************************************************************************/
u32 Xil_GetRISCVFrequency(void)
{
	return RISCVFreq;
}

/*****************************************************************************/
/**
 *
 * @brief    Provides delay for requested duration by using RISC-V counter
 *
 * @param        delay - delay time in seconds/milli seconds/micro seconds.
 *           frequency - Number of counts per
 *                       second/milli second/micro second.
 *
 * @return       None.
 *
 *
  ******************************************************************************/
static void Xil_SleepRISCVTimer(u32 delay, u64 frequency)
{
        u64 tEnd = 0U;
        u64 tCur = 0U;
        u32 TimeHighVal = 0U;
        u32 TimeLowVal1 = 0U;
        u32 TimeLowVal2 = 0U;

        TimeLowVal1 = rdtime();
        tEnd = (u64)TimeLowVal1 + ((u64)(delay) * frequency);
        do
        {
                TimeLowVal2 = rdtime();
                if (TimeLowVal2 < TimeLowVal1) {
                        TimeHighVal++;
                }
                TimeLowVal1 = TimeLowVal2;
                tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
        }while (tCur < tEnd);
}

#endif

/*****************************************************************************/
/**
* @brief    Provides delay for requested duration.
* @param	useconds - time in useconds.
* @return	0
*
* @note		Instruction cache should be enabled for this to work.
*
******************************************************************************/
void usleep_riscv(ULONG useconds)
{
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER)
	Xil_SleepRISCVTimer((u32)useconds, ITERS_PER_USEC);
#elif defined (XSLEEP_TIMER_IS_AXI_TIMER)
	/* Start Axi timer */
	XTime_StartAxiTimer();
	Xil_SleepAxiTimer(useconds, COUNTS_PER_USECOND);
#endif

}

/*****************************************************************************/
/**
* @brief    Provides delay for requested duration.
* @param	seconds - time in useconds.
* @return	0
*
* @note		Instruction cache should be enabled for this to work.
*
******************************************************************************/
void sleep_riscv(u32 seconds)
{
#if defined (XSLEEP_TIMER_IS_DEFAULT_TIMER)
	 Xil_SleepRISCVTimer(seconds, ITERS_PER_SEC);
#elif defined (XSLEEP_TIMER_IS_AXI_TIMER)
	/* Start Axi timer */
	XTime_StartAxiTimer();
	Xil_SleepAxiTimer(seconds, COUNTS_PER_SECOND);
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
  static void XTime_StartAxiTimer(void)
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

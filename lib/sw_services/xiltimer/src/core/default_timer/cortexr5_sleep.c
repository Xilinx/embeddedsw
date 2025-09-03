/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file cortexr5_sleep.c
 * @addtogroup xiltimer_api XilTimer APIs
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
*  	adk	 12/01/22 Fix compilation errors.
*  1.1	adk      08/08/22 Added support for versal net.
*  	adk      08/08/22 Added doxygen tags.
*  2.2  adk	 05/03/25 Update LPD_RST_TIMESTAMP and XIOU_SCNTRS_BASEADDR
*  			  defines for Versal 2VE and 2VM platforms.
*  	adk      09/05/25 Adjust the delay calculation logic for the R52
*  	                  processor to use XIOU_SCNTRS_FREQ.
*  2.3  adk      02/09/25 Updated the code to initialize global timer only
*  			  once to avoid issues in AMP systems.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xpm_counter.h"
#if defined(SDT) && defined (ARMR52)
#include "xcortexr52_config.h"
#elif defined(SDT)
#include "xcortexr5_config.h"
#endif

#if defined (ARMR52)
#if defined (VERSAL_2VE_2VM)
#define LPD_RST_TIMESTAMP  0xEB5E03A4U
#define XIOU_SCNTRS_BASEADDR 0xEA470000U
#else
#define LPD_RST_TIMESTAMP  0xEB5E035CU
#define XIOU_SCNTRS_BASEADDR 0xEB5B0000U
#endif
#define XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET 0x0U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK 0x1U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN 0x1U
#define XIOU_SCNTRS_FREQ_REG_OFFSET 0x20U
#ifdef SDT
#define XIOU_SCNTRS_FREQ XPAR_CPU_TIMESTAMP_CLK_FREQ
#else
#define XIOU_SCNTRS_FREQ XPAR_CPU_CORTEXR52_0_TIMESTAMP_CLK_FREQ
#endif
#endif

#ifdef XTIMER_IS_DEFAULT_TIMER
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XCortexr5_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType);
#if defined (ARMR52)
static void XGlobalTimer_Start(XTimer *InstancePtr);
#endif

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

#if defined (ARMR52)
/****************************************************************************/
/**
 * Start the Global Timer
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	None
 */
/****************************************************************************/
static void XGlobalTimer_Start(XTimer *InstancePtr)
{
	(void) InstancePtr;
	u32 IsEnabled;
	u32 TimeStampFreq;

	IsEnabled = Xil_In32(XIOU_SCNTRS_BASEADDR +
			     XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET);
	TimeStampFreq = Xil_In32(XIOU_SCNTRS_BASEADDR +
				 XIOU_SCNTRS_FREQ_REG_OFFSET);

	if ((!(IsEnabled & XIOU_SCNTRS_CNT_CNTRL_REG_EN)) && !TimeStampFreq) {
		/* Take LPD_TIMESTAMP out of reset */
		Xil_Out32(LPD_RST_TIMESTAMP, 0x0);

		/* write frequency to System Time Stamp Generator Register */
		Xil_Out32((XIOU_SCNTRS_BASEADDR +
			   XIOU_SCNTRS_FREQ_REG_OFFSET),
			  XIOU_SCNTRS_FREQ);

		/* Enable the timer/counter */
		Xil_Out32((XIOU_SCNTRS_BASEADDR +
			   XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET),
			  XIOU_SCNTRS_CNT_CNTRL_REG_EN);
	}
}

static inline u64 arch_counter_get_cntvct(void)
 {
          u64 cval;
          __asm__ __volatile__("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
          return cval;
  }
#endif

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
	(void) InstancePtr;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	/* For the CortexR5 PMU cycle counter. As boot code is setting up "D"
	 * bit in PMCR, cycle counter increments on every 64th bit of processor cycle
	 */
#if !defined (ARMR52)
#ifndef SDT
	u32 frequency = XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ/64;
#else
	u32 frequency = XGet_CpuFreq()/64;
#endif
#else
	u32 frequency = XIOU_SCNTRS_FREQ;
#endif
#if defined (ARMR52)
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XGlobalTimer_Start(InstancePtr);
		IsSleepTimerStarted = TRUE;
	}
	TimeLowVal1 = arch_counter_get_cntvct();
#elif defined (__GNUC__)
	TimeLowVal1 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
	Xpm_ReadCycleCounterVal(TimeLowVal1);
#endif

	tEnd = (u64)TimeLowVal1 + ((u64)(delay) * (frequency/(DelayType)));

	do {
#if defined (ARMR52)
		TimeLowVal2 = arch_counter_get_cntvct();
#elif defined (__GNUC__)
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

#if defined (ARMR52)
	*Xtime_Global = arch_counter_get_cntvct();
#else
	*Xtime_Global = Xpm_ReadCycleCounterVal();
#endif
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

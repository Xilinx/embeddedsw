/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file globaltimer_sleep_zynq.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * @details
 *
 * This file contains the definitions for sleep implementation using
 * global timer.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0  adk	 24/11/21 Initial release.
 * 1.1	adk      08/08/22 Added doxygen tags.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"

#ifdef XTIMER_IS_DEFAULT_TIMER
#ifdef SDT
#include "xcortexa9_config.h"
#endif

/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
#define	GLOBAL_TMR_BASEADDR	0xF8F00200U
#define GTIMER_COUNTER_LOWER_OFFSET	0x00U
#define GTIMER_COUNTER_UPPER_OFFSET	0x04U
#define GTIMER_CONTROL_OFFSET	0x08U

/************************** Function Prototypes ******************************/
static void XGlobalTimer_Start(XTimer *InstancePtr);
static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
					XTimer_DelayType DelayType);

/****************************************************************************/
/**
 * Initialize the global timer sleep timer
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XGlobalTimer_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;

	return XST_SUCCESS;
}

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

	/* Disable Global Timer */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_CONTROL_OFFSET, (u32)0x0);

	/* Updating Global Timer Counter Register */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_COUNTER_LOWER_OFFSET, 0x0);
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_COUNTER_UPPER_OFFSET,
		0x0);

	/* Enable Global Timer */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_CONTROL_OFFSET, (u32)0x1);
}

/*****************************************************************************/
/**
 * This function configures the sleep interval using global timer
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
					XTimer_DelayType DelayType)
{
#if defined(__GNUC__)
	(void) InstancePtr;
#endif
	XTime tEnd, tCur;
#ifndef SDT
        u32 CpuFreq = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ;
#else
        u32 CpuFreq = XGet_CpuFreq();
#endif

        /* Global Timer is always clocked at half of the CPU frequency */
        u32 TimerCountsPersec = CpuFreq/2;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XGlobalTimer_Start(InstancePtr);
		IsSleepTimerStarted = TRUE;
	}

	XTime_GetTime(&tCur);
	tEnd = tCur + (((XTime) delay) * (TimerCountsPersec / DelayType));
        do {
		XTime_GetTime(&tCur);
        } while (tCur < tEnd);

}

/****************************************************************************/
/**
 * Get the time from the Global Timer counter.
 *
 * @param	Xtime_Global: Pointer to the 64-bit location which will be
 * 		updated with the current timer value.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global)
{
	u32 low;
	u32 high;

	/* Reading Global Timer Counter Register */
	do
	{
		high = Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_UPPER_OFFSET);
		low = Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_LOWER_OFFSET);
	} while(Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_UPPER_OFFSET) != high);

	*Xtime_Global = (((XTime) high) << 32U) | (XTime) low;
}

#endif /* XTIMER_IS_DEFAULT_TIMER */

#ifdef XTIMER_NO_TICK_TIMER
/****************************************************************************/
/**
 * Initialize the global timer Tick Instance
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

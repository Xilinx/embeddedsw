/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file globaltimer_sleep.c
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
 * 1.0   adk  24/11/21 Initial release.
 *  	 adk  25/03/22 Fix compilation errors for a72 processor.
 * 1.1	 adk  08/08/22 Added support for versal net.
 *  	 adk  08/08/22 Added doxygen tags.
 * 2.0   adk  04/07/24 Update the checks for A53 32-bit configuration.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"

#ifdef XTIMER_IS_DEFAULT_TIMER
#ifdef SDT
#include "xarmv8_config.h"
#endif

/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
#if defined(VERSAL_NET)
#define XIOU_SCNTRS_BASEADDR	0xEC920000U
#elif defined (versal)
#define XIOU_SCNTRS_BASEADDR	0xFF140000U
#else
#define XIOU_SCNTRS_BASEADDR	0xFF260000U
#endif
#define XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET	0x00000000U
#define XIOU_SCNTRS_FREQ_REG_OFFSET		0x00000020U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN            0x00000001U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK	0x00000001U

/************************** Function Prototypes ******************************/
static void XGlobalTimer_Start(XTimer *InstancePtr);
static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
					XTimer_DelayType DelayType);

#if defined(ARMA53_32)
static inline u64 arch_counter_get_cntvct(void)
 {
          u64 cval;
          __asm__ __volatile__("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
          return cval;
  }
#endif
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

#if defined(SDT) && !defined(ARMA53_32)
	u32 TimerStampFreq = XGet_TimeStampFreq();
	mtcp(CNTFRQ_EL0, TimerStampFreq);
#endif

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
#ifndef SDT
#if defined(VERSAL_NET)
	u32 TimerStampFreq = XPAR_CPU_CORTEXA78_0_TIMESTAMP_CLK_FREQ;
#elif defined (versal)
        u32 TimerStampFreq = XPAR_CPU_CORTEXA72_0_TIMESTAMP_CLK_FREQ;
#else
        u32 TimerStampFreq = XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ;
#endif
#else
        u32 TimerStampFreq = XGet_TimeStampFreq();
#endif

	if (EL3 == 1){
                /* Enable the global timer counter only if it is disabled */
                if(((Xil_In32(XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET))
                                        & XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK) !=
                                        XIOU_SCNTRS_CNT_CNTRL_REG_EN){
                        /*write frequency to System Time Stamp Generator Register*/
                        Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_FREQ_REG_OFFSET),
                                   TimerStampFreq);
                        /*Enable the timer/counter*/
                        Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET)
                                                ,XIOU_SCNTRS_CNT_CNTRL_REG_EN);
                }
        }
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
	(void) InstancePtr;
	XTime tEnd, tCur;
#ifndef SDT
#if defined(VERSAL_NET)
	u32 TimerStampFreq = XPAR_CPU_CORTEXA78_0_TIMESTAMP_CLK_FREQ;
#elif defined (versal)
        u32 TimerStampFreq = XPAR_CPU_CORTEXA72_0_TIMESTAMP_CLK_FREQ;
#else
        u32 TimerStampFreq = XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ;
#endif
#else
        u32 TimerStampFreq = XGet_TimeStampFreq();
#endif
        u32 iterpersec = TimerStampFreq;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XGlobalTimer_Start(InstancePtr);
		IsSleepTimerStarted = TRUE;
	}
#if defined(ARMA53_32)
	tCur = arch_counter_get_cntvct();
#else
	tCur = mfcp(CNTPCT_EL0);
#endif
	tEnd = tCur + (((XTime) delay) * (iterpersec / DelayType));
        do {
#if defined(ARMA53_32)
                tCur = arch_counter_get_cntvct();
#else
                tCur = mfcp(CNTPCT_EL0);
#endif
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
#if defined(ARMA53_32)
	*Xtime_Global = arch_counter_get_cntvct();
#else
	*Xtime_Global = mfcp(CNTPCT_EL0);
#endif
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

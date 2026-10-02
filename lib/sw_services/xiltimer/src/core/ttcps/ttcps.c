/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file ttcps.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk   24/11/21 Initial release.
 *  	 adk   07/02/22 Updated the IntrHandler as per XTimer_SetHandler() API
 *  	 	        and removed the unneeded XTickTimer_SetPriority() API.
 * 1.1   adk   08/08/22 Added doxygen tags.
 * 1.3   gm    21/07/23 Added Timer Release Callback function support.
 * 1.3   asa   08/09/23 Added macros to ensure that for Zynq/CortexA9
 *                      16 bit TTC counters are used.
 * 2.0   ml    29/03/24 Added description to fix doxygen warnings.
 * 2.4   bdk   13/11/25 Typecasted with proper datatypes to fix MISRA-C
 *			violation rules for 10.1 and 17.7
 * 2.4   vmt   08/20/26 Added weak sleep and tick timer instance variables.
 * 2.4   vmt   09/01/26 Added XTimer_TtcTickIntervalHz() so TTC tick rates can
 *			be programmed in Hz.
 *  2.5  ml    09/22/26 Fixed doxygen 1.15.0 and compliance checker warnings.
*</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xttcps.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

#if defined (ARMR5) || (__aarch64__) || (ARMA53_32)

/**
 * the shift amount used when concatenating two registers
 * to form a 64-bit value. the shift amount is 32, indicating
 * a 32-bit register width.
 */
#define REG_SHIFT  				32U

/**
 * the type used to represent the value of a timer counter.
 * Depending on the architecture being targeted, it is a
 * 32-bit unsigned integer (u32).
 */
#define XCntrVal 			    u32
#else

/**
 * the shift amount used when concatenating two registers
 * to form a 64-bit value. the shift amount is 32, indicating
 * a 16-bit register width.
 */
#define REG_SHIFT  16U

/**
 * the type used to represent the value of a timer counter.
 * Depending on the architecture being targeted, it is a
 * 16-bit unsigned integer (u16).
 */
#define XCntrVal 			    u16
#endif


/************************** Function Prototypes ******************************/
static u32 XTimer_TtcInit(UINTPTR BaseAddress,
			  XTtcPs *TtcPsInstPtr);
#ifdef XSLEEPTIMER_IS_TTCPS
static void XTimer_TtcModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_TtcStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_TTCPS
void XTtc_CallbackHandler(void *CallBackRef, u32 StatusEvent);
static void XTimer_TtcTickInterval(XTimer *InstancePtr, u32 Delay);
static void XTimer_TtcTickIntervalHz(XTimer *InstancePtr, u32 FreqHz);
static void XTimer_TtcSetIntrHandler(XTimer *InstancePtr, u8 Priority);
static void XTickTimer_TtcStop(XTimer *InstancePtr);
static void XTickTimer_ClearTtcInterrupt(XTimer *InstancePtr);
#if defined  (XPM_SUPPORT)
static void XTickTimer_ReleaseTickTimer(XTimer *InstancePtr);
#endif
#endif

#ifdef XSLEEPTIMER_IS_TTCPS
/****************************************************************************/
/**
 * Initialize the scutimer Sleep Instance
 *
 * @param[out] InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XTimer_TtcModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_TtcStop;
	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_TTCPS
/****************************************************************************/
/**
 * Initialize the scutimer Tick Instance
 *
 * @param[out] InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XTimer_TtcSetIntrHandler;
	InstancePtr->XTimer_TickInterval = XTimer_TtcTickInterval;
	InstancePtr->XTimer_TickIntervalHz = XTimer_TtcTickIntervalHz;
	InstancePtr->XTickTimer_Stop = XTickTimer_TtcStop;
	InstancePtr->XTickTimer_ClearInterrupt = XTickTimer_ClearTtcInterrupt;
#if defined  (XPM_SUPPORT)
	InstancePtr->XTickTimer_ReleaseTickTimer = XTickTimer_ReleaseTickTimer;
#endif
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the TtcPs Instance
 *
 * @param[in] BaseAddress identifies the TTC instance by base address in SDT
 * 		builds or by device ID in non-SDT builds.
 * @param[out] TtcPsInstPtr is a pointer to the XTtcPs instance to initialize
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XTimer_TtcInit(UINTPTR BaseAddress,
			  XTtcPs *TtcPsInstPtr)
{
	u32 Status = XST_FAILURE;
	XTtcPs_Config *ConfigPtr;

	ConfigPtr = XTtcPs_LookupConfig(BaseAddress);
	if (!ConfigPtr) {
		return Status;
	}

	Status = XTtcPs_CfgInitialize(TtcPsInstPtr, ConfigPtr,
				      ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		XTtcPs_Stop(TtcPsInstPtr);
		Status = XTtcPs_CfgInitialize(TtcPsInstPtr, ConfigPtr,
					      ConfigPtr->BaseAddress);
	}

	XTtcPs_Start(TtcPsInstPtr);

	return Status;
}

#ifdef XTICKTIMER_IS_TTCPS
/*****************************************************************************/
/**
 * This function implements the interrupt callback handler
 *
 * @param[in] CallBackRef is Pointer to the XTimer instance
 * @param[in] StatusEvent is the status event
 *
 ****************************************************************************/
void XTtc_CallbackHandler(void *CallBackRef, u32 StatusEvent)
{
	(void)StatusEvent;
	XTimer *InstancePtr = (XTimer *)CallBackRef;

	InstancePtr->Handler(InstancePtr->CallBackRef, 0);
}

/*****************************************************************************/
/**
 * This function configures the ttcps tick interval from a frequency
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 * @param[in] FreqHz is the required tick frequency in Hz
 *
 * @note   The frequency is passed unchanged to XTtcPs_CalcIntervalFromFreq(),
 *	   so rates that are not exact divisors of 1000 are programmed as
 *	   accurately as the TTC input clock allows.
 *
 ****************************************************************************/
static void XTimer_TtcTickIntervalHz(XTimer *InstancePtr, u32 FreqHz)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;
	static XInterval Interval;
	static u8 Prescaler;
	static u32 IsTickTimerStarted = FALSE;

	Xil_AssertVoid(FreqHz > 0U);

	if (FALSE == IsTickTimerStarted) {
		XTimer_TtcInit(XTickTimer_Instance,
			       &InstancePtr->TtcPs_TickInst);
		IsTickTimerStarted = TRUE;
	}
	XTtcPs_SetOptions(TtcPsInstPtr, XTTCPS_OPTION_INTERVAL_MODE |
			  XTTCPS_OPTION_WAVE_DISABLE);
	XTtcPs_CalcIntervalFromFreq(TtcPsInstPtr, FreqHz, &Interval, &Prescaler);
	XTtcPs_SetInterval(TtcPsInstPtr, Interval);
	XTtcPs_SetPrescaler(TtcPsInstPtr, Prescaler);
	XTtcPs_EnableInterrupts(TtcPsInstPtr, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Start(TtcPsInstPtr);
}

/*****************************************************************************/
/**
 * This function configures the ttcps tick interval
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 * @param[in] Delay is the delay interval in milliseconds
 *
 ****************************************************************************/
static void XTimer_TtcTickInterval(XTimer *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(Delay > 0U);

	XTimer_TtcTickIntervalHz(InstancePtr, (u32)XTIMER_DELAY_MSEC / Delay);
}

/*****************************************************************************/
/**
 * This function implements the tick interrupt handler
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 * @param[in] Priority - Priority for the interrupt
 *
 ****************************************************************************/
static void XTimer_TtcSetIntrHandler(XTimer *InstancePtr, u8 Priority)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_SetStatusHandler(TtcPsInstPtr, InstancePtr,
				(XTtcPs_StatusHandler)XTtc_CallbackHandler);
	XSetupInterruptSystem(TtcPsInstPtr, XTtcPs_InterruptHandler,
#ifndef SDT
			      TtcPsInstPtr->Config.IntrId,
#else
			      TtcPsInstPtr->Config.IntrId[0],
#endif
			      TtcPsInstPtr->Config.IntrParent,
			      Priority);
}

/*****************************************************************************/
/**
 * This function implements the stop functionality for the tick timer
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 *
 ****************************************************************************/
static void XTickTimer_TtcStop(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_Stop(TtcPsInstPtr);
}

#if defined  (XPM_SUPPORT)
/*****************************************************************************/
/**
 * This function implements the release functionality for the tick timer
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 *
 ****************************************************************************/
static void XTickTimer_ReleaseTickTimer(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_Release(TtcPsInstPtr);
}
#endif

/*****************************************************************************/
/**
 * This function clears the tick interrupt status
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 *
 ****************************************************************************/
static void XTickTimer_ClearTtcInterrupt(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_ClearInterruptStatus(TtcPsInstPtr,
				    XTtcPs_GetInterruptStatus(TtcPsInstPtr));
}
#endif

#ifdef XSLEEPTIMER_IS_TTCPS
/*****************************************************************************/
/**
 * This function configures the sleep interval using ttcps
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 * @param[in] delay is the delay interval
 * @param[in] DelayType is the XTimer_DelayType
 *
 ****************************************************************************/
static void XTimer_TtcModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	XCntrVal TimeHighVal = 0U;
	XCntrVal TimeLowVal1 = 0U;
	XCntrVal TimeLowVal2 = 0U;
	static u32 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		(void)XTimer_TtcInit(XSleepTimer_Instance,
			       &InstancePtr->TtcPs_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XTtcPs_GetCounterValue(TtcPsInstPtr);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
				   TtcPsInstPtr->Config.InputClockHz / (u32)(DelayType));
	do {
		TimeLowVal2 = XTtcPs_GetCounterValue(TtcPsInstPtr);
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << REG_SHIFT) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

/*****************************************************************************/
/**
 * This function implements the stop functionality for the sleep timer
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 *
 ****************************************************************************/
static void XSleepTimer_TtcStop(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;

	XTtcPs_Stop(TtcPsInstPtr);
}

/****************************************************************************/
/**
 * Get the time from the TtcPS Counter Register.
 *
 * @param[out] Xtime_Global: Pointer to the 64-bit location which will be
 * 		updated with the current timer value.
 *
 ****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global)
{
	XTimer *InstancePtr = &TimerInst;
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;
	static u32 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		(void)XTimer_TtcInit(XSleepTimer_Instance,
			       &InstancePtr->TtcPs_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	*Xtime_Global = XTtcPs_GetCounterValue(TtcPsInstPtr);
}
#endif

/** @} */

/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file ttcps.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * @details
 *
 * This file contains the definitions for ttcps sleep implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk   24/11/21 Initial release.
 *  	 adk   07/02/22 Updated the IntrHandler as per XTimer_SetHandler() API
 *  	 	        and removed the unneeded XTickTimer_SetPriority() API.
 * 1.1   adk   08/08/22 Added doxygen tags.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xttcps.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

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
static void XTimer_TtcSetIntrHandler(XTimer *InstancePtr, u8 Priority);
static void XTickTimer_TtcStop(XTimer *InstancePtr);
static void XTickTimer_ClearTtcInterrupt(XTimer *InstancePtr);
#endif

#ifdef XSLEEPTIMER_IS_TTCPS
/****************************************************************************/
/**
 * Initialize the scutimer Sleep Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
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
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always

/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XTimer_TtcSetIntrHandler;
	InstancePtr->XTimer_TickInterval = XTimer_TtcTickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_TtcStop;
	InstancePtr->XTickTimer_ClearInterrupt = XTickTimer_ClearTtcInterrupt;
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the TtcPs Instance
 *
 * @param	BaseAddress is the Scutimer Instance baseaddress to be
 * 		worked on
 * @param	ScuTimerInstPtr is a pointer to the XScuTimer Instance
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
 * @param  CallBackRef is Pointer to the XTimer instance
 * @param  StatusEvent is the status event
 *
 * @return	None
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
 * This function configures the scutimer tick interval
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 *
 * @return	None
 *
 ****************************************************************************/
static void XTimer_TtcTickInterval(XTimer *InstancePtr, u32 Delay)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;
	static XInterval Interval;
	static u8 Prescaler;
	u32 Freq;
	static u8 IsTickTimerStarted = FALSE;

	if (FALSE == IsTickTimerStarted) {
		XTimer_TtcInit(XTICKTIMER_DEVICEID,
				&InstancePtr->TtcPs_TickInst);
		IsTickTimerStarted = TRUE;
	}
	Freq = XTIMER_DELAY_MSEC/Delay;
	XTtcPs_SetOptions(TtcPsInstPtr, XTTCPS_OPTION_INTERVAL_MODE |
			XTTCPS_OPTION_WAVE_DISABLE);
	XTtcPs_CalcIntervalFromFreq(TtcPsInstPtr, Freq, &Interval, &Prescaler);
	XTtcPs_SetInterval(TtcPsInstPtr, Interval);
	XTtcPs_SetPrescaler(TtcPsInstPtr, Prescaler);
	XTtcPs_EnableInterrupts(TtcPsInstPtr, XTTCPS_IXR_INTERVAL_MASK);
        XTtcPs_Start(TtcPsInstPtr);
}

/*****************************************************************************/
/**
 * This function implements the tick interrupt handler
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Priority - Priority for the interrupt
 *
 * @return	None
 *
 ****************************************************************************/
static void XTimer_TtcSetIntrHandler(XTimer *InstancePtr, u8 Priority)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_SetStatusHandler(TtcPsInstPtr, InstancePtr,
		              (XTtcPs_StatusHandler)XTtc_CallbackHandler);
	XSetupInterruptSystem(TtcPsInstPtr, XTtcPs_InterruptHandler,
			      TtcPsInstPtr->Config.IntrId,
			      TtcPsInstPtr->Config.IntrParent,
			      Priority);
}

/*****************************************************************************/
/**
 * This function implements the stop functionality for the tick timer
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 *
 * @return	None
 *
 ****************************************************************************/
static void XTickTimer_TtcStop(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_Stop(TtcPsInstPtr);
}

/*****************************************************************************/
/**
 * This function clears the tick interrupt status
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 *
 * @return	None
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
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XTimer_TtcModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XTimer_TtcInit(XSLEEPTIMER_DEVICEID,
				&InstancePtr->TtcPs_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XTtcPs_GetCounterValue(TtcPsInstPtr);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
                                   TtcPsInstPtr->Config.InputClockHz / (DelayType));
	do {
		TimeLowVal2 = XTtcPs_GetCounterValue(TtcPsInstPtr);
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

/*****************************************************************************/
/**
 * This function implements the stop functionality for the sleep timer
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 *
 * @return	None
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
	XTimer *InstancePtr = &TimerInst;
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;

	*Xtime_Global = XTtcPs_GetCounterValue(TtcPsInstPtr);
}/*@}*/
#endif

/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file axi_timer.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * @details
 *
 * This file contains the definitions for axi timer implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 *  1.0  adk   24/11/21 Initial release.
 *  	 adk   07/02/22 Updated the IntrHandler as per XTimer_SetHandler() API.
 *  1.1	 adk   08/08/22 Added doxygen tags.
 *</pre>
 *
 *@note
 *****************************************************************************/
#include "xiltimer.h"

/***************************** Include Files *********************************/
#include "xtmrctr.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XAxiTimer_Init(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTmrCtr *AxiTimerInstPtr);
#ifdef XSLEEPTIMER_IS_AXITIMER
static void XAxiTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_AxiTimerStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_AXITIMER
static void XAxiTimer_TickInterval(XTimer *InstancePtr, u32 Delay);
static void XAxiTimer_SetIntrHandler(XTimer *InstancePtr, u8 Priority);
void XAxiTimer_CallbackHandler(void *CallBackRef, u8 TmrCtrNumber);
static void XTickTimer_AxiTimerStop(XTimer *InstancePtr);
static void XTickTimer_ClearAxiTimerInterrupt(XTimer *InstancePtr);
#endif

#ifdef XSLEEPTIMER_IS_AXITIMER
/****************************************************************************/
/**
 * Initialize the Axi Timer Sleep Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XAxiTimer_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_AxiTimerStop;

	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_AXITIMER
/****************************************************************************/
/**
 * Initialize the Axi Timer Tick Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XAxiTimer_SetIntrHandler;
	InstancePtr->XTimer_TickInterval = XAxiTimer_TickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_AxiTimerStop;
	InstancePtr->XTickTimer_ClearInterrupt = XTickTimer_ClearAxiTimerInterrupt;

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the Axi Timer Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	BaseAddress is the Axi Timer Instance baseaddress to be
 * 		worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XAxiTimer_Init(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTmrCtr *AxiTimerInstPtr)
{
	u32 Status = XST_FAILURE;
	XTmrCtr_Config *ConfigPtr;
	(void)InstancePtr;

	ConfigPtr = XTmrCtr_LookupConfig(BaseAddress);
	if (!ConfigPtr) {
		return Status;
	}

	XTmrCtr_CfgInitialize(AxiTimerInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
	XTmrCtr_InitHw(AxiTimerInstPtr);
	XTmrCtr_SetOptions(AxiTimerInstPtr, 0,
			   XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_Start(AxiTimerInstPtr, 0);

	return XST_SUCCESS;
}

#ifdef XTICKTIMER_IS_AXITIMER
/*****************************************************************************/
/**
 * This function implements the interrupt callback handler
 *
 * @param  CallBackRef is Pointer to the XTimer instance
 *
 * @return	None
 *
 ****************************************************************************/
void XAxiTimer_CallbackHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	XTimer *InstancePtr = (XTimer *)CallBackRef;
	(void)TmrCtrNumber;

	InstancePtr->Handler(InstancePtr->CallBackRef, 0);
}

/*****************************************************************************/
/**
 * This function configures the axi timer tick interval
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 *
 * @return	None
 *
 ****************************************************************************/
static void XAxiTimer_TickInterval(XTimer *InstancePtr, u32 Delay)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;
	u32 Tlr;
	static u32 IsTickTimerStarted = FALSE;

	if (FALSE == IsTickTimerStarted) {
#ifdef SDT
		XAxiTimer_Init(InstancePtr, XTICKTIMER_BASEADDRESS,
#else
		XAxiTimer_Init(InstancePtr, XTICKTIMER_DEVICEID,
#endif
				&InstancePtr->AxiTimer_TickInst);
		IsTickTimerStarted = TRUE;
	}

	Tlr = Delay * (AxiTimerInstPtr->Config.SysClockFreqHz /
			XTIMER_DELAY_MSEC);
	XTmrCtr_SetOptions(AxiTimerInstPtr, 0,
                           XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION |
			   XTC_DOWN_COUNT_OPTION);

        /*
         * Set a reset value for the timer counter such that it will expire
         * eariler than letting it roll over from 0, the reset value is loaded
         * into the timer counter when it is started
         */
        XTmrCtr_SetResetValue(AxiTimerInstPtr, 0, Tlr);

        /*
         * Start the timer counter such that it's incrementing by default,
         * then wait for it to timeout a number of times
         */
        XTmrCtr_Start(AxiTimerInstPtr, 0);
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
static void XAxiTimer_SetIntrHandler(XTimer *InstancePtr, u8 Priority)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;

	XTmrCtr_SetHandler(AxiTimerInstPtr, XAxiTimer_CallbackHandler,
			   InstancePtr);
	XSetupInterruptSystem(AxiTimerInstPtr, XTmrCtr_InterruptHandler,
			      AxiTimerInstPtr->Config.IntrId,
			      AxiTimerInstPtr->Config.IntrParent,
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
static void XTickTimer_AxiTimerStop(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;

	XTmrCtr_Stop(AxiTimerInstPtr, 0);
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
static void XTickTimer_ClearAxiTimerInterrupt(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;
	unsigned long ulCSR;

	/* Clear the timer interrupt */
	ulCSR = XTmrCtr_GetControlStatusReg(AxiTimerInstPtr->Config.BaseAddress,
					    0);
	XTmrCtr_SetControlStatusReg(AxiTimerInstPtr->Config.BaseAddress,
				    0, ulCSR);
}
#endif

#ifdef XSLEEPTIMER_IS_AXITIMER
/*****************************************************************************/
/**
 * This function configures the sleep interval using axi timer
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XAxiTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u32 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
#ifdef SDT
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_BASEADDRESS,
#else
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_DEVICEID,
#endif
				&InstancePtr->AxiTimer_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
			AxiTimerInstPtr->Config.SysClockFreqHz / (DelayType));
	do {
		TimeLowVal2 = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
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
static void XSleepTimer_AxiTimerStop(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;

	XTmrCtr_Stop(AxiTimerInstPtr, 0);
}

/****************************************************************************/
/**
 * Get the time from the Axi Timer Counter Register.
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
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;
	static u32 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
#ifdef SDT
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_BASEADDRESS,
#else
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_DEVICEID,
#endif
				&InstancePtr->AxiTimer_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	*Xtime_Global = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
}
#endif
/*@}*/

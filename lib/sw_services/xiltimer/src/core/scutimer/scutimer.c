/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file scutimer.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * @details
 *
 * This file contains the definitions for scutimer implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 *  1.0  adk   24/11/21 Initial release.
 *  	 adk   07/02/22 Updated the IntrHandler as per XTimer_SetHandler() API
 *  	 	        and removed the unneeded XTickTimer_SetPriority() API.
 *  1.1	 adk   08/08/22 Added doxygen tags.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xscutimer.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XTimer_ScutimerInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			       XScuTimer *ScuTimerInstPtr);
#ifdef XSLEEPTIMER_IS_SCUTIMER
static void XTimer_ScutimerModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_ScutimerStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_SCUTIMER
void XScutimer_CallbackHandler(void *CallBackRef);
static void XTimer_ScutimerTickInterval(XTimer *InstancePtr, u32 Delay);
static void XTimer_ScutimerSetIntrHandler(XTimer *InstancePtr, u8 Priority);
static void XTickTimer_ScutimerStop(XTimer *InstancePtr);
static void XTickTimer_ClearScutimerInterrupt(XTimer *InstancePtr);
#endif

#ifdef XSLEEPTIMER_IS_SCUTIMER
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
	InstancePtr->XTimer_ModifyInterval = XTimer_ScutimerModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_ScutimerStop;
	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_SCUTIMER
/****************************************************************************/
/**
 * Initialize the scutimer Tick Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XTimer_ScutimerSetIntrHandler;
	InstancePtr->XTimer_TickInterval = XTimer_ScutimerTickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_ScutimerStop;
	InstancePtr->XTickTimer_ClearInterrupt = XTickTimer_ClearScutimerInterrupt;
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the Scutimer Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	BaseAddress is the Scutimer Instance baseaddress to be
 * 		worked on
 * @param	ScuTimerInstPtr is a pointer to the XScuTimer Instance
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XTimer_ScutimerInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			       XScuTimer *ScuTimerInstPtr)
{
	u32 Status = XST_FAILURE;
	XScuTimer_Config *ConfigPtr;
	(void) InstancePtr;

	ConfigPtr = XScuTimer_LookupConfig(BaseAddress);
        if (!ConfigPtr) {
                return Status;
        }

	Status = XScuTimer_CfgInitialize(ScuTimerInstPtr, ConfigPtr,
					 ConfigPtr->BaseAddr);
	XScuTimer_EnableAutoReload(ScuTimerInstPtr);
	XScuTimer_Start(ScuTimerInstPtr);

	return Status;
}

#ifdef XTICKTIMER_IS_SCUTIMER
/*****************************************************************************/
/**
 * This function implements the interrupt callback handler
 *
 * @param  CallBackRef is Pointer to the XTimer instance
 *
 * @return	None
 *
 ****************************************************************************/
void XScutimer_CallbackHandler(void *CallBackRef)
{
	XTimer *InstancePtr = (XTimer *)CallBackRef;
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XScuTimer_ClearInterruptStatus(ScuTimerInstPtr);
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
static void XTimer_ScutimerTickInterval(XTimer *InstancePtr, u32 Delay)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;
	u32 Freq;
	static u8 IsTickTimerStarted = FALSE;
	u32 ScuTimerFreq = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ  / 2U;

	if (FALSE == IsTickTimerStarted) {
		XTimer_ScutimerInit(InstancePtr, XTICKTIMER_DEVICEID,
				    &InstancePtr->ScuTimer_TickInst);
		IsTickTimerStarted = TRUE;
	}
	Freq = XTIMER_DELAY_MSEC/Delay;
	XScuTimer_Stop(ScuTimerInstPtr);
	XScuTimer_EnableAutoReload(ScuTimerInstPtr);
	XScuTimer_SetPrescaler(ScuTimerInstPtr, 0);
	XScuTimer_LoadTimer(ScuTimerInstPtr, ScuTimerFreq/Freq);
	XScuTimer_EnableInterrupt(ScuTimerInstPtr);
	XScuTimer_Start(ScuTimerInstPtr);
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
static void XTimer_ScutimerSetIntrHandler(XTimer *InstancePtr, u8 Priority)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XSetupInterruptSystem(InstancePtr, XScutimer_CallbackHandler,
			      ScuTimerInstPtr->Config.IntrId,
			      ScuTimerInstPtr->Config.IntrParent,
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
static void XTickTimer_ScutimerStop(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XScuTimer_Stop(ScuTimerInstPtr);
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
static void XTickTimer_ClearScutimerInterrupt(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XScuTimer_ClearInterruptStatus(ScuTimerInstPtr);
}
#endif

#ifdef XSLEEPTIMER_IS_SCUTIMER
/*****************************************************************************/
/**
 * This function configures the sleep interval using scutimer
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XTimer_ScutimerModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u8 IsSleepTimerStarted = FALSE;
	u32 ScuTimerFreq = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2U;

	if (FALSE == IsSleepTimerStarted) {
		XTimer_ScutimerInit(InstancePtr, XSLEEPTIMER_DEVICEID,
				    &InstancePtr->ScuTimer_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XScuTimer_GetCounterValue(ScuTimerInstPtr);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
                                   ScuTimerFreq / (DelayType));
	do {
		TimeLowVal2 = XScuTimer_GetCounterValue(ScuTimerInstPtr);
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
static void XSleepTimer_ScutimerStop(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;

	XScuTimer_Stop(ScuTimerInstPtr);
}

/****************************************************************************/
/**
 * Get the time from the Scutimer Counter Register.
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
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;

	*Xtime_Global = XScuTimer_GetCounterValue(ScuTimerInstPtr);
}
#endif

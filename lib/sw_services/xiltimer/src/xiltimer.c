/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiltimer.c
* @addtogroup xiltimer_api XilTimer APIs
*
* This file contains the sleep API's.
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
*  1.0  adk	 24/11/21 Initial release.
*  	adk	 07/02/22 Integrate configuring of priority field into
*  			  XTimer_SetHandler() API instead of a new API.
*  1.1	adk      08/08/22 Added support for versal net.
*  	adk      08/08/22 Added doxygen tags.
*  1.2  adk	 22/12/22 Fixed doxygen style and indentation issues.
*  1.3  gm	 21/07/23 Added Timer Release Callback function.
*  2.0  ml       28/03/24 added description and removed comments to
*                         fix doxygen warnings.
*  2.4  vmt      08/20/26 Added weak definitions of sleep and tick
*                         timer instance variables.
*  2.4  vmt      09/01/26 Added XTimer_SetIntervalHz() to configure the tick
*                         interval from frequency.
*  2.5  ml    09/22/26 Fixed doxygen 1.15.0 and compliance checker warnings.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "sleep.h"
#include "xiltimer.h"

/****************************  Constant Definitions  *************************/
XTimer TimerInst; /**< Global XTimer instance for timer operations. */
#ifdef XSLEEPTIMER_INSTANCE
__attribute__((weak)) UINTPTR XSleepTimer_Instance = XSLEEPTIMER_INSTANCE;
#endif
#ifdef XTICKTIMER_INSTANCE
__attribute__((weak)) UINTPTR XTickTimer_Instance = XTICKTIMER_INSTANCE;
#endif
void XilTimer_Sleep(unsigned long delay, XTimer_DelayType DelayType);

/*****************************************************************************/
/**
*
* This API gives delay in sec
*
* @param[in]        seconds Delay time in seconds
*
*****************************************************************************/
void sleep(unsigned int seconds) {
	XilTimer_Sleep(seconds, XTIMER_DELAY_SEC);
}

/****************************************************************************/
/**
*
* This API gives delay in msec
*
* @param[in]        mseconds Delay time in milliseconds
*
*****************************************************************************/
void msleep(unsigned long mseconds) {
	XilTimer_Sleep(mseconds, XTIMER_DELAY_MSEC);
}

/****************************************************************************/
/**
*
* This API gives delay in usec
*
* @param[in]        useconds Delay time in microseconds
*
*****************************************************************************/
void usleep(unsigned long useconds) {
	XilTimer_Sleep(useconds, XTIMER_DELAY_USEC);
}

/****************************************************************************/
/**
*
* This API contains common delay implementation using library APIs.
*
* @param[in]        delay Delay time value.
* @param[in]        DelayType Unit of delay (seconds, milliseconds, microseconds).
*
*****************************************************************************/
void XilTimer_Sleep(unsigned long delay, XTimer_DelayType DelayType) {
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTimer_ModifyInterval)
		InstancePtr->XTimer_ModifyInterval(InstancePtr, delay,
						   DelayType);
}

/****************************************************************************/
/**
*
* This API initializes the xiltimer library through constructor.
*
*****************************************************************************/
void __attribute__ ((constructor)) xtimerinit()
{
    XilSleepTimer_Init(&TimerInst);
    XilTickTimer_Init(&TimerInst);
}

/****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* FuncPtr.
*
* @param[in]	FuncPtr is the address of the callback function.
* @param[in]	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
* @param[in]    Priority - Priority for the interrupt
*
*****************************************************************************/
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef,
		       u8 Priority)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
	if (InstancePtr->XTimer_TickIntrHandler) {
		InstancePtr->XTimer_TickIntrHandler(InstancePtr, Priority);
	}
}

#if defined  (XPM_SUPPORT)
/****************************************************************************/
/**
*
* This API releases the timer instance.
*
*****************************************************************************/
void XTimer_ReleaseTickTimer(void)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTickTimer_ReleaseTickTimer)
		InstancePtr->XTickTimer_ReleaseTickTimer(InstancePtr);
}
#endif

/****************************************************************************/
/**
*
* This API sets the elapse interval for the timer instance.
*
* @param[in]        delay Delay time in milliseconds
*
*****************************************************************************/
void XTimer_SetInterval(unsigned long delay)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTimer_TickInterval)
		InstancePtr->XTimer_TickInterval(InstancePtr, delay);
}

/****************************************************************************/
/**
*
* This API configures the tick interval from a frequency in Hz, instead of
* whole milliseconds.
*
* @param[in] FreqHz Required tick frequency in Hz
*
* @note             If the selected tick timer does not implement
*                   XTimer_TickIntervalHz, this API uses Delay = 1000 /
*                   FreqHz with integer truncation, so the programmed rate
*                   matches FreqHz only when 1000 is an exact multiple of
*                   FreqHz.
*
*****************************************************************************/
void XTimer_SetIntervalHz(u32 FreqHz)
{
	XTimer *InstancePtr;
	u32 Delay;

	Xil_AssertVoid(FreqHz > 0U);

	InstancePtr = &TimerInst;
	if (InstancePtr->XTimer_TickIntervalHz != NULL) {
		InstancePtr->XTimer_TickIntervalHz(InstancePtr, FreqHz);
	} else if (InstancePtr->XTimer_TickInterval != NULL) {
		Delay = (u32)XTIMER_DELAY_MSEC / FreqHz;
		InstancePtr->XTimer_TickInterval(InstancePtr, Delay);
	}
}

/****************************************************************************/
/**
*
* This API clears the interrupt status of the tick timer instance.
*
*****************************************************************************/
void XTimer_ClearTickInterrupt( void )
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTickTimer_ClearInterrupt)
		InstancePtr->XTickTimer_ClearInterrupt(InstancePtr);
}

/** @} */

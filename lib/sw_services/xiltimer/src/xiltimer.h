/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiltimer.h
* @addtogroup xiltimer_api XilTimer APIs
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
*  1.0  adk	 24/11/21 Initial release.
*  	adk	 12/01/22 Fix compilation errors when cortexr5 internal pm
*  			  counter selected as default timer.
*  	adk	 07/02/22 Integrate configuring of priority field into
*  			  XTimer_SetHandler() API instead of a new API.
*  	adk	 25/03/22 Fix compilation errors when cortexa72 global timer
*  			  selected as default timer.
*  1.1	adk      08/08/22 Added support for versal net.
*  	adk      08/08/22 Added doxygen tags.
*  1.2  adk	 22/12/22 Fixed doxygen style and indentation issues.
*  1.3  gm      21/07/23 Added Timer Release Callback function.
*
* </pre>
******************************************************************************/
#ifndef XILTIMER_H
#define XILTIMER_H

#include "xtimer_config.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "bspconfig.h"
#include "xparameters.h"
#if defined(XSLEEPTIMER_IS_AXITIMER) || defined(XTICKTIMER_IS_AXITIMER)
#include "xtmrctr.h"
#endif
#if defined(XSLEEPTIMER_IS_TTCPS) || defined(XTICKTIMER_IS_TTCPS)
#include "xttcps.h"
#endif
#if defined(XSLEEPTIMER_IS_SCUTIMER) || defined(XTICKTIMER_IS_SCUTIMER)
#include "xscutimer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Type Definitions *******************************/

/**
 * This typedef contains different measures of time for the device.
 */
typedef enum {
	XTIMER_DELAY_SEC = 1,            /**< Time delay in seconds*/
	XTIMER_DELAY_MSEC = 1000,        /**< Time delay in milliseconds*/
	XTIMER_DELAY_USEC = 1000 * 1000, /**< Time delay in microseconds*/
} XTimer_DelayType;

typedef void (*XTimer_TickHandler) (void *CallBackRef, u32 StatusEvent);

/**
 * Structure to the XFpga instance.
 *
 * @param XTimer_ModifyInterval Modifies the timer interval
 * @param XTimer_TickIntrHandler Tick interrupt handler
 * @param XTimer_TickInterval Configures the tick interval
 * @param XSleepTimer_Stop Stops the sleep timer
 * @param XTickTimer_Stop Stops the tick timer
 * @param XTickTimer_ClearInterrupt Clears the Tick timer interrupt status
 * @param Handler Tick Handler
 * @param CallBackRef Callback reference for handler
 * @param AxiTimer_SleepInst Sleep Instance for AxiTimer
 * @param AxiTimer_TickInst Tick Instance for AxiTimer
 * @param TtcPs_SleepInst Sleep Instance for TTCPS
 * @param TtcPs_TickInst Tick Instance for TTCPS
 * @param ScuTimer_SleepInst Sleep Instance for Scutimer
 * @param ScuTimer_TickInst Tick Instance for Scutimer
 */
typedef struct XTimerTag {
	void (*XTimer_ModifyInterval)(struct XTimerTag *InstancePtr, u32 delay,
               XTimer_DelayType Delaytype); /**<Modifies the timer interval*/
	void (*XTimer_TickIntrHandler)(struct XTimerTag *InstancePtr,
               u8 Priority);                /**< Tick interrupt handler */
	void (*XTimer_TickInterval)(struct XTimerTag *InstancePtr, u32 Delay);
                                            /**< Configures the tick interval */
	void (*XSleepTimer_Stop)(struct XTimerTag *InstancePtr);
                                            /**< Stops the sleep timer */
	void (*XTickTimer_Stop)(struct XTimerTag *InstancePtr);
                                            /**< Stops the tick timer */
	void (*XTickTimer_ClearInterrupt)(struct XTimerTag *InstancePtr);
	                                    /**< Clears the Tick timer interrupt status */
	XTimer_TickHandler Handler;         /**< Callback function */
	void *CallBackRef;                  /**< Callback reference for handler */
#ifdef  XPM_SUPPORT
	void (*XTickTimer_ReleaseTickTimer)(struct XTimerTag *InstancePtr);
					/**< Timer Release Callback function */
#endif
#ifdef XSLEEPTIMER_IS_AXITIMER
	XTmrCtr AxiTimer_SleepInst;
#endif
#ifdef XTICKTIMER_IS_AXITIMER
	XTmrCtr AxiTimer_TickInst;
#endif
#ifdef XSLEEPTIMER_IS_TTCPS
	XTtcPs TtcPs_SleepInst;
#endif
#ifdef XTICKTIMER_IS_TTCPS
	XTtcPs TtcPs_TickInst;
#endif
#ifdef XSLEEPTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_SleepInst;
#endif
#ifdef XTICKTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_TickInst;
#endif
} XTimer;

typedef u64 XTime;
extern XTimer TimerInst;
/************************** Function Prototypes ******************************/
/**
 * This API is used for initializing sleep timer
 */
u32 XilSleepTimer_Init(XTimer *InstancePtr);
/**
 * This API is used for initializing Tick timer
 */
u32 XilTickTimer_Init(XTimer *InstancePtr);
/**
 * Get the time
 */
void XTime_GetTime(XTime *Xtime_Global);
void XTimer_SetInterval(unsigned long delay);
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef,
		       u8 Priority);
void XTimer_ClearTickInterrupt( void );
#ifdef XTIMER_DEFAULT_TIMER_IS_MB
u32 Xil_GetMBFrequency(void);
u32 Xil_SetMBFrequency(u32 Val);
#endif

#ifdef __cplusplus
}
#endif

#endif

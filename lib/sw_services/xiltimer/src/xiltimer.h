/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiltimer.h
* @addtogroup xfpga_apis XilFPGA APIs
* @{
*
* This library provides sleep and tick timer functionality, Hardware and
* Software features are differentiated using a layered approach.
* The top Level layer is platform-agnostic where the core layer(low level)
* is hardware dependent, Currently, this library supports TTC, scutimer,
* axi_timer IP's if none of these IP's present in a given design then the
* sleep functionality will be executed using processor instructions.
* For the sleep and tick functionality user can select a specific timer
* using the library software configuration wizard.
*
* @{
* @cond xilfpga_internal
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

typedef enum {
	XTIMER_DELAY_SEC = 1,
	XTIMER_DELAY_MSEC = 1000,
	XTIMER_DELAY_USEC = 1000 * 1000,
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
	void (*XTimer_ModifyInterval)(struct XTimerTag *InstancePtr, u32 delay, XTimer_DelayType Delaytype);
	void (*XTimer_TickIntrHandler)(struct XTimerTag *InstancePtr, u8 Priority);
	void (*XTimer_TickInterval)(struct XTimerTag *InstancePtr, u32 Delay);
	void (*XSleepTimer_Stop)(struct XTimerTag *InstancePtr);
	void (*XTickTimer_Stop)(struct XTimerTag *InstancePtr);
	void (*XTickTimer_ClearInterrupt)(struct XTimerTag *InstancePtr);
	XTimer_TickHandler Handler; /**< Callback function */
	void *CallBackRef;       /**< Callback reference for handler */
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
u32 XilSleepTimer_Init(XTimer *InstancePtr);
u32 XilTickTimer_Init(XTimer *InstancePtr);
void XTime_GetTime(XTime *Xtime_Global);
void XTimer_SetInterval(unsigned long delay);
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef,
		       u8 Priority);
void XTimer_ClearTickInterrupt( void );

#ifdef __cplusplus
}
#endif

#endif

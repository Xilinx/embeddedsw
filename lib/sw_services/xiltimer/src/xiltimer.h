/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
*  1.4  ht      09/12/23 Added code for versioning of library.
*  1.4  mus     15/02/24 Added correct APIs to set/get MB V frequency.
*  2.0  ml      28/03/24 Added description to fix doxygen warnings.
*  2.4  vmt     08/20/26 Added weak sleep and tick timer instance
*                        variables.
*  2.4  vmt     09/01/26 Added XTimer_SetIntervalHz() to configure the tick
*                        interval from frequency.
*  2.5  ml    09/22/26 Fixed doxygen 1.15.0 and compliance checker warnings.
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
#include "xil_util.h"
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

/************************** Constant Definitions *****************************/

/**
 * Library Major version info
 */
#define XTIMER_MAJOR_VERSION	1U

/**
 * Library Minor version info
 */
#define XTIMER_MINOR_VERSION	4U

/**************************** Type Definitions *******************************/

/**
 * This typedef contains different measures of time for the device.
 */
typedef enum {
	XTIMER_DELAY_SEC = 1,            /**< Time delay in seconds*/
	XTIMER_DELAY_MSEC = 1000,        /**< Time delay in milliseconds*/
	XTIMER_DELAY_USEC = 1000 * 1000, /**< Time delay in microseconds*/
} XTimer_DelayType;

typedef void (*XTimer_TickHandler) (void *CallBackRef, u32 StatusEvent); /**< Callback function type for tick timer interrupt handler. */

/**
 * @brief XTimer instance structure holding timer operation callbacks and instances.
 */
typedef struct XTimerTag {
	void (*XTimer_ModifyInterval)(struct XTimerTag *InstancePtr, u32 delay,
               XTimer_DelayType Delaytype); /**<Modifies the timer interval*/
	void (*XTimer_TickIntrHandler)(struct XTimerTag *InstancePtr,
               u8 Priority);                /**< Tick interrupt handler */
	void (*XTimer_TickInterval)(struct XTimerTag *InstancePtr, u32 Delay);
                                            /**< Configures the tick interval */
	void (*XTimer_TickIntervalHz)(struct XTimerTag *InstancePtr, u32 FreqHz);
                                            /**< Configures the tick interval
                                                 from a frequency in Hz */
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
	XTmrCtr AxiTimer_SleepInst; /**< Instance of the AXI Timer used for
				         sleep functionality. */
#endif
#ifdef XTICKTIMER_IS_AXITIMER
	XTmrCtr AxiTimer_TickInst; /**< Instance of the AXI Timer used for
				        Tick functionality. */
#endif
#ifdef XSLEEPTIMER_IS_TTCPS
	XTtcPs TtcPs_SleepInst; /**< Instance of the TTCPS used for sleep
				     functionality. */
#endif
#ifdef XTICKTIMER_IS_TTCPS
	XTtcPs TtcPs_TickInst; /**< Instance of the TTCPS used for Tick
				    functionality. */
#endif
#ifdef XSLEEPTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_SleepInst; /**< Instance of the SCU Timer used for
					   sleep functionality. */
#endif
#ifdef XTICKTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_TickInst; /**< Instance of the SCU Timer used for
				          Tick functionality. */
#endif
} XTimer;

typedef u64 XTime; /**< Type for representing timer count in u64 format. */
extern XTimer TimerInst; /**< Global XTimer instance for timer operations. */

/*
 * Weak default timer instance variables used by xiltimer.
 * Holds timer base addresses (SDT) or device IDs (non-SDT).
 * Applications may override these values to select a different
 * timer instance.
 */
#ifdef XSLEEPTIMER_BASEADDRESS
#define XSLEEPTIMER_INSTANCE XSLEEPTIMER_BASEADDRESS
#elif defined(XSLEEPTIMER_DEVICEID)
#define XSLEEPTIMER_INSTANCE XSLEEPTIMER_DEVICEID
#endif
#ifdef XTICKTIMER_BASEADDRESS
#define XTICKTIMER_INSTANCE XTICKTIMER_BASEADDRESS
#elif defined(XTICKTIMER_DEVICEID)
#define XTICKTIMER_INSTANCE XTICKTIMER_DEVICEID
#endif

#ifdef XSLEEPTIMER_INSTANCE
extern UINTPTR XSleepTimer_Instance;
#endif
#ifdef XTICKTIMER_INSTANCE
extern UINTPTR XTickTimer_Instance;
#endif

/****************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief	This function returns the version number of xiltimer library.
*
* @return	32-bit version number
*
******************************************************************************/
static __attribute__((always_inline)) INLINE
u32 XTimer_GetLibVersion(void)
{
	return (XIL_BUILD_VERSION(XTIMER_MAJOR_VERSION, XTIMER_MINOR_VERSION));
}
/************************** Function Prototypes ******************************/
/**
 * @brief Initialize the sleep timer instance.
 *
 * @param[out] InstancePtr Pointer to the XTimer instance.
 *
 * @return XST_SUCCESS always.
 */
u32 XilSleepTimer_Init(XTimer *InstancePtr);
/**
 * @brief Initialize the tick timer instance.
 *
 * @param[out] InstancePtr Pointer to the XTimer instance.
 *
 * @return XST_SUCCESS always.
 */
u32 XilTickTimer_Init(XTimer *InstancePtr);
/**
 * @brief Get the current timer count value.
 *
 * @param[out] Xtime_Global Pointer to store the current timer value.
 */
void XTime_GetTime(XTime *Xtime_Global);
void XTimer_SetInterval(unsigned long delay);
/**
 * Sets the tick interval from a frequency in Hz.
 * XTimer_SetInterval() uses whole milliseconds and cannot represent rates
 * that are not exact divisors of 1000. The requested rate is preserved in Hz
 * when the selected timer implements the Hz path.
 */
void XTimer_SetIntervalHz(u32 FreqHz);
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef,
		       u8 Priority);
void XTimer_ClearTickInterrupt( void );
#ifdef XTIMER_DEFAULT_TIMER_IS_MB
u32 Xil_GetMBFrequency(void);
u32 Xil_SetMBFrequency(u32 Val);
#endif

#ifdef XTIMER_DEFAULT_TIMER_IS_MB_RISCV
u32 Xil_GetRISCVFrequency(void);
u32 Xil_SetRISCVFrequency(u32 Val);
#endif


#ifdef __cplusplus
}
#endif

#endif

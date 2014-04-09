/******************************************************************************
*
* (c) Copyright 2002-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xtmrctr.h
*
* The Xilinx timer/counter component. This component supports the Xilinx
* timer/counter. More detailed description of the driver operation can
* be found in the xtmrctr.c file.
*
* The Xilinx timer/counter supports the following features:
*   - Polled mode.
*   - Interrupt driven mode
*   - enabling and disabling specific timers
*   - PWM operation
*   - Cascade Operation (This is to be used for getting a 64 bit timer and this
*     feature is present in the latest versions of the axi_timer IP)
*
* The driver does not currently support the PWM operation of the device.
*
* The timer counter operates in 2 primary modes, compare and capture. In
* either mode, the timer counter may count up or down, with up being the
* default.
*
* Compare mode is typically used for creating a single time period or multiple
* repeating time periods in the auto reload mode, such as a periodic interrupt.
* When started, the timer counter loads an initial value, referred to as the
* compare value, into the timer counter and starts counting down or up. The
* timer counter expires when it rolls over/under depending upon the mode of
* counting. An external compare output signal may be configured such that a
* pulse is generated with this signal when it hits the compare value.
*
* Capture mode is typically used for measuring the time period between
* external events. This mode uses an external capture input signal to cause
* the value of the timer counter to be captured. When started, the timer
* counter loads an initial value, referred to as the compare value,

* The timer can be configured to either cause an interrupt when the count
* reaches the compare value in compare mode or latch the current count
* value in the capture register when an external input is asserted
* in capture mode. The external capture input can be enabled/disabled using the
* XTmrCtr_SetOptions function. While in compare mode, it is also possible to
* drive an external output when the compare value is reached in the count
* register The external compare output can be enabled/disabled using the
* XTmrCtr_SetOptions function.
*
* <b>Interrupts</b>
*
* It is the responsibility of the application to connect the interrupt
* handler of the timer/counter to the interrupt source. The interrupt
* handler function, XTmrCtr_InterruptHandler, is visible such that the user
* can connect it to the interrupt source. Note that this interrupt handler
* does not provide interrupt context save and restore processing, the user
* must perform this processing.
*
* The driver services interrupts and passes timeouts to the upper layer
* software through callback functions. The upper layer software must register
* its callback functions during initialization. The driver requires callback
* functions for timers.
*
* @note
* The default settings for the timers are:
*   - Interrupt generation disabled
*   - Count up mode
*   - Compare mode
*   - Hold counter (will not reload the timer)
*   - External compare output disabled
*   - External capture input disabled
*   - Pulse width modulation disabled
*   - Timer disabled, waits for Start function to be called
* <br><br>
* A timer counter device may contain multiple timer counters. The symbol
* XTC_DEVICE_TIMER_COUNT defines the number of timer counters in the device.
* The device currently contains 2 timer counters.
* <br><br>
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only. Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 1.10b mta  03/21/07 Updated to new coding style.
* 1.11a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
*		      file
* 2.00a ktn  10/30/09 Updated to use HAL API's. _m is removed from all the macro
*		      definitions.
* 2.01a ktn  07/12/10 Renamed the macro XTimerCtr_ReadReg as XTmrCtr_ReadReg
*		      for naming consistency (CR 559142).
* 2.02a sdm  09/28/10 Updated the driver tcl to generate the xparameters
*		      for the timer clock frequency (CR 572679).
* 2.03a rvo  11/30/10 Added check to see if interrupt is enabled before further
*		      processing for CR 584557.
* 2.04a sdm  07/12/11 Added support for cascade mode operation.
* 		      The cascade mode of operation is present in the latest
*		      versions of the axi_timer IP. Please check the HW
*		      Datasheet to see whether this feature is present in the
*		      version of the IP that you are using.
* 2.05a adk  15/05/13 Fixed the CR:693066
*		      Added the IsStartedTmrCtr0/IsStartedTmrCtr1 members to the
*		      XTmrCtr instance structure.
*		      The IsStartedTmrCtrX will be assigned XIL_COMPONENT_IS_STARTED in
*		      the XTmrCtr_Start function.
*		      The IsStartedTmrCtrX will be cleared in the XTmrCtr_Stop function.
*		      There will be no Initialization done in the
*		      XTmrCtr_Initialize if both the timers have already started and
*		      the XST_DEVICE_IS_STARTED Status is returned.
*		      Removed the logic in the XTmrCtr_Initialize function
*		      which was checking the Register Value to know whether
*		      a timer has started or not.
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* </pre>
*
******************************************************************************/

#ifndef XTMRCTR_H		/* prevent circular inclusions */
#define XTMRCTR_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xtmrctr_l.h"

/************************** Constant Definitions *****************************/

/**
 * @name Configuration options
 * These options are used in XTmrCtr_SetOptions() and XTmrCtr_GetOptions()
 * @{
 */
/**
 * Used to configure the timer counter device.
 * <pre>
 * XTC_CASCADE_MODE_OPTION	Enables the Cascade Mode only valid for TCSRO.
 * XTC_ENABLE_ALL_OPTION	Enables all timer counters at once.
 * XTC_DOWN_COUNT_OPTION	Configures the timer counter to count down from
 *				start value, the default is to count up.
 * XTC_CAPTURE_MODE_OPTION	Configures the timer to capture the timer
 *				counter value when the external capture line is
 *				asserted. The default mode is compare mode.
 * XTC_INT_MODE_OPTION		Enables the timer counter interrupt output.
 * XTC_AUTO_RELOAD_OPTION	In compare mode, configures the timer counter to
 *				reload from the compare value. The default mode
 *				causes the timer counter to hold when the
 *				compare value is hit.
 *				In capture mode, configures the timer counter to
 *				not hold the previous capture value if a new
 *				event occurs. The default mode cause the timer
 *				counter to hold the capture value until
 *				recognized.
 * XTC_EXT_COMPARE_OPTION	Enables the external compare output signal.
 * </pre>
 */
#define XTC_CASCADE_MODE_OPTION		0x00000080UL
#define XTC_ENABLE_ALL_OPTION		0x00000040UL
#define XTC_DOWN_COUNT_OPTION		0x00000020UL
#define XTC_CAPTURE_MODE_OPTION		0x00000010UL
#define XTC_INT_MODE_OPTION		0x00000008UL
#define XTC_AUTO_RELOAD_OPTION		0x00000004UL
#define XTC_EXT_COMPARE_OPTION		0x00000002UL
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;	/**< Unique ID  of device */
	u32 BaseAddress;/**< Register base address */
} XTmrCtr_Config;

/**
 * Signature for the callback function.
 *
 * @param	CallBackRef is a callback reference passed in by the upper layer
 *		when setting the callback functions, and passed back to the
 *		upper layer when the callback is invoked. Its type is
 *		 unimportant to the driver, so it is a void pointer.
 * @param 	TmrCtrNumber is the number of the timer/counter within the
 *		device. The device typically contains at least two
 *		timer/counters. The timer number is a zero based number with a
 *		range of 0 to (XTC_DEVICE_TIMER_COUNT - 1).
 */
typedef void (*XTmrCtr_Handler) (void *CallBackRef, u8 TmrCtrNumber);


/**
 * Timer/Counter statistics
 */
typedef struct {
	u32 Interrupts;	 /**< The number of interrupts that have occurred */
} XTmrCtrStats;

/**
 * The XTmrCtr driver instance data. The user is required to allocate a
 * variable of this type for every timer/counter device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XTmrCtrStats Stats;	 /**< Component Statistics */
	u32 BaseAddress;	 /**< Base address of registers */
	u32 IsReady;		 /**< Device is initialized and ready */
	u32 IsStartedTmrCtr0;	 /**< Is Timer Counter 0 started */
	u32 IsStartedTmrCtr1;	 /**< Is Timer Counter 1 started */

	XTmrCtr_Handler Handler; /**< Callback function */
	void *CallBackRef;	 /**< Callback reference for handler */
} XTmrCtr;


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*
 * Required functions, in file xtmrctr.c
 */
int XTmrCtr_Initialize(XTmrCtr * InstancePtr, u16 DeviceId);
void XTmrCtr_Start(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
void XTmrCtr_Stop(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
u32 XTmrCtr_GetValue(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
void XTmrCtr_SetResetValue(XTmrCtr * InstancePtr, u8 TmrCtrNumber,
			   u32 ResetValue);
u32 XTmrCtr_GetCaptureValue(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
int XTmrCtr_IsExpired(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
void XTmrCtr_Reset(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
XTmrCtr_Config *XTmrCtr_LookupConfig(u16 DeviceId);

/*
 * Functions for options, in file xtmrctr_options.c
 */
void XTmrCtr_SetOptions(XTmrCtr * InstancePtr, u8 TmrCtrNumber, u32 Options);
u32 XTmrCtr_GetOptions(XTmrCtr * InstancePtr, u8 TmrCtrNumber);

/*
 * Functions for statistics, in file xtmrctr_stats.c
 */
void XTmrCtr_GetStats(XTmrCtr * InstancePtr, XTmrCtrStats * StatsPtr);
void XTmrCtr_ClearStats(XTmrCtr * InstancePtr);

/*
 * Functions for self-test, in file xtmrctr_selftest.c
 */
int XTmrCtr_SelfTest(XTmrCtr * InstancePtr, u8 TmrCtrNumber);

/*
 * Functions for interrupts, in file xtmrctr_intr.c
 */
void XTmrCtr_SetHandler(XTmrCtr * InstancePtr, XTmrCtr_Handler FuncPtr,
			void *CallBackRef);
void XTmrCtr_InterruptHandler(void *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps.h
* @addtogroup ttcps Overview
* @{
* @details
*
* This is the driver for one 16-bit timer counter in the Triple Timer Counter
* (TTC) module in the Ps block.
*
* The TTC module provides three independent timer/counter modules that can each
* be clocked using either the system clock (pclk) or an externally driven
* clock (ext_clk). In addition, each counter can independently prescale its
* selected clock input (divided by 2 to 65536). Counters can be set to
* decrement or increment.
*
* Each of the counters can be programmed to generate interrupt pulses:
*	. At a regular, predefined period, that is on a timed interval
*	. When the counter registers overflow
* 	. When the count matches any one of the three 'match' registers
*
* Therefore, up to six different events can trigger a timer interrupt: three
* match interrupts, an overflow interrupt, an interval interrupt and an event
* timer interrupt. Note that the overflow interrupt and the interval interrupt
* are mutually exclusive.
*
* <b>Initialization & Configuration</b>
*
* An XTtcPs_Config structure is used to configure a driver instance.
* Information in the XTtcPs_Config structure is the hardware properties
* about the device.
*
* A driver instance is initialized through
* XTtcPs_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr). Where CfgPtr
* is a pointer to the XTtcPs_Config structure, it can be looked up statically
* through XTtcPs_LookupConfig(BaseAddress), or passed in by the caller. The
* EffectiveAddr can be the static base address of the device or virtual
* mapped address if address translation is supported.
*
* <b>Interrupts</b>
*
* Interrupt handler is not provided by the driver, as handling of interrupt
* is application specific.
*
* <b>stack usage(in bytes)</b>
*
* 	XTtcPs_LookupConfig : 32
* 	XTtcPs_CfgInitialize : 80
* 	XTtcPs_SetMatchValue : 32
* 	XTtcPs_GetMatchValue : 48
* 	XTtcPs_SetPrescaler : 48
* 	XTtcPs_GetPrescaler : 48
* 	XTtcPs_CalcIntervalFromFreq : 48
* 	XTtcPs_SetOptions : 48
* 	XTtcPs_GetOptions : 48
* 	XTtcPs_SelfTest : 48
* 	XTtcPs_InterruptHandler : 48
* 	XTtcPs_SetStatusHandler : 48
*
* <b>Memory foot-print(in bytes)</b>
*
* 	XTtcPs_LookupConfig : 72
* 	XTtcPs_CfgInitialize : 304
* 	XTtcPs_SetMatchValue : 168
* 	XTtcPs_GetMatchValue : 176
* 	XTtcPs_SetPrescaler : 172
* 	XTtcPs_GetPrescaler : 152
* 	XTtcPs_CalcIntervalFromFreq : 228
* 	XTtcPs_SetOptions : 424
* 	XTtcPs_GetOptions : 200
* 	XTtcPs_SelfTest : 148
* 	XTtcPs_InterruptHandler : 88
* 	XTtcPs_SetStatusHandler : 140
*
* <b>Execution Time(in usec)</b>
*
* 	XTtcPs_LookupConfig : 8.31
* 	TtcPs_CfgInitialize : 1.30
* 	XTtcPs_SetMatchValue : 1.10
* 	XTtcPs_GetMatchValue : 1.00
* 	XTtcPs_SetPrescaler : 1.09
* 	XTtcPs_GetPrescaler : 1.00
* 	XTtcPs_CalcIntervalFromFreq : 1.29
* 	XTtcPs_SetOptions: 1.91
* 	XTtcPs_GetOptions: 2.55
* 	XTtcPs_SelfTest: .85
*
* <b>Assumptions of Use</b>

* 	1.The default setting for a timer/counter is:
* 	 - Overflow Mode
* 	 - Internal clock (pclk) selected
* 	 - Counter disabled
* 	 - All Interrupts disabled
* 	 - Output waveforms disabled
*
* <b>Compiler Name</b>
*
*	gcc
*
* <b>Compiler version</b>
*
*	8.2.0
*
* <b>Compiler options</b>
*
* 	-DARMR5 -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5 -mfloat-abi=hard  -mfpu=vfpv3-d16 -I<include_path>
* 	-Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a72 -I<include_path>
* 	-Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I<include_path>
*
* <b>User Defined data types</b>
*
* 	u8	1 byte
* 	u16	2 bytes
* 	u32	4 bytes / 1 word
* 	u64	8 bytes / double word
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------------
* 1.00a drg/jz 01/20/10 First release..
* 2.0   adk    12/10/13 Updated as per the New Tcl API's
* 3.0	pkp    12/09/14 Added support for Zynq Ultrascale Mp.Also code
*			modified for MISRA-C:2012 compliance.
* 3.2   mus    10/28/16 Modified XTtcPs_GetCounterValue and XTtcPs_SetInterval
*                       macros to return 32 bit values for zynq ultrascale+mpsoc
*       ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 3.4   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of ttcps in xparameters.h
* 3.5   srm  10/06/17 Added new typedef XMatchRegValue for match register width
* 3.8   aru  12/19/18 Modified in XTtcPs_ClearInterruptStatus function to clear
*                     Interrupt status register by reading instead of writing it.
* 3.14   mus 02/22/21 Updated XTtcPs_ClearInterruptStatus to fix compiler warning.
*                     It fixes CR#1084697.
* 3.16  adk  04/19/22 Fix infinite loop in the examples by adding polled
* 		      timeout loop.
* 3.18  adk  04/14/23 Added support for system device-tree flow.
* 3.21  ml   01/27/25 Added support for multiple counters in SDT flow.
* 3.21  ml   04/03/25 Added a global structure to store all handlers in a table.
* 3.23  bdk  12/08/25 Updated comments to support SDT flow for Doxygen
*                     documentation.
* 3.24  ml    09/16/26 Fixed doxygen 1.15.0 and compliance checker warnings.
* </pre>
*
******************************************************************************/
#ifndef XTTCPS_H
#define XTTCPS_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xttcps_hw.h"
#include "xstatus.h"

/*****************************************************************************/
typedef void (*XTtcPs_StatusHandler)(const void *CallBackRef, u32 StatusEvent);


/************************** Constant Definitions *****************************/

/*
 * Maximum Value for interval counter
 */
#if defined(ARMA9)
#define XTTCPS_MAX_INTERVAL_COUNT 0xFFFFU	/**< Maximum interval count for 16-bit mode. */
#else
#define XTTCPS_MAX_INTERVAL_COUNT 0xFFFFFFFFU	/**< Maximum interval count for 32-bit mode. */
#endif

#define XTTCPS_NUM_COUNTERS	1U			/**< Number of counters represented by one driver instance. */
/** @name Configuration options
 *
 * Options for the device. Each of the options is bit field, so more than one
 * options can be specified.
 *
 * @{
*/
#define XTTCPS_OPTION_EXTERNAL_CLK	0x00000001U 	/**< External clock source. */
#define XTTCPS_OPTION_CLK_EDGE_NEG	0x00000002U	/**< Clock on trailing edge for
						     external clock. */
#define XTTCPS_OPTION_INTERVAL_MODE	0x00000004U	/**< Interval mode. */
#define XTTCPS_OPTION_DECREMENT		0x00000008U	/**< Decrement the counter. */
#define XTTCPS_OPTION_MATCH_MODE	0x00000010U	/**< Match mode. */
#define XTTCPS_OPTION_WAVE_DISABLE	0x00000020U 	/**< No waveform output. */
#define XTTCPS_OPTION_WAVE_POLARITY	0x00000040U	/**< Waveform polarity. */
#define COUNTER_BASE_ADDRESS_MASK       0xFFFFFFF0U	/**< Mask to detect main
							     TTC node. */
/** @} */
/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;	  /**< Unique ID for device. */
#else
	char *Name;		/**< Unique name for device (SDT mode). */
#endif
	u32 BaseAddress;  /**< Base address for device. */
	u32 InputClockHz; /**< Input clock frequency. */
#if !defined(SDT) && defined(XIL_INTERRUPT)
	u32 IntrId;		/**< Interrupt ID for the device. */
	UINTPTR IntrParent;		/**< Interrupt parent: Bit[0] type, Bits[64/32:1] base address. */
#elif defined(SDT)
	u32 IntrId[XTTCPS_NUM_COUNTERS];		/**< Interrupt ID array: Bits[11:0] ID, Bits[15:12] flags. */
	UINTPTR IntrParent;		/**< Interrupt parent: Bit[0] type, Bits[64/32:1] base address. */
#endif
} XTtcPs_Config;

/**
 * The XTtcPs driver instance data. The user is required to allocate a
 * variable of this type for each PS timer/counter device in the system. A
 * pointer to a variable of this type is then passed to various driver API
 * functions.
*/
typedef struct {
	XTtcPs_Config Config;	/**< Configuration structure. */
	u32 IsReady;		/**< Device is initialized and ready. */
	XTtcPs_StatusHandler StatusHandler;	/**< Interrupt status callback function. */
	void *StatusRef;	/**< Callback reference for status handler. */
} XTtcPs;

typedef struct {
	XTtcPs_StatusHandler StatusHandler; /**< Interrupt Handler. */
	void *StatusRef;	/**< Callback reference for the status handler. */
} XTtcPs_StatusHandlerTableEntry;

/**
 * This typedef contains interval count and Match register value
*/
#if defined(ARMA9)
typedef u16 XInterval; /**< Interval count type for ARM A9 (16-bit). */
typedef u16 XMatchRegValue; /**< Match register value type for ARM A9 (16-bit). */
#else
typedef u32 XInterval; /**< Interval count type (32-bit). */
typedef u32 XMatchRegValue; /**< Match register value type (32-bit). */
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/*
 * Internal helper macros
 */
/** Read a TTC register using the instance base address */
#define InstReadReg(InstancePtr, RegOffset) \
	(Xil_In32(((InstancePtr)->Config.BaseAddress) + (u32)(RegOffset)))

/** Write a TTC register using the instance base address */
#define InstWriteReg(InstancePtr, RegOffset, Data) \
	(Xil_Out32(((InstancePtr)->Config.BaseAddress) + (u32)(RegOffset), (u32)(Data)))

/*****************************************************************************/
/**
*
* @brief Starts the counter/timer without resetting the counter value.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @note		C-Style signature:
*		void XTtcPs_Start(XTtcPs *InstancePtr)
*
****************************************************************************/
#define XTtcPs_Start(InstancePtr)	\
	InstWriteReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET,	\
		     (InstReadReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET) &	\
		      ~XTTCPS_CNT_CNTRL_DIS_MASK))

/*****************************************************************************/
/**
*
* @brief Stops the counter/timer.
*
* This macro may be called at any time to stop the counter. The counter holds
* the last value until it is reset, restarted or enabled.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @note		C-Style signature:
*		void XTtcPs_Stop(XTtcPs *InstancePtr)
*
****************************************************************************/
#define XTtcPs_Stop(InstancePtr)	\
	InstWriteReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET,	\
		     (InstReadReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET) |	\
		      XTTCPS_CNT_CNTRL_DIS_MASK))

/*****************************************************************************/
/**
*
* @brief Checks whether the timer counter has already started.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance
*
* @return	Non-zero if the device has started, '0' otherwise.
*
* @note		C-Style signature:
*		int XTtcPs_IsStarted(XTtcPs *InstancePtr)
*
****************************************************************************/
#define XTtcPs_IsStarted(InstancePtr) \
	((InstReadReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET) & \
	  XTTCPS_CNT_CNTRL_DIS_MASK) == 0U)

/*****************************************************************************/
/**
*
* @brief Returns the current counter value.
*
* It may be called at any time.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @return	zynq:16 bit counter value.
*           zynq ultrascale+mpsoc:32 bit counter value.
*
* @note		C-Style signature:
*		zynq: u16 XTtcPs_GetCounterValue(XTtcPs *InstancePtr)
*       zynq ultrascale+mpsoc: u32 XTtcPs_GetCounterValue(XTtcPs *InstancePtr)
*
****************************************************************************/
#if defined(ARMA9)
/*
 * ttc supports 16 bit counter for zynq
 */
#define XTtcPs_GetCounterValue(InstancePtr) \
	(u16)InstReadReg((InstancePtr), XTTCPS_COUNT_VALUE_OFFSET)
#else
/*
 * ttc supports 32 bit counter for zynq ultrascale+mpsoc
 */
#define XTtcPs_GetCounterValue(InstancePtr) \
	InstReadReg((InstancePtr), XTTCPS_COUNT_VALUE_OFFSET)
#endif

/*****************************************************************************/
/**
*
* @brief Sets the interval value to be used in interval mode.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
* @param[in]	Value is the interval value to be set in the interval register.
*
* @note		C-Style signature:
*		void XTtcPs_SetInterval(XTtcPs *InstancePtr, XInterval Value)
*
****************************************************************************/
#define XTtcPs_SetInterval(InstancePtr, Value)	\
	InstWriteReg((InstancePtr), XTTCPS_INTERVAL_VAL_OFFSET, (Value))

/*****************************************************************************/
/**
*
* @brief Gets the interval value from the interval register.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @return	zynq:16 bit interval value.
*           zynq ultrascale+mpsoc:32 bit interval value.
*
* @note		C-Style signature:
*		zynq: u16 XTtcPs_GetInterval(XTtcPs *InstancePtr)
*       zynq ultrascale+mpsoc: u32 XTtcPs_GetInterval(XTtcPs *InstancePtr)
*
****************************************************************************/
#if defined(ARMA9)
/*
 * ttc supports 16 bit interval counter for zynq
 */
#define XTtcPs_GetInterval(InstancePtr) \
	(u16)InstReadReg((InstancePtr), XTTCPS_INTERVAL_VAL_OFFSET)
#else
/*
 * ttc supports 32 bit interval counter for zynq ultrascale+mpsoc
 */
#define XTtcPs_GetInterval(InstancePtr) \
	InstReadReg((InstancePtr), XTTCPS_INTERVAL_VAL_OFFSET)
#endif
/*****************************************************************************/
/**
*
* @brief Resets the count register.
*
* It may be called at any time. The counter is reset to either 0 or 0xFFFF, or
* the interval value, depending on the increment/decrement mode. The state of
* the counter, as started or stopped, is not affected by calling reset.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @note		C-Style signature:
*		void XTtcPs_ResetCounterValue(XTtcPs *InstancePtr)
*
****************************************************************************/
#define XTtcPs_ResetCounterValue(InstancePtr) \
	InstWriteReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET,	\
		     (InstReadReg((InstancePtr), XTTCPS_CNT_CNTRL_OFFSET) | \
		      (u32)XTTCPS_CNT_CNTRL_RST_MASK))

/*****************************************************************************/
/**
*
* @brief Enables the interrupts.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
* @param[in]	InterruptMask defines which interrupt should be enabled.
*		Constants are defined in xttcps_hw.h as XTTCPS_IXR_*.
*		This is a bit mask, all set bits will be enabled, cleared bits
*		will not be disabled.
*
* @note		C-Style signature:
*	void XTtcPs_EnableInterrupts(XTtcPs *InstancePtr, u32 InterruptMask)
*
******************************************************************************/
#define XTtcPs_EnableInterrupts(InstancePtr, InterruptMask)	\
	InstWriteReg((InstancePtr), XTTCPS_IER_OFFSET,	\
		     (InstReadReg((InstancePtr), XTTCPS_IER_OFFSET) |	\
		      (InterruptMask)))

/*****************************************************************************/
/**
*
* @brief Disables the interrupts.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
* @param[in]	InterruptMask defines which interrupt should be disabled.
*		Constants are defined in xttcps_hw.h as XTTCPS_IXR_*.
*		This is a bit mask, all set bits will be disabled, cleared bits
*		will not be disabled.
*
* @note		C-Style signature:
*	void XTtcPs_DisableInterrupts(XTtcPs *InstancePtr, u32 InterruptMask)
*
******************************************************************************/
#define XTtcPs_DisableInterrupts(InstancePtr, InterruptMask) \
	InstWriteReg((InstancePtr), XTTCPS_IER_OFFSET,	\
		     (InstReadReg((InstancePtr), XTTCPS_IER_OFFSET) &	\
		      ~(InterruptMask)))

/*****************************************************************************/
/**
*
* @brief Reads the interrupt status.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
*
* @return	The current interrupt status register value. Reading the register
*		clears all pending interrupt status bits (read-to-clear behavior).
*
* @note		C-Style signature:
*		u32 XTtcPs_GetInterruptStatus(XTtcPs *InstancePtr)
*
******************************************************************************/
#define XTtcPs_GetInterruptStatus(InstancePtr) \
	InstReadReg((InstancePtr), XTTCPS_ISR_OFFSET)

/*****************************************************************************/
/**
*
* @brief Clears all pending interrupt status bits by reading the status register.
* @param[in]	InstancePtr is a pointer to the XTtcPs instance.
* @param[in]	InterruptMask is retained for API compatibility and is ignored.
*		The macro clears all pending bits by reading the read-to-clear ISR.
*
* @note		C-Style signature:
*	void XTtcPs_ClearInterruptStatus(XTtcPs *InstancePtr, u32 InterruptMask)
*
******************************************************************************/
#define XTtcPs_ClearInterruptStatus(InstancePtr, InterruptMask) \
	(void)	InterruptMask; \
	InstReadReg((InstancePtr), XTTCPS_ISR_OFFSET)


/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xttcps_sinit.c
 */
#ifndef SDT
XTtcPs_Config *XTtcPs_LookupConfig(u16 DeviceId);
#else
XTtcPs_Config *XTtcPs_LookupConfig(u32 BaseAddress);
#endif

/*
 * Required functions, in xttcps.c
 */
s32 XTtcPs_CfgInitialize(XTtcPs *InstancePtr,
			 XTtcPs_Config * ConfigPtr, u32 EffectiveAddr);

u32 XTtcPs_Release(XTtcPs *InstancePtr);
void XTtcPs_SetMatchValue(XTtcPs *InstancePtr, u8 MatchIndex, XMatchRegValue Value);
XMatchRegValue XTtcPs_GetMatchValue(XTtcPs *InstancePtr, u8 MatchIndex);

void XTtcPs_SetPrescaler(XTtcPs *InstancePtr, u8 PrescalerValue);
u8 XTtcPs_GetPrescaler(XTtcPs *InstancePtr);

void XTtcPs_CalcIntervalFromFreq(XTtcPs *InstancePtr, u32 Freq,
				 XInterval *Interval, u8 *Prescaler);

/*
 * Functions for options, in file xttcps_options.c
 */
s32 XTtcPs_SetOptions(XTtcPs *InstancePtr, u32 Options);
u32 XTtcPs_GetOptions(XTtcPs *InstancePtr);

/*
 * Function for self-test, in file xttcps_selftest.c
 */
s32 XTtcPs_SelfTest(XTtcPs *InstancePtr);
u32 XTtcPs_InterruptHandler(XTtcPs *InstancePtr);
void XTtcPs_SetStatusHandler(XTtcPs *InstancePtr, void *CallBackRef,
			     XTtcPs_StatusHandler FuncPointer);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */

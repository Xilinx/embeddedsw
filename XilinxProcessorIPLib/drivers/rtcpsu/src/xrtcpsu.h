/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XRTC_H_
#define XRTC_H_

/*****************************************************************************/
/**
 * @file xrtcpsu.h
 * @addtogroup rtcpsu Overview
 * @{
 * @brief Provides the main header file for the XRtcPsu RTC driver.
 *
 * @details
 *
 * The Xilinx RTC driver component supports the Xilinx RTC Controller. RTC
 * Core and RTC controller are the two main important sub-components for this
 * RTC module.
 *
 * RTC core can run even in the battery powered domain when the power from
 * auxiliary source is down. Because of this, RTC core latches the calibration
 * and programmed time. This core interfaces with the crystal oscillator and
 * maintains current time in seconds. Calibration circuitry calculates a second
 * with maximum 1 PPM inaccuracy using a crystal oscillator with arbitrary
 * static inaccuracy. The core is also responsible for maintaining the control
 * value used by the oscillator and power switching circuitry.
 *
 * RTC controller includes an APB interface responsible for register access
 * within the controller and core. It contains alarm generation logic including
 * the alarm register to hold alarm time in seconds. Interrupt status, Interrupt
 * mask, Interrupt enable, and Interrupt disable registers manage the alarm and
 * seconds interrupts. This driver component does not handle Address Slave error
 * interrupts.
 *
 * This driver supports the following features:
 * - Setting the RTC time.
 * - Setting the Alarm value that can be one-time alarm or a periodic alarm.
 * - Modifying the calibration value.
 *
 * <b>Initialization & Configuration</b>
 *
 * The driver uses the XRtcPsu_Config structure to configure itself.
 * Fields inside this structure are properties of XRtcPsu based on its hardware
 * build.
 *
 * To support multiple runtime loading and initialization strategies employed
 * by various operating systems, the driver instance can be initialized in the
 * following way:
 *
 *	- XRtcPsu_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
 *	configuration structure provided by the caller. If running in a system
 *	with address translation, the parameter EffectiveAddr must be the
 *	virtual address.
 *
 * <b>Interrupts</b>
 *
 * The driver defaults to no interrupts at initialization, so enable
 * interrupts if required. An interrupt is generated for one of the
 * following conditions.
 *
 * - Alarm is generated.
 * - A new second is generated.
 *
 * The application can control which interrupts are enabled using the
 * XRtcPsu_SetInterruptMask() function.
 *
 * To use interrupts, connect the driver interrupt handler,
 * XRtcPsu_InterruptHandler(), to the interrupt system of the application.
 * The application provides a separate handler to communicate with the
 * interrupt system and conduct application-specific interrupt handling. An
 * application registers its own handler through the XRtcPsu_SetHandler()
 * function.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date	Changes
 * ----- -----  -------- -----------------------------------------------
 * 1.00  kvn    04/21/15 First release
 * 1.1   kvn    09/25/15 Modify control register to enable battery
 *                       switching when vcc_psaux is not available.
 * 1.3	 vak    04/25/16 Corrected the RTC read and write time logic(cr#948833).
 * 1.4	 MNK    01/27/17 Corrected calibration and frequency macros based on
 *			rtc input oscillator frequency ( 32.768Khz).
 *       ms     03/17/17 Added readme.txt file in examples folder for doxygen
 *                       generation.
 *       ms     04/10/17 Modified filename tag in examples to include them in
 *                       doxygen examples.
 * 1.5   ms     08/27/17 Fixed compilation warnings in xrtcpsu.c file.
 *       ms     08/29/17 Updated the code as per source code style.
 * 1.6   aru    06/25/18 Modified logic to handle
 *			 the last day of month cotrrecly.(CR#1004282)
 * 1.6   aru    06/25/18 Remove the checkpatch warnings.
 * 1.6   aru    07/11/18 Resolved cppcheck warnings.
 * 1.6   aru    07/11/18 Resolved doxygen warnings.
 * 1.6   aru    08/17/18 Resolved MISRA-C mandatory violations.(CR#1007752)
 * 1.6   tjs    09/17/18 Fixed compilation warnings.
 * 1.7   sne    03/01/19 Added Versal support.
 * 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
 *                       modified the code such as
 *                       No brackets to loop body,Declared the pointer param
 *                       as Pointer to const,No brackets to then/else,
 *                       Literal value requires a U suffix,Casting operation to a pointer
 *                       Array has no bounds specified,Logical conjunctions need brackets.
 * 1.10	 sne    08/28/20 Modify Makefile to support parallel make execution.
 * 1.11	 sne	04/23/21 Fixed doxygen warnings.
 * 1.13  ht	06/22/23 Added support for system device-tree flow.
 * 1.17  vlt    12/30/25 Update Doxygen comments to include SDT flow details.
 * 1.17  ht     01/19/26 Add 3 macros (EPOCH_YEAR, EPOCH_WDAY, IS_LEAP_YEAR).
 * 1.18  vn     09/30/26 Align Doxygen comments with AMD standards
 * 1.18  vn     10/03/26 Fix MISRA C:2025 Rule 12.1 violation in IS_LEAP_YEAR.
 * </pre>
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xrtcpsu_hw.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/** @name Callback events
 *
 * These constants specify the handler events that an application can handle
 * using its specific handler function. These constants are not bit masks, so
 * only one event can be passed to an application at a time.
 *
 * @{
 */
/** Alarm generated event. */
#define XRTCPSU_EVENT_ALARM_GEN		1U
#define XRTCPSU_EVENT_SECS_GEN		2U /**< A new second generated event. */
/** @} */

/** Separate mask for the crystal oscillator enable bit. */
#define XRTCPSU_CRYSTAL_OSC_EN		((u32)1 << XRTC_CTL_OSC_SHIFT)

/**************************** Type Definitions *******************************/

/******************************************************************************/
/**
 * @brief Defines the application handler type for interrupt events.
 *
 * An application uses this handler type to communicate with the interrupt
 * system to retrieve state information about the application.
 *
 * @param[in]	CallBackRef Callback reference passed in by the upper layer
 *		when setting the handler, and passed back to the upper layer
 *		when the handler is called. It is used to find the device driver
 *		instance.
 * @param[in]	Event One of the event constants indicating events that
 *		have occurred.
 *
 ******************************************************************************/
typedef void (*XRtcPsu_Handler) (void *CallBackRef, u32 Event);

/**
 * @brief Holds the configuration information for a device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device. */
#else
	char *Name;		/**< Name of the device. */
#endif
	UINTPTR BaseAddr;	/**< Register base address. */
#ifdef SDT
	u16 IntrId[2];          /**< Bits[11:0] Interrupt-id Bits[15:12]
				 * trigger type and level flags. */
	UINTPTR IntrParent;     /**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address. */
#endif
} XRtcPsu_Config;

/**
 * @brief Stores the XRtcPsu driver instance data.
 *
 * Allocate a variable of this type for the RTC device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XRtcPsu_Config RtcConfig;	/**< Device configuration. */
	u32 IsReady;			/**< Device is initialized and ready. */
	u32 PeriodicAlarmTime;		/**< Alarm timer. */
	u8 IsPeriodicAlarm;		/**< Alarm Interrupt Information. */
	u32 OscillatorFreq;		/**< Device Oscillator Freq. */
	u32 CalibrationValue;		/**< Device Calibration Value. */
	XRtcPsu_Handler Handler;	/**< Handler Information. */
	void *CallBackRef;		/**< Callback reference for
					 * event handler. */
	u32 TimeUpdated;		/**< Time. */
	u32 CurrTimeUpdated;		/**< Current Time. */
} XRtcPsu;

/**
 * @brief Describes the DateTime format structure.
 */
typedef struct {
	u32 Year;	/**< Year. */
	u32 Month;	/**< Month. */
	u32 Day;	/**< Day. */
	u32 Hour;	/**< Hour. */
	u32 Min;	/**< Minute. */
	u32 Sec;	/**< Second. */
	u32 WeekDay;	/**< Week Day. */
} XRtcPsu_DT;


/************************* Variable Definitions ******************************/

/**
 * @brief Holds the configuration entry for each RTC PS instance.
 */
extern XRtcPsu_Config XRtcPsu_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Unix epoch constants
 * @{
 */
#define EPOCH_YEAR	1970U /**< Unix epoch year (1970-01-01 00:00:00 UTC). */
/** Weekday of epoch (0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat). */
#define EPOCH_WDAY	4U
/** @} */

/****************************************************************************/
/**
 * @brief Checks if a year is a leap year using complete Gregorian calendar
 * rules.
 *
 * - Divisible by 4: leap year.
 * - Exception: Divisible by 100: not a leap year.
 * - Exception to exception: Divisible by 400: leap year.
 *
 * Examples: 2000 (leap), 2004 (leap), 2100 (not leap).
 *
 * @param[in]	year Year to check.
 *
 * @return	1 if the year is a leap year, otherwise 0.
 *
 * @note	C-style signature:
 *		int IS_LEAP_YEAR(u32 year)
 *
 *****************************************************************************/
#define IS_LEAP_YEAR(year) \
	((((year) % 4U) == 0U) && \
	 ((((year) % 100U) != 0U) || (((year) % 400U) == 0U)))

/** @name Default calibration settings
 * @{
 */
#define XRTC_CALIBRATION_VALUE 0x7FFFU	/**< Calibration value. */
#define XRTC_TYPICAL_OSC_FREQ 32768U	/**< Oscillator frequency. */
/** @} */

/****************************************************************************/
/**
 *
 * @brief Updates the current time of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 * @param[in]	Time Desired time for RTC in seconds.
 *
 * @note	C-style signature:
 *		void XRtcPsu_WriteSetTime(XRtcPsu *InstancePtr, u32 Time)
 *
 *****************************************************************************/
#define XRtcPsu_WriteSetTime(InstancePtr, Time) \
	XRtcPsu_WriteReg(((InstancePtr)->RtcConfig.BaseAddr + \
			  XRTC_SET_TIME_WR_OFFSET), (Time))

/****************************************************************************/
/**
 *
 * @brief Returns the last set time of the RTC device.
 *
 * Whenever a reset happens, the last set time is the zeroth day first sec.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 *
 * @return	The last set time in seconds.
 *
 * @note	C-style signature:
 *		u32 XRtcPsu_GetLastSetTime(XRtcPsu  *InstancePtr)
 *
 *****************************************************************************/
#define XRtcPsu_GetLastSetTime(InstancePtr) \
	XRtcPsu_ReadReg((InstancePtr)->RtcConfig.BaseAddr + \
			XRTC_SET_TIME_RD_OFFSET)

/****************************************************************************/
/**
 *
 * @brief Returns the calibration value of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 *
 * @return	Calibration value for RTC.
 *
 * @note	C-style signature:
 *		u32 XRtcPsu_GetCalibration(XRtcPsu  *InstancePtr)
 *
 *****************************************************************************/
#define XRtcPsu_GetCalibration(InstancePtr) \
	XRtcPsu_ReadReg((InstancePtr)->RtcConfig.BaseAddr+XRTC_CALIB_RD_OFFSET)

/****************************************************************************/
/**
 *
 * @brief Returns the current time of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 *
 * @return	Current Time. This current time is in seconds.
 *
 * @note	C-style signature:
 *		u32 XRtcPsu_ReadCurrentTime(XRtcPsu  *InstancePtr)
 *
 *****************************************************************************/
#define XRtcPsu_ReadCurrentTime(InstancePtr) \
	XRtcPsu_ReadReg((InstancePtr)->RtcConfig.BaseAddr+XRTC_CUR_TIME_OFFSET)

/****************************************************************************/
/**
 *
 * @brief Sets the control register value of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 * @param[in]	Value Desired control register value for RTC.
 *
 * @note	C-style signature:
 *		void
 *		XRtcPsu_SetControlRegister(XRtcPsu *InstancePtr, u32 Value)
 *
 *****************************************************************************/
#define XRtcPsu_SetControlRegister(InstancePtr, Value) \
	XRtcPsu_WriteReg((InstancePtr)->RtcConfig.BaseAddr + \
			 XRTC_CTL_OFFSET, (Value))

/****************************************************************************/
/**
 *
 * @brief Returns the safety check register value of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 *
 * @return	Safety check register value.
 *
 * @note	C-style signature:
 *		u32 XRtcPsu_GetSafetyCheck(XRtcPsu  *InstancePtr)
 *
 *****************************************************************************/
#define XRtcPsu_GetSafetyCheck(InstancePtr)	\
	XRtcPsu_ReadReg((InstancePtr)->RtcConfig.BaseAddr+XRTC_SFTY_CHK_OFFSET)

/****************************************************************************/
/**
 *
 * @brief Sets the safety check register value of the RTC device.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 * @param[in]	Value Safety check value to be written in register.
 *
 * @note	C-style signature:
 *		void XRtcPsu_SetSafetyCheck(XRtcPsu  *InstancePtr, u32 Value)
 *
 *****************************************************************************/
#define XRtcPsu_SetSafetyCheck(InstancePtr, Value)	\
	XRtcPsu_WriteReg((InstancePtr)->RtcConfig.BaseAddr + \
			 XRTC_SFTY_CHK_OFFSET, (Value))

/****************************************************************************/
/**
 *
 * @brief Resets the alarm register.
 *
 * @param[in]	InstancePtr Pointer to the XRtcPsu instance.
 *
 * @note	C-style signature:
 *		void XRtcPsu_ResetAlarm(XRtcPsu *InstancePtr)
 *
 *****************************************************************************/
#define XRtcPsu_ResetAlarm(InstancePtr) \
	XRtcPsu_WriteReg((InstancePtr)->RtcConfig.BaseAddr + \
			 XRTC_ALRM_OFFSET, XRTC_ALRM_RSTVAL)

/****************************************************************************/
/**
 *
 * @brief Rounds off the given number.
 *
 * @param[in]	Number Number that needs to be rounded off.
 *
 * @return	The rounded off value of the input number.
 *
 * @note	C-style signature:
 *		u32 XRtcPsu_RoundOff(float Number)
 *
 *****************************************************************************/
#define XRtcPsu_RoundOff(Number) \
	(u32)(((Number) < (float)0) ? ((Number) - (float)0.5) : \
	      ((Number) + (float)0.5))

/************************** Function Prototypes ******************************/

/* Functions in xrtcpsu.c */
s32 XRtcPsu_CfgInitialize(XRtcPsu *InstancePtr, XRtcPsu_Config *ConfigPtr,
			  UINTPTR EffectiveAddr);

void XRtcPsu_SetAlarm(XRtcPsu *InstancePtr, u32 Alarm, u32 Periodic);
void XRtcPsu_SecToDateTime(u32 Seconds, XRtcPsu_DT *dt);
u32 XRtcPsu_DateTimeToSec(XRtcPsu_DT *dt);
void XRtcPsu_CalculateCalibration(XRtcPsu *InstancePtr, u32 TimeReal,
				  u32 CrystalOscFreq);
u32 XRtcPsu_IsSecondsEventGenerated(XRtcPsu *InstancePtr);
u32 XRtcPsu_IsAlarmEventGenerated(XRtcPsu *InstancePtr);
u32 XRtcPsu_GetCurrentTime(XRtcPsu *InstancePtr);
void XRtcPsu_SetTime(XRtcPsu *InstancePtr, u32 Time);

/* interrupt functions in xrtcpsu_intr.c */
void XRtcPsu_SetInterruptMask(XRtcPsu *InstancePtr, u32 Mask);
void XRtcPsu_ClearInterruptMask(XRtcPsu *InstancePtr, u32 Mask);
void XRtcPsu_InterruptHandler(XRtcPsu *InstancePtr);
void XRtcPsu_SetHandler(XRtcPsu *InstancePtr, XRtcPsu_Handler FunctionPtr,
			void *CallBackRef);

/* Functions in xrtcpsu_selftest.c */
s32 XRtcPsu_SelfTest(XRtcPsu *InstancePtr);

/* Functions in xrtcpsu_sinit.c */
#ifndef SDT
XRtcPsu_Config *XRtcPsu_LookupConfig(u16 DeviceId);
#else
XRtcPsu_Config *XRtcPsu_LookupConfig(UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* XRTC_H_ */

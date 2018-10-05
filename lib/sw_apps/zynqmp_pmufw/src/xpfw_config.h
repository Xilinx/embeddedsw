/******************************************************************************
*
* Copyright (C) 2015 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPFW_CONFIG_H_
#define XPFW_CONFIG_H_

/************* User Configurable Options ***************/

/* PMUFW print levels */
#define XPFW_PRINT_VAL (1U)
#define XPFW_DEBUG_ERROR_VAL (0U)
#define XPFW_DEBUG_DETAILED_VAL (0U)

/**
 * PMUFW Debug options
 */

#if XPFW_PRINT_VAL
#define XPFW_PRINT
#endif

#if XPFW_DEBUG_ERROR_VAL
#define XPFW_DEBUG_ERROR
#endif

#if XPFW_DEBUG_DETAILED_VAL
#define XPFW_DEBUG_DETAILED
#endif

/* PMU clock frequency in Hz */
#ifndef XPFW_CFG_PMU_CLK_FREQ
#define XPFW_CFG_PMU_CLK_FREQ XPAR_CPU_CORE_CLOCK_FREQ_HZ
#endif

/* Let the MB sleep when it is Idle in Main Loop */
#define SLEEP_WHEN_IDLE

/*
 * PMU Firmware code include options
 *
 * PMU Firmware by default disables some functionality and enables some
 * Here we are listing all the build flags with the default option.
 * User can modify these flags to enable or disable any module/functionality
 * 	- ENABLE_PM : Enables Power Management Module
 * 	- ENABLE_EM : Enables Error Management Module
 * 	- ENABLE_SCHEDULER : Enables the scheduler
 * 	- ENABLE_RECOVERY : Enables WDT based restart of APU sub-system
 *	- ENABLE_RECOVERY_RESET_SYSTEM : Enables WDT based restart of system
 *	- ENABLE_RECOVERY_RESET_PS_ONLY : Enables WDT based restart of PS
 * 	- ENABLE_ESCALATION : Enables escalation of sub-system restart to
 * 	                      SRST/PS-only if the first restart attempt fails
 * 	- ENABLE_WDT : Enables WDT based restart functionality for PMU
 * 	- ENABLE_STL : Enables STL Module
 * 	- ENABLE_RTC_TEST : Enables RTC Event Handler Test Module
 * 	- ENABLE_SAFETY : Enables CRC calculation for IPI messages
 * 	- ENABLE_FPGA_LOAD : Enables FPGA bit stream loading feature
 * 	- ENABLE_SECURE : Enables security features
 * 	- XPU_INTR_DEBUG_PRINT_ENABLE : Enables debug for XMPU/XPPU functionality
 *
 * 	- PM_LOG_LEVEL : Enables print based debug functions for PM. Possible
 *			values are: 1 (alerts), 2 (errors), 3 (warnings),
 *			4 (info). Higher numbers include the debug scope of
 *			lower number, i.e. enabling 3 (warnings) also enables
 *			1 (alerts) and 2 (errors).
 * 	- IDLE_PERIPHERALS : Enables idling peripherals before PS or System reset
 * 	- ENABLE_NODE_IDLING : Enables idling and reset of nodes before force
 * 	                       of a sub-system
 * 	- DEBUG_MODE : This macro enables PM debug prints if XPFW_DEBUG_DETAILED
 * 	               macro is also defined
 *	- ENABLE_POS : Enables Power Off Suspend feature
 *	- ENABLE_DDR_SR_WR : Enables DDR self refresh over warm restart feature
 *	- ENABLE_UNUSED_RPU_PWR_DWN : Enables unused RPU power down feature
 *	- DISABLE_CLK_PERMS : Disable clock permission checking (it is not safe
 *			to ever disable clock permission checking). Do this at
 *			your own responsibility.
 *	- ENABLE_EFUSE_ACCESS : Enables efuse access feature
 *
 * 	These macros are specific to ZCU100 design where it uses GPO1[2] as a
 * 	board power line and
 * 	- PMU_MIO_INPUT_PIN : Enables board shutdown related code for ZCU100
 * 	- BOARD_SHUTDOWN_PIN : Tells board shutdown pin. In case of ZCU100,
 * 	                       GPO1[2] is the board power line.
 * 	- BOARD_SHUTDOWN_PIN_STATE : Tells what should be the state of board power
 * 	                             line when system shutdown request comes
 */

#define	ENABLE_PM_VAL					(1U)
#define	ENABLE_EM_VAL					(0U)
#define	ENABLE_SCHEDULER_VAL			(0U)
#define	ENABLE_RECOVERY_VAL				(0U)
#define	ENABLE_RECOVERY_RESET_SYSTEM_VAL		(0U)
#define	ENABLE_RECOVERY_RESET_PS_ONLY_VAL		(0U)
#define	ENABLE_ESCALATION_VAL			(0U)
#define CHECK_HEALTHY_BOOT_VAL			(0U)
#define	ENABLE_WDT_VAL					(0U)
#define	ENABLE_STL_VAL					(0U)
#define	ENABLE_RTC_TEST_VAL				(0U)
#define	ENABLE_SAFETY_VAL				(0U)
#define	ENABLE_FPGA_LOAD_VAL			(1U)
#define	ENABLE_SECURE_VAL				(1U)
#define ENABLE_EFUSE_ACCESS				(0U)
#define	XPU_INTR_DEBUG_PRINT_ENABLE_VAL	(0U)

#define	PM_LOG_LEVEL_VAL				(0U)
#define	IDLE_PERIPHERALS_VAL			(0U)
#define	ENABLE_NODE_IDLING_VAL			(0U)
#define	DEBUG_MODE_VAL					(0U)
#define	ENABLE_POS_VAL					(0U)
#define	ENABLE_DDR_SR_WR_VAL				(0U)
#define DISABLE_CLK_PERMS_VAL				(0U)
#define ENABLE_UNUSED_RPU_PWR_DWN_VAL			(1U)

#define	PMU_MIO_INPUT_PIN_VAL			(0U)
#define	BOARD_SHUTDOWN_PIN_VAL			(0U)
#define	BOARD_SHUTDOWN_PIN_STATE_VAL	(0U)

#define SECURE_ACCESS_VAL		(0U)

#if ENABLE_PM_VAL
#define ENABLE_PM
#endif

#if ENABLE_EM_VAL
#define ENABLE_EM
#endif

#if ENABLE_SCHEDULER_VAL
#define ENABLE_SCHEDULER
#endif

#if ENABLE_RECOVERY_VAL
#define ENABLE_RECOVERY
#endif

#if ENABLE_RECOVERY_RESET_SYSTEM_VAL
#define ENABLE_RECOVERY_RESET_SYSTEM
#endif

#if ENABLE_RECOVERY_RESET_PS_ONLY_VAL
#define ENABLE_RECOVERY_RESET_PS_ONLY
#endif

#if ENABLE_ESCALATION_VAL
#define ENABLE_ESCALATION
#endif

#if CHECK_HEALTHY_BOOT_VAL
#define CHECK_HEALTHY_BOOT
#endif

#if ENABLE_WDT_VAL
#define ENABLE_WDT
#endif

#if ENABLE_STL_VAL
#define ENABLE_STL
#endif

#if ENABLE_RTC_TEST_VAL
#define ENABLE_RTC_TEST
#endif

#if ENABLE_FPGA_LOAD_VAL
#define ENABLE_FPGA_LOAD
#endif
#if ENABLE_SECURE_VAL
#define ENABLE_SECURE
#endif
#if ENABLE_SAFETY_VAL
#define ENABLE_SAFETY
#endif

#if XPU_INTR_DEBUG_PRINT_ENABLE_VAL
#define XPU_INTR_DEBUG_PRINT_ENABLE
#endif

#if PM_LOG_LEVEL_VAL > 0
#define PM_LOG_LEVEL	PM_LOG_LEVEL_VAL
#endif

#if IDLE_PERIPHERALS_VAL
#define IDLE_PERIPHERALS
#endif

#if ENABLE_NODE_IDLING_VAL
#define ENABLE_NODE_IDLING
#endif

#if DEBUG_MODE_VAL
#define DEBUG_MODE
#endif

#if ENABLE_POS_VAL
#define ENABLE_POS
#endif

#if ENABLE_DDR_SR_WR_VAL
#define ENABLE_DDR_SR_WR
#endif

#if DISABLE_CLK_PERMS_VAL
#define DISABLE_CLK_PERMS
#endif

#if ENABLE_UNUSED_RPU_PWR_DWN_VAL
#define ENABLE_UNUSED_RPU_PWR_DWN
#endif

#if PMU_MIO_INPUT_PIN_VAL
#define PMU_MIO_INPUT_PIN			0U
#endif

#if BOARD_SHUTDOWN_PIN_VAL
#define BOARD_SHUTDOWN_PIN			2U
#endif

#if BOARD_SHUTDOWN_PIN_STATE_VAL
#define BOARD_SHUTDOWN_PIN_STATE	0U
#endif

#if SECURE_ACCESS_VAL
#define SECURE_ACCESS
#endif

#if ENABLE_EFUSE_ACCESS
#define EFUSE_ACCESS
#endif

/* FPD WDT recovery action */
#ifdef ENABLE_RECOVERY
#define FPD_WDT_EM_ACTION EM_ACTION_CUSTOM
#else
#define FPD_WDT_EM_ACTION EM_ACTION_SRST
#endif

#ifdef ENABLE_POS
#define ENABLE_POS_QSPI
#endif

#endif /* XPFW_CONFIG_H_ */

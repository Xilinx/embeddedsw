/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_CONFIG_H_
#define XPFW_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"

/************* User Configurable Options ***************/

/**
 * PMU Firmware print levels
 *
 * XPFW_PRINT : Prints PMU Firmware header and any mandatory prints
 * XPFW_DEBUG_ERROR : Enables Firmware error prints
 * XPFW_DEBUG_DETAILED : Enables Firmware detailed prints
 */
#ifndef XPFW_PRINT_VAL
#define XPFW_PRINT_VAL						(1U)
#endif

#ifndef XPFW_DEBUG_ERROR_VAL
#define XPFW_DEBUG_ERROR_VAL				(0U)
#endif

#ifndef XPFW_DEBUG_DETAILED_VAL
#define XPFW_DEBUG_DETAILED_VAL				(0U)
#endif

/**
 * PMUFW Debug options
 */

#if (XPFW_PRINT_VAL) && (!defined(XPFW_PRINT))
#define XPFW_PRINT
#endif

#if (XPFW_DEBUG_ERROR_VAL) && (!defined(XPFW_DEBUG_ERROR))
#define XPFW_DEBUG_ERROR
#endif

#if (XPFW_DEBUG_DETAILED_VAL) && (!defined(XPFW_DEBUG_DETAILED))
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
 *  - ENABLE_MOD_ULTRA96 : Enables support for Ultra96 power button
 * 	- ENABLE_RECOVERY : Enables WDT based restart of APU sub-system
 *	- ENABLE_RECOVERY_RESET_SYSTEM : Enables WDT based restart of system
 *	- ENABLE_RECOVERY_RESET_PS_ONLY : Enables WDT based restart of PS
 * 	- ENABLE_ESCALATION : Enables escalation of sub-system restart to
 * 	                      SRST/PS-only if the first restart attempt fails
 * 	- ENABLE_WDT : Enables WDT based restart functionality for PMU
 * 	- ENABLE_STL : Enables STL Module
 * 	- ENABLE_RTC_TEST : Enables RTC Event Handler Test Module
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
 *  - USE_DDR_FOR_APU_RESTART : If this macro is enabled, PMU writes FSBL image
 *              to DDR from OCM if FSBL is running on APU. This is to free-up
 *              OCM memory for other uses.
 *  - ENABLE_RPU_RUN_MODE: Enables RPU monitoring module
 *
 * 	These macros are specific to ZCU100 design where it uses GPO1[2] as a
 * 	board power line and
 * 	- PMU_MIO_INPUT_PIN : Enables board shutdown related code for ZCU100
 * 	- BOARD_SHUTDOWN_PIN : Tells board shutdown pin. In case of ZCU100,
 * 	                       GPO1[2] is the board power line.
 * 	- BOARD_SHUTDOWN_PIN_STATE : Tells what should be the state of board power
 * 	                             line when system shutdown request comes
 */
#ifndef ENABLE_PM_VAL
#define	ENABLE_PM_VAL						(1U)
#endif

#ifndef ENABLE_EM_VAL
#define	ENABLE_EM_VAL						(0U)
#endif

#ifndef ENABLE_SCHEDULER_VAL
#define	ENABLE_SCHEDULER_VAL				(1U)
#endif

#ifndef ENABLE_MOD_ULTRA96_VAL
#define ENABLE_MOD_ULTRA96_VAL				(0U)
#endif

#ifndef ENABLE_RECOVERY_VAL
#define	ENABLE_RECOVERY_VAL					(0U)
#endif

#ifndef ENABLE_RECOVERY_RESET_SYSTEM_VAL
#define	ENABLE_RECOVERY_RESET_SYSTEM_VAL	(0U)
#endif

#ifndef ENABLE_RECOVERY_RESET_PS_ONLY_VAL
#define	ENABLE_RECOVERY_RESET_PS_ONLY_VAL	(0U)
#endif

#ifndef ENABLE_ESCALATION_VAL
#define	ENABLE_ESCALATION_VAL				(0U)
#endif

#ifndef CHECK_HEALTHY_BOOT_VAL
#define CHECK_HEALTHY_BOOT_VAL				(0U)
#endif

#ifndef ENABLE_WDT_VAL
#define	ENABLE_WDT_VAL						(0U)
#endif

#ifndef ENABLE_CUSTOM_MOD_VAL
#define ENABLE_CUSTOM_MOD_VAL				(0U)
#endif

#ifndef ENABLE_STL_VAL
#define	ENABLE_STL_VAL						(0U)
#endif

#ifndef ENABLE_RTC_TEST_VAL
#define	ENABLE_RTC_TEST_VAL					(0U)
#endif

#ifndef ENABLE_FPGA_LOAD_VAL
#define	ENABLE_FPGA_LOAD_VAL				(1U)
#endif

#ifndef ENABLE_FPGA_READ_CONFIG_DATA_VAL
#define ENABLE_FPGA_READ_CONFIG_DATA_VAL	(1U)
#endif

#ifndef ENABLE_FPGA_READ_CONFIG_REG_VAL
#define ENABLE_FPGA_READ_CONFIG_REG_VAL		(1U)
#endif

#ifndef ENABLE_SECURE_VAL
#define	ENABLE_SECURE_VAL					(1U)
#endif

#ifndef ENABLE_EFUSE_ACCESS
#define ENABLE_EFUSE_ACCESS					(0U)
#endif

#ifndef XPU_INTR_DEBUG_PRINT_ENABLE_VAL
#define	XPU_INTR_DEBUG_PRINT_ENABLE_VAL		(0U)
#endif

#ifndef PM_LOG_LEVEL_VAL
#define	PM_LOG_LEVEL_VAL					(0U)
#endif

#ifndef IDLE_PERIPHERALS_VAL
#define	IDLE_PERIPHERALS_VAL				(0U)
#endif

#ifndef ENABLE_NODE_IDLING_VAL
#define	ENABLE_NODE_IDLING_VAL				(0U)
#endif

#ifndef DEBUG_MODE_VAL
#define	DEBUG_MODE_VAL						(0U)
#endif

#ifndef ENABLE_POS_VAL
#define	ENABLE_POS_VAL						(0U)
#endif

#ifndef ENABLE_DDR_SR_WR_VAL
#define	ENABLE_DDR_SR_WR_VAL				(0U)
#endif

#ifndef DISABLE_CLK_PERMS_VAL
#define DISABLE_CLK_PERMS_VAL				(0U)
#endif

#ifndef ENABLE_UNUSED_RPU_PWR_DWN_VAL
#define ENABLE_UNUSED_RPU_PWR_DWN_VAL		(1U)
#endif

#ifndef PMU_MIO_INPUT_PIN_VAL
#define	PMU_MIO_INPUT_PIN_VAL				(0U)
#endif

#ifndef BOARD_SHUTDOWN_PIN_VAL
#define	BOARD_SHUTDOWN_PIN_VAL				(0U)
#endif

#ifndef BOARD_SHUTDOWN_PIN_STATE_VAL
#define	BOARD_SHUTDOWN_PIN_STATE_VAL		(0U)
#endif

#ifndef CONNECT_PMU_GPO_2_VAL
#define CONNECT_PMU_GPO_2_VAL				(1U)
#endif

#ifndef CONNECT_PMU_GPO_3_VAL
#define CONNECT_PMU_GPO_3_VAL				(1U)
#endif

#ifndef CONNECT_PMU_GPO_4_VAL
#define CONNECT_PMU_GPO_4_VAL				(1U)
#endif

#ifndef CONNECT_PMU_GPO_5_VAL
#define CONNECT_PMU_GPO_5_VAL				(1U)
#endif

#ifndef SECURE_ACCESS_VAL
#define SECURE_ACCESS_VAL					(0U)
#endif

#ifndef USE_DDR_FOR_APU_RESTART_VAL
#define USE_DDR_FOR_APU_RESTART_VAL			(1U)
#endif

#ifndef ENABLE_RPU_RUN_MODE_VAL
#define ENABLE_RPU_RUN_MODE_VAL				(0U)
#endif

#ifndef ENABLE_IOCTL_VAL
#define ENABLE_IOCTL_VAL				(0U)
#endif

#ifndef ENABLE_RUNTIME_OVERTEMP_VAL
#define ENABLE_RUNTIME_OVERTEMP_VAL 			(0U)
#endif

#ifndef ENABLE_RUNTIME_EXTWDT_VAL
#define ENABLE_RUNTIME_EXTWDT_VAL 			(0U)
#endif

/*
 * XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 * 		Default watchdog timeout
 *
 * XPFW_CFG_PMU_FPGA_WDT_TIMEOUT
 * 		This watchdog timeout is applied during bitstream download, provided
 * 		its value is greater than XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 *
 * XPFW_CFG_PMU_SHA3_WDT_TIMEOUT
 * 		This watchdog timeout is applied during SHA3 request, provided
 * 		its value is greater than XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 *
 * XPFW_CFG_PMU_RSA_WDT_TIMEOUT
 * 		This watchdog timeout is applied during RSA request, provided
 * 		its value is greater than XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 *
 * XPFW_CFG_PMU_AES_WDT_TIMEOUT
 * 		This watchdog timeout is applied during AES request, provided
 * 		its value is greater than XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 *
 * XPFW_CFG_PMU_SECURE_IMAGE_WDT_TIMEOUT
 * 		This watchdog timeout is applied during secure image download, provided
 * 		its value is greater than XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
 *
 */
#ifndef XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT
#define XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT			(90U) /* ms */
#endif

#ifndef XPFW_CFG_PMU_FPGA_WDT_TIMEOUT
#define XPFW_CFG_PMU_FPGA_WDT_TIMEOUT				(500U) /* ms */
#endif

#ifndef XPFW_CFG_PMU_SHA3_WDT_TIMEOUT
#define XPFW_CFG_PMU_SHA3_WDT_TIMEOUT				(500U) /* ms */
#endif

#ifndef XPFW_CFG_PMU_RSA_WDT_TIMEOUT
#define XPFW_CFG_PMU_RSA_WDT_TIMEOUT				(500U) /* ms */
#endif

#ifndef XPFW_CFG_PMU_AES_WDT_TIMEOUT
#define XPFW_CFG_PMU_AES_WDT_TIMEOUT				(500U) /* ms */
#endif

#ifndef XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT
#define XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT	(500U) /* ms */
#endif

#if (ENABLE_PM_VAL) && (!defined(ENABLE_PM))
#define ENABLE_PM
#endif

#if (ENABLE_EM_VAL) && (!defined(ENABLE_EM))
#define ENABLE_EM
#endif

#if (ENABLE_SCHEDULER_VAL) && (!defined(ENABLE_SCHEDULER))
#define ENABLE_SCHEDULER
#endif

#if (ENABLE_MOD_ULTRA96_VAL) && (!defined(ENABLE_MOD_ULTRA96))
#define ENABLE_MOD_ULTRA96
#endif

#if (ENABLE_RECOVERY_VAL) && (!defined(ENABLE_RECOVERY))
#define ENABLE_RECOVERY
#endif

#if (ENABLE_RECOVERY_RESET_SYSTEM_VAL) && \
	(!defined(ENABLE_RECOVERY_RESET_SYSTEM))
#define ENABLE_RECOVERY_RESET_SYSTEM
#endif

#if (ENABLE_RECOVERY_RESET_PS_ONLY_VAL) && \
	(!defined(ENABLE_RECOVERY_RESET_PS_ONLY))
#define ENABLE_RECOVERY_RESET_PS_ONLY
#endif

#if (ENABLE_ESCALATION_VAL) && (!defined(ENABLE_ESCALATION))
#define ENABLE_ESCALATION
#endif

#if (CHECK_HEALTHY_BOOT_VAL) && (!defined(CHECK_HEALTHY_BOOT))
#define CHECK_HEALTHY_BOOT
#endif

#if (ENABLE_WDT_VAL) && (!defined(ENABLE_WDT))
#define ENABLE_WDT
#endif

#if (ENABLE_CUSTOM_MOD_VAL) && (!defined(ENABLE_CUSTOM_MOD))
#define ENABLE_CUSTOM_MOD
#endif

#if (ENABLE_STL_VAL) && (!defined(ENABLE_STL))
#define ENABLE_STL
#endif

#if (ENABLE_RTC_TEST_VAL) && (!defined(ENABLE_RTC_TEST))
#define ENABLE_RTC_TEST
#endif

#if (ENABLE_RPU_RUN_MODE_VAL) && (!defined(ENABLE_RPU_RUN_MODE))
#define ENABLE_RPU_RUN_MODE
#endif

#if (ENABLE_FPGA_LOAD_VAL) && (!defined(ENABLE_FPGA_LOAD))
#define ENABLE_FPGA_LOAD
#endif

#if (ENABLE_FPGA_READ_CONFIG_DATA_VAL) && \
	(!defined(ENABLE_FPGA_READ_CONFIG_DATA))
#define ENABLE_FPGA_READ_CONFIG_DATA
#endif

#if (ENABLE_FPGA_READ_CONFIG_REG_VAL) && \
	(!defined(ENABLE_FPGA_READ_CONFIG_REG))
#define ENABLE_FPGA_READ_CONFIG_REG
#endif

#if (ENABLE_SECURE_VAL) && (!defined(ENABLE_SECURE))
#define ENABLE_SECURE
#endif

#if (XPU_INTR_DEBUG_PRINT_ENABLE_VAL) && \
	(!defined(XPU_INTR_DEBUG_PRINT_ENABLE))
#define XPU_INTR_DEBUG_PRINT_ENABLE
#endif

#if (PM_LOG_LEVEL_VAL > 0) && (!defined(PM_LOG_LEVEL))
#define PM_LOG_LEVEL	PM_LOG_LEVEL_VAL
#endif

#if (IDLE_PERIPHERALS_VAL) && (!defined(IDLE_PERIPHERALS))
#define IDLE_PERIPHERALS
#endif

#if (ENABLE_NODE_IDLING_VAL) && (!defined(ENABLE_NODE_IDLING))
#define ENABLE_NODE_IDLING
#endif

#if (DEBUG_MODE_VAL) && (!defined(DEBUG_MODE))
#define DEBUG_MODE
#endif

#ifdef XPAR_DDRCPSU_0_DEVICE_ID
#if (ENABLE_POS_VAL) && (!defined(ENABLE_POS))
#define ENABLE_POS
#endif
#else
#ifdef ENABLE_POS
#error "Error: POS feature is not supported in DDR less design"
#endif
#endif

#ifdef XPAR_DDRCPSU_0_DEVICE_ID
#if (ENABLE_DDR_SR_WR_VAL) && (!defined(ENABLE_DDR_SR_WR))
#define ENABLE_DDR_SR_WR
#endif
#else
#ifdef ENABLE_DDR_SR_WR
#error "Error: DDR_SR_WR feature is not supported in DDR less design"
#endif
#endif

#if (DISABLE_CLK_PERMS_VAL) && (!defined(DISABLE_CLK_PERMS))
#define DISABLE_CLK_PERMS
#endif

#if (ENABLE_UNUSED_RPU_PWR_DWN_VAL) && (!defined(ENABLE_UNUSED_RPU_PWR_DWN))
#define ENABLE_UNUSED_RPU_PWR_DWN
#endif

#if (PMU_MIO_INPUT_PIN_VAL) && (!defined(PMU_MIO_INPUT_PIN))
#define PMU_MIO_INPUT_PIN			0U
#endif

#if (BOARD_SHUTDOWN_PIN_VAL) && (!defined(BOARD_SHUTDOWN_PIN))
#define BOARD_SHUTDOWN_PIN			2U
#endif

#if (BOARD_SHUTDOWN_PIN_STATE_VAL) && (!defined(BOARD_SHUTDOWN_PIN_STATE))
#define BOARD_SHUTDOWN_PIN_STATE	0U
#endif

#if (SECURE_ACCESS_VAL) && (!defined(SECURE_ACCESS))
#define SECURE_ACCESS
#endif

#if (ENABLE_EFUSE_ACCESS) && (!defined(EFUSE_ACCESS))
#define EFUSE_ACCESS
#endif

/* FPD WDT recovery action */
#ifndef SWDT_EM_ACTION
#ifdef ENABLE_RECOVERY
#define SWDT_EM_ACTION EM_ACTION_CUSTOM
#else
#define SWDT_EM_ACTION EM_ACTION_SRST
#endif
#endif

#if defined(ENABLE_POS) && (!defined(ENABLE_POS_QSPI))
#define ENABLE_POS_QSPI
#endif

#if (CONNECT_PMU_GPO_2_VAL) && (!defined(CONNECT_PMU_GPO_2))
#define CONNECT_PMU_GPO_2
#endif

#if (CONNECT_PMU_GPO_3_VAL) && (!defined(CONNECT_PMU_GPO_3))
#define CONNECT_PMU_GPO_3
#endif

#if (CONNECT_PMU_GPO_4_VAL) && (!defined(CONNECT_PMU_GPO_4))
#define CONNECT_PMU_GPO_4
#endif

#if (CONNECT_PMU_GPO_5_VAL) && (!defined(CONNECT_PMU_GPO_5))
#define CONNECT_PMU_GPO_5
#endif

#ifdef XPAR_DDRCPSU_0_DEVICE_ID
#if (USE_DDR_FOR_APU_RESTART_VAL) && (!defined(USE_DDR_FOR_APU_RESTART))
#define USE_DDR_FOR_APU_RESTART
#endif
#endif

#if (ENABLE_IOCTL_VAL) && (!defined(ENABLE_IOCTL))
#define ENABLE_IOCTL
#endif

#if (ENABLE_RUNTIME_OVERTEMP_VAL) && (!defined(ENABLE_RUNTIME_OVERTEMP))
#define ENABLE_RUNTIME_OVERTEMP
#ifndef ENABLE_MOD_OVERTEMP
#define ENABLE_MOD_OVERTEMP
#endif
#ifndef ENABLE_EM
#define ENABLE_EM
#endif
#ifndef ENABLE_IOCTL
#define ENABLE_IOCTL
#endif
#endif

#if (ENABLE_RUNTIME_EXTWDT_VAL) && (!defined(ENABLE_RUNTIME_EXTWDT))
#define ENABLE_RUNTIME_EXTWDT
#ifndef ENABLE_MOD_EXTWDT
#define ENABLE_MOD_EXTWDT
#endif
#ifndef ENABLE_IOCTL
#define ENABLE_IOCTL
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPFW_CONFIG_H_ */

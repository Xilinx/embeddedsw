/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 * @file pm_defs.h
 *
 * PM Definitions implementation
 * @addtogroup xpm_apis XilPM APIs
 * @{
 *****************************************************************************/

#ifndef PM_DEFS_H_
#define PM_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name PM Version Number macros
 * @{
 */
/**
 * PM Version Number macros
 */
#define PM_VERSION_MAJOR	1
#define PM_VERSION_MINOR	1

#define PM_VERSION	((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)
/**@}*/

/**
 * @name Capabilities for RAM
 * @{
 */
/**
 * Capabilities for RAM
 */
#define PM_CAP_ACCESS	0x1U
#define PM_CAP_CONTEXT	0x2U
#define PM_CAP_WAKEUP	0x4U
/**@}*/

/**
 * @name Node default states macros
 * @{
 */
/**
 * Node State
 */
#define NODE_STATE_OFF		0
#define NODE_STATE_ON		1
/**@}*/

/**
 * @name Processor's states macros
 * @{
 */
/**
 * Processor's state
 */
#define PROC_STATE_FORCEDOFF	0
#define PROC_STATE_ACTIVE	1
#define PROC_STATE_SLEEP	2
#define PROC_STATE_SUSPENDING	3
/**@}*/

/**
 * @name Maximum Latency/QOS macros
 * @{
 */
/**
 * Maximum Latency/QOS
 */
#define MAX_LATENCY	(~0U)
#define MAX_QOS		100U
/**@}*/

/**
 * @name System shutdown/Restart macros
 * @{
 */
/**
 * System shutdown/Restart
 */
#define PMF_SHUTDOWN_TYPE_SHUTDOWN	0U
#define PMF_SHUTDOWN_TYPE_RESET		1U

#define PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM	0U
#define PMF_SHUTDOWN_SUBTYPE_PS_ONLY	1U
#define PMF_SHUTDOWN_SUBTYPE_SYSTEM	2U
/**@}*/

/** @cond INTERNAL */
#define PM_CLOCK_DIV0_ID	0U
#define PM_CLOCK_DIV1_ID	1U
/** @endcond */

/**
 * APIs for Miscellaneous functions, suspending of PUs, managing PM slaves and Direct control.
 */
enum XPmApiId {
	/* Miscellaneous API functions: */
	PM_GET_API_VERSION = 1,				/**< 0x1 */
	PM_SET_CONFIGURATION,				/**< 0x2 */
	PM_GET_NODE_STATUS,				/**< 0x3 */
	PM_GET_OP_CHARACTERISTIC,			/**< 0x4 */
	PM_REGISTER_NOTIFIER,				/**< 0x5 */
	/*  API for suspending of PUs: */
	PM_REQUEST_SUSPEND,				/**< 0x6 */
	PM_SELF_SUSPEND,				/**< 0x7 */
	PM_FORCE_POWERDOWN,				/**< 0x8 */
	PM_ABORT_SUSPEND,				/**< 0x9 */
	PM_REQUEST_WAKEUP,				/**< 0xA */
	PM_SET_WAKEUP_SOURCE,				/**< 0xB */
	PM_SYSTEM_SHUTDOWN,				/**< 0xC */
	/*API for managing PM slaves: */
	PM_REQUEST_NODE,				/**< 0xD */
	PM_RELEASE_NODE,				/**< 0xE */
	PM_SET_REQUIREMENT,				/**< 0xF */
	PM_SET_MAX_LATENCY,				/**< 0x10 */
	/* Direct control API functions: */
	PM_RESET_ASSERT,				/**< 0x11 */
	PM_RESET_GET_STATUS,				/**< 0x12 */
	PM_MMIO_WRITE,					/**< 0x13 */
	PM_MMIO_READ,					/**< 0x14 */
	PM_INIT_FINALIZE,				/**< 0x15 */
	PM_FPGA_LOAD,					/**< 0x16 */
	PM_FPGA_GET_STATUS,				/**< 0x17 */
	PM_GET_CHIPID,					/**< 0x18 */
	/* Secure library generic API functions */
	PM_SECURE_SHA = 26U,				/**< 0x1A */
	PM_SECURE_RSA,					/**< 0x1B */
	PM_PINCTRL_REQUEST,				/**< 0x1C */
	PM_PINCTRL_RELEASE,				/**< 0x1D */
	PM_PINCTRL_GET_FUNCTION,			/**< 0x1E */
	PM_PINCTRL_SET_FUNCTION,			/**< 0x1F */
	PM_PINCTRL_CONFIG_PARAM_GET,			/**< 0x20 */
	PM_PINCTRL_CONFIG_PARAM_SET,			/**< 0x21 */
	/* PM IOCTL API */
	PM_IOCTL,					/**< 0x22 */
	/* API to query information from firmware */
	PM_QUERY_DATA,					/**< 0x23 */
	/* Clock control API functions */
	PM_CLOCK_ENABLE,				/**< 0x24 */
	PM_CLOCK_DISABLE,				/**< 0x25 */
	PM_CLOCK_GETSTATE,				/**< 0x26 */
	PM_CLOCK_SETDIVIDER,				/**< 0x27 */
	PM_CLOCK_GETDIVIDER,				/**< 0x28 */
	PM_CLOCK_SETRATE,				/**< 0x29 */
	PM_CLOCK_GETRATE,				/**< 0x2A */
	PM_CLOCK_SETPARENT,				/**< 0x2B */
	PM_CLOCK_GETPARENT,				/**< 0x2C */
	/* Secure image */
	PM_SECURE_IMAGE,				/**< 0x2D */
	PM_FPGA_READ,					/**< 0x2E */
	PM_SECURE_AES,					/**< 0x2F */
	/* PLL direct control API functions */
	PM_PLL_SET_PARAMETER,				/**< 0x30 */
	PM_PLL_GET_PARAMETER,				/**< 0x31 */
	PM_PLL_SET_MODE,				/**< 0x32 */
	PM_PLL_GET_MODE,				/**< 0x33 */
	PM_REGISTER_ACCESS,				/**< 0x34 */
	PM_EFUSE_ACCESS,				/**< 0x35 */
	PM_API_MAX					/**< 0x36 */
};

/** @cond INTERNAL */
#define PM_API_MIN	PM_GET_API_VERSION
/** @endcond */

/**
 * PM API Callback ID
 */
enum XPmApiCbId {
	PM_INIT_SUSPEND_CB = 30,			/**< Suspend callback */
	PM_ACKNOWLEDGE_CB,				/**< Acknowledge callback */
	PM_NOTIFY_CB,					/**< Notify callback */
	PM_NOTIFY_STL_NO_OP				/**< Notify STL No OP */
};

/**
 * PM Node ID
 */
enum XPmNodeId {
	NODE_UNKNOWN,					/**< 0x0  */
	NODE_APU,					/**< 0x1  */
	NODE_APU_0,					/**< 0x2  */
	NODE_APU_1,					/**< 0x3  */
	NODE_APU_2,					/**< 0x4  */
	NODE_APU_3,					/**< 0x5  */
	NODE_RPU,					/**< 0x6  */
	NODE_RPU_0,					/**< 0x7  */
	NODE_RPU_1,					/**< 0x8  */
	NODE_PLD,					/**< 0x9  */
	NODE_FPD,					/**< 0xA  */
	NODE_OCM_BANK_0,				/**< 0xB  */
	NODE_OCM_BANK_1,				/**< 0xC  */
	NODE_OCM_BANK_2,				/**< 0xD  */
	NODE_OCM_BANK_3,				/**< 0xE  */
	NODE_TCM_0_A,					/**< 0xF  */
	NODE_TCM_0_B,					/**< 0x10 */
	NODE_TCM_1_A,					/**< 0x11 */
	NODE_TCM_1_B,					/**< 0x12 */
	NODE_L2,					/**< 0x13 */
	NODE_GPU_PP_0,					/**< 0x14 */
	NODE_GPU_PP_1,					/**< 0x15 */
	NODE_USB_0,					/**< 0x16 */
	NODE_USB_1,					/**< 0x17 */
	NODE_TTC_0,					/**< 0x18 */
	NODE_TTC_1,					/**< 0x19 */
	NODE_TTC_2,					/**< 0x1A */
	NODE_TTC_3,					/**< 0x1B */
	NODE_SATA,					/**< 0x1C */
	NODE_ETH_0,					/**< 0x1D */
	NODE_ETH_1,					/**< 0x1E */
	NODE_ETH_2,					/**< 0x1F */
	NODE_ETH_3,					/**< 0x20 */
	NODE_UART_0,					/**< 0x21 */
	NODE_UART_1,					/**< 0x22 */
	NODE_SPI_0,					/**< 0x23 */
	NODE_SPI_1,					/**< 0x24 */
	NODE_I2C_0,					/**< 0x25 */
	NODE_I2C_1,					/**< 0x26 */
	NODE_SD_0,					/**< 0x27 */
	NODE_SD_1,					/**< 0x28 */
	NODE_DP,					/**< 0x29 */
	NODE_GDMA,					/**< 0x2A */
	NODE_ADMA,					/**< 0x2B */
	NODE_NAND,					/**< 0x2C */
	NODE_QSPI,					/**< 0x2D */
	NODE_GPIO,					/**< 0x2E */
	NODE_CAN_0,					/**< 0x2F */
	NODE_CAN_1,					/**< 0x30 */
	NODE_EXTERN,					/**< 0x31 */
	NODE_APLL,					/**< 0x32 */
	NODE_VPLL,					/**< 0x33 */
	NODE_DPLL,					/**< 0x34 */
	NODE_RPLL,					/**< 0x35 */
	NODE_IOPLL,					/**< 0x36 */
	NODE_DDR,					/**< 0x37 */
	NODE_IPI_APU,					/**< 0x38 */
	NODE_IPI_RPU_0,					/**< 0x39 */
	NODE_GPU,					/**< 0x3A */
	NODE_PCIE,					/**< 0x3B */
	NODE_PCAP,					/**< 0x3C */
	NODE_RTC,					/**< 0x3D */
	NODE_LPD,					/**< 0x3E */
	NODE_VCU,					/**< 0x3F */
	NODE_IPI_RPU_1,					/**< 0x40 */
	NODE_IPI_PL_0,					/**< 0x41 */
	NODE_IPI_PL_1,					/**< 0x42 */
	NODE_IPI_PL_2,					/**< 0x43 */
	NODE_IPI_PL_3,					/**< 0x44 */
	NODE_PL,					/**< 0x45 */
	NODE_ID_MAX					/**< 0x46 */
};

/**
 * PM Acknowledge Request
 */
enum XPmRequestAck {
	REQUEST_ACK_NO = 1,				/**< No Ack */
	REQUEST_ACK_BLOCKING,				/**< Blocking Ack */
	REQUEST_ACK_NON_BLOCKING,			/**< Non blocking Ack */
	REQUEST_ACK_CB_CERROR,				/**< Callback Error */
};

/**
 * PM Abort Reasons
 */
enum XPmAbortReason {
	ABORT_REASON_WKUP_EVENT = 100,			/**< Wakeup Event */
	ABORT_REASON_PU_BUSY,				/**< Processor Busy */
	ABORT_REASON_NO_PWRDN,				/**< No Powerdown */
	ABORT_REASON_UNKNOWN,				/**< Unknown Reason */
};

/**
 * PM Suspend Reasons
 */
enum XPmSuspendReason {
	SUSPEND_REASON_PU_REQ = 201,			/**< Processor request */
	SUSPEND_REASON_ALERT,				/**< Alert */
	SUSPEND_REASON_SYS_SHUTDOWN,			/**< System shutdown */
};

/**
 * PM RAM States
 */
enum XPmRamState {
	PM_RAM_STATE_OFF = 0,				/**< Off */
	PM_RAM_STATE_RETENTION,				/**< Retention */
	PM_RAM_STATE_ON,				/**< On */
};

/**
 * PM Operating Characteristic
 */
enum XPmOpCharType {
	PM_OPCHAR_TYPE_POWER = 1,			/**< Operating characteristic ID power */
	PM_OPCHAR_TYPE_TEMP,				/**< Operating characteristic ID temperature */
	PM_OPCHAR_TYPE_LATENCY,				/**< Operating characteristic ID latency */
};

/**
 * @name Power management specific return error status
 * @defgroup pmstatmacro
 * @{
 */
/**
 * Power management specific return error status
 */
/** An internal error occurred while performing the requested operation */
#define XST_PM_INTERNAL  2000L
/** Conflicting requirements have been asserted when more than one processing
 * cluster is using the same PM slave */
#define XST_PM_CONFLICT  2001L
/** The processing cluster does not have access to the requested node or
 * operation */
#define XST_PM_NO_ACCESS  2002L
/** The API function does not apply to the node passed as argument */
#define XST_PM_INVALID_NODE  2003L
/**  A processing cluster has already been assigned access to a PM slave and
 * has issued a duplicate request for that PM slave */
#define XST_PM_DOUBLE_REQ  2004L
/** The target processing cluster has aborted suspend */
#define XST_PM_ABORT_SUSPEND  2005L
/** A timeout occurred while performing the requested operation*/
#define XST_PM_TIMEOUT   2006L
/**  Slave request cannot be granted since node is non-shareable and used */
#define XST_PM_NODE_USED  2007L
/**@}*/

/**
 * Boot Status
 */
enum XPmBootStatus {
	PM_INITIAL_BOOT,				/**< boot is a fresh system startup */
	PM_RESUME,					/**< boot is a resume */
	PM_BOOT_ERROR,					/**< error, boot cause cannot be identified */
};

/**
 * PM Reset Action types
 */
enum XPmResetAction {
	XILPM_RESET_ACTION_RELEASE,			/**< Reset action release */
	XILPM_RESET_ACTION_ASSERT,			/**< Reset action assert */
	XILPM_RESET_ACTION_PULSE,			/**< Reset action pulse */
};

/**
 * PM Reset Line IDs
 */
enum XPmReset {
	XILPM_RESET_PCIE_CFG = 1000,			/**< Reset ID PCIE_CFG */
	XILPM_RESET_PCIE_BRIDGE,			/**< Reset ID PCIE_BRIDGE */
	XILPM_RESET_PCIE_CTRL,				/**< Reset ID PCIE_CTRL */
	XILPM_RESET_DP,					/**< Reset ID DP */
	XILPM_RESET_SWDT_CRF,				/**< Reset ID SWDT_CRF */
	XILPM_RESET_AFI_FM5,				/**< Reset ID AFI_FM5 */
	XILPM_RESET_AFI_FM4,				/**< Reset ID AFI_FM4 */
	XILPM_RESET_AFI_FM3,				/**< Reset ID AFI_FM3 */
	XILPM_RESET_AFI_FM2,				/**< Reset ID AFI_FM2 */
	XILPM_RESET_AFI_FM1,				/**< Reset ID AFI_FM1 */
	XILPM_RESET_AFI_FM0,				/**< Reset ID AFI_FM0 */
	XILPM_RESET_GDMA,				/**< Reset ID GDMA */
	XILPM_RESET_GPU_PP1,				/**< Reset ID GPU_PP1 */
	XILPM_RESET_GPU_PP0,				/**< Reset ID GPU_PP0 */
	XILPM_RESET_GPU,				/**< Reset ID GPU */
	XILPM_RESET_GT,					/**< Reset ID GT */
	XILPM_RESET_SATA,				/**< Reset ID SATA */
	XILPM_RESET_ACPU3_PWRON,			/**< Reset ID ACPU3_PWRON */
	XILPM_RESET_ACPU2_PWRON,			/**< Reset ID ACPU2_PWRON */
	XILPM_RESET_ACPU1_PWRON,			/**< Reset ID ACPU1_PWRON */
	XILPM_RESET_ACPU0_PWRON,			/**< Reset ID ACPU0_PWRON */
	XILPM_RESET_APU_L2,				/**< Reset ID APU_L2 */
	XILPM_RESET_ACPU3,				/**< Reset ID ACPU3 */
	XILPM_RESET_ACPU2,				/**< Reset ID ACPU2 */
	XILPM_RESET_ACPU1,				/**< Reset ID ACPU1 */
	XILPM_RESET_ACPU0,				/**< Reset ID ACPU0 */
	XILPM_RESET_DDR,				/**< Reset ID DDR */
	XILPM_RESET_APM_FPD,				/**< Reset ID APM_FPD */
	XILPM_RESET_SOFT,				/**< Reset ID SOFT */
	XILPM_RESET_GEM0,				/**< Reset ID GEM0 */
	XILPM_RESET_GEM1,				/**< Reset ID GEM1 */
	XILPM_RESET_GEM2,				/**< Reset ID GEM2 */
	XILPM_RESET_GEM3,				/**< Reset ID GEM3 */
	XILPM_RESET_QSPI,				/**< Reset ID QSPI */
	XILPM_RESET_UART0,				/**< Reset ID UART0 */
	XILPM_RESET_UART1,				/**< Reset ID UART1 */
	XILPM_RESET_SPI0,				/**< Reset ID SPI0 */
	XILPM_RESET_SPI1,				/**< Reset ID SPI1 */
	XILPM_RESET_SDIO0,				/**< Reset ID SDIO0 */
	XILPM_RESET_SDIO1,				/**< Reset ID SDIO1 */
	XILPM_RESET_CAN0,				/**< Reset ID CAN0 */
	XILPM_RESET_CAN1,				/**< Reset ID CAN1 */
	XILPM_RESET_I2C0,				/**< Reset ID I2C0 */
	XILPM_RESET_I2C1,				/**< Reset ID I2C1 */
	XILPM_RESET_TTC0,				/**< Reset ID TTC0 */
	XILPM_RESET_TTC1,				/**< Reset ID TTC1 */
	XILPM_RESET_TTC2,				/**< Reset ID TTC2 */
	XILPM_RESET_TTC3,				/**< Reset ID TTC3 */
	XILPM_RESET_SWDT_CRL,				/**< Reset ID SWDT_CRL */
	XILPM_RESET_NAND,				/**< Reset ID NAND */
	XILPM_RESET_ADMA,				/**< Reset ID ADMA */
	XILPM_RESET_GPIO,				/**< Reset ID GPIO */
	XILPM_RESET_IOU_CC,				/**< Reset ID IOU_CC */
	XILPM_RESET_TIMESTAMP,				/**< Reset ID TIMESTAMP */
	XILPM_RESET_RPU_R50,				/**< Reset ID RPU_R50 */
	XILPM_RESET_RPU_R51,				/**< Reset ID RPU_R51 */
	XILPM_RESET_RPU_AMBA,				/**< Reset ID RPU_AMBA */
	XILPM_RESET_OCM,				/**< Reset ID OCM */
	XILPM_RESET_RPU_PGE,				/**< Reset ID RPU_PGE */
	XILPM_RESET_USB0_CORERESET,			/**< Reset ID USB0_CORERESE */
	XILPM_RESET_USB1_CORERESET,			/**< Reset ID USB1_CORERESE */
	XILPM_RESET_USB0_HIBERRESET,			/**< Reset ID USB0_HIBERRES */
	XILPM_RESET_USB1_HIBERRESET,			/**< Reset ID USB1_HIBERRES */
	XILPM_RESET_USB0_APB,				/**< Reset ID USB0_APB */
	XILPM_RESET_USB1_APB,				/**< Reset ID USB1_APB */
	XILPM_RESET_IPI,				/**< Reset ID IPI */
	XILPM_RESET_APM_LPD,				/**< Reset ID APM_LPD */
	XILPM_RESET_RTC,				/**< Reset ID RTC */
	XILPM_RESET_SYSMON,				/**< Reset ID SYSMON */
	XILPM_RESET_AFI_FM6,				/**< Reset ID AFI_FM6 */
	XILPM_RESET_LPD_SWDT,				/**< Reset ID LPD_SWDT */
	XILPM_RESET_FPD,				/**< Reset ID FPD */
	XILPM_RESET_RPU_DBG1,				/**< Reset ID RPU_DBG1 */
	XILPM_RESET_RPU_DBG0,				/**< Reset ID RPU_DBG0 */
	XILPM_RESET_DBG_LPD,				/**< Reset ID DBG_LPD */
	XILPM_RESET_DBG_FPD,				/**< Reset ID DBG_FPD */
	XILPM_RESET_APLL,				/**< Reset ID APLL */
	XILPM_RESET_DPLL,				/**< Reset ID DPLL */
	XILPM_RESET_VPLL,				/**< Reset ID VPLL */
	XILPM_RESET_IOPLL,				/**< Reset ID IOPLL */
	XILPM_RESET_RPLL,				/**< Reset ID RPLL */
	XILPM_RESET_GPO3_PL_0,				/**< Reset ID GPO3_PL_0 */
	XILPM_RESET_GPO3_PL_1,				/**< Reset ID GPO3_PL_1 */
	XILPM_RESET_GPO3_PL_2,				/**< Reset ID GPO3_PL_2 */
	XILPM_RESET_GPO3_PL_3,				/**< Reset ID GPO3_PL_3 */
	XILPM_RESET_GPO3_PL_4,				/**< Reset ID GPO3_PL_4 */
	XILPM_RESET_GPO3_PL_5,				/**< Reset ID GPO3_PL_5 */
	XILPM_RESET_GPO3_PL_6,				/**< Reset ID GPO3_PL_6 */
	XILPM_RESET_GPO3_PL_7,				/**< Reset ID GPO3_PL_7 */
	XILPM_RESET_GPO3_PL_8,				/**< Reset ID GPO3_PL_8 */
	XILPM_RESET_GPO3_PL_9,				/**< Reset ID GPO3_PL_9 */
	XILPM_RESET_GPO3_PL_10,				/**< Reset ID GPO3_PL_10 */
	XILPM_RESET_GPO3_PL_11,				/**< Reset ID GPO3_PL_11 */
	XILPM_RESET_GPO3_PL_12,				/**< Reset ID GPO3_PL_12 */
	XILPM_RESET_GPO3_PL_13,				/**< Reset ID GPO3_PL_13 */
	XILPM_RESET_GPO3_PL_14,				/**< Reset ID GPO3_PL_14 */
	XILPM_RESET_GPO3_PL_15,				/**< Reset ID GPO3_PL_15 */
	XILPM_RESET_GPO3_PL_16,				/**< Reset ID GPO3_PL_16 */
	XILPM_RESET_GPO3_PL_17,				/**< Reset ID GPO3_PL_17 */
	XILPM_RESET_GPO3_PL_18,				/**< Reset ID GPO3_PL_18 */
	XILPM_RESET_GPO3_PL_19,				/**< Reset ID GPO3_PL_19 */
	XILPM_RESET_GPO3_PL_20,				/**< Reset ID GPO3_PL_20 */
	XILPM_RESET_GPO3_PL_21,				/**< Reset ID GPO3_PL_21 */
	XILPM_RESET_GPO3_PL_22,				/**< Reset ID GPO3_PL_22 */
	XILPM_RESET_GPO3_PL_23,				/**< Reset ID GPO3_PL_23 */
	XILPM_RESET_GPO3_PL_24,				/**< Reset ID GPO3_PL_24 */
	XILPM_RESET_GPO3_PL_25,				/**< Reset ID GPO3_PL_25 */
	XILPM_RESET_GPO3_PL_26,				/**< Reset ID GPO3_PL_26 */
	XILPM_RESET_GPO3_PL_27,				/**< Reset ID GPO3_PL_27 */
	XILPM_RESET_GPO3_PL_28,				/**< Reset ID GPO3_PL_28 */
	XILPM_RESET_GPO3_PL_29,				/**< Reset ID GPO3_PL_29 */
	XILPM_RESET_GPO3_PL_30,				/**< Reset ID GPO3_PL_30 */
	XILPM_RESET_GPO3_PL_31,				/**< Reset ID GPO3_PL_31 */
	XILPM_RESET_RPU_LS,				/**< Reset ID RPU_LS */
	XILPM_RESET_PS_ONLY,				/**< Reset ID PS_ONLY */
	XILPM_RESET_PL,					/**< Reset ID PL */
	XILPM_RESET_GPIO5_EMIO_92,			/**< Reset ID GPIO5_EMIO_92 */
	XILPM_RESET_GPIO5_EMIO_93,			/**< Reset ID GPIO5_EMIO_93 */
	XILPM_RESET_GPIO5_EMIO_94,			/**< Reset ID GPIO5_EMIO_94 */
	XILPM_RESET_GPIO5_EMIO_95,			/**< Reset ID GPIO5_EMIO_95 */
};

/**
 * PM Notify Events Enum
 */
enum XPmNotifyEvent {
        EVENT_STATE_CHANGE = 1,				/**< State change event */
        EVENT_ZERO_USERS = 2,				/**< Zero user event */
};

/**
 * PM Clock IDs
 */
enum XPmClock {
	PM_CLOCK_IOPLL,					/**< Clock ID IOPLL */
	PM_CLOCK_RPLL,					/**< Clock ID RPLL */
	PM_CLOCK_APLL,					/**< Clock ID APLL */
	PM_CLOCK_DPLL,					/**< Clock ID DPLL */
	PM_CLOCK_VPLL,					/**< Clock ID VPLL */
	PM_CLOCK_IOPLL_TO_FPD,				/**< Clock ID IOPLL_TO_FPD */
	PM_CLOCK_RPLL_TO_FPD,				/**< Clock ID RPLL_TO_FPD */
	PM_CLOCK_APLL_TO_LPD,				/**< Clock ID APLL_TO_LPD */
	PM_CLOCK_DPLL_TO_LPD,				/**< Clock ID DPLL_TO_LPD */
	PM_CLOCK_VPLL_TO_LPD,				/**< Clock ID VPLL_TO_LPD */
	PM_CLOCK_ACPU,					/**< Clock ID ACPU */
	PM_CLOCK_ACPU_HALF,				/**< Clock ID ACPU_HALF */
	PM_CLOCK_DBG_FPD,				/**< Clock ID DBG_FPD */
	PM_CLOCK_DBG_LPD,				/**< Clock ID DBG_LPD */
	PM_CLOCK_DBG_TRACE,				/**< Clock ID DBG_TRACE */
	PM_CLOCK_DBG_TSTMP,				/**< Clock ID DBG_TSTMP */
	PM_CLOCK_DP_VIDEO_REF,				/**< Clock ID DP_VIDEO_REF */
	PM_CLOCK_DP_AUDIO_REF,				/**< Clock ID DP_AUDIO_REF */
	PM_CLOCK_DP_STC_REF,				/**< Clock ID DP_STC_REF */
	PM_CLOCK_GDMA_REF,				/**< Clock ID GDMA_REF */
	PM_CLOCK_DPDMA_REF,				/**< Clock ID DPDMA_REF */
	PM_CLOCK_DDR_REF,				/**< Clock ID DDR_REF */
	PM_CLOCK_SATA_REF,				/**< Clock ID SATA_REF */
	PM_CLOCK_PCIE_REF,				/**< Clock ID PCIE_REF */
	PM_CLOCK_GPU_REF,				/**< Clock ID GPU_REF */
	PM_CLOCK_GPU_PP0_REF,				/**< Clock ID GPU_PP0_REF */
	PM_CLOCK_GPU_PP1_REF,				/**< Clock ID GPU_PP1_REF */
	PM_CLOCK_TOPSW_MAIN,				/**< Clock ID TOPSW_MAIN */
	PM_CLOCK_TOPSW_LSBUS,				/**< Clock ID TOPSW_LSBUS */
	PM_CLOCK_GTGREF0_REF,				/**< Clock ID GTGREF0_REF */
	PM_CLOCK_LPD_SWITCH,				/**< Clock ID LPD_SWITCH */
	PM_CLOCK_LPD_LSBUS,				/**< Clock ID LPD_LSBUS */
	PM_CLOCK_USB0_BUS_REF,				/**< Clock ID USB0_BUS_REF */
	PM_CLOCK_USB1_BUS_REF,				/**< Clock ID USB1_BUS_REF */
	PM_CLOCK_USB3_DUAL_REF,				/**< Clock ID USB3_DUAL_REF */
	PM_CLOCK_USB0,					/**< Clock ID USB0 */
	PM_CLOCK_USB1,					/**< Clock ID USB1 */
	PM_CLOCK_CPU_R5,				/**< Clock ID CPU_R5 */
	PM_CLOCK_CPU_R5_CORE,				/**< Clock ID CPU_R5_CORE */
	PM_CLOCK_CSU_SPB,				/**< Clock ID CSU_SPB */
	PM_CLOCK_CSU_PLL,				/**< Clock ID CSU_PLL */
	PM_CLOCK_PCAP,					/**< Clock ID PCAP */
	PM_CLOCK_IOU_SWITCH,				/**< Clock ID IOU_SWITCH */
	PM_CLOCK_GEM_TSU_REF,				/**< Clock ID GEM_TSU_REF */
	PM_CLOCK_GEM_TSU,				/**< Clock ID GEM_TSU */
	PM_CLOCK_GEM0_TX,				/**< Clock ID GEM0_TX */
	PM_CLOCK_GEM1_TX,				/**< Clock ID GEM1_TX */
	PM_CLOCK_GEM2_TX,				/**< Clock ID GEM2_TX */
	PM_CLOCK_GEM3_TX,				/**< Clock ID GEM3_TX */
	PM_CLOCK_GEM0_RX,				/**< Clock ID GEM0_RX */
	PM_CLOCK_GEM1_RX,				/**< Clock ID GEM1_RX */
	PM_CLOCK_GEM2_RX,				/**< Clock ID GEM2_RX */
	PM_CLOCK_GEM3_RX,				/**< Clock ID GEM3_RX */
	PM_CLOCK_QSPI_REF,				/**< Clock ID QSPI_REF */
	PM_CLOCK_SDIO0_REF,				/**< Clock ID SDIO0_REF */
	PM_CLOCK_SDIO1_REF,				/**< Clock ID SDIO1_REF */
	PM_CLOCK_UART0_REF,				/**< Clock ID UART0_REF */
	PM_CLOCK_UART1_REF,				/**< Clock ID UART1_REF */
	PM_CLOCK_SPI0_REF,				/**< Clock ID SPI0_REF */
	PM_CLOCK_SPI1_REF,				/**< Clock ID SPI1_REF */
	PM_CLOCK_NAND_REF,				/**< Clock ID NAND_REF */
	PM_CLOCK_I2C0_REF,				/**< Clock ID I2C0_REF */
	PM_CLOCK_I2C1_REF,				/**< Clock ID I2C1_REF */
	PM_CLOCK_CAN0_REF,				/**< Clock ID CAN0_REF */
	PM_CLOCK_CAN1_REF,				/**< Clock ID CAN1_REF */
	PM_CLOCK_CAN0,					/**< Clock ID CAN0 */
	PM_CLOCK_CAN1,					/**< Clock ID CAN1 */
	PM_CLOCK_DLL_REF,				/**< Clock ID DLL_REF */
	PM_CLOCK_ADMA_REF,				/**< Clock ID ADMA_REF */
	PM_CLOCK_TIMESTAMP_REF,				/**< Clock ID TIMESTAMP_REF */
	PM_CLOCK_AMS_REF,				/**< Clock ID AMS_REF */
	PM_CLOCK_PL0_REF,				/**< Clock ID PL0_REF */
	PM_CLOCK_PL1_REF,				/**< Clock ID PL1_REF */
	PM_CLOCK_PL2_REF,				/**< Clock ID PL2_REF */
	PM_CLOCK_PL3_REF,				/**< Clock ID PL3_REF */
	PM_CLOCK_WDT,					/**< Clock ID WDT */
	PM_CLOCK_IOPLL_INT,				/**< Clock ID IOPLL_INT */
	PM_CLOCK_IOPLL_PRE_SRC,				/**< Clock ID IOPLL_PRE_SRC */
	PM_CLOCK_IOPLL_HALF,				/**< Clock ID IOPLL_HALF */
	PM_CLOCK_IOPLL_INT_MUX,				/**< Clock ID IOPLL_INT_MUX */
	PM_CLOCK_IOPLL_POST_SRC,			/**< Clock ID IOPLL_POST_SRC */
	PM_CLOCK_RPLL_INT,				/**< Clock ID RPLL_INT */
	PM_CLOCK_RPLL_PRE_SRC,				/**< Clock ID RPLL_PRE_SRC */
	PM_CLOCK_RPLL_HALF,				/**< Clock ID RPLL_HALF */
	PM_CLOCK_RPLL_INT_MUX,				/**< Clock ID RPLL_INT_MUX */
	PM_CLOCK_RPLL_POST_SRC,				/**< Clock ID RPLL_POST_SRC */
	PM_CLOCK_APLL_INT,				/**< Clock ID APLL_INT */
	PM_CLOCK_APLL_PRE_SRC,				/**< Clock ID APLL_PRE_SRC */
	PM_CLOCK_APLL_HALF,				/**< Clock ID APLL_HALF */
	PM_CLOCK_APLL_INT_MUX,				/**< Clock ID APLL_INT_MUX */
	PM_CLOCK_APLL_POST_SRC,				/**< Clock ID APLL_POST_SRC */
	PM_CLOCK_DPLL_INT,				/**< Clock ID DPLL_INT */
	PM_CLOCK_DPLL_PRE_SRC,				/**< Clock ID DPLL_PRE_SRC */
	PM_CLOCK_DPLL_HALF,				/**< Clock ID DPLL_HALF */
	PM_CLOCK_DPLL_INT_MUX,				/**< Clock ID DPLL_INT_MUX */
	PM_CLOCK_DPLL_POST_SRC,				/**< Clock ID DPLL_POST_SRC */
	PM_CLOCK_VPLL_INT,				/**< Clock ID VPLL_INT */
	PM_CLOCK_VPLL_PRE_SRC,				/**< Clock ID VPLL_PRE_SRC */
	PM_CLOCK_VPLL_HALF,				/**< Clock ID VPLL_HALF */
	PM_CLOCK_VPLL_INT_MUX,				/**< Clock ID VPLL_INT_MUX */
	PM_CLOCK_VPLL_POST_SRC,				/**< Clock ID VPLL_POST_SRC */
	PM_CLOCK_CAN0_MIO,				/**< Clock ID CAN0_MIO */
	PM_CLOCK_CAN1_MIO,				/**< Clock ID CAN1_MIO */
	PM_CLOCK_ACPU_FULL,				/**< Clock ID ACPU_FULL */
	PM_CLOCK_GEM0_REF,				/**< Clock ID GEM0_REF */
	PM_CLOCK_GEM1_REF,				/**< Clock ID GEM1_REF */
	PM_CLOCK_GEM2_REF,				/**< Clock ID GEM2_REF */
	PM_CLOCK_GEM3_REF,				/**< Clock ID GEM3_REF */
	PM_CLOCK_GEM0_REF_UNGATED,			/**< Clock ID GEM0_REF_UNGATED */
	PM_CLOCK_GEM1_REF_UNGATED,			/**< Clock ID GEM1_REF_UNGATED */
	PM_CLOCK_GEM2_REF_UNGATED,			/**< Clock ID GEM2_REF_UNGATED */
	PM_CLOCK_GEM3_REF_UNGATED,			/**< Clock ID GEM3_REF_UNGATED */
	PM_CLOCK_EXT_PSS_REF,				/**< Clock ID EXT_PSS_REF */
	PM_CLOCK_EXT_VIDEO,				/**< Clock ID EXT_VIDEO */
	PM_CLOCK_EXT_PSS_ALT_REF,			/**< Clock ID EXT_PSS_ALT_REF */
	PM_CLOCK_EXT_AUX_REF,				/**< Clock ID EXT_AUX_REF */
	PM_CLOCK_EXT_GT_CRX_REF,			/**< Clock ID EXT_GT_CRX_REF */
	PM_CLOCK_EXT_SWDT0,				/**< Clock ID EXT_SWDT0 */
	PM_CLOCK_EXT_SWDT1,				/**< Clock ID EXT_SWDT1 */
	PM_CLOCK_EXT_GEM0_TX_EMIO,			/**< Clock ID EXT_GEM0_TX_EMIO */
	PM_CLOCK_EXT_GEM1_TX_EMIO,			/**< Clock ID EXT_GEM1_TX_EMIO */
	PM_CLOCK_EXT_GEM2_TX_EMIO,			/**< Clock ID EXT_GEM2_TX_EMIO */
	PM_CLOCK_EXT_GEM3_TX_EMIO,			/**< Clock ID EXT_GEM3_TX_EMIO */
	PM_CLOCK_EXT_GEM0_RX_EMIO,			/**< Clock ID EXT_GEM0_RX_EMIO */
	PM_CLOCK_EXT_GEM1_RX_EMIO,			/**< Clock ID EXT_GEM1_RX_EMIO */
	PM_CLOCK_EXT_GEM2_RX_EMIO,			/**< Clock ID EXT_GEM2_RX_EMIO */
	PM_CLOCK_EXT_GEM3_RX_EMIO,			/**< Clock ID EXT_GEM3_RX_EMIO */
	PM_CLOCK_EXT_MIO50_OR_MIO51,			/**< Clock ID EXT_MIO50_OR_MIO51 */
	PM_CLOCK_EXT_MIO0,				/**< Clock ID EXT_MIO0 */
	PM_CLOCK_EXT_MIO1,				/**< Clock ID EXT_MIO1 */
	PM_CLOCK_EXT_MIO2,				/**< Clock ID EXT_MIO2 */
	PM_CLOCK_EXT_MIO3,				/**< Clock ID EXT_MIO3 */
	PM_CLOCK_EXT_MIO4,				/**< Clock ID EXT_MIO4 */
	PM_CLOCK_EXT_MIO5,				/**< Clock ID EXT_MIO5 */
	PM_CLOCK_EXT_MIO6,				/**< Clock ID EXT_MIO6 */
	PM_CLOCK_EXT_MIO7,				/**< Clock ID EXT_MIO7 */
	PM_CLOCK_EXT_MIO8,				/**< Clock ID EXT_MIO8 */
	PM_CLOCK_EXT_MIO9,				/**< Clock ID EXT_MIO9 */
	PM_CLOCK_EXT_MIO10,				/**< Clock ID EXT_MIO10 */
	PM_CLOCK_EXT_MIO11,				/**< Clock ID EXT_MIO11 */
	PM_CLOCK_EXT_MIO12,				/**< Clock ID EXT_MIO12 */
	PM_CLOCK_EXT_MIO13,				/**< Clock ID EXT_MIO13 */
	PM_CLOCK_EXT_MIO14,				/**< Clock ID EXT_MIO14 */
	PM_CLOCK_EXT_MIO15,				/**< Clock ID EXT_MIO15 */
	PM_CLOCK_EXT_MIO16,				/**< Clock ID EXT_MIO16 */
	PM_CLOCK_EXT_MIO17,				/**< Clock ID EXT_MIO17 */
	PM_CLOCK_EXT_MIO18,				/**< Clock ID EXT_MIO18 */
	PM_CLOCK_EXT_MIO19,				/**< Clock ID EXT_MIO19 */
	PM_CLOCK_EXT_MIO20,				/**< Clock ID EXT_MIO20 */
	PM_CLOCK_EXT_MIO21,				/**< Clock ID EXT_MIO21 */
	PM_CLOCK_EXT_MIO22,				/**< Clock ID EXT_MIO22 */
	PM_CLOCK_EXT_MIO23,				/**< Clock ID EXT_MIO23 */
	PM_CLOCK_EXT_MIO24,				/**< Clock ID EXT_MIO24 */
	PM_CLOCK_EXT_MIO25,				/**< Clock ID EXT_MIO25 */
	PM_CLOCK_EXT_MIO26,				/**< Clock ID EXT_MIO26 */
	PM_CLOCK_EXT_MIO27,				/**< Clock ID EXT_MIO27 */
	PM_CLOCK_EXT_MIO28,				/**< Clock ID EXT_MIO28 */
	PM_CLOCK_EXT_MIO29,				/**< Clock ID EXT_MIO29 */
	PM_CLOCK_EXT_MIO30,				/**< Clock ID EXT_MIO30 */
	PM_CLOCK_EXT_MIO31,				/**< Clock ID EXT_MIO31 */
	PM_CLOCK_EXT_MIO32,				/**< Clock ID EXT_MIO32 */
	PM_CLOCK_EXT_MIO33,				/**< Clock ID EXT_MIO33 */
	PM_CLOCK_EXT_MIO34,				/**< Clock ID EXT_MIO34 */
	PM_CLOCK_EXT_MIO35,				/**< Clock ID EXT_MIO35 */
	PM_CLOCK_EXT_MIO36,				/**< Clock ID EXT_MIO36 */
	PM_CLOCK_EXT_MIO37,				/**< Clock ID EXT_MIO37 */
	PM_CLOCK_EXT_MIO38,				/**< Clock ID EXT_MIO38 */
	PM_CLOCK_EXT_MIO39,				/**< Clock ID EXT_MIO39 */
	PM_CLOCK_EXT_MIO40,				/**< Clock ID EXT_MIO40 */
	PM_CLOCK_EXT_MIO41,				/**< Clock ID EXT_MIO41 */
	PM_CLOCK_EXT_MIO42,				/**< Clock ID EXT_MIO42 */
	PM_CLOCK_EXT_MIO43,				/**< Clock ID EXT_MIO43 */
	PM_CLOCK_EXT_MIO44,				/**< Clock ID EXT_MIO44 */
	PM_CLOCK_EXT_MIO45,				/**< Clock ID EXT_MIO45 */
	PM_CLOCK_EXT_MIO46,				/**< Clock ID EXT_MIO46 */
	PM_CLOCK_EXT_MIO47,				/**< Clock ID EXT_MIO47 */
	PM_CLOCK_EXT_MIO48,				/**< Clock ID EXT_MIO48 */
	PM_CLOCK_EXT_MIO49,				/**< Clock ID EXT_MIO49 */
	PM_CLOCK_EXT_MIO50,				/**< Clock ID EXT_MIO50 */
	PM_CLOCK_EXT_MIO51,				/**< Clock ID EXT_MIO51 */
	PM_CLOCK_EXT_MIO52,				/**< Clock ID EXT_MIO52 */
	PM_CLOCK_EXT_MIO53,				/**< Clock ID EXT_MIO53 */
	PM_CLOCK_EXT_MIO54,				/**< Clock ID EXT_MIO54 */
	PM_CLOCK_EXT_MIO55,				/**< Clock ID EXT_MIO55 */
	PM_CLOCK_EXT_MIO56,				/**< Clock ID EXT_MIO56 */
	PM_CLOCK_EXT_MIO57,				/**< Clock ID EXT_MIO57 */
	PM_CLOCK_EXT_MIO58,				/**< Clock ID EXT_MIO58 */
	PM_CLOCK_EXT_MIO59,				/**< Clock ID EXT_MIO59 */
	PM_CLOCK_EXT_MIO60,				/**< Clock ID EXT_MIO60 */
	PM_CLOCK_EXT_MIO61,				/**< Clock ID EXT_MIO61 */
	PM_CLOCK_EXT_MIO62,				/**< Clock ID EXT_MIO62 */
	PM_CLOCK_EXT_MIO63,				/**< Clock ID EXT_MIO63 */
	PM_CLOCK_EXT_MIO64,				/**< Clock ID EXT_MIO64 */
	PM_CLOCK_EXT_MIO65,				/**< Clock ID EXT_MIO65 */
	PM_CLOCK_EXT_MIO66,				/**< Clock ID EXT_MIO66 */
	PM_CLOCK_EXT_MIO67,				/**< Clock ID EXT_MIO67 */
	PM_CLOCK_EXT_MIO68,				/**< Clock ID EXT_MIO68 */
	PM_CLOCK_EXT_MIO69,				/**< Clock ID EXT_MIO69 */
	PM_CLOCK_EXT_MIO70,				/**< Clock ID EXT_MIO70 */
	PM_CLOCK_EXT_MIO71,				/**< Clock ID EXT_MIO71 */
	PM_CLOCK_EXT_MIO72,				/**< Clock ID EXT_MIO72 */
	PM_CLOCK_EXT_MIO73,				/**< Clock ID EXT_MIO73 */
	PM_CLOCK_EXT_MIO74,				/**< Clock ID EXT_MIO74 */
	PM_CLOCK_EXT_MIO75,				/**< Clock ID EXT_MIO75 */
	PM_CLOCK_EXT_MIO76,				/**< Clock ID EXT_MIO76 */
	PM_CLOCK_EXT_MIO77,				/**< Clock ID EXT_MIO77 */
};

/**
 * PLL parameters
 */
enum XPmPllParam {
	PM_PLL_PARAM_ID_DIV2,				/**< PLL param ID DIV2 */
	PM_PLL_PARAM_ID_FBDIV,				/**< PLL param ID FBDIV */
	PM_PLL_PARAM_ID_DATA,				/**< PLL param ID DATA */
	PM_PLL_PARAM_ID_PRE_SRC,			/**< PLL param ID PRE_SRC */
	PM_PLL_PARAM_ID_POST_SRC,			/**< PLL param ID POST_SRC */
	PM_PLL_PARAM_ID_LOCK_DLY,			/**< PLL param ID LOCK_DLY */
	PM_PLL_PARAM_ID_LOCK_CNT,			/**< PLL param ID LOCK_CNT */
	PM_PLL_PARAM_ID_LFHF,				/**< PLL param ID LFHF */
	PM_PLL_PARAM_ID_CP,				/**< PLL param ID CP */
	PM_PLL_PARAM_ID_RES,				/**< PLL param ID RES */
};

/**
 * PLL Modes
 */
enum XPmPllMode {
	PM_PLL_MODE_RESET,				/**< PLL mode reset */
	PM_PLL_MODE_INTEGER,				/**< PLL mode integer */
	PM_PLL_MODE_FRACTIONAL,				/**< PLL mode fractional */
};

/**
 * Pin Function IDs
 */
enum XPmPinFn {
	PINCTRL_FUNC_CAN0,				/**< Pin Function CAN0 */
	PINCTRL_FUNC_CAN1,				/**< Pin Function CAN1 */
	PINCTRL_FUNC_ETHERNET0,				/**< Pin Function ETHERNET0 */
	PINCTRL_FUNC_ETHERNET1,				/**< Pin Function ETHERNET1 */
	PINCTRL_FUNC_ETHERNET2,				/**< Pin Function ETHERNET2 */
	PINCTRL_FUNC_ETHERNET3,				/**< Pin Function ETHERNET3 */
	PINCTRL_FUNC_GEMTSU0,				/**< Pin Function GEMTSU0 */
	PINCTRL_FUNC_GPIO0,				/**< Pin Function GPIO0 */
	PINCTRL_FUNC_I2C0,				/**< Pin Function I2C0 */
	PINCTRL_FUNC_I2C1,				/**< Pin Function I2C1 */
	PINCTRL_FUNC_MDIO0,				/**< Pin Function MDIO0 */
	PINCTRL_FUNC_MDIO1,				/**< Pin Function MDIO1 */
	PINCTRL_FUNC_MDIO2,				/**< Pin Function MDIO2 */
	PINCTRL_FUNC_MDIO3,				/**< Pin Function MDIO3 */
	PINCTRL_FUNC_QSPI0,				/**< Pin Function QSPI0 */
	PINCTRL_FUNC_QSPI_FBCLK,			/**< Pin Function QSPI_FBCLK */
	PINCTRL_FUNC_QSPI_SS,				/**< Pin Function QSPI_SS */
	PINCTRL_FUNC_SPI0,				/**< Pin Function SPI0 */
	PINCTRL_FUNC_SPI1,				/**< Pin Function SPI1 */
	PINCTRL_FUNC_SPI0_SS,				/**< Pin Function SPI0_SS */
	PINCTRL_FUNC_SPI1_SS,				/**< Pin Function SPI1_SS */
	PINCTRL_FUNC_SDIO0,				/**< Pin Function SDIO0 */
	PINCTRL_FUNC_SDIO0_PC,				/**< Pin Function SDIO0_PC */
	PINCTRL_FUNC_SDIO0_CD,				/**< Pin Function SDIO0_CD */
	PINCTRL_FUNC_SDIO0_WP,				/**< Pin Function SDIO0_WP */
	PINCTRL_FUNC_SDIO1,				/**< Pin Function SDIO1 */
	PINCTRL_FUNC_SDIO1_PC,				/**< Pin Function SDIO1_PC */
	PINCTRL_FUNC_SDIO1_CD,				/**< Pin Function SDIO1_CD */
	PINCTRL_FUNC_SDIO1_WP,				/**< Pin Function SDIO1_WP */
	PINCTRL_FUNC_NAND0,				/**< Pin Function NAND0 */
	PINCTRL_FUNC_NAND0_CE,				/**< Pin Function NAND0_CE */
	PINCTRL_FUNC_NAND0_RB,				/**< Pin Function NAND0_RB */
	PINCTRL_FUNC_NAND0_DQS,				/**< Pin Function NAND0_DQS */
	PINCTRL_FUNC_TTC0_CLK,				/**< Pin Function TTC0_CLK */
	PINCTRL_FUNC_TTC0_WAV,				/**< Pin Function TTC0_WAV */
	PINCTRL_FUNC_TTC1_CLK,				/**< Pin Function TTC1_CLK */
	PINCTRL_FUNC_TTC1_WAV,				/**< Pin Function TTC1_WAV */
	PINCTRL_FUNC_TTC2_CLK,				/**< Pin Function TTC2_CLK */
	PINCTRL_FUNC_TTC2_WAV,				/**< Pin Function TTC2_WAV */
	PINCTRL_FUNC_TTC3_CLK,				/**< Pin Function TTC3_CLK */
	PINCTRL_FUNC_TTC3_WAV,				/**< Pin Function TTC3_WAV */
	PINCTRL_FUNC_UART0,				/**< Pin Function UART0 */
	PINCTRL_FUNC_UART1,				/**< Pin Function UART1 */
	PINCTRL_FUNC_USB0,				/**< Pin Function USB0 */
	PINCTRL_FUNC_USB1,				/**< Pin Function USB1 */
	PINCTRL_FUNC_SWDT0_CLK,				/**< Pin Function SWDT0_CLK */
	PINCTRL_FUNC_SWDT0_RST,				/**< Pin Function SWDT0_RST */
	PINCTRL_FUNC_SWDT1_CLK,				/**< Pin Function SWDT1_CLK */
	PINCTRL_FUNC_SWDT1_RST,				/**< Pin Function SWDT1_RST */
	PINCTRL_FUNC_PMU0,				/**< Pin Function PMU0 */
	PINCTRL_FUNC_PCIE0,				/**< Pin Function PCIE0 */
	PINCTRL_FUNC_CSU0,				/**< Pin Function CSU0 */
	PINCTRL_FUNC_DPAUX0,				/**< Pin Function DPAUX0 */
	PINCTRL_FUNC_PJTAG0,				/**< Pin Function PJTAG0 */
	PINCTRL_FUNC_TRACE0,				/**< Pin Function TRACE0 */
	PINCTRL_FUNC_TRACE0_CLK,			/**< Pin Function TRACE0_CLK */
	PINCTRL_FUNC_TESTSCAN0,				/**< Pin Function TESTSCAN0 */
};

/**
 * PIN Control Parameters
 */
enum XPmPinParam {
	PINCTRL_CONFIG_SLEW_RATE,			/**< Pin config slew rate */
	PINCTRL_CONFIG_BIAS_STATUS,			/**< Pin config bias status */
	PINCTRL_CONFIG_PULL_CTRL,			/**< Pin config pull control */
	PINCTRL_CONFIG_SCHMITT_CMOS,			/**< Pin config schmitt CMOS */
	PINCTRL_CONFIG_DRIVE_STRENGTH,			/**< Pin config drive strength */
	PINCTRL_CONFIG_VOLTAGE_STATUS,			/**< Pin config voltage status */
};

/**
 * IOCTL IDs
 */
typedef enum {
	IOCTL_GET_RPU_OPER_MODE,			/**< Get RPU mode */
	IOCTL_SET_RPU_OPER_MODE,			/**< Set RPU mode */
	IOCTL_RPU_BOOT_ADDR_CONFIG,			/**< RPU boot address config */
	IOCTL_TCM_COMB_CONFIG,				/**< TCM config */
	IOCTL_SET_TAPDELAY_BYPASS,			/**< TAP delay bypass */
	IOCTL_SET_SGMII_MODE,				/**< SGMII mode */
	IOCTL_SD_DLL_RESET,				/**< SD DLL reset */
	IOCTL_SET_SD_TAPDELAY,				/**< SD TAP delay */
	/* Ioctl for clock driver */
	IOCTL_SET_PLL_FRAC_MODE,			/**< Set PLL frac mode */
	IOCTL_GET_PLL_FRAC_MODE,			/**< Get PLL frac mode */
	IOCTL_SET_PLL_FRAC_DATA,			/**< Set PLL frac data */
	IOCTL_GET_PLL_FRAC_DATA,			/**< Get PLL frac data */
	IOCTL_WRITE_GGS,				/**< Write GGS */
	IOCTL_READ_GGS,					/**< Read GGS */
	IOCTL_WRITE_PGGS,				/**< Write PGGS */
	IOCTL_READ_PGGS,				/**< Read PGGS */
	/* IOCTL for ULPI reset */
	IOCTL_ULPI_RESET,				/**< ULPI reset */
	/* Set healthy bit value */
	IOCTL_SET_BOOT_HEALTH_STATUS,			/**< Set boot status */
	IOCTL_AFI,					/**< AFI */
	/* Probe counter read/write */
	IOCTL_PROBE_COUNTER_READ,			/**< Probe counter read */
	IOCTL_PROBE_COUNTER_WRITE,			/**< Probe counter write */
	/* Ospi mux select */
	IOCTL_OSPI_MUX_SELECT,				/**< OSPI mux select */
	/* USB PMU state req */
	IOCTL_USB_SET_STATE,				/**< USB set state */
	IOCTL_GET_LAST_RESET_REASON,			/**< Get last reset reason */
	/* AIE ISR Clear */
	IOCTL_AIE_ISR_CLEAR,				/**< AIE ISR clear */
	/* Register SGI to ATF */
	IOCTL_REGISTER_SGI,				/**< Register SGI to ATF */
	/* Runtime feature configuration */
	IOCTL_SET_FEATURE_CONFIG,			/**< Set feature config */
	IOCTL_GET_FEATURE_CONFIG,			/**< Get feature config */
} pm_ioctl_id;

/**
 * IOCTL IDs
 */
typedef enum {
	XPM_FEATURE_INVALID,			/**< Invalid ID */
	XPM_FEATURE_OVERTEMP_STATUS,		/**< Over temperature status */
	XPM_FEATURE_OVERTEMP_VALUE,		/**< Over temperature limit */
	XPM_FEATURE_EXTWDT_STATUS,		/**< External watchdog status */
	XPM_FEATURE_EXTWDT_VALUE,		/**< External watchdog interval */
}pm_feature_id;

#ifdef __cplusplus
}
#endif

 /** @} */
#endif /* PM_DEFS_H_ */

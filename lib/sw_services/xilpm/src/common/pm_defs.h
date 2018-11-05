/******************************************************************************
*
* Copyright (C) 2015-2016 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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

/** @name PM Version Number macros
 *
 * @{
 */
#define PM_VERSION_MAJOR	1
#define PM_VERSION_MINOR	0

#define PM_VERSION	((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)
/*@}*/

/** @name Capabilities for RAM
 *
 * @{
 */
#define PM_CAP_ACCESS	0x1U
#define PM_CAP_CONTEXT	0x2U
#define PM_CAP_WAKEUP	0x4U
/*@}*/

/** @name Node default states macros
 *
 * @{
 */
#define NODE_STATE_OFF		0
#define NODE_STATE_ON		1
/*@}*/

/** @name Processor's states macros
 *
 * @{
 */
#define PROC_STATE_FORCEDOFF	0
#define PROC_STATE_ACTIVE	1
#define PROC_STATE_SLEEP	2
#define PROC_STATE_SUSPENDING	3
/*@}*/

/** @name Maximum Latency/QOS macros
 *
 * @{
 */
#define MAX_LATENCY	(~0U)
#define MAX_QOS		100U
/*@}*/

/** @name System shutdown/Restart macros
 *
 * @{
 */
#define PMF_SHUTDOWN_TYPE_SHUTDOWN	0U
#define PMF_SHUTDOWN_TYPE_RESET		1U

#define PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM	0U
#define PMF_SHUTDOWN_SUBTYPE_PS_ONLY	1U
#define PMF_SHUTDOWN_SUBTYPE_SYSTEM	2U
/*@}*/

/**
 *  @name APIs for Miscellaneous functions, suspending of PUs, managing PM slaves and Direct control.
 */
enum XPmApiId {
	/** Miscellaneous API functions: */
	PM_GET_API_VERSION = 1, /**< Do not change or move */
	PM_SET_CONFIGURATION,
	PM_GET_NODE_STATUS,
	PM_GET_OP_CHARACTERISTIC,
	PM_REGISTER_NOTIFIER,
	/** API for suspending of PUs: */
	PM_REQUEST_SUSPEND,
	PM_SELF_SUSPEND,
	PM_FORCE_POWERDOWN,
	PM_ABORT_SUSPEND,
	PM_REQUEST_WAKEUP,
	PM_SET_WAKEUP_SOURCE,
	PM_SYSTEM_SHUTDOWN,
	/** API for managing PM slaves: */
	PM_REQUEST_NODE,
	PM_RELEASE_NODE,
	PM_SET_REQUIREMENT,
	PM_SET_MAX_LATENCY,
	/** Direct control API functions: */
	PM_RESET_ASSERT,
	PM_RESET_GET_STATUS,
	PM_MMIO_WRITE,
	PM_MMIO_READ,
	PM_INIT_FINALIZE,
	PM_FPGA_LOAD,
	PM_FPGA_GET_STATUS,
	PM_GET_CHIPID,
	/* Secure library generic API functions */
	PM_SECURE_RSA_AES,
	PM_SECURE_SHA,
	PM_SECURE_RSA,
	PM_PINCTRL_REQUEST,
	PM_PINCTRL_RELEASE,
	PM_PINCTRL_GET_FUNCTION,
	PM_PINCTRL_SET_FUNCTION,
	PM_PINCTRL_CONFIG_PARAM_GET,
	PM_PINCTRL_CONFIG_PARAM_SET,
	/* PM IOCTL API */
	PM_IOCTL,
	/* API to query information from firmware */
	PM_QUERY_DATA,
	/* Clock control API functions */
	PM_CLOCK_ENABLE,
	PM_CLOCK_DISABLE,
	PM_CLOCK_GETSTATE,
	PM_CLOCK_SETDIVIDER,
	PM_CLOCK_GETDIVIDER,
	PM_CLOCK_SETRATE,
	PM_CLOCK_GETRATE,
	PM_CLOCK_SETPARENT,
	PM_CLOCK_GETPARENT,
	/* Secure image */
	PM_SECURE_IMAGE,
	PM_API_MAX
};

/** @name PM API Min and Max macros
 *
 * @{
 */
#define PM_API_MIN	PM_GET_API_VERSION
/*@}*/

/**
 *  @name PM API Callback Id Enum
 */
enum XPmApiCbId {
	PM_INIT_SUSPEND_CB = 30,
	PM_ACKNOWLEDGE_CB,
	PM_NOTIFY_CB,
};

/**
 *  @name PM Node ID Enum
 */
enum XPmNodeId {
	NODE_UNKNOWN,
	NODE_APU,
	NODE_APU_0,
	NODE_APU_1,
	NODE_APU_2,
	NODE_APU_3,
	NODE_RPU,
	NODE_RPU_0,
	NODE_RPU_1,
	NODE_PLD,
	NODE_FPD,
	NODE_OCM_BANK_0,
	NODE_OCM_BANK_1,
	NODE_OCM_BANK_2,
	NODE_OCM_BANK_3,
	NODE_TCM_0_A,
	NODE_TCM_0_B,
	NODE_TCM_1_A,
	NODE_TCM_1_B,
	NODE_L2,
	NODE_GPU_PP_0,
	NODE_GPU_PP_1,
	NODE_USB_0,
	NODE_USB_1,
	NODE_TTC_0,
	NODE_TTC_1,
	NODE_TTC_2,
	NODE_TTC_3,
	NODE_SATA,
	NODE_ETH_0,
	NODE_ETH_1,
	NODE_ETH_2,
	NODE_ETH_3,
	NODE_UART_0,
	NODE_UART_1,
	NODE_SPI_0,
	NODE_SPI_1,
	NODE_I2C_0,
	NODE_I2C_1,
	NODE_SD_0,
	NODE_SD_1,
	NODE_DP,
	NODE_GDMA,
	NODE_ADMA,
	NODE_NAND,
	NODE_QSPI,
	NODE_GPIO,
	NODE_CAN_0,
	NODE_CAN_1,
	NODE_EXTERN,
	NODE_APLL,
	NODE_VPLL,
	NODE_DPLL,
	NODE_RPLL,
	NODE_IOPLL,
	NODE_DDR,
	NODE_IPI_APU,
	NODE_IPI_RPU_0,
	NODE_GPU,
	NODE_PCIE,
	NODE_PCAP,
	NODE_RTC,
	NODE_LPD,
	NODE_VCU,
	NODE_IPI_RPU_1,
	NODE_IPI_PL_0,
	NODE_IPI_PL_1,
	NODE_IPI_PL_2,
	NODE_IPI_PL_3,
	NODE_PL,
	NODE_ID_MAX
};

/**
 *  @name PM Acknowledge Request Types
 */
enum XPmRequestAck {
	REQUEST_ACK_NO = 1,
	REQUEST_ACK_BLOCKING,
	REQUEST_ACK_NON_BLOCKING,
	REQUEST_ACK_CB_CERROR,
};

/**
 *  @name PM Abort Reasons Enum
 */
enum XPmAbortReason {
	ABORT_REASON_WKUP_EVENT = 100,
	ABORT_REASON_PU_BUSY,
	ABORT_REASON_NO_PWRDN,
	ABORT_REASON_UNKNOWN,
};

/**
 *  @name PM Suspend Reasons Enum
 */
enum XPmSuspendReason {
	SUSPEND_REASON_PU_REQ = 201,
	SUSPEND_REASON_ALERT,
	SUSPEND_REASON_SYS_SHUTDOWN,
};

/**
 *  @name PM RAM States Enum
 */
enum XPmRamState {
	PM_RAM_STATE_OFF = 0,
	PM_RAM_STATE_RETENTION,
	PM_RAM_STATE_ON,
};

/**
 *  @name PM Operating Characteristic types Enum
 */
enum XPmOpCharType {
	PM_OPCHAR_TYPE_POWER = 1,
	PM_OPCHAR_TYPE_TEMP,
	PM_OPCHAR_TYPE_LATENCY,
};

 /* Power management specific return error statuses */
/** @defgroup pmstatmacro
 * @{
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
 *  @name Boot Status Enum
 */
enum XPmBootStatus {
	PM_INITIAL_BOOT,	/**< boot is a fresh system startup */
	PM_RESUME,			/**< boot is a resume */
	PM_BOOT_ERROR,		/**< error, boot cause cannot be identified */
};

/**
 *  @name PM Reset Action types
 */
enum XPmResetAction {
	XILPM_RESET_ACTION_RELEASE,
	XILPM_RESET_ACTION_ASSERT,
	XILPM_RESET_ACTION_PULSE,
};

/**
 *  @name PM Reset Line IDs
 */
enum XPmReset {
	XILPM_RESET_PCIE_CFG = 1000,
	XILPM_RESET_PCIE_BRIDGE,
	XILPM_RESET_PCIE_CTRL,
	XILPM_RESET_DP,
	XILPM_RESET_SWDT_CRF,
	XILPM_RESET_AFI_FM5,
	XILPM_RESET_AFI_FM4,
	XILPM_RESET_AFI_FM3,
	XILPM_RESET_AFI_FM2,
	XILPM_RESET_AFI_FM1,
	XILPM_RESET_AFI_FM0,
	XILPM_RESET_GDMA,
	XILPM_RESET_GPU_PP1,
	XILPM_RESET_GPU_PP0,
	XILPM_RESET_GPU,
	XILPM_RESET_GT,
	XILPM_RESET_SATA,
	XILPM_RESET_ACPU3_PWRON,
	XILPM_RESET_ACPU2_PWRON,
	XILPM_RESET_ACPU1_PWRON,
	XILPM_RESET_ACPU0_PWRON,
	XILPM_RESET_APU_L2,
	XILPM_RESET_ACPU3,
	XILPM_RESET_ACPU2,
	XILPM_RESET_ACPU1,
	XILPM_RESET_ACPU0,
	XILPM_RESET_DDR,
	XILPM_RESET_APM_FPD,
	XILPM_RESET_SOFT,
	XILPM_RESET_GEM0,
	XILPM_RESET_GEM1,
	XILPM_RESET_GEM2,
	XILPM_RESET_GEM3,
	XILPM_RESET_QSPI,
	XILPM_RESET_UART0,
	XILPM_RESET_UART1,
	XILPM_RESET_SPI0,
	XILPM_RESET_SPI1,
	XILPM_RESET_SDIO0,
	XILPM_RESET_SDIO1,
	XILPM_RESET_CAN0,
	XILPM_RESET_CAN1,
	XILPM_RESET_I2C0,
	XILPM_RESET_I2C1,
	XILPM_RESET_TTC0,
	XILPM_RESET_TTC1,
	XILPM_RESET_TTC2,
	XILPM_RESET_TTC3,
	XILPM_RESET_SWDT_CRL,
	XILPM_RESET_NAND,
	XILPM_RESET_ADMA,
	XILPM_RESET_GPIO,
	XILPM_RESET_IOU_CC,
	XILPM_RESET_TIMESTAMP,
	XILPM_RESET_RPU_R50,
	XILPM_RESET_RPU_R51,
	XILPM_RESET_RPU_AMBA,
	XILPM_RESET_OCM,
	XILPM_RESET_RPU_PGE,
	XILPM_RESET_USB0_CORERESET,
	XILPM_RESET_USB1_CORERESET,
	XILPM_RESET_USB0_HIBERRESET,
	XILPM_RESET_USB1_HIBERRESET,
	XILPM_RESET_USB0_APB,
	XILPM_RESET_USB1_APB,
	XILPM_RESET_IPI,
	XILPM_RESET_APM_LPD,
	XILPM_RESET_RTC,
	XILPM_RESET_SYSMON,
	XILPM_RESET_AFI_FM6,
	XILPM_RESET_LPD_SWDT,
	XILPM_RESET_FPD,
	XILPM_RESET_RPU_DBG1,
	XILPM_RESET_RPU_DBG0,
	XILPM_RESET_DBG_LPD,
	XILPM_RESET_DBG_FPD,
	XILPM_RESET_APLL,
	XILPM_RESET_DPLL,
	XILPM_RESET_VPLL,
	XILPM_RESET_IOPLL,
	XILPM_RESET_RPLL,
	XILPM_RESET_GPO3_PL_0,
	XILPM_RESET_GPO3_PL_1,
	XILPM_RESET_GPO3_PL_2,
	XILPM_RESET_GPO3_PL_3,
	XILPM_RESET_GPO3_PL_4,
	XILPM_RESET_GPO3_PL_5,
	XILPM_RESET_GPO3_PL_6,
	XILPM_RESET_GPO3_PL_7,
	XILPM_RESET_GPO3_PL_8,
	XILPM_RESET_GPO3_PL_9,
	XILPM_RESET_GPO3_PL_10,
	XILPM_RESET_GPO3_PL_11,
	XILPM_RESET_GPO3_PL_12,
	XILPM_RESET_GPO3_PL_13,
	XILPM_RESET_GPO3_PL_14,
	XILPM_RESET_GPO3_PL_15,
	XILPM_RESET_GPO3_PL_16,
	XILPM_RESET_GPO3_PL_17,
	XILPM_RESET_GPO3_PL_18,
	XILPM_RESET_GPO3_PL_19,
	XILPM_RESET_GPO3_PL_20,
	XILPM_RESET_GPO3_PL_21,
	XILPM_RESET_GPO3_PL_22,
	XILPM_RESET_GPO3_PL_23,
	XILPM_RESET_GPO3_PL_24,
	XILPM_RESET_GPO3_PL_25,
	XILPM_RESET_GPO3_PL_26,
	XILPM_RESET_GPO3_PL_27,
	XILPM_RESET_GPO3_PL_28,
	XILPM_RESET_GPO3_PL_29,
	XILPM_RESET_GPO3_PL_30,
	XILPM_RESET_GPO3_PL_31,
	XILPM_RESET_RPU_LS,
	XILPM_RESET_PS_ONLY,
	XILPM_RESET_PL,
	XILPM_RESET_GPIO5_EMIO_92,
	XILPM_RESET_GPIO5_EMIO_93,
	XILPM_RESET_GPIO5_EMIO_94,
	XILPM_RESET_GPIO5_EMIO_95,
};

/**
 *  @name PM Notify Events Enum
 */
enum XPmNotifyEvent {
	EVENT_STATE_CHANGE = 1,
	EVENT_ZERO_USERS = 2,
	EVENT_ERROR_CONDITION = 4,
};
 /** @} */
#endif /* PM_DEFS_H_ */

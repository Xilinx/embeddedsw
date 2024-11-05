/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xpm_defs.h
 *
 * @addtogroup xpm_versal_apis XilPM Versal APIs
 * @{
 *****************************************************************************/

#ifndef XPM_DEFS_H_
#define XPM_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_api_version.h"

/**
 * @name PM Version Number
 * @{
 *
 * Version number is a 32bit value, like:
 * (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR
 */
/**
 * PM Version Number
 */
#define PM_VERSION_MAJOR    1UL
#define PM_VERSION_MINOR    0UL
#define PM_VERSION	((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)
/** @} */

/**
 * PM abort reasons
 */
enum XPmAbortReason {
	ABORT_REASON_WKUP_EVENT = 100,			/**< Wakeup Event */
	ABORT_REASON_PU_BUSY,				/**< Processor Busy */
	ABORT_REASON_NO_PWRDN,				/**< No Powerdown */
	ABORT_REASON_UNKNOWN,				/**< Unknown Reason */
};

/**
 * @name PM Abort Reasons
 * @{
 */
/**
 * PM Abort Reasons
 */
#define ABORT_REASON_MIN	ABORT_REASON_WKUP_EVENT
#define ABORT_REASON_MAX	ABORT_REASON_UNKNOWN
/** @} */

/**
 * Boot status enumeration.
 */
enum XPmBootStatus {
	PM_INITIAL_BOOT,				/**< boot is a fresh system startup */
	PM_RESUME,					/**< boot is a resume */
	PM_BOOT_ERROR,					/**< error, boot cause cannot be identified */
};

/**
 *  PM Acknowledge Request Types
 */
enum XPmRequestAck {
	REQUEST_ACK_NO = 1,				/**< No Ack */
	REQUEST_ACK_BLOCKING,				/**< Blocking Ack */
	REQUEST_ACK_NON_BLOCKING,			/**< Non blocking Ack */
	REQUEST_ACK_CB_CERROR,				/**< Callback Error */
};

/**
 *  Device capability requirements enumeration.
 */
enum XPmCapability {
	PM_CAP_ACCESS = 0x1U,				/**< Full access */
	PM_CAP_CONTEXT = 0x2U,				/**< Configuration and contents retained */
	PM_CAP_WAKEUP = 0x4U,				/**< Enabled as a wake-up source */
	PM_CAP_UNUSABLE = 0x8U,				/**< Not usable */
	PM_CAP_SECURE = 0x10U,				/**< Secure access type (non-secure/secure) */
	PM_CAP_COHERENT	= 0x20U,			/**< Device Coherency */
	PM_CAP_VIRTUALIZED = 0x40U,			/**< Device Virtualization */
};

/**
 * Usage status, returned by PmGetNodeStatus
 */
enum XPmDeviceUsage {
	PM_USAGE_CURRENT_SUBSYSTEM = 0x1U,		/**< Current subsystem is using */
	PM_USAGE_OTHER_SUBSYSTEM   = 0x2U,		/**< Other subsystem is using */
};

/**
 * Reset configuration argument
 */
enum XPmResetActions {
	PM_RESET_ACTION_RELEASE,			/**< Reset action release */
	PM_RESET_ACTION_ASSERT,				/**< Reset action assert */
	PM_RESET_ACTION_PULSE,				/**< Reset action pulse */
};

/**
 * Suspend reasons
 */
enum XPmSuspendReason {
	SUSPEND_REASON_PU_REQ		= (201U),	/**< Processor request */
	SUSPEND_REASON_ALERT		= (202U),	/**< Alert */
	SUSPEND_REASON_SYS_SHUTDOWN	= (203U),	/**< System shutdown */
};

/**
 * PM API callback IDs
 */
typedef enum {
	PM_INIT_SUSPEND_CB		= (30),		/**< Suspend callback */
	PM_ACKNOWLEDGE_CB		= (31),		/**< Acknowledge callback */
	PM_NOTIFY_CB			= (32),		/**< Notify callback */
} XPmApiCbId_t;

/**
 * Contains the device status information.
 */
typedef struct XPm_DeviceStatus {
	u32 Status;					/**< Device power state */
	u32 Requirement;				/**< Requirements placed on the device by the caller */
	u32 Usage;					/**< Usage info (which subsystem is using the device) */
} XPm_DeviceStatus;

/**
 * @name Requirement limits
 * @{
 */
/**
 * Requirement limits
 */
#define XPM_MAX_CAPABILITY	((u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT | (u32)PM_CAP_WAKEUP)
#define XPM_MAX_LATENCY		(0xFFFFU)
#define XPM_MAX_QOS		(100U)
#define XPM_MIN_CAPABILITY	(0U)
#define XPM_MIN_LATENCY		(0U)
#define XPM_MIN_QOS		(0U)
#define XPM_DEF_CAPABILITY	XPM_MAX_CAPABILITY
#define XPM_DEF_LATENCY		XPM_MAX_LATENCY
#define XPM_DEF_QOS		XPM_MAX_QOS
/** @} */

/**
 * @name Device node status
 * @{
 */
/**
 * Device node status
 */
#define NODE_STATE_OFF			(0U)
#define NODE_STATE_ON			(1U)
/** @} */

/**
 * @name Processor node status
 * @{
 */
/**
 * Processor node status
 */
#define PROC_STATE_SLEEP		NODE_STATE_OFF
#define PROC_STATE_ACTIVE		NODE_STATE_ON
#define PROC_STATE_FORCEDOFF		(7U)
#define PROC_STATE_SUSPENDING		(8U)
/** @} */

/**
 * Query IDs
 */
enum pm_query_id {
	XPM_QID_INVALID,				/**< Invalid Query ID */
	XPM_QID_CLOCK_GET_NAME,				/**< Get clock name */
	XPM_QID_CLOCK_GET_TOPOLOGY,			/**< Get clock topology */
	XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS,		/**< Get clock fixedfactor parameter */
	XPM_QID_CLOCK_GET_MUXSOURCES,			/**< Get clock mux sources */
	XPM_QID_CLOCK_GET_ATTRIBUTES,			/**< Get clock attributes */
	XPM_QID_PINCTRL_GET_NUM_PINS,			/**< Get total pins */
	XPM_QID_PINCTRL_GET_NUM_FUNCTIONS,		/**< Get total pin functions */
	XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS,	/**< Get total pin function groups */
	XPM_QID_PINCTRL_GET_FUNCTION_NAME,		/**< Get pin function name */
	XPM_QID_PINCTRL_GET_FUNCTION_GROUPS,		/**< Get pin function groups */
	XPM_QID_PINCTRL_GET_PIN_GROUPS,			/**< Get pin groups */
	XPM_QID_CLOCK_GET_NUM_CLOCKS,			/**< Get number of clocks */
	XPM_QID_CLOCK_GET_MAX_DIVISOR,			/**< Get max clock divisor */
	XPM_QID_PLD_GET_PARENT,				/**< Get PLD parent */
	XPM_QID_PINCTRL_GET_ATTRIBUTES,			/**< Get pin attributes */
};

/*
 * This is an automatically generated enum from script.
 * Please do not modify this!
 */
/**
 * Pin Function IDs
 */
enum PmPinFunIds {
	PIN_FUNC_SD0 = 0,			/**< Pin function ID of SD0 */
	PIN_FUNC_SD1 = 1,			/**< Pin function ID of SD1 */
	PIN_FUNC_SMP = 2,			/**< Pin function ID of SMP */
	PIN_FUNC_CAN0 = 3,			/**< Pin function ID of CAN0 */
	PIN_FUNC_CAN1 = 4,			/**< Pin function ID of CAN1 */
	PIN_FUNC_GEM0 = 5,			/**< Pin function ID of GEM0 */
	PIN_FUNC_GEM1 = 6,			/**< Pin function ID of GEM1 */
	PIN_FUNC_I2C0 = 7,			/**< Pin function ID of I2C0 */
	PIN_FUNC_I2C1 = 8,			/**< Pin function ID of I2C1 */
	PIN_FUNC_SPI0 = 9,			/**< Pin function ID of SPI0 */
	PIN_FUNC_SPI1 = 10,			/**< Pin function ID of SPI1 */
	PIN_FUNC_USB0 = 11,			/**< Pin function ID of USB0 */
	PIN_FUNC_USB1 = 12,			/**< Pin function ID of USB1 */
	PIN_FUNC_EMIO0 = 13,			/**< Pin function ID of EMIO0 */
	PIN_FUNC_GPIO0 = 14,			/**< Pin function ID of GPIO0 */
	PIN_FUNC_GPIO1 = 15,			/**< Pin function ID of GPIO1 */
	PIN_FUNC_GPIO2 = 16,			/**< Pin function ID of GPIO2 */
	PIN_FUNC_MDIO0 = 17,			/**< Pin function ID of MDIO0 */
	PIN_FUNC_MDIO1 = 18,			/**< Pin function ID of MDIO1 */
	PIN_FUNC_OSPI0 = 19,			/**< Pin function ID of OSPI0 */
	PIN_FUNC_PCIE0 = 20,			/**< Pin function ID of PCIE0 */
	PIN_FUNC_QSPI0 = 21,			/**< Pin function ID of QSPI0 */
	PIN_FUNC_SMAP0 = 22,			/**< Pin function ID of SMAP0 */
	PIN_FUNC_UART0 = 23,			/**< Pin function ID of UART0 */
	PIN_FUNC_UART1 = 24,			/**< Pin function ID of UART1 */
	PIN_FUNC_WWDT0 = 25,			/**< Pin function ID of WWDT0 */
	PIN_FUNC_WWDT1 = 26,			/**< Pin function ID of WWDT1 */
	PIN_FUNC_SD0_CD = 27,			/**< Pin function ID of SD0_CD */
	PIN_FUNC_SD0_PC = 28,			/**< Pin function ID of SD0_PC */
	PIN_FUNC_SD0_WP = 29,			/**< Pin function ID of SD0_WP */
	PIN_FUNC_SD1_PC = 30,			/**< Pin function ID of SD1_PC */
	PIN_FUNC_TRACE0 = 31,			/**< Pin function ID of TRACE0 */
	PIN_FUNC_SD1_DQS = 32,			/**< Pin function ID of SD1_DQS */
	PIN_FUNC_SPI0_SS = 33,			/**< Pin function ID of SPI0_SS */
	PIN_FUNC_SPI1_SS = 34,			/**< Pin function ID of SPI1_SS */
	PIN_FUNC_GEM_TSU0 = 35,			/**< Pin function ID of GEM_TSU0 */
	PIN_FUNC_OSPI0_SS = 36,			/**< Pin function ID of OSPI0_SS */
	PIN_FUNC_QSPI0_SS = 37,			/**< Pin function ID of QSPI0_SS */
	PIN_FUNC_TEST_CLK = 38,			/**< Pin function ID of TEST_CLK */
	PIN_FUNC_TTC0_CLK = 39,			/**< Pin function ID of TTC0_CLK */
	PIN_FUNC_TTC0_WAV = 40,			/**< Pin function ID of TTC0_WAV */
	PIN_FUNC_TTC1_CLK = 41,			/**< Pin function ID of TTC1_CLK */
	PIN_FUNC_TTC1_WAV = 42,			/**< Pin function ID of TTC1_WAV */
	PIN_FUNC_TTC2_CLK = 43,			/**< Pin function ID of TTC2_CLK */
	PIN_FUNC_TTC2_WAV = 44,			/**< Pin function ID of TTC2_WAV */
	PIN_FUNC_TTC3_CLK = 45,			/**< Pin function ID of TTC3_CLK */
	PIN_FUNC_TTC3_WAV = 46,			/**< Pin function ID of TTC3_WAV */
	PIN_FUNC_TEST_SCAN = 47,			/**< Pin function ID of TEST_SCAN */
	PIN_FUNC_TRACE0_CLK = 48,			/**< Pin function ID of TRACE0_CLK */
	PIN_FUNC_UART0_CTRL = 49,			/**< Pin function ID of UART0_CTRL */
	PIN_FUNC_UART1_CTRL = 50,			/**< Pin function ID of UART1_CTRL */
	PIN_FUNC_OSPI0_RST_N = 51,			/**< Pin function ID of OSPI0_RST_N */
	PIN_FUNC_QSPI0_FBCLK = 52,			/**< Pin function ID of QSPI0_FBCLK */
	PIN_FUNC_SYSMON_I2C0 = 53,			/**< Pin function ID of SYSMON_I2C0 */
	PIN_FUNC_OSPI0_ECC_FAIL = 54,			/**< Pin function ID of OSPI0_ECC_FAIL */
	PIN_FUNC_TAMPER_TRIGGER = 55,			/**< Pin function ID of TAMPER_TRIGGER */
	PIN_FUNC_SYSMON_I2C0_ALRT = 56,			/**< Pin function ID of SYSMON_I2C0_ALRT */
	MAX_FUNCTION = 57			/**< Max Pin function */
};

/**
 * Pin Control Configuration
 */
enum pm_pinctrl_config_param {
	PINCTRL_CONFIG_SLEW_RATE,			/**< Pin config slew rate */
	PINCTRL_CONFIG_BIAS_STATUS,			/**< Pin config bias status */
	PINCTRL_CONFIG_PULL_CTRL,			/**< Pin config pull control */
	PINCTRL_CONFIG_SCHMITT_CMOS,			/**< Pin config schmitt CMOS */
	PINCTRL_CONFIG_DRIVE_STRENGTH,			/**< Pin config drive strength */
	PINCTRL_CONFIG_VOLTAGE_STATUS,			/**< Pin config voltage status */
	PINCTRL_CONFIG_TRI_STATE,			/**< Pin config tri state */
	PINCTRL_CONFIG_MAX,				/**< Max Pin config */
};

/**
 * Pin Control Slew Rate
 */
enum pm_pinctrl_slew_rate {
	PINCTRL_SLEW_RATE_FAST,				/**< Fast slew rate */
	PINCTRL_SLEW_RATE_SLOW,				/**< Slow slew rate */
};

/**
 * Pin Control Bias Status
 */
enum pm_pinctrl_bias_status {
	PINCTRL_BIAS_DISABLE,				/**< Bias disable */
	PINCTRL_BIAS_ENABLE,				/**< Bias enable */
};

/**
 * Pin Control Pull Control
 */
enum pm_pinctrl_pull_ctrl {
	PINCTRL_BIAS_PULL_DOWN,				/**< Bias pull-down */
	PINCTRL_BIAS_PULL_UP,				/**< Bias pull-up */
};

/**
 * Pin Control Input Type
 */
enum pm_pinctrl_schmitt_cmos {
	PINCTRL_INPUT_TYPE_CMOS,			/**< Input type CMOS */
	PINCTRL_INPUT_TYPE_SCHMITT,			/**< Input type SCHMITT */
};

/**
 * Pin Control Drive Strength
 */
enum pm_pinctrl_drive_strength {
	PINCTRL_DRIVE_STRENGTH_TRISTATE,		/**< tri-state */
	PINCTRL_DRIVE_STRENGTH_4MA,			/**< 4mA */
	PINCTRL_DRIVE_STRENGTH_8MA,			/**< 8mA */
	PINCTRL_DRIVE_STRENGTH_12MA,			/**< 12mA */
	PINCTRL_DRIVE_STRENGTH_MAX,			/**< Max value */
};

/**
 * Pin Control Tri State
 */
enum pm_pinctrl_tri_state {
	PINCTRL_TRI_STATE_DISABLE,			/**< Tri state disable */
	PINCTRL_TRI_STATE_ENABLE,			/**< Tri state enable */
};
/**
 * PM init node functions
 */
enum XPmInitFunctions {
	FUNC_INIT_START,				/**< Function ID INIT_START */
	FUNC_INIT_FINISH,				/**< Function ID INIT_FINISH */
	FUNC_SCAN_CLEAR,				/**< Function ID SCAN_CLEAR */
	FUNC_BISR,					/**< Function ID BISR */
	FUNC_LBIST,					/**< Function ID LBIST */
	FUNC_MEM_INIT,					/**< Function ID MEM_INIT */
	FUNC_MBIST_CLEAR,				/**< Function ID MBIST_CLEAR */
	FUNC_HOUSECLEAN_PL,				/**< Function ID HOUSECLEAN_PL */
	FUNC_HOUSECLEAN_COMPLETE,			/**< Function ID HOUSECLEAN_COMPLETE */
	FUNC_MIO_FLUSH,					/**< Function ID MIO FLUSH */
	FUNC_MEM_CTRLR_MAP,				/**< Function ID MEM_CTRLR_MAP */
	FUNC_AMS_TRIM,
	/* Should be last item */
	FUNC_MAX_COUNT_PMINIT,				/**< Function ID MAX */
};

/**
 * IOCTL IDs
 */
typedef enum {
	IOCTL_GET_RPU_OPER_MODE = 0,			/**< Get RPU mode */
	IOCTL_SET_RPU_OPER_MODE = 1,			/**< Set RPU mode */
	IOCTL_RPU_BOOT_ADDR_CONFIG = 2,			/**< RPU boot address config */
	IOCTL_TCM_COMB_CONFIG = 3,			/**< TCM config */
	IOCTL_SET_TAPDELAY_BYPASS = 4,			/**< TAP delay bypass */
	IOCTL_SD_DLL_RESET = 6,				/**< SD DLL reset */
	IOCTL_SET_SD_TAPDELAY = 7,			/**< SD TAP delay */
	/* Ioctl for clock driver */
	IOCTL_SET_PLL_FRAC_MODE = 8,			/**< Set PLL frac mode */
	IOCTL_GET_PLL_FRAC_MODE = 9,			/**< Get PLL frac mode */
	IOCTL_SET_PLL_FRAC_DATA = 10,			/**< Set PLL frac data */
	IOCTL_GET_PLL_FRAC_DATA = 11,			/**< Get PLL frac data */
	IOCTL_WRITE_GGS = 12,				/**< Write GGS */
	IOCTL_READ_GGS = 13,				/**< Read GGS */
	IOCTL_WRITE_PGGS = 14,				/**< Write PGGS */
	IOCTL_READ_PGGS = 15,				/**< Read PGGS */
	/* IOCTL for ULPI reset */
	IOCTL_ULPI_RESET = 16,				/**< ULPI reset */
	/* Set healthy bit value */
	IOCTL_SET_BOOT_HEALTH_STATUS = 17,		/**< Set boot status */
	IOCTL_AFI = 18,					/**< AFI */
	/* Ospi mux select */
	IOCTL_OSPI_MUX_SELECT = 21,			/**< OSPI mux select */
	/* USB PMU state req */
	IOCTL_USB_SET_STATE = 22,			/**< USB set state */
	IOCTL_GET_LAST_RESET_REASON = 23,		/**< Get last reset reason */
	/* AIE ISR Clear */
	IOCTL_AIE_ISR_CLEAR = 24,			/**< AIE ISR clear */
	/* Register SGI to ATF */
	IOCTL_REGISTER_SGI = 25,			/**< Register SGI to ATF */
	/* Runtime feature configuration */
	IOCTL_SET_FEATURE_CONFIG = 26,			/**< Set runtime feature config */
	IOCTL_GET_FEATURE_CONFIG = 27,			/**< Get runtime feature config */
	/* Generic IOCTL Read/Write */
	IOCTL_READ_REG = 28,				/**< Read a 32-bit register */
	IOCTL_MASK_WRITE_REG = 29,			/**< RMW a 32-bit register */
	/* Dynamic MIO config */
	IOCTL_SET_SD_CONFIG = 30,			/**< Set SD config register value */
	IOCTL_SET_GEM_CONFIG = 31,			/**< Set GEM config register value */
	IOCTL_SET_USB_CONFIG = 32,			/**< Set USB config register value */
	/* AIE1/AIEML Run Time Operations */
	IOCTL_AIE_OPS = 33,				/**< AIE1/AIEML Run Time Operations */
	IOCTL_GET_QOS = 34,				/**< Get Device QoS value */
	IOCTL_GET_APU_OPER_MODE = 35,			/**< Get APU operation mode */
	IOCTL_SET_APU_OPER_MODE = 36,			/**< Set APU operation mode */
	IOCTL_PREPARE_DDR_SHUTDOWN = 37,		/**< Prepare DDR for shut down */
	IOCTL_GET_SSIT_TEMP = 38,			/**< Read secondary SLR temperature */
	IOCTL_AIE2PS_OPS = 39,				/**< AIE2PS Operations */
} pm_ioctl_id;
/** @endcond */

/**
 * PLL parameters
 */
enum XPm_PllConfigParams {
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
	PM_PLL_PARAM_MAX,				/**< PLL param ID max */
};

/**
 * PLL modes
 */
enum XPmPllMode {
	PM_PLL_MODE_INTEGER		= (0U),		/**< PLL mode integer */
	PM_PLL_MODE_FRACTIONAL		= (1U),		/**< PLL mode fractional */
	PM_PLL_MODE_RESET		= (2U),		/**< PLL mode reset */
};


/**
 * PM operating characteristic types
 */
enum XPmOpCharType {
	PM_OPCHAR_TYPE_POWER = 1,			/**< Operating characteristic ID power */
	PM_OPCHAR_TYPE_TEMP,				/**< Operating characteristic ID temperature */
	PM_OPCHAR_TYPE_LATENCY,				/**< Operating characteristic ID latency */
};

/**
 * PM notify events
 */
enum XPmNotifyEvent {
	EVENT_STATE_CHANGE = 1,				/**< State change event */
	EVENT_ZERO_USERS = 2,				/**< Zero user event */
	EVENT_CPU_IDLE_FORCE_PWRDWN = 4,		/**< CPU idle event during force power down */
};

/**
 * @name System shutdown macros
 * @{
 */
/**
 * System shutdown macros
 */
#define PM_SHUTDOWN_TYPE_SHUTDOWN		(0U)
#define PM_SHUTDOWN_TYPE_RESET			(1U)

#define PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM	(0U)
#define PM_SHUTDOWN_SUBTYPE_RST_PS_ONLY		(1U)
#define PM_SHUTDOWN_SUBTYPE_RST_SYSTEM		(2U)
/** @} */

/**
 * @name State arguments of the self suspend
 * @{
 */
/**
 * State arguments of the self suspend
 */
#define PM_SUSPEND_STATE_CPU_IDLE		0x0U
#define PM_SUSPEND_STATE_CPU_OFF		0x1U
#define PM_SUSPEND_STATE_SUSPEND_TO_RAM		0xFU
/** @} */

/**
 * @name RPU operation mode
 * @{
 */
/**
 * RPU operation mode
 */
#define XPM_RPU_MODE_LOCKSTEP	0U
#define XPM_RPU_MODE_SPLIT	1U
/** @} */

/**
 * @name APU operation mode
 * @{
 */
/**
 * APU operation mode
 */
#define XPM_APU_MODE_LOCKSTEP	1U
#define XPM_APU_MODE_SPLIT	0U
/** @} */

/**
 * @name RPU Boot memory
 * @{
 */
/**
 * RPU Boot memory
 */
#define XPM_RPU_BOOTMEM_LOVEC	(0U)
#define XPM_RPU_BOOTMEM_HIVEC	(1U)
/** @} */

/**
 * @name RPU TCM mode
 * @{
 */
/**
 * RPU TCM mode
 */
#define XPM_RPU_TCM_SPLIT	0U
#define XPM_RPU_TCM_COMB	1U
/** @} */

/**
 * @name Boot health status mask
 * @{
 */
/**
 * Boot health status mask
 */
#define XPM_BOOT_HEALTH_STATUS_MASK	(0x1U)
/** @} */

/**
 * @name Tap delay signal type
 * @{
 */
/**
 * Tap delay signal type
 */
#define XPM_TAPDELAY_QSPI		(2U)
/** @} */

/**
 * @name Tap delay bypass
 * @{
 */
/**
 * Tap delay bypass
 */
#define XPM_TAPDELAY_BYPASS_DISABLE	(0U)
#define XPM_TAPDELAY_BYPASS_ENABLE	(1U)
/** @} */

/**
 * @name Ospi AXI Mux select
 * @{
 */
/**
 * Ospi AXI Mux select
 */
#define XPM_OSPI_MUX_SEL_DMA		(0U)
#define XPM_OSPI_MUX_SEL_LINEAR		(1U)
#define XPM_OSPI_MUX_GET_MODE		(2U)
/** @} */

/**
 * @name Tap delay type
 * @{
 */
/**
 * Tap delay type
 */
#define XPM_TAPDELAY_INPUT		(0U)
#define XPM_TAPDELAY_OUTPUT		(1U)
/** @} */

/**
 * @name Dll reset type
 * @{
 */
/**
 * Dll reset type
 */
#define XPM_DLL_RESET_ASSERT		(0U)
#define XPM_DLL_RESET_RELEASE		(1U)
#define XPM_DLL_RESET_PULSE		(2U)
/** @} */

/**
 * @name Reset Reason
 * @{
 */
/**
 * Reset Reason
 */
#define XPM_RESET_REASON_EXT_POR	(0U)
#define XPM_RESET_REASON_SW_POR		(1U)
#define XPM_RESET_REASON_SLR_POR	(2U)
#define XPM_RESET_REASON_ERR_POR	(3U)
#define XPM_RESET_REASON_DAP_SRST	(7U)
#define XPM_RESET_REASON_ERR_SRST	(8U)
#define XPM_RESET_REASON_SW_SRST	(9U)
#define XPM_RESET_REASON_SLR_SRST	(10U)
#define XPM_RESET_REASON_INVALID	(0xFFU)
/** @} */

/**
 * @name PM API versions
 * @{
 */
/**
 * PM API versions
 */
#define XST_API_BASE_VERSION			(1U)
#define XST_API_QUERY_DATA_VERSION		(2U)
#define XST_API_REG_NOTIFIER_VERSION		(2U)
#define XST_API_PM_IOCTL_VERSION		(2U)
#define XST_API_PM_FEATURE_CHECK_VERSION	(2U)
/* Version 3 supports the CPU idling feature during force power down */
#define XST_API_SELF_SUSPEND_VERSION		(3U)
#define XST_API_FORCE_POWERDOWN_VERSION		(2U)
/*
 * Version 2 supports some extra security checks for REQ_ACCESS_SECURE
 * and REQ_ACCESS_SECURE_NONSECURE policies.
 */
#define XST_API_REQUEST_NODE_VERSION		(2U)
#define XST_API_RELEASE_NODE_VERSION		(2U)
/*
 * Version 2 supports the bitmask functionality of GET_OP_CHAR IDs
 * where the user can check whether the ID is supported or not in
 * the firmware
 */
#define XST_API_GET_OP_CHAR_VERSION		(2U)
/** @} */

/**
 * @name Destination Cluster/Core Ids
 * @{
 */
/**
 * Destination Cluster Ids
 */
#define XPM_DSTN_CLUSTER_0 (0x0U)
#define XPM_DSTN_CLUSTER_1 (0x1U)
#define XPM_DSTN_CLUSTER_2 (0x2U)
#define XPM_DSTN_CLUSTER_3 (0x3U)

/**
 * Destination Core Ids
 */
#define XPM_DSTN_CORE_0 (u8)(0x0U)
#define XPM_DSTN_CORE_1 (u8)(0x1U)
#define XPM_DSTN_CORE_2 (u8)(0x2U)
#define XPM_DSTN_CORE_3 (u8)(0x3U)

/**
 * Macros to read Cluster/Core Ids
 */
#define GET_APU_CLUSTER_ID(DeviceId) \
	((NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_ACPU_0_3)?(u8)XPM_DSTN_CLUSTER_0: \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_ACPU_1_3)?(u8)XPM_DSTN_CLUSTER_1: \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_ACPU_2_3)?(u8)XPM_DSTN_CLUSTER_2: \
	(u8)XPM_DSTN_CLUSTER_3)

#define GET_CORE(DeviceId,Index) \
	(NODEINDEX(DeviceId)-Index)

#define GET_APU_CORE_NUM(DeviceId) \
	((NODEINDEX(DeviceId)<==\XPM_NODEIDX_DEV_ACPU_0_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_ACPU_0_0): \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_ACPU_1_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_ACPU_1_0): \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_ACPU_2_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_ACPU_2_0): \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_ACPU_3_0))

#define GET_RPU_CLUSTER_ID(DeviceId) \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_RPU_A_1)? \
	(u8)XPM_DSTN_CLUSTER_0: (u8)XPM_DSTN_CLUSTER_1

#define GET_RPU_CORE_NUM(DeviceId) \
	((DeviceId == PM_DEV_RPU_A_0) || (DeviceId == PM_DEV_RPU_B_0)? \
	XPM_DSTN_CORE_0:XPM_DSTN_CORE_1)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEFS_H_ */
 /** @} */

/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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
#define PM_VERSION      ((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)
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
};

/**
 * Pin Function IDs
 */
enum PmPinFunIds {
	PIN_FUNC_SPI0,					/**< Pin function ID of SPI0 */
	PIN_FUNC_SPI0_SS,				/**< Pin function ID of SPI0_SS */
	PIN_FUNC_SPI1,					/**< Pin function ID of SPI1 */
	PIN_FUNC_SPI1_SS,				/**< Pin function ID of SPI1_SS */
	PIN_FUNC_CAN0,					/**< Pin function ID of CAN0 */
	PIN_FUNC_CAN1,					/**< Pin function ID of CAN1 */
	PIN_FUNC_I2C0,					/**< Pin function ID of I2C0 */
	PIN_FUNC_I2C1,					/**< Pin function ID of I2C1 */
	PIN_FUNC_I2C_PMC,				/**< Pin function ID of I2C_PMC */
	PIN_FUNC_TTC0_CLK,				/**< Pin function ID of TTC0_CLK */
	PIN_FUNC_TTC0_WAV,				/**< Pin function ID of TTC0_WAV */
	PIN_FUNC_TTC1_CLK,				/**< Pin function ID of TTC1_CLK */
	PIN_FUNC_TTC1_WAV,				/**< Pin function ID of TTC1_WAV */
	PIN_FUNC_TTC2_CLK,				/**< Pin function ID of TTC2_CLK */
	PIN_FUNC_TTC2_WAV,				/**< Pin function ID of TTC2_WAV */
	PIN_FUNC_TTC3_CLK,				/**< Pin function ID of TTC3_CLK */
	PIN_FUNC_TTC3_WAV,				/**< Pin function ID of TTC3_WAV */
	PIN_FUNC_WWDT0,					/**< Pin function ID of WWDT0 */
	PIN_FUNC_WWDT1,					/**< Pin function ID of WWDT1 */
	PIN_FUNC_SYSMON_I2C0,				/**< Pin function ID of SYSMON_I2C0 */
	PIN_FUNC_SYSMON_I2C0_ALERT,			/**< Pin function ID of SYSMON_I2C0_AL */
	PIN_FUNC_UART0,					/**< Pin function ID of UART0 */
	PIN_FUNC_UART0_CTRL,				/**< Pin function ID of UART0_CTRL */
	PIN_FUNC_UART1,					/**< Pin function ID of UART1 */
	PIN_FUNC_UART1_CTRL,				/**< Pin function ID of UART1_CTRL */
	PIN_FUNC_GPIO0,					/**< Pin function ID of GPIO0 */
	PIN_FUNC_GPIO1,					/**< Pin function ID of GPIO1 */
	PIN_FUNC_GPIO2,					/**< Pin function ID of GPIO2 */
	PIN_FUNC_EMIO0,					/**< Pin function ID of EMIO0 */
	PIN_FUNC_GEM0,					/**< Pin function ID of GEM0 */
	PIN_FUNC_GEM1,					/**< Pin function ID of GEM1 */
	PIN_FUNC_TRACE0,				/**< Pin function ID of TRACE0 */
	PIN_FUNC_TRACE0_CLK,				/**< Pin function ID of TRACE0_CLK */
	PIN_FUNC_MDIO0,					/**< Pin function ID of MDIO0 */
	PIN_FUNC_MDIO1,					/**< Pin function ID of MDIO1 */
	PIN_FUNC_GEM_TSU0,				/**< Pin function ID of GEM_TSU0 */
	PIN_FUNC_PCIE0,					/**< Pin function ID of PCIE0 */
	PIN_FUNC_SMAP0,					/**< Pin function ID of SMAP0 */
	PIN_FUNC_USB0,					/**< Pin function ID of USB0 */
	PIN_FUNC_SD0,					/**< Pin function ID of SD0 */
	PIN_FUNC_SD0_PC,				/**< Pin function ID of SD0_PC */
	PIN_FUNC_SD0_CD,				/**< Pin function ID of SD0_CD */
	PIN_FUNC_SD0_WP,				/**< Pin function ID of SD0_WP */
	PIN_FUNC_SD1,					/**< Pin function ID of SD1 */
	PIN_FUNC_SD1_PC,				/**< Pin function ID of SD1_PC */
	PIN_FUNC_SD1_CD,				/**< Pin function ID of SD1_CD */
	PIN_FUNC_SD1_WP,				/**< Pin function ID of SD1_WP */
	PIN_FUNC_OSPI0,					/**< Pin function ID of OSPI0 */
	PIN_FUNC_OSPI0_SS,				/**< Pin function ID of OSPI0_SS */
	PIN_FUNC_QSPI0,					/**< Pin function ID of QSPI0 */
	PIN_FUNC_QSPI0_FBCLK,				/**< Pin function ID of QSPI0_FBCLK */
	PIN_FUNC_QSPI0_SS,				/**< Pin function ID of QSPI0_SS */
	PIN_FUNC_TEST_CLK,				/**< Pin function ID of TEST_CLK */
	PIN_FUNC_TEST_SCAN,				/**< Pin function ID of TEST_SCAN */
	PIN_FUNC_TAMPER_TRIGGER,			/**< Pin function ID of TAMPER_TRIGGER */
	MAX_FUNCTION,					/**< Max Pin function */
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
	IOCTL_SET_FEATURE_CONFIG,			/**< Set runtime feature config */
	IOCTL_GET_FEATURE_CONFIG,			/**< Get runtime feature config */
} pm_ioctl_id;

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
	FUNC_MAX_COUNT_PMINIT,				/**< Function ID MAX */
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
 * @name Probe Counter Type
 * @{
 */
/**
 * Probe Counter Type
 */
#define XPM_PROBE_COUNTER_TYPE_LAR_LSR		(0U)
#define XPM_PROBE_COUNTER_TYPE_MAIN_CTL		(1U)
#define XPM_PROBE_COUNTER_TYPE_CFG_CTL		(2U)
#define XPM_PROBE_COUNTER_TYPE_STATE_PERIOD	(3U)
#define XPM_PROBE_COUNTER_TYPE_PORT_SEL		(4U)
#define XPM_PROBE_COUNTER_TYPE_SRC		(5U)
#define XPM_PROBE_COUNTER_TYPE_VAL		(6U)
/** @} */

/**
 * @name PM API versions
 * @{
 */
/**
 * PM API versions
 */
#define XST_API_BASE_VERSION		(1U)
#define XST_API_QUERY_DATA_VERSION	(2U)
#define XST_API_REG_NOTIFIER_VERSION	(2U)
/** @} */

/**
 * PM API IDs
 */
typedef enum {
	PM_API_MIN,					/**< 0x0 */
	PM_GET_API_VERSION,				/**< 0x1 */
	PM_SET_CONFIGURATION,				/**< 0x2 */
	PM_GET_NODE_STATUS,				/**< 0x3 */
	PM_GET_OP_CHARACTERISTIC,			/**< 0x4 */
	PM_REGISTER_NOTIFIER,				/**< 0x5 */
	PM_REQUEST_SUSPEND,				/**< 0x6 */
	PM_SELF_SUSPEND,				/**< 0x7 */
	PM_FORCE_POWERDOWN,				/**< 0x8 */
	PM_ABORT_SUSPEND,				/**< 0x9 */
	PM_REQUEST_WAKEUP,				/**< 0xA */
	PM_SET_WAKEUP_SOURCE,				/**< 0xB */
	PM_SYSTEM_SHUTDOWN,				/**< 0xC */
	PM_REQUEST_NODE,				/**< 0xD */
	PM_RELEASE_NODE,				/**< 0xE */
	PM_SET_REQUIREMENT,				/**< 0xF */
	PM_SET_MAX_LATENCY,				/**< 0x10 */
	PM_RESET_ASSERT,				/**< 0x11 */
	PM_RESET_GET_STATUS,				/**< 0x12 */
	PM_MMIO_WRITE,					/**< 0x13 */
	PM_MMIO_READ,					/**< 0x14 */
	PM_INIT_FINALIZE,				/**< 0x15 */
	PM_FPGA_LOAD,					/**< 0x16 */
	PM_FPGA_GET_STATUS,				/**< 0x17 */
	PM_GET_CHIPID,					/**< 0x18 */
	PM_SECURE_RSA_AES,				/**< 0x19 */
	PM_SECURE_SHA,					/**< 0x1A */
	PM_SECURE_RSA,					/**< 0x1B */
	PM_PINCTRL_REQUEST,				/**< 0x1C */
	PM_PINCTRL_RELEASE,				/**< 0x1D */
	PM_PINCTRL_GET_FUNCTION,			/**< 0x1E */
	PM_PINCTRL_SET_FUNCTION,			/**< 0x1F */
	PM_PINCTRL_CONFIG_PARAM_GET,			/**< 0x20 */
	PM_PINCTRL_CONFIG_PARAM_SET,			/**< 0x21 */
	PM_IOCTL,					/**< 0x22 */
	PM_QUERY_DATA,					/**< 0x23 */
	PM_CLOCK_ENABLE,				/**< 0x24 */
	PM_CLOCK_DISABLE,				/**< 0x25 */
	PM_CLOCK_GETSTATE,				/**< 0x26 */
	PM_CLOCK_SETDIVIDER,				/**< 0x27 */
	PM_CLOCK_GETDIVIDER,				/**< 0x28 */
	PM_CLOCK_SETRATE,				/**< 0x29 */
	PM_CLOCK_GETRATE,				/**< 0x2A */
	PM_CLOCK_SETPARENT,				/**< 0x2B */
	PM_CLOCK_GETPARENT,				/**< 0x2C */
	PM_SECURE_IMAGE,				/**< 0x2D */
	PM_FPGA_READ,					/**< 0x2E */
	PM_API_RESERVED_1,				/**< 0x2F */
	PM_PLL_SET_PARAMETER,				/**< 0x30 */
	PM_PLL_GET_PARAMETER,				/**< 0x31 */
	PM_PLL_SET_MODE,				/**< 0x32 */
	PM_PLL_GET_MODE,				/**< 0x33 */
	PM_REGISTER_ACCESS,				/**< 0x34 */
	PM_EFUSE_ACCESS,				/**< 0x35 */
	PM_ADD_SUBSYSTEM,				/**< 0x36 */
	PM_DESTROY_SUBSYSTEM,				/**< 0x37 */
	PM_DESCRIBE_NODES,				/**< 0x38 */
	PM_ADD_NODE,					/**< 0x39 */
	PM_ADD_NODE_PARENT,				/**< 0x3A */
	PM_ADD_NODE_NAME,				/**< 0x3B */
	PM_ADD_REQUIREMENT,				/**< 0x3C */
	PM_SET_CURRENT_SUBSYSTEM,			/**< 0x3D */
	PM_INIT_NODE,					/**< 0x3E */
	PM_FEATURE_CHECK,				/**< 0x3F */
	PM_ISO_CONTROL,					/**< 0x40 */
	PM_ACTIVATE_SUBSYSTEM,				/**< 0x41 */
	PM_API_MAX					/**< 0x42 */
} XPm_ApiId;

/**
 * @name Houseclean Disable Masks
 * @{
 */
/**
 * Houseclean Disable Masks
 */
#define HOUSECLEAN_DISABLE_DEFAULT_MASK        (0x0000U)
#define HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK (0x0001U)
#define HOUSECLEAN_DISABLE_BISR_MASK       (0x0002U)
#define HOUSECLEAN_DISABLE_LBIST_MASK      (0x0004U)
#define HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK    (0x0008U)
#define HOUSECLEAN_DISABLE_PL_HC_MASK      (0x0010U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEFS_H_ */
 /** @} */

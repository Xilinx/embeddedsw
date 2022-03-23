/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
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

#include "xpm_regs.h"
#include "xil_types.h"

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
#define XPM_MAX_QOS		(100U)
#define XPM_DEF_QOS		XPM_MAX_QOS
#define XPM_MAX_LATENCY		(0xFFFFU)

/**
 * Usage status, returned by PmGetNodeStatus
 */
enum XPmDeviceUsage {
	PM_USAGE_CURRENT_SUBSYSTEM = 0x1U,		/**< Current subsystem is using */
	PM_USAGE_OTHER_SUBSYSTEM   = 0x2U,		/**< Other subsystem is using */
};

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
} pm_ioctl_id;


/**
 * Contains the device status information.
 */
typedef struct XPm_DeviceStatus {
	u32 Status;					/**< Device power state */
	u32 Requirement;				/**< Requirements placed on the device by the caller */
	u32 Usage;					/**< Usage info (which subsystem is using the device) */
} XPm_DeviceStatus;

/**
 * PM API callback IDs
 */
typedef enum {
	PM_INIT_SUSPEND_CB		= (30),		/**< Suspend callback */
	PM_ACKNOWLEDGE_CB		= (31),		/**< Acknowledge callback */
	PM_NOTIFY_CB			= (32),		/**< Notify callback */
} XPmApiCbId_t;

/**
 * RPU operation mode
 */
#define XPM_RPU_MODE_LOCKSTEP	0U
#define XPM_RPU_MODE_SPLIT	1U

#define XPM_RPU_SLSPLIT_MASK		BIT(3)
#define XPM_RPU_TCM_COMB_MASK		BIT(6)
#define XPM_RPU_SLCLAMP_MASK		BIT(4)

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
	PM_BISR = 0x43,					/**< 0x43 */
	PM_API_MAX					/**< 0x44 */
} XPm_ApiId;

#define CRL_PSM_RST_MODE_OFFSET					(0x00000370U)
#define CRP_RESET_REASON_ERR_POR_MASK				(0x00000008U)
#define CRP_RESET_REASON_SLR_POR_MASK				(0x00000004U)
#define CRP_RESET_REASON_SW_POR_MASK				(0x00000002U)
#define PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0			( ( PMC_GLOBAL_BASEADDR ) + 0x00000050U )

#define XPM_DSTN_CLUSTER_0 (0x0U)
#define XPM_DSTN_CLUSTER_1 (0x1U)
#define XPM_DSTN_CLUSTER_2 (0x2U)
#define XPM_DSTN_CLUSTER_3 (0x3U)

#define XPM_DSTN_CORE_0 (u8)(0x0U)
#define XPM_DSTN_CORE_1 (u8)(0x1U)
#define XPM_DSTN_CORE_2 (u8)(0x2U)
#define XPM_DSTN_CORE_3 (u8)(0x3U)

#define GET_APU_CLUSTER_ID(DeviceId) \
	((NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_0_ACPU_3)?(u8)XPM_DSTN_CLUSTER_0: \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_1_ACPU_3)?(u8)XPM_DSTN_CLUSTER_1: \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_2_ACPU_3)?(u8)XPM_DSTN_CLUSTER_2: \
	(u8)XPM_DSTN_CLUSTER_3)

#define GET_CORE(DeviceId,Index) \
	(NODEINDEX(DeviceId)-Index)

#define GET_APU_CORE_NUM(DeviceId) \
	((NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_0_ACPU_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_CLUSTER_0_ACPU_0): \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_1_ACPU_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_CLUSTER_1_ACPU_0): \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_CLUSTER_2_ACPU_3)? \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_CLUSTER_2_ACPU_0): \
	GET_CORE(DeviceId,XPM_NODEIDX_DEV_CLUSTER_3_ACPU_0))

#define GET_RPU_CLUSTER_ID(DeviceId) \
	(NODEINDEX(DeviceId)<=XPM_NODEIDX_DEV_RPU0_1_CLUSTER_0)? \
	(u8)XPM_DSTN_CLUSTER_0: (u8)XPM_DSTN_CLUSTER_1

#define GET_RPU_CORE_NUM(DeviceId) \
	((DeviceId == PM_DEV_CLUSTER0_RPU0_0) || (DeviceId == PM_DEV_CLUSTER1_RPU0_0)? \
	XPM_DSTN_CORE_0:XPM_DSTN_CORE_1)

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEFS_H_ */
 /** @} */

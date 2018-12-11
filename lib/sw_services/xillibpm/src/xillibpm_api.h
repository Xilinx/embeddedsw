/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef XILLIBPM_API_H_
#define XILLIBPM_API_H_

#include "xil_types.h"
#include "xstatus.h"

#define XIpiPsu	u32

/*
 * Version number is a 32bit value, like:
 * (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR
 */
#define PM_VERSION_MAJOR    1U
#define PM_VERSION_MINOR    0U
#define PM_VERSION      ((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)

/**
 *  @name Boot Status Enum
 */
enum XPmBootStatus {
	PM_INITIAL_BOOT, /**< boot is a fresh system startup */
	PM_RESUME, /**< boot is a resume */
	PM_BOOT_ERROR, /**< error, boot cause cannot be identified */
};

/**
 *  @name Device Capability Requirements Enum
 */
enum XPmCapability {
	PM_CAP_ACCESS = 0x1U, /**< Full access */
	PM_CAP_CONTEXT = 0x2U, /**< Configuration and contents retained */
	PM_CAP_WAKEUP = 0x4U, /**< Enabled as a wake-up source */
};

/**
 * XPm_DeviceStatus - struct containing device status information
 */
typedef struct XPm_DeviceStatus {
	u32 Status; /**< Device power state */
	u32 Requirement; /**< Requirements placed on the device by the caller */
	u32 Usage; /**< Usage info (which subsystem is using the device) */
} XPm_DeviceStatus;

/* Requirement limits */
#define XPM_MAX_CAPABILITY	(PM_CAP_ACCESS | PM_CAP_CONTEXT)
#define XPM_MAX_LATENCY		(0xFFFFU)
#define XPM_MAX_QOS			(100)
#define XPM_MIN_CAPABILITY	(0)
#define XPM_MIN_LATENCY		(0)
#define XPM_MIN_QOS			(0)
#define XPM_DEF_CAPABILITY	XPM_MAX_CAPABILITY
#define XPM_DEF_LATENCY		XPM_MAX_LATENCY
#define XPM_DEF_QOS			XPM_MAX_QOS

enum pm_query_id {
	XPM_QID_INVALID,
	XPM_QID_CLOCK_GET_NAME,
	XPM_QID_CLOCK_GET_TOPOLOGY,
	XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS,
	XPM_QID_CLOCK_GET_MUXSOURCES,
	XPM_QID_CLOCK_GET_ATTRIBUTES,
	XPM_QID_PINCTRL_GET_NUM_PINS,
	XPM_QID_PINCTRL_GET_NUM_FUNCTIONS,
	XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS,
	XPM_QID_PINCTRL_GET_FUNCTION_NAME,
	XPM_QID_PINCTRL_GET_FUNCTION_GROUPS,
	XPM_QID_PINCTRL_GET_PIN_GROUPS,
	XPM_QID_CLOCK_GET_NUM_CLOCKS,
};

enum pm_pinctrl_config_param {
	PINCTRL_CONFIG_SLEW_RATE,
	PINCTRL_CONFIG_BIAS_STATUS,
	PINCTRL_CONFIG_PULL_CTRL,
	PINCTRL_CONFIG_SCHMITT_CMOS,
	PINCTRL_CONFIG_DRIVE_STRENGTH,
	PINCTRL_CONFIG_VOLTAGE_STATUS,
	PINCTRL_CONFIG_TRI_STATE,
	PINCTRL_CONFIG_MAX,
};

enum pm_pinctrl_slew_rate {
	PINCTRL_SLEW_RATE_SLOW,
	PINCTRL_SLEW_RATE_FAST,
};

enum pm_pinctrl_bias_status {
	PINCTRL_BIAS_DISABLE,
	PINCTRL_BIAS_ENABLE,
};

enum pm_pinctrl_pull_ctrl {
	PINCTRL_BIAS_PULL_DOWN,
	PINCTRL_BIAS_PULL_UP,
};

enum pm_pinctrl_schmitt_cmos {
	PINCTRL_INPUT_TYPE_CMOS,
	PINCTRL_INPUT_TYPE_SCHMITT,
};

enum pm_pinctrl_drive_strength {
	PINCTRL_DRIVE_STRENGTH_TRISTATE,
	PINCTRL_DRIVE_STRENGTH_4MA,
	PINCTRL_DRIVE_STRENGTH_8MA,
	PINCTRL_DRIVE_STRENGTH_12MA,
	PINCTRL_DRIVE_STRENGTH_MAX,
};

enum pm_pinctrl_tri_state {
	PINCTRL_TRI_STATE_DISABLE,
	PINCTRL_TRI_STATE_ENABLE,
};

enum pm_ioctl_id {
	IOCTL_GET_RPU_OPER_MODE,
	IOCTL_SET_RPU_OPER_MODE,
	IOCTL_RPU_BOOT_ADDR_CONFIG,
	IOCTL_TCM_COMB_CONFIG,
	IOCTL_SET_TAPDELAY_BYPASS,
	IOCTL_SET_SGMII_MODE,
	IOCTL_SD_DLL_RESET,
	IOCTL_SET_SD_TAPDELAY,
	/* Ioctl for clock driver */
	IOCTL_SET_PLL_FRAC_MODE,
	IOCTL_GET_PLL_FRAC_MODE,
	IOCTL_SET_PLL_FRAC_DATA,
	IOCTL_GET_PLL_FRAC_DATA,
	IOCTL_WRITE_GGS,
	IOCTL_READ_GGS,
	IOCTL_WRITE_PGGS,
	IOCTL_READ_PGGS,
	/* IOCTL for ULPI reset */
	IOCTL_ULPI_RESET,
	/* Set healthy bit value */
	IOCTL_SET_BOOT_HEALTH_STATUS,
	IOCTL_AFI,
};

/* RPU operation mode */
#define XPM_RPU_MODE_LOCKSTEP	0U
#define XPM_RPU_MODE_SPLIT	1U

/* RPU Boot memory */
#define XPM_RPU_BOOTMEM_LOVEC	(0U)
#define XPM_RPU_BOOTMEM_HIVEC	(1U)

/* Global general storage register base address */
#define GGS_BASEADDR	(0xFFC90030U)
#define GGS_NUM_REGS	(4)

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xFFD90050U)
#define PGGS_NUM_REGS	(4)

XStatus XPm_Init(XIpiPsu *const IpiInst,
			void (* const RequestCb)(u32 SubsystemId, const u32 EventId));

XStatus XPm_GetApiVersion(u32 *Version);

enum XPmBootStatus XPm_GetBootStatus();

XStatus XPm_CreateSubsystem(void (*const NotifyCb)(u32 SubsystemId,
						   const u32 EventId),
			    u32 *SubsystemId);

XStatus XPm_DestroySubsystem(u32 SubsystemId);

XStatus XPm_RequestWakeUp(const u32 DeviceId,
			const u32 SetAddress,
			const u64 Address);

XStatus XPm_RequestDevice(const u32 SubsystemId,
			const u32 TargetSubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS);

XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId);

XStatus XPm_SetRequirement(const u32 SubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS);

XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);

XStatus XPm_Query(const u32 Qid,
			const u32 Arg1,
			const u32 Arg2,
			u32 *const Output);

XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable);

XStatus XPm_GetClockState(const u32 ClockId, u32 *const State);

XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider);

XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider);

XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentId);

XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentId);

XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value);

XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value);

XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value);

XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value);

XStatus XPm_SetResetState(const u32 SubsystemId, const u32 DeviceId, const u32 Reset);

XStatus XPm_GetResetState(const u32 DeviceId, u32 *const Reset);

XStatus XPm_SetPinFunction(const u32 SubsystemId, const u32 PinId, const u32 FunctionId);

XStatus XPm_GetPinFunction(const u32 PinId, u32 *const DeviceId);

XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal);

XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 *const ParamVal);

XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId);

XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId);

XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
                        const u32 IoctlId,
                        const u32 Arg1,
                        const u32 Arg2,u32 *const Response);

XStatus XPm_DescribeNodes(u32 *Args, u32 NumArgs);
XStatus XPm_AddNodeParent(u32 *Args, u32 NumArgs);
XStatus XPm_AddNodeName(u32 *Args, u32 NumArgs);
XStatus XPm_AddNode(u32 *Args, u32 NumArgs);

/** @} */
#endif /* XILLIBPM_API_H_ */

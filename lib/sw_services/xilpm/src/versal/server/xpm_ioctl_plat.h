/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_IOCTL_PLAT_H_
#define XPM_IOCTL_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_access.h"
#include "xpm_defs.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Probe Counter Register related macros */
#define PROBE_COUNTER_REQ_TYPE_SHIFT		(16U)
#define PROBE_COUNTER_REQ_TYPE_MASK		(0xFFU)
#define PROBE_COUNTER_TYPE_SHIFT		(8U)
#define PROBE_COUNTER_TYPE_MASK			(0xFFU)
#define PROBE_COUNTER_IDX_MASK			(0xFFU)

#define PROBE_COUNTER_CPU_R5_MAX_IDX		(9U)
#define PROBE_COUNTER_LPD_MAX_IDX		(5U)
#define PROBE_COUNTER_FPD_MAX_IDX		(15U)

#define PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE	(3U)
#define PROBE_COUNTER_LPD_MAX_REQ_TYPE		(7U)
#define PROBE_COUNTER_FPD_MAX_REQ_TYPE		(3U)

#define IS_DEV_USB(DeviceId)			(PM_DEV_USB_0 == (DeviceId))

maybe_unused static XStatus XPm_ValidateDeviceId(const pm_ioctl_id IoctlId, const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	if (((u32)IOCTL_GET_RPU_OPER_MODE == IoctlId) ||
	    ((u32)IOCTL_SET_RPU_OPER_MODE == IoctlId) ||
	    ((u32)IOCTL_RPU_BOOT_ADDR_CONFIG == IoctlId) ||
	    ((u32)IOCTL_TCM_COMB_CONFIG == IoctlId)) {
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_GetQos(const u32 DeviceId, pm_ioctl_id IoctlId, u32 *Response);
XStatus XPm_AieOperation(u32 SubsystemId, u32 Id, pm_ioctl_id IoctlId, u32 Part, u32 Ops);
XStatus XPm_AieISRClear(u32 SubsystemId, u32 AieDeviceId, u32 Value);
XStatus XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write);
XStatus XPmIoctl_IsOperationAllowed(u32 RegNum, u32 SubsystemId,
		const u32 *Perms, u32 Type, u32 CmdType);
maybe_unused static inline XStatus XPm_ApuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	(void)DeviceId;
	(void)Mode;
	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPm_ApuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	(void)DeviceId;
	(void)Mode;
	return XPM_ERR_IOCTL;
}
maybe_unused static inline u32 XPm_GetUsbPwrReqOffset(const u32 DeviceId)
{
	(void)DeviceId;
	return XPM_USB_PWR_REQ_OFFSET;
}
maybe_unused static inline u32 XPm_GetUsbCurrPwrOffset(const u32 DeviceId)
{
	(void)DeviceId;
	return XPM_USB_CUR_PWR_OFFSET;
}
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_IOCTL_PLAT_H_ */

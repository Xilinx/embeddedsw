/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_IOCTL_PLAT_H_
#define XPM_IOCTL_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_DEV_USB(DeviceId)			((PM_DEV_USB_0 == (DeviceId)) || (PM_DEV_USB_1 == (DeviceId)))

maybe_unused static XStatus XPm_ValidateDeviceId(const pm_ioctl_id IoctlId, const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	if (((IOCTL_GET_RPU_OPER_MODE == IoctlId) ||
	    (IOCTL_SET_RPU_OPER_MODE == IoctlId) ||
	    (IOCTL_RPU_BOOT_ADDR_CONFIG == IoctlId)) &&
	    ((PM_DEV_RPU_A_0 > DeviceId) ||
	    (PM_DEV_RPU_B_1 < DeviceId))) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	if (((IOCTL_GET_APU_OPER_MODE == IoctlId) ||
	    (IOCTL_SET_APU_OPER_MODE == IoctlId)) &&
	    ((PM_DEV_ACPU_0_0 > DeviceId) ||
	    (PM_DEV_ACPU_3_3 < DeviceId))) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* SDIO1 is EMMC and there no DLL_RESET for SDIO1 */
	if ((IOCTL_SD_DLL_RESET == IoctlId) &&
	    (PM_DEV_SDIO_0 != DeviceId)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

maybe_unused static inline XStatus XPm_GetQos(const u32 DeviceId, pm_ioctl_id IoctlId, u32 *Response)
{
	(void)DeviceId;
	(void)IoctlId;
	(void)Response;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPm_AieOperation(u32 SubsystemId, u32 Id,
						    pm_ioctl_id IoctlId, u32 Part, u32 Ops)
{
	(void)SubsystemId;
	(void)Id;
	(void)IoctlId;
	(void)Part;
	(void)Ops;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPm_AieISRClear(u32 SubsystemId, u32 AieDeviceId, u32 Value)
{
	(void)SubsystemId;
	(void)AieDeviceId;
	(void)Value;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write)
{
	(void)DeviceId;
	(void)Arg1;
	(void)Value;
	(void)Response;
	(void)Write;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config)
{
	(void)DeviceId;
	(void)Config;

	return XPM_ERR_IOCTL;
}
maybe_unused static u32 XPm_GetUsbPwrReqOffset(const u32 DeviceId)
{
	u32 PwrReqOffset;

	if (PM_DEV_USB_0 == DeviceId) {
		PwrReqOffset = XPM_USB0_PWR_REQ_OFFSET;
	} else {
		PwrReqOffset = XPM_USB1_PWR_REQ_OFFSET;
	}

	return PwrReqOffset;
}
maybe_unused static u32 XPm_GetUsbCurrPwrOffset(const u32 DeviceId)
{
	u32 CurrPwrOffset;

	if (PM_DEV_USB_0 == DeviceId) {
		CurrPwrOffset = XPM_USB0_CUR_PWR_OFFSET;
	} else {
		CurrPwrOffset = XPM_USB1_CUR_PWR_OFFSET;
	}

	return CurrPwrOffset;
}
maybe_unused static inline XStatus XPmAccess_ReadReg(u32 SubsystemId, u32 DeviceId,
			  pm_ioctl_id IoctlId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType)
{
	(void)SubsystemId;
	(void)DeviceId;
	(void)IoctlId;
	(void)Offset;
	(void)Count;
	(void)Response;
	(void)CmdType;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPmAccess_MaskWriteReg(u32 SubsystemId, u32 DeviceId,
			       pm_ioctl_id IoctlId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType)
{
	(void)SubsystemId;
	(void)DeviceId;
	(void)IoctlId;
	(void)Offset;
	(void)Mask;
	(void)Value;
	(void)CmdType;

	return XPM_ERR_IOCTL;
}
maybe_unused static inline XStatus XPmIoctl_IsOperationAllowed(u32 RegNum, u32 SubsystemId,
		const u32 *Perms, u32 Type, u32 CmdType)
{
	(void)RegNum;
	(void)SubsystemId;
	(void)Perms;
	(void)Type;
	(void)CmdType;

	return XST_SUCCESS;
}
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_IOCTL_PLAT_H_ */

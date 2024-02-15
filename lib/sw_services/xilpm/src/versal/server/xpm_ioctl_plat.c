/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_ioctl.h"
#include "xpm_rpucore.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_access.h"
#include "xpm_aiedevice.h"
#include "xpm_requirement.h"
#include "xpm_common.h"

XStatus XPm_AieISRClear(u32 SubsystemId, u32 AieDeviceId, u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Aie = NULL;
	u32 IntrClear = 0x0U;
	u32 IdCode = XPm_GetIdCode();
	u32 PlatformVersion = XPm_GetPlatformVersion();

	if (PM_DEV_AIE != AieDeviceId) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Aie = XPmDevice_GetById(AieDeviceId);
	if (NULL == Aie) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Only needed for XCVC1902 ES1 devices */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Check whether given subsystem has access to the device */
		Status = XPm_IsAccessAllowed(SubsystemId, AieDeviceId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
		/* Unlock the AIE PCSR register to allow register writes */
		XPm_UnlockPcsr(Aie->Node.BaseAddress);

		/* Clear ISR */
		IntrClear = Value & ME_NPI_ME_ISR_MASK;
		XPm_Out32(Aie->Node.BaseAddress + ME_NPI_ME_ISR_OFFSET, IntrClear);

		/* Re-lock the AIE PCSR register for protection */
		XPm_LockPcsr(Aie->Node.BaseAddress);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_AieOperation(u32 SubsystemId, u32 Id, pm_ioctl_id IoctlId, u32 Part, u32 Ops)
{
	XStatus Status = XST_FAILURE;
	(void)Id;
	(void)SubsystemId;

	u32 NumArgs = 4U;
	u32 ArgBuf[4U];
	ArgBuf[0] = Id;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Part;
	ArgBuf[3] = Ops;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs,
				    (u32)NO_HEADER_CMDTYPE, NULL);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	/* To-Do: Add Permission Check */

	Status = Aie_Operations(Part, Ops);

done:
	return Status;
}

XStatus XPm_GetQos(const u32 DeviceId, pm_ioctl_id IoctlId, u32 *Response)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;

	u32 NumArgs = 2U;
	u32 ArgBuf[2U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs,
				    (u32)NO_HEADER_CMDTYPE, Response);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if (IS_DEV_AIE(DeviceId)) {
		XPmAieDevice_QueryDivider(Device, Response);
		Status = XST_SUCCESS;
	} else {
		/* Device not supported */
		Status = XST_INVALID_PARAM;
	}

done:
	return Status;
}

static XStatus XPmIoctl_IsRegRequested(u32 SubsystemId, u32 Register, u32 Type)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId;
	u32 NodeClass = (u32)XPM_NODECLASS_DEVICE;
	u32 NodeSubClass = (u32)XPM_NODESUBCL_DEV_PERIPH;
	u32 RegNum = Register;

	switch (Type) {
		case (u32)XPM_NODETYPE_DEV_PGGS:
			/*
			 * +1 is needed as the first PGGS Node ID is after the last
			 * GGS Node ID.
			 */
			RegNum += (u32)GGS_MAX + 1U;
			break;
		case (u32)XPM_NODETYPE_DEV_GGS:
			break;
		default:
			Status = XPM_INVALID_TYPEID;
			break;
	}

	if (XPM_INVALID_TYPEID == Status) {
		goto done;
	}

	DeviceId = NODEID(NodeClass, NodeSubClass, Type, RegNum);

	if (NULL == XPmDevice_GetById(DeviceId)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = XPmDevice_FindRequirement(DeviceId, SubsystemId);
	if ((NULL == Reqm) || (1U != Reqm->Allocated)) {
		Status = XPM_ERR_DEVICE_STATUS;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmIoctl_IsOperationAllowed(u32 RegNum, u32 SubsystemId,
		const u32 *Perms, u32 Type, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 PermissionMask = 0;
	u32 SecureMask;
	u32 NonSecureMask;

	/*
	 * RegNum is validated later in each operation so do not need to
	 * validate here in case of PMC subsystem.
	 */
	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Other subsystems have RegNum validated in this function. */
	Status = XPmIoctl_IsRegRequested(SubsystemId, RegNum, Type);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PermissionMask = Perms[RegNum];
	SecureMask = ((u32)1U << SUBSYS_TO_S_BITPOS(SubsystemId));
	NonSecureMask = ((u32)1U << SUBSYS_TO_NS_BITPOS(SubsystemId));

	/* Have Target check if Host can enact the operation */
	if ((XPLMI_CMD_SECURE == CmdType) &&
			(SecureMask == (PermissionMask & SecureMask))) {
		Status = XST_SUCCESS;
	} else if (NonSecureMask == (PermissionMask & NonSecureMask)) {
		Status = XST_SUCCESS;
	} else {
		Status = XPM_PM_NO_ACCESS;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL read secondary SLR temperature data
 *
 * @param  DeviceId: Device Id (Sysmon node ID + SLR number)
 * @param  IoctlId: IOCTL Id
 * @param  Offset: Local sysmon min/max temperature register offset
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPm_GetSsitTemp(u32 DeviceId, pm_ioctl_id IoctlId,
			u32 Offset, u32 *const Response)
{
	XStatus Status = XST_FAILURE;
	u32 DataIn = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 NumArgs = 3U;
	u32 ArgBuf[3];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Offset;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs, (u32)NO_HEADER_CMDTYPE, Response);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	/* Check if offset is valid */
	if ((PMC_SYSMON_DEVICE_TEMP_MIN_OFFSET != Offset) &&
	    (PMC_SYSMON_DEVICE_TEMP_MAX_OFFSET != Offset)) {
		DbgErr = XPM_INT_ERR_INVALID_ADDR;
		Status = XST_FAILURE;
		goto done;
	}

	/* Read temperature data */
	DataIn = XPm_In32(PMC_SYSMON_BASEADDR + Offset);
	*Response = DataIn;

	Status = XST_SUCCESS;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

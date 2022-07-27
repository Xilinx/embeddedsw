/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_subsystem.h"
#include "xpm_subsystem_plat.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_pin.h"
#include "xpm_regs.h"
#include "xpm_rpucore.h"
#include "xpm_notifier.h"
#include "xpm_requirement.h"

XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType)
{
	const XPm_Subsystem *TargetSubsystem = XPmSubsystem_GetById(TargetId);
	u32 PermissionMask = 0;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XStatus Status = XST_FAILURE;
	u32 SecureMask;
	u32 NonSecureMask;

	if ((PM_SUBSYS_PMC == TargetId) && (PM_SUBSYS_PMC != HostId)) {
		Status = XPM_PM_NO_ACCESS;
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		goto done;
	} else if (PM_SUBSYS_PMC == HostId) {
		Status = XST_SUCCESS;
		goto done;
	} else {
		/* Required by MISRA */
	}

	if ((NULL == TargetSubsystem)) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		goto done;
	}

	switch (Operation)
	{
		case SUB_PERM_WAKE_MASK:
			PermissionMask = TargetSubsystem->Perms.WakeupPerms;
			Status = XST_SUCCESS;
			break;
		case SUB_PERM_PWRDWN_MASK:
			PermissionMask = TargetSubsystem->Perms.PowerdownPerms;
			Status = XST_SUCCESS;
			break;
		case SUB_PERM_SUSPEND_MASK:
			PermissionMask = TargetSubsystem->Perms.SuspendPerms;
			Status = XST_SUCCESS;
			break;
		default:
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	SecureMask = ((u32)1U << SUBSYS_TO_S_BITPOS(HostId));
	NonSecureMask = ((u32)1U << SUBSYS_TO_NS_BITPOS(HostId));

	/* Have Target check if Host can enact the operation */
	if ((XPLMI_CMD_SECURE == CmdType) &&
	    (SecureMask == (PermissionMask & SecureMask))) {
			Status = XST_SUCCESS;
	} else if (NonSecureMask == (PermissionMask & NonSecureMask)) {
		Status = XST_SUCCESS;
	} else {
		/* Required by MISRA */
		Status = XPM_PM_NO_ACCESS;
		DbgErr = XPM_INT_ERR_SUBSYS_ACCESS;
	}

done:
	if (XST_SUCCESS != Status) {
		XPm_PrintDbgErr(Status, DbgErr);
	}
	return Status;
}

/*
 * Handle the healthy boot notification from the subsystem
 */
XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	u32 Idx;

	for (Idx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
			Idx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; Idx++) {
		/*
		 * Iterate through available Healthy Boot Monitor nodes
		 * and release it, if it is part of the given subsystem
		 */
		Device = XPmDevice_GetHbMonDeviceByIndex(Idx);
		if ((NULL != Device) &&
			(NULL != XPmDevice_FindRequirement(Device->Node.Id, SubsystemId))) {
			Status = XPmDevice_Release(SubsystemId, Device->Node.Id,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPm_PinCheckPermission(const XPm_Subsystem *Subsystem, u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinNode *Pin;
        const XPm_Device *Device = NULL;
        u32 DevId;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Pin = XPmPin_GetById(NodeId);
	if (NULL == Pin) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	if ((u8)XPM_PINSTATE_UNUSED == Pin->Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * Note: XPmDevice_GetByIndex() assumes that the caller
	 * is responsible for validating the Node ID attributes
	 * other than node index.
	 */
	Device = XPmDevice_GetByIndex(Pin->PinFunc->DevIdx);
	if (NULL == Device) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	DevId = Device->Node.Id;
	if (((u8)XPM_PINSTATE_UNUSED == Pin->Node.State) || (0U == DevId)) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmDevice_CheckPermissions(Subsystem, DevId);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PIN_PERMISSION;
		goto done;
	}

done:
        XPm_PrintDbgErr(Status, DbgErr);
        return Status;

}

/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
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

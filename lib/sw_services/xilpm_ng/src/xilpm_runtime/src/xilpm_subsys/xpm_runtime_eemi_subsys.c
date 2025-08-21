/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xstatus.h"
#include "xplmi.h"
#include "xpm_subsystem.h"
#include "xpm_alloc.h"
#include "xpm_requirement.h"
#include "xpm_runtime_device.h"
#include "xpm_runtime_clock.h"
#include "xpm_runtime_reset.h"
#include "xpm_runtime_pin.h"
#include "xplmi_scheduler.h"
#include "xpm_access.h"
#include "xpm_ioctl.h"
#include "xpm_runtime_api.h"
#include "xpm_debug.h"
#include "xpm_update.h"

static XStatus XPmSubsystem_Generic_Activate(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_SetState(XPm_Subsystem *Subsystem, u32 State);
static XStatus XPmSubsystem_Generic_GetStatus(XPm_Subsystem *Subsystem, XPm_DeviceStatus *const DeviceStatus);
static XStatus XPmSubsystem_Generic_ShutDown(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_WakeUp(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_Suspend(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_Idle(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_InitFinalize(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_AddPermissions(XPm_Subsystem *Subsystem, u32 TargetId, u32 Operations);
static XStatus XPmSubsystem_Generic_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen);
static XStatus XPmSubsystem_Generic_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId);
static XStatus XPmSubsystem_Generic_StartBootTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_StopBootTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_StartRecoveryTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Generic_StopRecoveryTimer(XPm_Subsystem *Subsystem);

XPm_SubsystemMgr SubsysMgr XPM_INIT_DATA(SubsysMgr) = {
	.Subsystems = { .Root = NULL },
	.NumSubsystems = 0,
};
XPm_SubsystemOps SubsystemOpsTable[] = {
	/* Generic Subsystem Operations */
	[SUBSYS_OPS_GENERIC] = {
		.Activate = XPmSubsystem_Generic_Activate,
		.SetState = XPmSubsystem_Generic_SetState,
		.InitFinalize = XPmSubsystem_Generic_InitFinalize,
		.GetStatus = XPmSubsystem_Generic_GetStatus,
		.AddRequirement = XPmSubsystem_Generic_AddRequirement,
		.ShutDown = XPmSubsystem_Generic_ShutDown,
		.WakeUp = XPmSubsystem_Generic_WakeUp,
		.Suspend = XPmSubsystem_Generic_Suspend,
		.Idle = XPmSubsystem_Generic_Idle,
		.IsAccessAllowed = XPmSubsystem_Generic_IsAccessAllowed,
		.StartBootTimer = XPmSubsystem_Generic_StartBootTimer,
		.StopBootTimer = XPmSubsystem_Generic_StopBootTimer,
		.StartRecoveryTimer = XPmSubsystem_Generic_StartRecoveryTimer,
		.StopRecoveryTimer = XPmSubsystem_Generic_StopRecoveryTimer,
	},
};

static XStatus XPmSubsystem_Generic_StartBootTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_StopBootTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_StartRecoveryTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_StopRecoveryTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId)
{
	XStatus Status = XST_FAILURE;

	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	if (Subsystem->Id == PM_SUBSYS_PMC) {
		Status = XST_SUCCESS;
		goto done;
	}

	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_POWER:
		/* TODO: Check if an implementation is needed for this case */
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		Status = XPmClock_CheckPermissions(NODEINDEX(Subsystem->Id), NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XPmReset_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_STMIC:
		Status = XPmPin_CheckPerms(Subsystem->Id, NodeId);
		break;
	default:
		/* XXX - Not implemented yet. */
		break;
	}
done:
	if (Status != XST_SUCCESS) {
		PmErr("Permission denied 0x%x, Subsys 0x%x NodeId 0x%x\r\n",
			Status, Subsystem->Id, NodeId);
	}
	return Status;
}

static XStatus XPmSubsystem_Generic_AddPermissions(XPm_Subsystem *Host, u32 TargetId, u32 Operations)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Target = XPmSubsystem_GetById(TargetId);
	if (NULL == Target) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if ((NULL == Target) || (NULL == Host) ||
	    (PM_SUBSYS_PMC == Host->Id) || (PM_SUBSYS_PMC == Target->Id) ||
	    (PM_SUBSYS_ASU == Host->Id) || (PM_SUBSYS_ASU == Target->Id)) {
		goto done;
	}
	/*
	 * For each operation update permission for secure and non secure
	 * requests.
	 */
	Target->Perms.WakeupPerms       |= PERM_BITMASK(Operations,
							SUB_PERM_WAKE_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));
	Target->Perms.PowerdownPerms    |= PERM_BITMASK(Operations,
							SUB_PERM_PWRDWN_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));
	Target->Perms.SuspendPerms      |= PERM_BITMASK(Operations,
							SUB_PERM_SUSPEND_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));

	Target->Perms.WakeupPerms       |= PERM_BITMASK(Operations,
							SUB_PERM_WAKE_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Target->Perms.PowerdownPerms    |= PERM_BITMASK(Operations,
							SUB_PERM_PWRDWN_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Target->Perms.SuspendPerms      |= PERM_BITMASK(Operations,
							SUB_PERM_SUSPEND_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmSubsystem_Generic_InitFinalize(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_Idle(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_Suspend(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_WakeUp(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Generic_ShutDown(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId = 0U;

	if ((u32)POWERED_OFF == Subsystem->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Power down the cores */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;
		PmDbg("Reqm: DeviceId: 0x%x Allocated: %d\r\n", DeviceId, Reqm->Allocated);
		if ((1U == Reqm->Allocated) && ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId))) {
			Status = XPmCore_ForcePwrDwn(DeviceId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* Idle the subsystem */
	Status = XPmSubsystem_Idle(Subsystem);
	if(XST_SUCCESS != Status) {
		goto done;
	}
	Subsystem->Flags &= (u8)(~SUBSYSTEM_IS_CONFIGURED);

	/* Release the resources */
	Status = XPmSubsystem_ForceDownCleanup(Subsystem->Id);
	if(XST_SUCCESS != Status) {
		goto done;
	}
	/* Clear the pending suspend cb reason */
	Subsystem->PendCb.Reason = 0U;

	Status = XPmSubsystem_SetState(Subsystem, (u32)POWERED_OFF);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("Subsystem 0x%x shutdown failed\r\n", Subsystem->Id);
	} else {
		PmInfo("Subsystem 0x%x shutdown successful\r\n", Subsystem->Id);
	}

	return Status;
}

static XStatus XPmSubsystem_Generic_SetState(XPm_Subsystem *Subsystem, u32 State)
{
	XStatus Status = XST_FAILURE;

	if (((u32)MAX_STATE <= State) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem->State = (u16)State;
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmSubsystem_Generic_GetStatus(XPm_Subsystem *Subsystem,
				      XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;

	if ((NULL == Subsystem)|| (NULL == DeviceStatus)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	DeviceStatus->Status = Subsystem->State;
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmSubsystem_Add(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_SubsystemList *SubsysList = &SubsysMgr.Subsystems;
	XPm_Subsystem *Subsystem = NULL;

	/* Check if subsystem already exists */
	if (NULL != XPmSubsystem_GetById(SubsystemId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	/* Create subsystem */
	Subsystem = (XPm_Subsystem *)XPm_AllocBytesSubsys(sizeof(XPm_Subsystem));
	if (NULL == Subsystem) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Initialize subsystem */
	Subsystem->Ops = &SubsysMgr.SubsysOps[SUBSYS_OPS_GENERIC];
	Subsystem->Id = SubsystemId;
	Subsystem->IpiMask = 0;
	Subsystem->State = ONLINE;
	Subsystem->AllSubsystems = SubsysList;	/* Pointer to the list */
	Subsystem->Requirements = Make_XPm_RequirementList();
	if (NULL == Subsystem->Requirements){
		PmErr("There's no space to allocate RequirementList");
		goto done;
	}

	/* Add the subsystem to the list */
	LIST_PREPEND(SubsysList, Subsystem);
	SubsysMgr.NumSubsystems++;

	Status = XST_SUCCESS;
done:
	return Status;
}

XPm_Subsystem * XPmSubsystem_GetById(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem = NULL;
	XPm_SubsystemList* PmSubsystems = &SubsysMgr.Subsystems;

	if ((INVALID_SUBSYSID == SubsystemId) ||
	    (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId))) {
		goto done;
	}

	LIST_FOREACH(PmSubsystems, SsNode) {
		if (SsNode->Data->Id == SubsystemId) {
			Subsystem = SsNode->Data;
			break;
		}
	}

done:
	return Subsystem;
}


static XStatus XPmSubsystem_Generic_Activate(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;
	u32 DeviceId;

	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Consider request as success if subsystem is already configured */
	if (IS_SUBSYS_CONFIGURED(Subsystem->Flags)) {
		PmInfo("Subsystem 0x%x is already configured\r\n", Subsystem->Id);
		Status = XST_SUCCESS;
		goto done;
	}

	/* Set subsystem to online if powered off */
	if (Subsystem->State == (u16)POWERED_OFF) {
		Subsystem->State = (u16)ONLINE;
	}
	PmDbg("Configuring Subsystem: 0x%x, reqlist\r\n", Subsystem->Id, Subsystem->Requirements);
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		if (NULL == ReqmNode || NULL == ReqmNode->Data) {
			PmErr("ReqmNode is NULL\r\n");
			Status = XST_FAILURE;
			goto done;
		}
		Reqm = ReqmNode->Data;

		if ((Reqm->Allocated != 1) && (1U == PREALLOC((u32)Reqm->Flags))) {
			if (Reqm->Device == NULL) {
				PmErr("Reqm->Device is NULL\r\n");
				Status = XST_FAILURE;
				goto done;
			}
			DeviceId = Reqm->Device->Node.Id;
			Status = XPmDevice_Request(Subsystem->Id, DeviceId,
						   Reqm->PreallocCaps,
						   Reqm->PreallocQoS,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				PmErr("Requesting prealloc device 0x%x failed.\n\r", DeviceId);
				Status = XPM_ERR_DEVICE_REQ;
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;
	Subsystem->Flags |= SUBSYSTEM_IS_CONFIGURED;

done:
	return Status;

}


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
		/* Only PMC subsystem can perform operations on itself */
		Status = XPM_PM_NO_ACCESS;
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		goto done;
	} else if (PM_SUBSYS_PMC == HostId) {
		/* If PMC Subsystem is the host, it can perform any operation */
		Status = XST_SUCCESS;
		goto done;
	} else if ((PM_SUBSYS_DEFAULT == TargetId) || (PM_SUBSYS_DEFAULT == HostId)) {
		/* Calling this function when default subsystem is an error! */
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
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

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device = XPmDevice_GetById(NodeId);
	u32 CoreSubsystemId;

	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (SubsystemId == PM_SUBSYS_PMC || SubsystemId == PM_SUBSYS_DEFAULT) {
		/* PMC can wake up any Node */
		Status = XST_SUCCESS;
		goto done;
	}
	switch (NODECLASS(NodeId))
	{
		case (u32)XPM_NODECLASS_SUBSYSTEM:
			/* Check that request wakeup is not for self */
			if (SubsystemId == NodeId) {
				Status = XST_INVALID_PARAM;
				break;
			}
			if (NULL == XPmSubsystem_GetById(NodeId)) {
				Status = XPM_INVALID_SUBSYSID;
				break;
			}
			Status = XPmSubsystem_IsOperationAllowed(SubsystemId,
								 NodeId,
								 SUB_PERM_WAKE_MASK,
								 CmdType);
			if (XST_SUCCESS != Status) {
				Status = XPM_PM_NO_ACCESS;
			}

			break;
		case (u32)XPM_NODECLASS_DEVICE:
			if (((u32)XPM_NODECLASS_DEVICE != NODECLASS(NodeId)) ||
			    ((u32)XPM_NODESUBCL_DEV_CORE != NODESUBCLASS(NodeId)))
			{
				Status = XST_INVALID_PARAM;
				break;
			}
			/* Validate core before querying access */
			if (NULL == Device) {
				Status = XST_INVALID_PARAM;
				break;
			}
			CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore(Device);
			if (INVALID_SUBSYSID == CoreSubsystemId) {
				Status = XST_INVALID_PARAM;
				break;
			} else if ((PM_SUBSYS_PMC != CoreSubsystemId) &&
				   (CoreSubsystemId == SubsystemId)) {
				   Status = XST_SUCCESS;
				   goto done;
			} else {
				/* Required by MISRA */
			}
			Status = XPmSubsystem_IsOperationAllowed(SubsystemId,
								 CoreSubsystemId,
								 SUB_PERM_WAKE_MASK,
								 CmdType);
			if (XST_SUCCESS != Status) {
				Status = XPM_PM_NO_ACCESS;
				break;
			}

			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}


XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		if (NULL == XPmSubsystem_GetById(NodeId)) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/* Check that force powerdown is not for self or PMC/ASU subsystem */
		if ((SubsystemId == NodeId) ||
		    (PM_SUBSYS_PMC == NodeId) ||
		    (PM_SUBSYS_ASU == NodeId) ||
		    (PM_SUBSYS_DEFAULT == NodeId)) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Status = XPmSubsystem_IsOperationAllowed(SubsystemId, NodeId,
							 SUB_PERM_PWRDWN_MASK,
							 CmdType);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else {
		/* Force powerdown of only Subsystem is allowed */
		PmErr("ForcePowerdown support for processor and power domain is deprecated.\r\n");
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_AddDevRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId = 0U, DeviceId = 0U, Flags = 0U;
	u32 PreallocCaps = 0U, PreallocQoS = 0U;
	XPm_Device *Device = NULL;

	/* Check the minimum basic arguments required for this command */
	if (6U > PayloadLen) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Parse the basic arguments */
	SubsysId = Subsystem->Id;
	DeviceId = Payload[1];
	Flags = Payload[2];
	(void)Payload[3]; /* Reserved */
	PreallocCaps = Payload[4];
	PreallocQoS = Payload[5];

	/* Device must be present in the topology at this point */
	Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = XPmRequirement_Add(Subsystem, Device, Flags, PreallocCaps, PreallocQoS);

	PmDbg("SubsysId: 0x%x, DeviceId: 0x%x, Flags: 0x%x, PreallocCaps: 0x%x, PreallocQoS: 0x%x\n\r",
		SubsysId, DeviceId, Flags, PreallocCaps, PreallocQoS);

done:
	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, DeviceId: 0x%x, Status: 0x%x\n\r", SubsysId, DeviceId, Status);
	}
	return Status;
}

static XStatus XPm_AddGgsPggsRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId = 0U, DeviceId = 0U, Permissions = 0U;
	XPm_Device *Device = NULL;
	/* Preallocate the GGS and PGGS */
	u32 Flags = REQUIREMENT_FLAGS(1U, (u32)REQ_ACCESS_SECURE_NONSECURE, (u32)REQ_NO_RESTRICTION);
	u32 PreallocCaps = (u32)PM_CAP_ACCESS;
	u32 PreallocQoS = XPM_DEF_QOS;

	/* Check the minimum basic arguments required for this command */
	if (3U > PayloadLen) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Parse the basic arguments */
	SubsysId = Subsystem->Id;
	DeviceId = Payload[1];
	Permissions = Payload[2];

	/* Device must be present in the topology at this point */
	Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Add the GGS and PGGS permission to the subsystem */
	Status = XPmIoctl_AddRegPermission(Subsystem, DeviceId, Permissions);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Preallocate the GGS and PGGS */
	Status = XPmRequirement_Add(Subsystem, Device, Flags, PreallocCaps, PreallocQoS);

	PmDbg("SubsysId: 0x%x, DeviceId: 0x%x, Flags: 0x%x, PreallocCaps: 0x%x, PreallocQoS: 0x%x\n\r",
		SubsysId, DeviceId, Flags, PreallocCaps, PreallocQoS);

done:
	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, DeviceId: 0x%x, Status: 0x%x\n\r", SubsysId, DeviceId, Status);
	}
	return Status;
}

static XStatus XPm_AddRstRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId = 0U, ResetId = 0U, Flags = 0U;
	XPm_ResetNode *Reset = NULL;

	/* Check the minimum basic arguments required for this command */
	if (3U > PayloadLen) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Parse the basic arguments */
	SubsysId = Subsystem->Id;
	ResetId = Payload[1];
	Flags = Payload[2];

	/* Device must be present in the topology at this point */
	Reset = (XPm_ResetNode *)XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = XPmReset_AddPermission(Reset, Subsystem, Flags);

	PmDbg("SubsysId: 0x%x, ResetId: 0x%x, Flags: 0x%x\n\r",
		SubsysId, ResetId, Flags);

done:
	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, ResetId: 0x%x, Status: 0x%x\n\r", SubsysId, ResetId, Status);
	}
	return Status;
}

static XStatus XPm_AddSubsysRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId = 0U, TargetSubsysId = 0U, Operations = 0U;

	/* Check the minimum basic arguments required for this command */
	if (3U > PayloadLen) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Parse the basic arguments */
	SubsysId = Subsystem->Id;
	TargetSubsysId = Payload[1];
	Operations = Payload[2];

	Status = XPmSubsystem_Generic_AddPermissions(Subsystem, TargetSubsysId, Operations);

	PmDbg("SubsysId: 0x%x, NodeId: 0x%x, Flags: 0x%x\n\r",
		SubsysId, TargetSubsysId, Operations);

done:
	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, TargetSubsysId: 0x%x, Status: 0x%x\n\r", SubsysId, TargetSubsysId, Status);
	}
	return Status;
}

static XStatus XPmSubsystem_Generic_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId = Payload[1];
	u32 NodeType = NODETYPE(NodeId);

	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		if (NodeType == (u32)XPM_NODETYPE_DEV_GGS ||
		    NodeType == (u32)XPM_NODETYPE_DEV_PGGS) {
			Status = XPm_AddGgsPggsRequirement(Subsystem, Payload, PayloadLen);
			break;
		}
		Status = XPm_AddDevRequirement(Subsystem, Payload, PayloadLen);
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XPm_AddRstRequirement(Subsystem, Payload, PayloadLen);
		break;
	case (u32)XPM_NODECLASS_SUBSYSTEM:
		Status = XPm_AddSubsysRequirement(Subsystem, Payload, PayloadLen);
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Status = XPmAccess_AddRegnodeRequirement(Subsystem->Id, NodeId);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, NodeId: 0x%x, Status: 0x%x\n\r", Subsystem->Id, NodeId, Status);
	}
	return Status;
}

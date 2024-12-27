/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xpm_runtime_api.h"

#define SUBSYS_OPS_GENERIC	0U
static XStatus XPmSubsystem_Activate(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_SetState(XPm_Subsystem *Subsystem, u32 State);
static XStatus XPmSubsystem_GetStatus(XPm_Subsystem *Subsystem, XPm_DeviceStatus *const DeviceStatus);
static XStatus XPmSubsystem_ShutDown(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_WakeUp(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Suspend(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_Idle(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_InitFinalize(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_AddPermissions(XPm_Subsystem *Subsystem, u32 TargetId, u32 Operations);
static XStatus XPmSubsystem_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId);
static XStatus XPmSubsystem_StartBootTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_StopBootTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_StartRecoveryTimer(XPm_Subsystem *Subsystem);
static XStatus XPmSubsystem_StopRecoveryTimer(XPm_Subsystem *Subsystem);

XPm_SubsystemMgr SubsysMgr = {
	.Subsystems = { .Root = NULL },
	.NumSubsystems = 0,
	.SubsysOps[SUBSYS_OPS_GENERIC] = {
		.Activate = XPmSubsystem_Activate,
		.SetState = XPmSubsystem_SetState,
		.InitFinalize = XPmSubsystem_InitFinalize,
		.GetStatus = XPmSubsystem_GetStatus,
		.AddPermissions = XPmSubsystem_AddPermissions,
		.ShutDown = XPmSubsystem_ShutDown,
		.WakeUp = XPmSubsystem_WakeUp,
		.Suspend = XPmSubsystem_Suspend,
		.Idle = XPmSubsystem_Idle,
		.IsAccessAllowed = XPmSubsystem_IsAccessAllowed,
		.StartBootTimer = XPmSubsystem_StartBootTimer,
		.StopBootTimer = XPmSubsystem_StopBootTimer,
		.StartRecoveryTimer = XPmSubsystem_StartRecoveryTimer,
		.StopRecoveryTimer = XPmSubsystem_StopRecoveryTimer,
	},
};

static XStatus XPmSubsystem_StartBootTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_StopBootTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_StartRecoveryTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_StopRecoveryTimer(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId)
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
		PmErr("Permission denied, Subsys 0x%x NodeId 0x%x\r\n", Subsystem->Id, NodeId);
	}
	return Status;
}

static XStatus XPmSubsystem_AddPermissions(XPm_Subsystem *Host, u32 TargetId, u32 Operations)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Target = XPmSubsystem_GetById(TargetId);
	if (NULL == Target) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if ((NULL == Target) || (NULL == Host) ||
	    (PM_SUBSYS_PMC == Host->Id) || (PM_SUBSYS_PMC == Target->Id)) {
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

static XStatus XPmSubsystem_InitFinalize(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Idle(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_Suspend(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_WakeUp(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;

	return XST_SUCCESS;
}

static XStatus XPmSubsystem_ShutDown(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId = 0U;
	u32 Ack = 0U;
	u32 IpiMask = 0U;
	u32 NodeState = 0U;

	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Ack = Subsystem->FrcPwrDwnReq.AckType;
	IpiMask = Subsystem->FrcPwrDwnReq.InitiatorIpiMask;
	NodeState = Subsystem->State;

	if ((u32)POWERED_OFF == Subsystem->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	LIST_FOREACH(Subsystem->Requirements, ReqmNode){
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;
/**
		 * PSM is required to be up when any application processor is
		 * running. In case of default subsystem, PSM is part of
		 * pre-alloc. So PSM might be powered down during force power
		 * down of subsystem. Currently there is no user option to
		 * force power down default subsystem because every processor
		 * is part of default subsystem and we don't allow force power
		 * down of own subsystem. However, if we want to use
		 * XPmSubsystem_ForcePwrDwn() from other cases (e.g. subsystem
		 * restart) then PSM power down will happen. So skip PSM power
		 * down from XPmSubsystem_ForcePwrDwn().
		 */
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId)) &&
		    (DeviceId != PM_DEV_PSM_PROC)) {
			Status = XPmCore_ForcePwrDwn(DeviceId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* Idle the subsystem */
	Status = Subsystem->Ops->Idle(Subsystem);
	if(XST_SUCCESS != Status) {
		Status = XPM_ERR_SUBSYS_IDLE;
		goto done;
	}

	Subsystem->Flags &= (u8)(~SUBSYSTEM_IS_CONFIGURED);

	Status = XPmSubsystem_ForceDownCleanup(Subsystem->Id);
	if(XST_SUCCESS != Status) {
		Status = XPM_ERR_CLEANUP;
		goto done;
	}

	/* Clear the pending suspend cb reason */
	Subsystem->PendCb.Reason = 0U;

	Status = Subsystem->Ops->SetState(Subsystem, (u32)POWERED_OFF);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	NodeState = Subsystem->State;
	Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_XILPM_ID,
					   XPm_ForcePwrDwnCb, 0U,
					   (void *)Subsystem->Id);
	if (XST_SUCCESS != Status) {
		PmDbg("Task not present\r\n");
		Status = XST_SUCCESS;
	}

done:
	XPm_ProcessAckReq(Ack, IpiMask, Status, Subsystem->Id, NodeState);

	return Status;
}

static XStatus XPmSubsystem_SetState(XPm_Subsystem *Subsystem, u32 State)
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

static XStatus XPmSubsystem_GetStatus(XPm_Subsystem *Subsystem,
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


static XStatus XPmSubsystem_Activate(XPm_Subsystem *Subsystem)
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
	PmInfo("!!!! Configuring Subsystem: 0x%x reqlist = %x %x\r\n", Subsystem->Id, (u32)Subsystem->Requirements, Subsystem->Requirements->Root);
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
	PmInfo("End Subsystem Configure Status: 0x%x\r\n", Status);
	return Status;

}


XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType)
{
	(void)HostId;
	(void)TargetId;
	(void)Operation;
	(void)CmdType;

	return XST_SUCCESS;
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

	if (SubsystemId == PM_SUBSYS_PMC) {
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
	const XPm_Device *Device = XPmDevice_GetById(NodeId);
	u32 CoreSubsystemId;

	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		if (NULL == XPmSubsystem_GetById(NodeId)) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/* Check that force powerdown is not for self or PMC subsystem */
		if ((SubsystemId == NodeId) || (PM_SUBSYS_PMC == NodeId)) {
			goto done;
		}
		Status = XPmSubsystem_IsOperationAllowed(SubsystemId, NodeId,
							 SUB_PERM_PWRDWN_MASK,
							 CmdType);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
		   ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(NodeId))) {
		if (NULL == Device) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore(Device);
		if (INVALID_SUBSYSID == CoreSubsystemId) {
			Status = XST_INVALID_PARAM;
			goto done;
		} else if ((PM_SUBSYS_PMC != CoreSubsystemId) &&
			   (CoreSubsystemId == SubsystemId)) {
			   Status = XST_SUCCESS;
			   goto done;
		} else {
			/* Required by MISRA */
		}

		Status = XPmSubsystem_IsOperationAllowed(SubsystemId, CoreSubsystemId,
							 SUB_PERM_PWRDWN_MASK,
							 CmdType);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else if (((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId)) &&
		   ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(NodeId))) {
		if ((PM_SUBSYS_PMC == SubsystemId) ||
		    (PM_SUBSYS_DEFAULT == SubsystemId)) {
			Status = XST_SUCCESS;
		} else {
			/*
			 * Only PMC and default subsystem can enact force power down
			 * upon power domains.
			 */
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

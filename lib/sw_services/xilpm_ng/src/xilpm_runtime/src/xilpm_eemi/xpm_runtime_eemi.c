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
	.MaxSubsystemIdx = 0,
};
const XPm_SubsystemOps SubsystemOpsTable[] = {
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

maybe_unused static XStatus XPmSubsystem_Generic_AddPermissions(XPm_Subsystem *Subsystem, u32 TargetNodeId, u32 Operations)
{
	(void)Subsystem;
	(void)TargetNodeId;
	(void)Operations;

	return XST_SUCCESS;
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
	(void)Subsystem;

	return XST_SUCCESS;
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
	if (SubsysMgr.MaxSubsystemIdx < NODEINDEX(Subsystem->Id)) {
		SubsysMgr.MaxSubsystemIdx = NODEINDEX(Subsystem->Id);
	}

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
	(void)HostId;
	(void)TargetId;
	(void)Operation;
	(void)CmdType;

	return XST_SUCCESS;
}

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;

	return XST_SUCCESS;
}


XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;

	return XST_INVALID_PARAM;
}


static XStatus XPmSubsystem_Generic_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	(void)Subsystem;
	(void)Payload;
	(void)PayloadLen;

	return XST_SUCCESS;
}

/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xplmi.h"
#include "xstatus.h"
#include "xpm_nodeid.h"
#include "xpm_device.h"
#include "xpm_requirement.h"
#include "xpm_alloc.h"
#include "xpm_update.h"
#include "xpm_runtime_device.h"
#include "xpm_runtime_clock.h"
#include "xpm_runtime_reset.h"
#include "xpm_runtime_pin.h"
#include "xpm_subsystem.h"

/*
 * ============================================================================
 * EEMI-Specific Generic Operation Implementations (Baseline stubs & common functions)
 * ============================================================================
 * These implementations are common to both EEMI-only and full subsystem builds.
 * They are non-static so they can be referenced by SubsystemOps table in
 * top subsystem manager module (xpm_subsystem.c).
 *
 * Full subsystem-specific implementations are in xpm_runtime_subsys.c
 * EEMI-only stub implementations are in xpm_runtime_eemi.c
 */

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_NotifyHealthyBoot(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_StartRecoveryTimer(XPm_Subsystem *Subsystem, u32 CmdType)
{
	(void)Subsystem;
	(void)CmdType;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_InitFinalize(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_ShutDown(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	(void)Subsystem;
	(void)Payload;
	(void)PayloadLen;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Idle(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Suspend(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_WakeUp(XPm_Subsystem *Subsystem)
{
	(void)Subsystem;
	return XST_SUCCESS;
}


__attribute__((weak, noinline))
XStatus XPmSubsystemOp_SetState(XPm_Subsystem *Subsystem, u32 State)
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

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_GetStatus(XPm_Subsystem *Subsystem,
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

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Activate(XPm_Subsystem *Subsystem)
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

	PmInfo("Subsystem 0x%x configured successfully\r\n", Subsystem->Id);

done:
	return Status;
}

__attribute__((weak, noinline))
XStatus XPmSubsystemOp_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId)
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
		if (NULL != Subsystem) {
			PmErr("Permission denied 0x%x, Subsys 0x%x NodeId 0x%x\r\n",
				Status, Subsystem->Id, NodeId);
		} else {
			PmErr("Permission denied 0x%x, Subsys NULL NodeId 0x%x\r\n",
				Status, NodeId);
		}
	}
	return Status;
}

/*
 * ============================================================================
 * Utility Functions - EEMI-Only Stub Implementations
 * ============================================================================
 */

/****************************************************************************/
/**
 * @brief  Check if host subsystem has permission to perform operation on target subsystem
 *
 * @param HostId        Host subsystem ID requesting the operation
 * @param TargetId      Target subsystem ID on which operation is requested
 * @param Operation     Operation type (wake/powerdown)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, error code if denied
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         returns success. Full permission checking implementation available
 *         in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType)
{
	(void)HostId;
	(void)TargetId;
	(void)Operation;
	(void)CmdType;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to wake up a node
 *
 * @param SubsystemId   Subsystem ID requesting wakeup
 * @param NodeId        Node ID to wake (subsystem or core device)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, error code if denied
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         returns success. Full permission checking implementation available
 *         in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to force power down a node
 *
 * @param SubsystemId   Subsystem ID requesting force powerdown
 * @param NodeId        Node ID to power down (subsystem node)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_INVALID_PARAM (stub always denies)
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         denies force powerdown requests. Full permission checking implementation
 *         available in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;
	return XST_INVALID_PARAM;
}

/****************************************************************************/
/**
 * @brief  Flush common memory (CMN) cache for subsystem
 *
 * @param SubsystemId   Subsystem ID for which to flush cache
 *
 * @return XST_SUCCESS (stub always succeeds)
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. No-op
 *         that always succeeds. Full implementation available in subsystem
 *         builds via strong symbol override in xpm_runtime_mem_subsys.c.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystem_CmnFlush(const u32 SubsystemId)
{
	(void)SubsystemId;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Get healthy boot monitor finite state machine
 *
 * @return NULL (feature not supported in EEMI-only builds)
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Prints
 *         warning and returns NULL. Full implementation available in subsystem
 *         builds via strong symbol override in xpm_runtime_hbmon_subsys.c.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XPm_Fsm *XPmSubsystem_GetHbMonFsm(void)
{
	PmWarn("Boot/recovery timer feature is not supported in EEMI\r\n");
	return NULL;
}

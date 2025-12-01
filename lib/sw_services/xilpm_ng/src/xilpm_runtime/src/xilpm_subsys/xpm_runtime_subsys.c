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
#include "xpm_notifier.h"
#include "xpm_runtime_api.h"
#include "xpm_debug.h"
#include "xpm_update.h"

/*
 * ============================================================================
 * Full Subsystem-Specific Generic Operation Implementations
 * ============================================================================
 */

/****************************************************************************/
/**
 * @brief  Notify that subsystem has booted successfully and release healthy boot monitors
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, error code on failure
 *
 * @note   Strong symbol override - full implementation for subsystem builds.
 *         Iterates through healthy boot monitor devices and releases those
 *         allocated to the subsystem.
 *
 ****************************************************************************/
XStatus XPmSubsystemOp_NotifyHealthyBoot(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceIdx;

	for (DeviceIdx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
	     DeviceIdx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; DeviceIdx++) {
		/*
		 * Iterate through available Healthy Boot Monitor nodes
		 * and release it, if it is part of the given subsystem
		 */
		Device = XPmDevice_GetHbMonDeviceByIndex(DeviceIdx);
		if (NULL == Device) {
			continue;
		}
		Reqm = XPmDevice_FindRequirement(Device->Node.Id, Subsystem->Id);
		/* Make sure to release only the currently requested healthy boot nodes */
		if ((NULL == Reqm) || (1U != Reqm->Allocated)) {
			continue;
		}
		Status = XPmDevice_Release(Subsystem->Id, Device->Node.Id,
				XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Start recovery timer for subsystem using healthy boot monitor
 *
 * @param Subsystem     Pointer to subsystem object
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND if no runtime
 *         healthy boot monitor available, error code on failure
 *
 * @note   Strong symbol override - full implementation for subsystem builds.
 *         Requests runtime healthy boot monitor device (skips pre-allocated
 *         boot-time monitors) to start recovery timer.
 *
 ****************************************************************************/
XStatus XPmSubsystemOp_StartRecoveryTimer(XPm_Subsystem *Subsystem, u32 CmdType)
{
	u32 DeviceIdx;
	XStatus Status = XST_DEVICE_NOT_FOUND;
	const XPm_Device *Device;
	const XPm_Requirement *Reqm = NULL;

	/* Request run time Healthy Boot Monitor node if it is added */
	for (DeviceIdx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
	     DeviceIdx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; DeviceIdx++) {
		Device = XPmDevice_GetHbMonDeviceByIndex(DeviceIdx);
		if (NULL == Device) {
			continue;
		}
		Reqm = XPmDevice_FindRequirement(Device->Node.Id, Subsystem->Id);
		/* Skip if boot time healthy boot monitor node found */
		if ((NULL == Reqm) || (1U == PREALLOC((u32)Reqm->Flags))) {
			continue;
		}
		Status = XPm_RequestDevice(Subsystem->Id, Device->Node.Id,
					   (u32)PM_CAP_ACCESS, Reqm->PreallocQoS,
					   0U, CmdType);
		break;
	}

	return Status;
}

/* XPmSubsystemOp_IsAccessAllowed is now a common function in xpm_subsystem.c */

static XStatus XPm_AddSubsysPermissions(XPm_Subsystem *Host, u32 TargetId, u32 Operations)
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

	Target->Perms.WakeupPerms       |= PERM_BITMASK(Operations,
							SUB_PERM_WAKE_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Target->Perms.PowerdownPerms    |= PERM_BITMASK(Operations,
							SUB_PERM_PWRDWN_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Finalize subsystem initialization after successful boot
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, error code on failure
 *
 * @note   Strong symbol override - full implementation for subsystem builds.
 *         Notifies healthy boot to stop healthy boot monitors after successful
 *         subsystem boot.
 *
 ****************************************************************************/
XStatus XPmSubsystemOp_InitFinalize(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;

	/*
	 * As the subsystem boot is successfully,
	 * notify healthy to stop healthy boot monitors
	 */
	Status = XPmSubsystem_NotifyHealthyBoot(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO: implement me */
done:
	return Status;
}

static XStatus XPmSubsystem_ForceDownCleanup(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;

	Status = XPmRequirement_Release(LIST_FIRST_DATA(Subsystem->Requirements), RELEASE_ALL);
		/* Todo: Cancel wakeup if scheduled
		 * Should be included with wakeup support
		XPm_WakeUpCancelScheduled(SubSysIdx);*/
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unregister all notifiers for this subsystem */
	Status = XPmNotifier_UnregisterAll(Subsystem);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Shutdown subsystem by idling cores, releasing resources, and powering down
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, error code on failure
 *
 * @note   Strong symbol override - full implementation for subsystem builds.
 *         Performs complete shutdown sequence: idle subsystem, release all
 *         requirements and notifiers, clear flags, and set state to POWERED_OFF.
 *
 ****************************************************************************/
XStatus XPmSubsystemOp_ShutDown(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;

	PmInfo("%s: Shutdown 0x%x (state: 0x%x)\r\n", __func__, Subsystem->Id, Subsystem->State);

	if ((u32)POWERED_OFF == Subsystem->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Idle the subsystem */
	Status = XPmSubsystem_Idle(Subsystem);
	if(XST_SUCCESS != Status) {
		goto done;
	}
	Subsystem->Flags &= (u8)(~SUBSYSTEM_IS_CONFIGURED);
	Subsystem->Flags &= (u8)(~SUBSYSTEM_IDLE_CB_IS_SENT);

	/* Release the resources */
	Status = XPmSubsystem_ForceDownCleanup(Subsystem);
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
		PmErr("Subsystem 0x%x shutdown failed: 0x%x\r\n", Subsystem->Id, Status);
	} else {
		PmInfo("Subsystem 0x%x shutdown successful, state: 0x%x\r\n",
				Subsystem->Id, Subsystem->State);
	}

	return Status;
}

/* Helper functions for AddRequirement */
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

	Status = XPm_AddSubsysPermissions(Subsystem, TargetSubsysId, Operations);

	PmDbg("SubsysId: 0x%x, NodeId: 0x%x, Flags: 0x%x\n\r",
		SubsysId, TargetSubsysId, Operations);

done:
	if (XST_SUCCESS != Status) {
		PmErr("SubsysId: 0x%x, TargetSubsysId: 0x%x, Status: 0x%x\n\r", SubsysId, TargetSubsysId, Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add requirement (device/reset/subsystem/regnode) to subsystem
 *
 * @param Subsystem     Pointer to subsystem object
 * @param Payload       Requirement payload containing node-specific arguments
 * @param PayloadLen    Length of payload array
 *
 * @return XST_SUCCESS on success, error code on failure
 *
 * @note   Strong symbol override - full implementation for subsystem builds.
 *         Supports adding requirements for devices, resets, subsystems, and
 *         register nodes based on node class in payload.
 *
 ****************************************************************************/
XStatus XPmSubsystemOp_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId = Payload[1];

	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_DEVICE:
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

/*
 * ============================================================================
 * Utility Functions - Full Subsystem Implementations
 * ============================================================================
 */

/**
 * @brief  Target checks whether Host has permission to enact the operation
 */
static XStatus CheckTargetToHostPerm(const u32 CmdType, const u32 HostId, u32 PermissionMask)
{
	volatile XStatus Status = XPM_PM_NO_ACCESS;
	volatile u32 SecureMask = ((u32)1U << SUBSYS_TO_S_BITPOS(HostId));
	volatile u32 NonSecureMask = ((u32)1U << SUBSYS_TO_NS_BITPOS(HostId));

	if ((XPLMI_CMD_SECURE == CmdType) &&
	    (SecureMask == (PermissionMask & SecureMask))) {
			Status = XST_SUCCESS;
	} else if (NonSecureMask == (PermissionMask & NonSecureMask)) {
		Status = XST_SUCCESS;
	} else {
		/* Required by MISRA */
		Status = XPM_PM_NO_ACCESS;
	}

	return Status;
}

/**
 * @brief  Checks whether given Target Subsystem has required Permission Mask
 */
static XStatus CheckTargetPermMask(const u32 Operation, const XPm_Subsystem *TargetSubsystem,
						u32 *PermissionMask, u16 *DbgErr)
{
	volatile XStatus Status = XST_FAILURE;

	switch (Operation)
	{
		case SUB_PERM_WAKE_MASK:
			*PermissionMask = TargetSubsystem->Perms.WakeupPerms;
			Status = XST_SUCCESS;
			break;
		case SUB_PERM_PWRDWN_MASK:
			*PermissionMask = TargetSubsystem->Perms.PowerdownPerms;
			Status = XST_SUCCESS;
			break;
		default:
			*DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Check if host subsystem has permission to perform operation on target subsystem
 *
 * @param HostId        Host subsystem ID requesting the operation
 * @param TargetId      Target subsystem ID on which operation is requested
 * @param Operation     Operation type (SUB_PERM_WAKE_MASK or SUB_PERM_PWRDWN_MASK)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, XPM_PM_NO_ACCESS if denied, error code on failure
 *
 * @note   Strong symbol override - full permission checking implementation for
 *         subsystem builds. Validates permissions based on target subsystem's
 *         permission masks and command type (secure/non-secure).
 *
 ****************************************************************************/
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType)
{
	const XPm_Subsystem *TargetSubsystem = XPmSubsystem_GetById(TargetId);
	u32 PermissionMask = 0;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	volatile u32 HostIdTemp = HostId;

	if ((PM_SUBSYS_PMC == TargetId) && (PM_SUBSYS_PMC != HostId)) {
		/* Only PMC subsystem can perform operations on itself */
		Status = XPM_PM_NO_ACCESS;
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		goto done;
	} else if ((PM_SUBSYS_PMC == HostId) && (PM_SUBSYS_PMC == HostIdTemp)) {
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

	XSECURE_TEMPORAL_CHECK(done, Status, CheckTargetPermMask, Operation, TargetSubsystem, &PermissionMask, &DbgErr);

	/* Have Target check if Host can enact the operation */
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, CheckTargetToHostPerm, CmdType, HostId, PermissionMask);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		Status |= StatusTmp;
		DbgErr = XPM_INT_ERR_SUBSYS_ACCESS;
	}

done:
	if (XST_SUCCESS != Status) {
		XPm_PrintDbgErr(Status, DbgErr);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to wake up a node (subsystem or core device)
 *
 * @param SubsystemId   Subsystem ID requesting wakeup
 * @param NodeId        Node ID to wake (subsystem or core device)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, XPM_PM_NO_ACCESS if denied, error code on failure
 *
 * @note   Strong symbol override - full permission checking implementation for
 *         subsystem builds. Validates wake permissions for subsystems and core
 *         devices, checking subsystem ownership and operation permissions.
 *
 ****************************************************************************/
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

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to force power down a subsystem node
 *
 * @param SubsystemId   Subsystem ID requesting force powerdown
 * @param NodeId        Node ID to power down (must be subsystem node)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, XPM_PM_NO_ACCESS if denied, error code on failure
 *
 * @note   Strong symbol override - full permission checking implementation for
 *         subsystem builds. Validates force powerdown permissions for subsystem
 *         nodes only. Prevents force powerdown of self, PMC, ASU, or default subsystems.
 *
 ****************************************************************************/
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
		Status = XST_FEATURE_DEPRECATE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/* XPmSubsystem_CmnFlush and XPmSubsystem_GetHbMonFsm are provided by */
/* specialized files (xpm_runtime_mem_subsys.c and xpm_runtime_hbmon_subsys.c) */

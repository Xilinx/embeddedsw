/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_subsystem.h"
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

XPm_Subsystem *PmSubsystems;
static u32 MaxSubsysIdx;

XStatus XPmSubsystem_AddPermission(const XPm_Subsystem *Host,
				   XPm_Subsystem *Target,
				   const u32 Operations)
{
	XStatus Status = XST_FAILURE;

	if ((NULL == Target) || (NULL == Host) ||
	    (PM_SUBSYS_PMC == Host->Id) || (PM_SUBSYS_PMC == Target->Id)) {
		goto done;
	}

	/*
	 * For each operation update permission for secure and non secure
	 * requests.
	 */
	Target->Perms.WakeupPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_WAKE_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));
	Target->Perms.PowerdownPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_PWRDWN_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));
	Target->Perms.SuspendPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_SUSPEND_SHIFT_NS,
							SUBSYS_TO_NS_BITPOS(Host->Id));

	Target->Perms.WakeupPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_WAKE_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Target->Perms.PowerdownPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_PWRDWN_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));
	Target->Perms.SuspendPerms	|= PERM_BITMASK(Operations,
							SUB_PERM_SUSPEND_SHIFT_S,
							SUBSYS_TO_S_BITPOS(Host->Id));

	Status = XST_SUCCESS;

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

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId)
{
	const XPm_Subsystem *Subsystem;
	u32 IpiMaskVal = 0;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	IpiMaskVal = Subsystem->IpiMask;

done:
	return IpiMaskVal;
}

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	u32 SubsystemId = INVALID_SUBSYSID;
	const XPm_Subsystem *Subsystem;

	/*
	 * Subsystem with least one IPI channel
	 * will have non-zero IPI mask.
	 */
	if (0U == IpiMask) {
		goto done;
	}

	Subsystem = PmSubsystems;
	while (NULL != Subsystem) {
		if (((Subsystem->IpiMask & IpiMask) == IpiMask) &&
		    ((u8)OFFLINE != Subsystem->State)) {
			SubsystemId = Subsystem->Id;
			break;
		}
		Subsystem = Subsystem->NextSubsystem;
	}

done:
	return SubsystemId;
}

XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

        Status = XPmRequirement_Release(Subsystem->Requirements, RELEASE_ALL);
		/* Todo: Cancel wakeup if scheduled
		 * Should be included with wakeup support
		XPm_WakeUpCancelScheduled(SubSysIdx);*/

        /* Unregister all notifiers for this subsystem */
	XPmNotifier_UnregisterAll(Subsystem);

done:
        return Status;
}

/*
 * Handle the healthy boot notification from the subsystem
 */
static XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId)
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

XStatus XPmSubsystem_InitFinalize(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	const XPm_Device *Device;
	const XPm_Power *Power;
	const XPm_Requirement *Reqm;
	u32 DeviceInUse = 0;
	u32 Idx;
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem->Flags |= SUBSYSTEM_INIT_FINALIZED;

	/*
	 * As the subsystem boot is successfully,
	 * notify healthy to stop healthy boot monitors
	 */
	Status = XPmSubsystem_NotifyHealthyBoot(SubsystemId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	for (Idx = (u32)XPM_NODEIDX_DEV_MIN + 1U;
	     Idx < (u32)XPM_NODEIDX_DEV_MAX; Idx++) {
		DeviceInUse = 0;

		Device = XPmDevice_GetByIndex(Idx);
		if (NULL == Device) {
			continue;
		}

		/* Exclude device if its parent power domain is OFF */
		Power = Device->Power;
		if ((u32)XPM_NODESUBCL_POWER_ISLAND ==
		    NODESUBCLASS(Power->Node.Id)) {
			/* Get parent of island */
			Power = Power->Parent;
		}
		if (((u32)XPM_NODESUBCL_POWER_DOMAIN ==
		     NODESUBCLASS(Power->Node.Id)) &&
		    ((u8)XPM_POWER_STATE_OFF == Power->Node.State)) {
			continue;
		}

		/**
		 * NOTE: Skip for child of NOC power domain, as powering down
		 * the NOC domain may cause PLM to hang.
		 */
		if (PM_POWER_NOC == Power->Node.Id) {
			continue;
		}

		/*
		 * Exclude devices which are expected not to be requested by
		 * any subsystem but should be kept on for basic functionalities
		 * to work:
		 *	- Soc, PMC, Efuse are required for basic boot
		 *	- Ams root is required for any sysmon activities
		 *	- Usage of GTs is board dependent and used by multiple
		 *	devices so should be kept on
		 *	- Misc devices: L2 Bank 0
		 */
		if (((u32)PM_DEV_L2_BANK_0 == Device->Node.Id) ||
		    ((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_GT == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODEIDX_DEV_AMS_ROOT == NODEINDEX(Device->Node.Id))) {
			continue;
		}

		if (((u32)PM_DEV_GPIO == Device->Node.Id) &&
		    ((u32)PLATFORM_VERSION_SILICON == Platform) &&
		    ((u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))
		{
			continue;
		}

		/* Iterate over all subsystems for particular device */
		Reqm = Device->Requirements;
		while (NULL != Reqm) {
			if ((u8)OFFLINE == Reqm->Subsystem->State) {
				Reqm = Reqm->NextSubsystem;
				continue;
			}

			if ((1U == Reqm->Allocated) ||
			    (((u8)ONLINE == Reqm->Subsystem->State) &&
			     !IS_SUBSYS_INIT_FINALIZED(Reqm->Subsystem->Flags))) {
				DeviceInUse = 1;
				break;
			}

			Reqm = Reqm->NextSubsystem;
		}

		/* Power down the device if device is unused */
		if (0U == DeviceInUse) {
			/*
			 * Here device needs to be requested and released to handle
			 * the use count of its clock and power. This makes unused
			 * clock and power to be powered down.
			 */
			Status = XPmDevice_Request(PM_SUBSYS_PMC, Device->Node.Id,
						   (u32)PM_CAP_ACCESS, XPM_MAX_QOS,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			Status = XPmDevice_Release(PM_SUBSYS_PMC, Device->Node.Id,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			Status = XST_SUCCESS;
		}
	}

done:
	return Status;
}

XStatus XPmSubsystem_Idle(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	XPm_Device *Device;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_FAILURE;
                goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		Device = Reqm->Device;
		u32 Usage = XPmDevice_GetUsageStatus(Subsystem, Device);
		s32 IsClkActive = XPmDevice_IsClockActive(Device);

		/* Check if device is requested and its clock is active */
		if ((1U == Reqm->Allocated) &&
		    (0U == (Device->Node.Flags & NODE_IDLE_DONE)) &&
		    (XST_SUCCESS == IsClkActive) &&
		    ((u32)PM_USAGE_CURRENT_SUBSYSTEM == Usage)) {
			Status = XPmDevice_SoftResetIdle(Device, DEVICE_IDLE_REQ);
			if (XST_SUCCESS != Status) {
				PmErr("Node idling failed for 0x%x\r\n", Device->Node.Id);
			}
			Device->Node.Flags |= NODE_IDLE_DONE;
		}

		Reqm = Reqm->NextDevice;
	}

	Status = XST_SUCCESS;

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

/****************************************************************************/
/**
 * @brief  This function gives Subsystem from SubsystemId.
 *
 * @param SubsystemId	Subsystem ID
 *
 * @return XPm_Subsystem if successful else NULL
 *
 * @note
 *  If the ID is greater than MAX_NUM_SUBSYSTEMS+2 then the ID is  outside of
 *  supported IDs for subsystem permissions logic. The +2 is for PMC and
 *  default subsystem.
 *
 ****************************************************************************/
XPm_Subsystem * XPmSubsystem_GetById(u32 SubsystemId)
{
	XPm_Subsystem *SubSystem = NULL;

	if ((INVALID_SUBSYSID == SubsystemId) ||
	    (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId))) {
		goto done;
	}

	SubSystem = PmSubsystems;
	while (NULL != SubSystem) {
		if (SubSystem->Id == SubsystemId) {
			break;
		}
		SubSystem = SubSystem->NextSubsystem;
	}

done:
	return SubSystem;
}

/****************************************************************************/
/**
 * @brief  This function gives Subsystem from Subsystem "INDEX".
 *
 * @param SubSysIdx	Subsystem Index
 *
 * @return Pointer to XPm_Subsystem if successful else NULL
 *
 * @note
 * This is a less strict version of XPmSubsystem_GetByIndex(),
 * and mainly is implemented due to other modules such as xpm_device
 * needs to access the subsystem database and iterate over it using
 * indexes only, without the need to use the complete subsystem ID.
 * Use this function where it is absolutely necessary.
 *
 ****************************************************************************/
XPm_Subsystem *XPmSubsystem_GetByIndex(u32 SubSysIdx)
{
	XPm_Subsystem *Subsystem = PmSubsystems;

	/*
	 * We assume that Subsystem class, subclass and type have been
	 * validated before, so just validate index against bounds here
	 */
	while (NULL != Subsystem) {
		if (SubSysIdx == NODEINDEX(Subsystem->Id)) {
			break;
		}
		Subsystem = Subsystem->NextSubsystem;
	}

	return Subsystem;
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

XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_PinNode *Pin;
	const XPm_Device *Device = NULL;
	u32 DevId;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (SubsystemId == PM_SUBSYS_PMC) {
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_POWER:
		/* TODO: Check if an implementation is needed for this case */
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		Status = XPmClock_CheckPermissions(NODEINDEX(Subsystem->Id), NodeId);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_CLOCK_PERMISSION;
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XPmReset_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RESET_PERMISSION;
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_PERMISSION;
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_STMIC:
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
		break;
	default:
		/* XXX - Not implemented yet. */
		break;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);

	if (((u32)MAX_STATE <= State) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)POWERED_OFF == State) || ((u32)OFFLINE == State)) {
		Subsystem->Flags &= (u8)(~SUBSYSTEM_IS_CONFIGURED);
	}

	Subsystem->State = (u8)State;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmSubsystem_Configure(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	u32 DeviceId;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Consider request as success if subsystem is already configured */
	if (IS_SUBSYS_CONFIGURED(Subsystem->Flags)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Set subsystem to online if powered off */
	if (Subsystem->State == (u8)POWERED_OFF) {
		Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	PmDbg("Configuring Subsystem: 0x%x\r\n", SubsystemId);
	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if ((1U != Reqm->Allocated) && (1U == PREALLOC((u32)Reqm->Flags))) {
			DeviceId = Reqm->Device->Node.Id;
			Status = XPm_RequestDevice(SubsystemId, DeviceId,
						   Reqm->PreallocCaps,
						   Reqm->PreallocQoS, 0U,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				PmErr("Requesting prealloc device 0x%x failed.\n\r", DeviceId);
				Status = XPM_ERR_DEVICE_REQ;
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

	Subsystem->Flags |= SUBSYSTEM_IS_CONFIGURED;

done:
	return Status;
}

XStatus XPmSubsystem_Add(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (((u32)XPM_NODECLASS_SUBSYSTEM != NODECLASS(SubsystemId)) ||
	    ((u32)XPM_NODESUBCL_SUBSYSTEM != NODESUBCLASS(SubsystemId)) ||
	    ((u32)XPM_NODETYPE_SUBSYSTEM != NODETYPE(SubsystemId))) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/*
	 * Ensure the subsystem being added is within the range of supported
	 * subsystem IDs for the subsystem permissions logic.
	 */
	if (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId)) {
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		Status = XST_INVALID_PARAM;
		goto done;
	}


	/* Check if subsystem is being re-added. */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((NULL != Subsystem) && ((u8)OFFLINE != Subsystem->State)) {
		DbgErr = XPM_INT_ERR_SUBSYS_ADDED;
		Status = XST_FAILURE;
		goto done;
	}

	Subsystem = (XPm_Subsystem *)XPm_AllocBytes(sizeof(XPm_Subsystem));
	if (NULL == Subsystem) {
		DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Subsystem->NextSubsystem = PmSubsystems;
	Subsystem->Id = SubsystemId;
	Subsystem->PendCb.Reason = 0U;
	Subsystem->PendCb.Latency = 0U;
	Subsystem->PendCb.State = 0U;
	if (PM_SUBSYS_PMC == SubsystemId) {
		Subsystem->Flags = SUBSYSTEM_INIT_FINALIZED;
		Subsystem->IpiMask = PMC_IPI_MASK;
	} else {
		Subsystem->Flags = 0U;
		Subsystem->IpiMask = 0U;
	}
	PmSubsystems = Subsystem;

	if (NODEINDEX(SubsystemId) > MaxSubsysIdx) {
		MaxSubsysIdx = NODEINDEX(SubsystemId);
	}

	Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SUBSYS_SET_STATE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmSubsystem_Destroy(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	XPm_Device *Device;

	if (((u32)XPM_NODECLASS_SUBSYSTEM != NODECLASS(SubsystemId)) ||
	    ((u32)XPM_NODESUBCL_SUBSYSTEM != NODESUBCLASS(SubsystemId)) ||
	    ((u32)XPM_NODETYPE_SUBSYSTEM != NODETYPE(SubsystemId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State != (u8)ONLINE)) {
		Status = XST_FAILURE;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			Device = Reqm->Device;
			Status = Device->DeviceOps->Release(Device, Subsystem,
							    XPLMI_CMD_SECURE);
			if (XST_FAILURE == Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	/* Clear the pending suspend cb reason */
	Subsystem->PendCb.Reason = 0U;

	Status = XPmSubsystem_SetState(SubsystemId, (u32)OFFLINE);
done:
	return Status;
}

XStatus XPmSubsystem_GetStatus(const u32 SubsystemId, const u32 DeviceId,
			       XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	const XPm_Subsystem *Subsystem;
	const XPm_Subsystem *Target_Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	Target_Subsystem = XPmSubsystem_GetById(DeviceId);
	if ((NULL == Subsystem) || (NULL == Target_Subsystem) ||
	    (NULL == DeviceStatus)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	DeviceStatus->Status = Target_Subsystem->State;
	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

u32 XPmSubsystem_GetMaxSubsysIdx(void)
{
	return MaxSubsysIdx;
}

/****************************************************************************/
/**
 * @brief  Handler for idle subsystem and force down cleanup
 *
 * @param Subsystem	Target Subsystem
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmSubsystem_ForcePwrDwn(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId = 0U;
	u32 Ack = 0U;
	u32 IpiMask = 0U;
	u32 NodeState = 0U;

	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if ((u32)POWERED_OFF == Subsystem->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
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
		    ((u32)XPM_NODESUBCL_DEV_CORE ==
		    NODESUBCLASS(DeviceId)) &&
		    (DeviceId != PM_DEV_PSM_PROC)) {
			Status = XPmCore_ForcePwrDwn(DeviceId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	/* Idle the subsystem */
	Status = XPmSubsystem_Idle(Subsystem->Id);
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

	Status = XPmSubsystem_SetState(Subsystem->Id, (u32)POWERED_OFF);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Ack = Subsystem->FrcPwrDwnReq.AckType;
	IpiMask = Subsystem->FrcPwrDwnReq.InitiatorIpiMask;
	NodeState = Subsystem->State;
	Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_XILPM_ID,
					   XPm_ForcePwrDwnCb, 0U,
					   (void *)SubsystemId);
	if (XST_SUCCESS != Status) {
		PmDbg("Task not present\r\n");
		Status = XST_SUCCESS;
	}

done:
	XPm_ProcessAckReq(Ack, IpiMask, Status, SubsystemId, NodeState);

	return Status;
}

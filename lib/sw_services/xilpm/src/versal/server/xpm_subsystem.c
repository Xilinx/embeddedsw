/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_subsystem.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_pin.h"
#include "xpm_regs.h"
#include "xpm_rpucore.h"
#include "xpm_notifier.h"
#include "xpm_requirement.h"

static XPm_Subsystem *PmSubsystems;
static u32 MaxSubsysIdx;

/*
 * Global SubsystemId which is set and is valid during XPm_CreateSubsystem()
 */
static u32 CurrentSubsystemId = INVALID_SUBSYSID;

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem;
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
	XPm_Subsystem *Subsystem;

	/* If default subsystem is active, return default subsystem id
	  as it does not have ipi channel mapped to it.*/
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	if ((NULL != Subsystem) && ((u8)OFFLINE != Subsystem->State)) {
		SubsystemId = Subsystem->Id;
		goto done;
	}

	Subsystem = PmSubsystems;
	while (NULL != Subsystem) {
		if ((Subsystem->IpiMask == IpiMask) &&
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
	XPm_Subsystem *Subsystem;

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

int XPmSubsystem_InitFinalize(const u32 SubsystemId)
{
	int Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Device *Device;
	XPm_Requirement *Reqm;
	int DeviceInUse = 0;
	u32 Idx, Idx2;
	/* TODO: Remove this device list when CDO change is available */
	u32 ExcludeDevList[] = {
		PM_DEV_L2_BANK_0,
		PM_DEV_IPI_0,
		PM_DEV_IPI_1,
		PM_DEV_IPI_2,
		PM_DEV_IPI_3,
		PM_DEV_IPI_4,
		PM_DEV_IPI_5,
		PM_DEV_IPI_6,
	};

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem->Flags |= SUBSYSTEM_INIT_FINALIZED;

	for (Idx = 1; Idx < (u32)XPM_NODEIDX_DEV_MAX; Idx++) {
		DeviceInUse = 0;

		Device = XPmDevice_GetByIndex(Idx);
		/* Exclude devices which are expected not to be requested by anyone
		but should be kept on for basic functionalities to work
		Soc, PMC, Efuse are required for basic boot
		Ams root is root device for all sysmons and required for
		any sysmon activities
		Usage of GTs is board dependednt and used by multiple devices
		so should be kept on*/
		if ((NULL == Device) ||
		    ((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_GT == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODEIDX_DEV_AMS_ROOT == NODEINDEX(Device->Node.Id))) {
			continue;
		}

		/* Skip if device falls in ExcludeDevList */
		for (Idx2 = 0; Idx2 < ARRAY_SIZE(ExcludeDevList); Idx2++) {
			if (Device->Node.Id == ExcludeDevList[Idx2]) {
				break;
			}
		}
		if (Idx2 < ARRAY_SIZE(ExcludeDevList)) {
			continue;
		}

		if (((u32)PM_DEV_GPIO == Device->Node.Id) &&
		    (PLATFORM_VERSION_SILICON == Platform) &&
		    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
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
			     (0U == (Reqm->Subsystem->Flags & SUBSYSTEM_INIT_FINALIZED)))) {
				DeviceInUse = 1;
				break;
			}

			Reqm = Reqm->NextSubsystem;
		}

		/* Power down the device if device is unused */
		if (0 == DeviceInUse) {
			/*
			 * Here device needs to be requested and released to handle
			 * the use count of its clock and power. This makes unused
			 * clock and power to be powered down.
			 */
			Status = XPmDevice_Request(PM_SUBSYS_PMC, Device->Node.Id,
						   (u32)PM_CAP_ACCESS, XPM_MAX_QOS);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			Status = XPmDevice_Release(PM_SUBSYS_PMC, Device->Node.Id);
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

int XPmSubsystem_Idle(u32 SubsystemId)
{
	int Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Requirement *Reqm;
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
			XPmDevice_SoftResetIdle(Device, DEVICE_IDLE_REQ);
			Device->Node.Flags |= NODE_IDLE_DONE;
		}

		Reqm = Reqm->NextDevice;
	}

	Status = XST_SUCCESS;

done:
        return Status;
}

XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId)
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

		/* Check that force powerdown is not for self or PMC subsystem */
		if ((SubsystemId == NodeId) || (PM_SUBSYS_PMC == NodeId)) {
			goto done;
		}
	}
	/*TODO: Add validation based on permissions defined by user*/
	/* No permission should return XPM_PM_NO_ACCESS */

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
 * @note   None
 *
 ****************************************************************************/
XPm_Subsystem * XPmSubsystem_GetById(u32 SubsystemId)
{
	XPm_Subsystem *SubSystem = NULL;

	if (SubsystemId == INVALID_SUBSYSID) {
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

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_FAILURE;

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
			Status = XST_SUCCESS;
			break;
		case (u32)XPM_NODECLASS_DEVICE:
			if ((u32)XPM_NODESUBCL_DEV_CORE != NODESUBCLASS(NodeId))
			{
				Status = XST_INVALID_PARAM;
				break;
			}
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	/*TODO: Add validation based on permissions defined by user*/

done:
	return Status;
}

XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_PinNode *Pin;
	XPm_Device *Device = NULL;
	u32 DevId;

	if (SubsystemId == PM_SUBSYS_PMC) {
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_POWER:
		/*
		Node = (XPm_Node *)XPmPower_GetById(NodeId);
		if (NULL == Node) {
			goto done;
		}
		*/
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
		Pin = XPmPin_GetById(NodeId);
		if (NULL == Pin) {
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
			goto done;
		}
		break;
	default:
		/* XXX - Not implemented yet. */
		break;
	}
done:
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

	Subsystem->State = (u8)State;

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPmSubsystem_GetCurrent(void)
{
	return CurrentSubsystemId;
}


XStatus XPmSubsystem_SetCurrent(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;

	if ((INVALID_SUBSYSID != SubsystemId) &&
	    (NULL == XPmSubsystem_GetById(SubsystemId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	CurrentSubsystemId = SubsystemId;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmSubsystem_Add(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u32 i = 0;

	if (((u32)XPM_NODECLASS_SUBSYSTEM != NODECLASS(SubsystemId)) ||
	    ((u32)XPM_NODESUBCL_SUBSYSTEM != NODESUBCLASS(SubsystemId)) ||
	    ((u32)XPM_NODETYPE_SUBSYSTEM != NODETYPE(SubsystemId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* If default subsystem is online, no other subsystem is allowed to be created */
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	if ((NULL != Subsystem) && ((u8)OFFLINE != Subsystem->State)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((NULL != Subsystem) && ((u8)OFFLINE != Subsystem->State)) {
		Status = XST_FAILURE;
		goto done;
	}

	Subsystem = (XPm_Subsystem *)XPm_AllocBytes(sizeof(XPm_Subsystem));
	if (NULL == Subsystem) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Subsystem->NextSubsystem = PmSubsystems;
	Subsystem->Id = SubsystemId;
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

	/* Add all requirements for default subsystem */
	if(SubsystemId == PM_SUBSYS_DEFAULT)
	{
		for (i = 0; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
			/*
			 * Note: XPmDevice_GetByIndex() assumes that the caller
			 * is responsible for validating the Node ID attributes
			 * other than node index.
			 */
			XPm_Device *Device = XPmDevice_GetByIndex(i);
			if (NULL != Device) {
				Status = XPmRequirement_Add(Subsystem, Device, (((u32)REQ_ACCESS_SECURE_NONSECURE << REG_FLAGS_SECURITY_OFFSET) | (u32)REQ_NO_RESTRICTION), NULL, 0);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}

		for (i = 0; i < (u32)XPM_NODEIDX_DEV_PLD_MAX; i++) {
			XPm_Device *Device = XPmDevice_GetPlDeviceByIndex(i);
			if (NULL != Device) {
				Status = XPmRequirement_Add(Subsystem, Device,
							    (((u32)REQ_ACCESS_SECURE_NONSECURE <<
							    REG_FLAGS_SECURITY_OFFSET) |
							    (u32)REQ_NO_RESTRICTION),
							    NULL, 0U);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}

	Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

XStatus XPmSubsystem_IsAllProcDwn(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Requirement *Reqm;
	XPm_Device *Device;
	u32 SubClass;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			Device = Reqm->Device;
			SubClass = NODESUBCLASS(Device->Node.Id);
			if (((u32)XPM_NODESUBCL_DEV_CORE == SubClass) &&
			    ((u8)XPM_DEVSTATE_RUNNING == Device->Node.State)) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmSubsystem_Destroy(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Requirement *Reqm;
	XPm_Device *Device;

	if (((u32)XPM_NODECLASS_SUBSYSTEM != NODECLASS(SubsystemId)) ||
	    ((u32)XPM_NODESUBCL_SUBSYSTEM != NODESUBCLASS(SubsystemId)) ||
	    ((u32)XPM_NODETYPE_SUBSYSTEM != NODETYPE(SubsystemId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (Subsystem == NULL || Subsystem->State != (u8)ONLINE) {
		Status = XST_FAILURE;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			Device = Reqm->Device;
			Status = Device->DeviceOps->Release(Device, Subsystem);
			if (XST_FAILURE == Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	Status = XPmSubsystem_SetState(SubsystemId, (u32)OFFLINE);
done:
	return Status;
}

XStatus XPmSubsystem_Restart(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Requirement *Reqm;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Idle the subsystem */
	Status = XPmSubsystem_Idle(SubsystemId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * In case the application has not released its
	 * devices prior to restart request, it is
	 * released here.
	 * Also all the cores from subsystem are gets released.
	 * Don't release DDR as there is no DDR CDO
	 * to bring it up back again.
	 */
	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODETYPE_DEV_DDR != NODETYPE(Reqm->Device->Node.Id))) {
			Status = XPmRequirement_Release(Reqm, RELEASE_ONE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

done:
	return Status;
}

XStatus XPmSubsystem_GetStatus(const u32 SubsystemId, const u32 DeviceId,
			       XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	XPm_Subsystem *Subsystem, *Target_Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	Target_Subsystem = XPmSubsystem_GetById(DeviceId);
	if (NULL == Subsystem || NULL == Target_Subsystem ||
	    NULL == DeviceStatus) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	DeviceStatus->Status = Target_Subsystem->State;
	Status = XST_SUCCESS;

done:
	if (Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}

	return Status;
}

u32 XPmSubsystem_GetMaxSubsysIdx(void)
{
	return MaxSubsysIdx;
}

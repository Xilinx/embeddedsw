/******************************************************************************
*
* Copyright (C) 2018-2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_subsystem.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_pin.h"
#include "xpm_rpucore.h"
#include "xpm_notifier.h"
#include "xpm_requirement.h"

static XPm_Subsystem PmSubsystems[XPM_NODEIDX_SUBSYS_MAX] =
{
	[XPM_NODEIDX_SUBSYS_DEFAULT] = {
		.Id = PM_SUBSYS_DEFAULT,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000000U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_PMC] = {
		.Id = PM_SUBSYS_PMC,
		.State = (u8)ONLINE,
		.IpiMask = 0x00000002U,
		.Flags = SUBSYSTEM_INIT_FINALIZED,
	},
	[XPM_NODEIDX_SUBSYS_PSM] = {
		.Id = PM_SUBSYS_PSM,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000001U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_APU] = {
		.Id = PM_SUBSYS_APU,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000004U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_LOCK] = {
		.Id = PM_SUBSYS_RPU0_LOCK,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000008U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_0] = {
		.Id = PM_SUBSYS_RPU0_0,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000008U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_1] = {
		.Id = PM_SUBSYS_RPU0_1,
		.State = (u8)OFFLINE,
		.IpiMask = 0x00000010U,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_DDR0] = {
		.Id = PM_SUBSYS_DDR0,
		.State = (u8)OFFLINE,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_ME] = {
		.Id = PM_SUBSYS_ME,
		.State = (u8)OFFLINE,
		.Flags = 0U,
	},
	[XPM_NODEIDX_SUBSYS_PL] = {
		.Id = PM_SUBSYS_PL,
		.State = (u8)OFFLINE,
		.Flags = 0U,
	}
};

/*
 * Global SubsystemId which is set and is valid during XPm_CreateSubsystem()
 */
static u32 CurrentSubsystemId = INVALID_SUBSYSID;

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem;
	u32 IpiMaskVal = 0;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubSysIdx];
	IpiMaskVal = Subsystem->IpiMask;

done:
	return IpiMaskVal;
}

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	u32 SubSysIdx;
	u32 RpuBootMode;

	/* If default subsystem is active, return default subsystem id
	  as it does not have ipi channel mapped to it.*/
	/* TODO: remove this when ipi channel is requested through CDO
	and assigned to susbystemid */
	if ((u8)ONLINE == PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].State) {
		return PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].Id;
	}

	for (SubSysIdx = 0; SubSysIdx < (u32)XPM_NODEIDX_SUBSYS_MAX; SubSysIdx++)
	{
		if (PmSubsystems[SubSysIdx].IpiMask == IpiMask) {
			break;
		}
	}

	if (((u32)XPM_NODEIDX_SUBSYS_RPU0_LOCK == SubSysIdx) ||
	    ((u32)XPM_NODEIDX_SUBSYS_RPU0_0 == SubSysIdx)) {
		XPm_RpuGetOperMode(PM_DEV_RPU0_0, &RpuBootMode);

		if (XPM_RPU_MODE_SPLIT == RpuBootMode) {
			SubSysIdx = (u32)XPM_NODEIDX_SUBSYS_RPU0_0;
		} else {
			SubSysIdx = (u32)XPM_NODEIDX_SUBSYS_RPU0_LOCK;
		}
	}

	if (SubSysIdx == (u32)XPM_NODEIDX_SUBSYS_MAX) {
		return INVALID_SUBSYSID;
	} else {
		return PmSubsystems[SubSysIdx].Id;
	}
}

XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}
	Subsystem = &PmSubsystems[SubSysIdx];

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
	XPm_Subsystem *PlSubsystem;
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
		if ((NULL == Device) ||
		    ((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_GT == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(Device->Node.Id)) ||
		    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(Device->Node.Id))) {
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

		PlSubsystem = XPmSubsystem_GetById(PM_SUBSYS_PL);
		if ((((u32)PM_DEV_PL_0 == Device->Node.Id) ||
		    ((u32)PM_DEV_PL_1 == Device->Node.Id) ||
		    ((u32)PM_DEV_PL_2 == Device->Node.Id) ||
		    ((u32)PM_DEV_PL_3 == Device->Node.Id)) &&
		    ((u8)ONLINE == PlSubsystem->State)) {
			continue;
		}

		if ((((u32)PM_DEV_GPIO_PMC == Device->Node.Id) ||
		    ((u32)PM_DEV_GPIO == Device->Node.Id)) &&
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
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		if ((u32)XPM_NODEIDX_SUBSYS_MAX <= NODEINDEX(NodeId)) {
			goto done;
		}
		/* Check that force powerdown is not for self */
		if (SubSysIdx == NODEINDEX(NodeId)) {
			goto done;
		}
		if ((PM_SUBSYS_PMC == NodeId)) {
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
	u32 SubSysIdx;

	if (SubsystemId == INVALID_SUBSYSID) {
		goto done;
	}

	for (SubSysIdx = 0; SubSysIdx < (u32)XPM_NODEIDX_SUBSYS_MAX; SubSysIdx++) {
		if (PmSubsystems[SubSysIdx].Id == SubsystemId) {
			break;
		}
	}

	if (SubSysIdx == (u32)XPM_NODEIDX_SUBSYS_MAX) {
		SubSystem = NULL;
	} else {
		SubSystem = &PmSubsystems[SubSysIdx];
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
	XPm_Subsystem *Subsystem = NULL;

	/*
	 * We assume that Subsystem class, subclass and type have been
	 * validated before, so just validate index against bounds here
	 */
	if (SubSysIdx < (u32)XPM_NODEIDX_SUBSYS_MAX) {
		Subsystem = &PmSubsystems[SubSysIdx];
	}

	return Subsystem;
}

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	switch (NODECLASS(NodeId))
	{
		case (u32)XPM_NODECLASS_SUBSYSTEM:
			/* Check that request wakeup is not for self */
			if (SubSysIdx == NODEINDEX(NodeId)) {
				Status = XST_INVALID_PARAM;
				break;
			}
			SubSysIdx = NODEINDEX(NodeId);
			if ((u32)XPM_NODEIDX_SUBSYS_MAX <= SubSysIdx) {
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
	u32 SubSysIdx = NODEINDEX(SubsystemId);
	u32 DevId;

	if (SubsystemId == PM_SUBSYS_PMC) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubSysIdx];

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
		Status = XPmClock_CheckPermissions(SubSysIdx, NodeId);
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
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if ((State >= (u32)MAX_STATE) || (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX)) {
		goto done;
	}

	PmSubsystems[SubSysIdx].State = (u8)State;
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

	if ((!ISVALIDSUBSYSTEM(SubsystemId)) &&
	    (INVALID_SUBSYSID != SubsystemId)) {
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

	/* If default subsystem is online, no other subsystem is allowed to be created */
	if (!ISVALIDSUBSYSTEM(SubsystemId) || PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].State == (u8)ONLINE || SubsystemId == PM_SUBSYS_PMC) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (Subsystem == NULL || Subsystem->State != (u8)OFFLINE) {
		Status = XST_FAILURE;
		goto done;
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
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= (u32)XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubSysIdx];
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

	if (!ISVALIDSUBSYSTEM(SubsystemId)) {
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
	XPm_Device *Device;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Idle the subsystem */
	Status = XPmSubsystem_Idle(SubsystemId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			Device = Reqm->Device;
			if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Device->Node.Id)) {
				Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
				if (XST_SUCCESS != Status) {
					goto done;
				}
				Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC,
							     (u32)PM_RESET_ACTION_PULSE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Device->Node.Id)) {
				Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
				if (XST_SUCCESS != Status) {
					goto done;
				}
				/*
				 * Put the RPU to halt state so that TCM init
				 * can be done during loading of RPU CDO.
				 */
				Status = XPmRpuCore_Halt(Device);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else {
				/*
				 * In case the application has not released its
				 * devices prior to restart request, it is
				 * released here.  Don't release DDR as there is no DDR CDO
				 * to bring it up back again.  TODO - need to understand
				 * why releasing TCM0_A causes failure.
				 */
				if (((u32)XPM_NODETYPE_DEV_DDR != NODETYPE(Device->Node.Id)) &&
				    ((u32)XPM_NODEIDX_DEV_TCM_0_A != NODEINDEX(Device->Node.Id))) {
					Status = XPmRequirement_Release(Reqm, RELEASE_ONE);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
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

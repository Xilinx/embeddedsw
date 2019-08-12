/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

static XPm_Subsystem PmSubsystems[XPM_NODEIDX_SUBSYS_MAX] =
{
	[XPM_NODEIDX_SUBSYS_DEFAULT] = {
		.Id = XPM_SUBSYSID_DEFAULT,
		.State = OFFLINE,
		.IpiMask = 0x00000000U,
	},
	[XPM_NODEIDX_SUBSYS_PMC] = {
		.Id = XPM_SUBSYSID_PMC,
		.State = ONLINE,
		.IpiMask = 0x00000002U,
	},
	[XPM_NODEIDX_SUBSYS_PSM] = {
		.Id = XPM_SUBSYSID_PSM,
		.State = OFFLINE,
		.IpiMask = 0x00000001U,
	},
	[XPM_NODEIDX_SUBSYS_APU] = {
		.Id = XPM_SUBSYSID_APU,
		.State = OFFLINE,
		.IpiMask = 0x00000004U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_LOCK] = {
		.Id = XPM_SUBSYSID_RPU0_LOCK,
		.State = OFFLINE,
		.IpiMask = 0x00000008U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_0] = {
		.Id = XPM_SUBSYSID_RPU0_0,
		.State = OFFLINE,
		.IpiMask = 0x00000008U,
	},
	[XPM_NODEIDX_SUBSYS_RPU0_1] = {
		.Id = XPM_SUBSYSID_RPU0_1,
		.State = OFFLINE,
		.IpiMask = 0x00000010U,
	},
	[XPM_NODEIDX_SUBSYS_DDR0] = {
		.Id = XPM_SUBSYSID_DDR0,
		.State = OFFLINE,
	},
	[XPM_NODEIDX_SUBSYS_ME] = {
		.Id = XPM_SUBSYSID_ME,
		.State = OFFLINE,
	},
	[XPM_NODEIDX_SUBSYS_PL] = {
		.Id = XPM_SUBSYSID_PL,
		.State = OFFLINE,
	}
};

/*
 * Global SubsystemId which is set and is valid during XPm_CreateSubsystem()
 */
static u32 CurrentSubsystemId = INVALID_SUBSYSID;

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	VERIFY(SubSysIdx < XPM_NODEIDX_SUBSYS_MAX);
	Subsystem = &PmSubsystems[SubSysIdx];
	return Subsystem->IpiMask;
}

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	u32 SubSysIdx;
	u32 RpuBootMode;

	/* If default subsystem is active, return default subsystem id
	  as it doesnt have ipi channel mapped to it.*/
	/* TODO: remove this when ipi channel is requested through CDO
	and assigned to susbystemid */
	if (ONLINE == PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].State) {
		return PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].Id;
	}

	for (SubSysIdx = 0; SubSysIdx < XPM_NODEIDX_SUBSYS_MAX; SubSysIdx++)
	{
		if (PmSubsystems[SubSysIdx].IpiMask == IpiMask) {
			break;
		}
	}

	if ((XPM_NODEIDX_SUBSYS_RPU0_LOCK == SubSysIdx) ||
	    (XPM_NODEIDX_SUBSYS_RPU0_0 == SubSysIdx)) {
		XPm_RpuGetOperMode(XPM_DEVID_R50_0, &RpuBootMode);

		if (XPM_RPU_MODE_SPLIT == RpuBootMode) {
			SubSysIdx = XPM_NODEIDX_SUBSYS_RPU0_0;
		} else {
			SubSysIdx = XPM_NODEIDX_SUBSYS_RPU0_LOCK;
		}
	}

	if(SubSysIdx == XPM_NODEIDX_SUBSYS_MAX) {
		return INVALID_SUBSYSID;
	} else {
		return PmSubsystems[SubSysIdx].Id;
	}
}

XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId)
{
	XStatus Status;
	XPm_Subsystem *Subsystem;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	VERIFY(SubSysIdx < XPM_NODEIDX_SUBSYS_MAX);
	Subsystem = &PmSubsystems[SubSysIdx];

        Status = XPmRequirement_Release(Subsystem->Requirements, RELEASE_ALL);
		/* Todo: Cancel wakeup if scheduled
		 * Should be included with wakeup support
		XPm_WakeUpCancelScheduled(SubSysIdx);*/

        /* Unregister all notifiers for this subsystem */
	XPmNotifier_UnregisterAll(Subsystem);

        return Status;
}

int XPmSubsystem_Idle(u32 SubsystemId)
{
	int Status = XST_SUCCESS;
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

		/* Check if device is requested and its clock is active */
		if ((TRUE == Reqm->Allocated) &&
		    (0 == (Device->Node.Flags & NODE_IDLE_DONE)) &&
		    (XST_SUCCESS == XPmDevice_IsClockActive(Device)) &&
		    (PM_USAGE_CURRENT_SUBSYSTEM == Usage)) {
			XPmDevice_SoftResetIdle(Device, DEVICE_IDLE_REQ);
			Device->Node.Flags |= NODE_IDLE_DONE;
		}

		Reqm = Reqm->NextDevice;
	}

done:
        return Status;
}

XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_SUCCESS;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= XPM_NODEIDX_SUBSYS_MAX) {
		Status = XST_FAILURE;
                goto done;
	}

	/*Warning Fix*/
	(void) (NodeId);

	/*TODO: Add validation based on permissions defined by user*/
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

	if(SubsystemId == INVALID_SUBSYSID)
		goto done;

	for (SubSysIdx = 0; SubSysIdx < XPM_NODEIDX_SUBSYS_MAX; SubSysIdx++)
	{
		if (PmSubsystems[SubSysIdx].Id == SubsystemId) {
			break;
		}
	}

	if(SubSysIdx == XPM_NODEIDX_SUBSYS_MAX) {
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
	if (SubSysIdx < XPM_NODEIDX_SUBSYS_MAX) {
		Subsystem = &PmSubsystems[SubSysIdx];
	}

	return Subsystem;
}

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_SUCCESS;
	u32 SubSysIdx = NODEINDEX(SubsystemId);

	if (SubSysIdx >= XPM_NODEIDX_SUBSYS_MAX) {
		Status = XST_FAILURE;
                goto done;
        }
	if(NODECLASS(NodeId) != XPM_NODECLASS_DEVICE || NODESUBCLASS(NodeId) != XPM_NODESUBCL_DEV_CORE)
	{
                Status = XST_INVALID_PARAM;
                goto done;
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

	if (SubsystemId == XPM_SUBSYSID_PMC) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (SubSysIdx >= XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubSysIdx];

	switch (NODECLASS(NodeId)) {
	case XPM_NODECLASS_POWER:
		/*
		Node = (XPm_Node *)XPmPower_GetById(NodeId);
		if (NULL == Node) {
			goto done;
		}
		*/
		break;
	case XPM_NODECLASS_CLOCK:
		Status = XPmClock_CheckPermissions(SubSysIdx, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case XPM_NODECLASS_RESET:
		Status = XPmReset_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case XPM_NODECLASS_DEVICE:
		Status = XPmDevice_CheckPermissions(Subsystem, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case XPM_NODECLASS_STMIC:
		Pin = XPmPin_GetById(NodeId);
		if (NULL == Pin) {
			goto done;
		}

		if (XPM_PINSTATE_UNUSED == Pin->Node.State) {
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
		if ((XPM_PINSTATE_UNUSED == Pin->Node.State) || (0 == DevId)) {
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

	if ((State >= MAX_STATE) || (SubSysIdx >= XPM_NODEIDX_SUBSYS_MAX)) {
		goto done;
	}

	PmSubsystems[SubSysIdx].State = State;
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
	XStatus Status = XST_SUCCESS;

	if ((!ISVALIDSUBSYSTEM(SubsystemId)) &&
	    (INVALID_SUBSYSID != SubsystemId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	CurrentSubsystemId = SubsystemId;

done:
	return Status;
}

XStatus XPmSubsystem_Add(u32 SubsystemId)
{
	XStatus Status = XST_SUCCESS;
	XPm_Subsystem *Subsystem;
	u32 i = 0;

	/* If default subsystem is online, no other subsystem is allowed to be created */
	if (!ISVALIDSUBSYSTEM(SubsystemId) || PmSubsystems[XPM_NODEIDX_SUBSYS_DEFAULT].State == ONLINE || SubsystemId == XPM_SUBSYSID_PMC) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (Subsystem == NULL || Subsystem->State != OFFLINE) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Add all requirements for default subsystem */
	if(SubsystemId == XPM_SUBSYSID_DEFAULT)
	{
		for (i = 0; i < XPM_NODEIDX_DEV_MAX; i++) {
			/*
			 * Note: XPmDevice_GetByIndex() assumes that the caller
			 * is responsible for validating the Node ID attributes
			 * other than node index.
			 */
			XPm_Device *Device = XPmDevice_GetByIndex(i);
			if (NULL != Device) {
				Status = XPmRequirement_Add(Subsystem, Device, ((REQ_ACCESS_SECURE_NONSECURE << REG_FLAGS_SECURITY_OFFSET)|REQ_NO_RESTRICTION), NULL, 0);
				if (XST_SUCCESS != Status)
					goto done;
			}
		}
	}

	Status = XPmSubsystem_SetState(SubsystemId, ONLINE);
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

	if (SubSysIdx >= XPM_NODEIDX_SUBSYS_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubSysIdx];
	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (TRUE == Reqm->Allocated) {
			Device = Reqm->Device;
			SubClass = NODESUBCLASS(Device->Node.Id);
			if ((XPM_NODESUBCL_DEV_CORE == SubClass) &&
			    (XPM_DEVSTATE_RUNNING == Device->Node.State)) {
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
	if (Subsystem == NULL || Subsystem->State != ONLINE) {
		Status = XST_FAILURE;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if (TRUE == Reqm->Allocated) {
			Device = Reqm->Device;
			Status = Device->DeviceOps->Release(Device, Subsystem);
			if (XST_FAILURE == Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	Status = XPmSubsystem_SetState(SubsystemId, OFFLINE);
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
		if (TRUE == Reqm->Allocated) {
			Device = Reqm->Device;
			if (XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Device->Node.Id)) {
				Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
				if (XST_SUCCESS != Status) {
					goto done;
				}
				Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_ACPU_GIC),
							     PM_RESET_ACTION_PULSE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else if (XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Device->Node.Id)) {
				Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
				if (XST_SUCCESS != Status) {
					goto done;
				}
				/*
				 * Put the RPU to halt state so that TCM init
				 * can be done during loading of RPU CDO.
				 */
				XPmRpuCore_Halt(Device);
			} else {
				/*
				 * In case the application has not released its
				 * devices prior to restart request, it is
				 * released here.  Don't release DDR as there is no DDR CDO
				 * to bring it up back again.  TODO - need to understand
				 * why releasing TCM0_A causes failure.
				 */
				if ((XPM_NODETYPE_DEV_DDR != NODETYPE(Device->Node.Id)) &&
				    (XPM_NODEIDX_DEV_TCM_0_A != NODEINDEX(Device->Node.Id))) {
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

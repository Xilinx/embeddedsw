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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpm_subsystem.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_device.h"
#include "xpm_pin.h"

XPm_Subsystem PmSubsystems[XPM_SUBSYSID_MAX] =
{
	[XPM_SUBSYSID_PMC] = {
		.Id = XPM_SUBSYSID_PMC,
		.State = ONLINE,
		.IpiMask = 0x00000002U,
	},
	[XPM_SUBSYSID_PSM] = {
		.Id = XPM_SUBSYSID_PSM,
		.State = OFFLINE,
		.IpiMask = 0x00000001U,
	},
	[XPM_SUBSYSID_APU] = {
		.Id = XPM_SUBSYSID_APU,
		.State = OFFLINE,
		.IpiMask = 0x00000004U,
	},
	[XPM_SUBSYSID_RPU0_LOCK] = {
		.Id = XPM_SUBSYSID_RPU0_LOCK,
		.State = OFFLINE,
		.IpiMask = 0x00000008U,
	},
	[XPM_SUBSYSID_RPU0_0] = {
		.Id = XPM_SUBSYSID_RPU0_0,
		.State = OFFLINE,
		.IpiMask = 0x00000008U,
	},
	[XPM_SUBSYSID_RPU0_1] = {
		.Id = XPM_SUBSYSID_RPU0_1,
		.State = OFFLINE,
		.IpiMask = 0x00000010U,
	},
	[XPM_SUBSYSID_DDR0] = {
		.Id = XPM_SUBSYSID_DDR0,
		.State = OFFLINE,
	},
	[XPM_SUBSYSID_ME] = {
		.Id = XPM_SUBSYSID_ME,
		.State = OFFLINE,
	},
	[XPM_SUBSYSID_PL] = {
		.Id = XPM_SUBSYSID_PL,
		.State = OFFLINE,
	}
};

/*
 * Global SubsystemId which is set and is valid during XPm_CreateSubsystem()
 */
u32 ReservedSubsystemId = INVALID_SUBSYSID;
static u32 CurrentSubsystemId = INVALID_SUBSYSID;

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem;

	VERIFY(SubsystemId < XPM_SUBSYSID_MAX);
	Subsystem = &PmSubsystems[SubsystemId];
	return Subsystem->IpiMask;
}

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	u32 SubSysId;

	for (SubSysId = 0; SubSysId < XPM_SUBSYSID_MAX; SubSysId++)
	{
		if (PmSubsystems[SubSysId].IpiMask == IpiMask) {
			break;
		}
	}

	if(SubSysId == XPM_SUBSYSID_MAX) {
		SubSysId = INVALID_SUBSYSID;
	}

	return SubSysId;
}

XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId)
{
        XStatus Status;
	XPm_Subsystem *Subsystem;

	VERIFY(SubsystemId < XPM_SUBSYSID_MAX);
	Subsystem = &PmSubsystems[SubsystemId];

        Status = XPmRequirement_Release(Subsystem->Requirements, RELEASE_ALL);
		/* Todo: Cancel wakeup if scheduled
		 * Should be included with wakeup support
		XPm_WakeUpCancelScheduled(SubsystemId);*/

        /*Todo: Unregister all notifier for this subsystem
		 * Should be included with register notifier API support */

        return Status;
}

XStatus XPmSubsystem_Idle(u32 SubsystemId)
{
        XStatus Status = XST_SUCCESS;

	VERIFY(SubsystemId < XPM_SUBSYSID_MAX);
	/* TBD: Add diling support */
        return Status;
}

XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_SUCCESS;

	if (SubsystemId > XPM_SUBSYSID_MAX) {
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
	XPm_Subsystem *SubSystem;

	if (SubsystemId > XPM_SUBSYSID_MAX) {
		SubSystem = NULL;
	} else {
		SubSystem = &PmSubsystems[SubsystemId];
	}

	return SubSystem;
}

XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_SUCCESS;

	if (SubsystemId > XPM_SUBSYSID_MAX) {
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
	XPm_Requirement *Reqm;
	XPm_Node *Node = NULL;
	XPm_PinNode *Pin;

	if (SubsystemId > XPM_SUBSYSID_MAX) {
		goto done;
	}

	if (XPM_SUBSYSID_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = &PmSubsystems[SubsystemId];
	Reqm = Subsystem->Requirements;

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
		Status = XPmClock_CheckPermissions(SubsystemId, NodeId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case XPM_NODECLASS_RESET:
		Node = (XPm_Node *)XPmReset_GetById(NodeId);
		XPm_ResetNodeList *RstList;
		if (NULL == Node) {
			goto done;
		}
		while (NULL != Reqm) {
			RstList = Reqm->Device->ResetList;
			while ((NULL != RstList) && (TRUE == Reqm->Allocated)) {
				if ((XPm_ResetNode *)Node == RstList->Reset) {
					Status = XST_SUCCESS;
					goto done;
				}
				RstList = RstList->NextNode;
			}
			Reqm = Reqm->NextDevice;
		}
		break;
	case XPM_NODECLASS_DEVICE:
		Node = (XPm_Node *)XPmDevice_GetById(NodeId);
		if (NULL == Node) {
			goto done;
		}
		while (NULL != Reqm) {
			if (((XPm_Device *)Node == Reqm->Device) &&
				(TRUE == Reqm->Allocated)) {
					Status = XST_SUCCESS;
					goto done;
			}
			Reqm = Reqm->NextDevice;
		}
		break;
	case XPM_NODECLASS_STMIC:
		Pin = XPmPin_GetById(NodeId);
		if (NULL == Pin) {
			goto done;
		} else if ((XPM_PINSTATE_UNUSED == Pin->Node.State) ||
			(0 == Pin->PinFunc->DeviceId)) {
			Status = XST_SUCCESS;
			goto done;
		} else if (TRUE ==
			XPmDevice_IsAllocated(Pin->PinFunc->DeviceId, Subsystem)) {
			Status = XST_SUCCESS;
			goto done;
		} else {
			/* Required by MISRA */
		}
		break;
	default:
		/* XXX - Not implemented yet. */
		break;
	}
done:
	return Status;
}

XStatus XPmSubsystem_Reserve(u32 *SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	int i;

	VERIFY(ReservedSubsystemId == INVALID_SUBSYSID);

	for (i = 0; i < XPM_SUBSYSID_MAX; i++) {
		Subsystem = &PmSubsystems[i];
		if (Subsystem->State != OFFLINE) {
			continue;
		}
		Subsystem->State = RESERVED;
		ReservedSubsystemId = i;
		*SubsystemId = i;
		Status = XST_SUCCESS;
		goto done;
	}
	if (XPM_SUBSYSID_MAX == i) {
		goto done;
	}
done:
	return Status;
}

XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State)
{
	XStatus Status = XST_FAILURE;

	if ((State >= MAX_STATE) || (SubsystemId >= XPM_SUBSYSID_MAX)) {
		goto done;
	}

	PmSubsystems[SubsystemId].State = State;
	ReservedSubsystemId = INVALID_SUBSYSID;
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

	if (SubsystemId >= XPM_SUBSYSID_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	CurrentSubsystemId = SubsystemId;

done:
	return Status;
}

XStatus XPmSubsystem_Create(void (*const NotifyCb)(u32 SubsystemId,
						   const u32 EventId),
			    u32 *SubsystemId)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId;

	Status = XPmSubsystem_Reserve(&SubsysId);
	if (XST_FAILURE == Status) {
		goto done;
	}

#if 0	/* include when libcdo is available */
	Status = XilCdo_ProcessCdo(SubsystemCdo);
#else
	Status = XST_SUCCESS;
#endif
	if (XST_FAILURE == Status) {
		Status = XPmSubsystem_SetState(SubsysId, OFFLINE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		(void) XPmSubsystem_Destroy(SubsysId);
		goto done;
	}

	PmSubsystems[SubsysId].NotifyCb = NotifyCb;
	Status = XPmSubsystem_SetState(SubsysId, ONLINE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*SubsystemId = SubsysId;
	Status = XST_SUCCESS;
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

	if (SubsystemId >= XPM_SUBSYSID_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubsystemId];
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

	if (SubsystemId > XPM_SUBSYSID_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubsystemId];
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

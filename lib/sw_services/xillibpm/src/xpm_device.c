/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
#include "xpm_device.h"
#include "xpm_core.h"
#include "xillibpm_api.h"

const char *PmDevStates[] = {
	"UNUSED",
	"RUNNING",
	"PWR_ON",
	"CLK_ON",
	"RST_OFF",
	"RST_ON",
	"CLK_OFF",
	"PWR_OFF",
};

const char *PmDevEvents[] = {
	"BRINGUP_ALL",
	"BRINGUP_CLKRST",
	"SHUTDOWN",
	"TIMER",
};

static XPm_DeviceOps PmDeviceOps;
XPm_Device *PmDevices[XPM_NODEIDX_DEV_MAX];
u32 MaxDevices=XPM_NODEIDX_DEV_MAX;
u32 PmNumDevices=0;

XPm_Requirement *FindReqm(XPm_Device *Device, XPm_Subsystem *Subsystem)
{
	XPm_Requirement *Reqm = NULL;

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (Reqm->Subsystem == Subsystem) {
			break;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Reqm;
}

static u32 IsRunning(XPm_Device *Device)
{
	u32 Running = 0;
	XPm_Requirement *Reqm = Device->Requirements;

	while (NULL != Reqm) {
		if (Reqm->Allocated > 0) {
			if (Reqm->Curr.Capabilities > 0) {
				Running = 1;
				break;
			}
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Running;
}

static XStatus BringUp(XPm_Node *Node)
{
	u32 Status = XST_FAILURE;
	XPm_Device *Device = (XPm_Device *)Node;

	if (NULL == Device->Power) {
		goto done;
	}

	Device->WfPwrUseCnt = Device->Power->UseCount + 1;
	Status = Device->Power->Node.HandleEvent(
		&Device->Power->Node, XPM_POWER_EVENT_PWR_UP);
	if (XST_SUCCESS == Status) {
		Node->State = XPM_DEVSTATE_PWR_ON;
		/* Todo: Start timer to poll the power node */
		/* Hack */
		Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
	}

done:
	return Status;
}

static XStatus SetClocks(XPm_Device *Device, u32 Enable)
{
	XPm_ClockHandle *ClkHandle = Device->ClkHandles;

	/* Enable all the clock gates, skip over others */
	while (NULL != ClkHandle) {
		if (NULL != ClkHandle->Clock) {
			XPmClock_SetGate((XPm_OutClockNode *)ClkHandle->Clock, Enable);
		}
		ClkHandle = ClkHandle->NextClock;
	}

	return XST_SUCCESS;
}

static XStatus HandleDeviceEvent(XPm_Node *Node, u32 Event)
{
	u32 Status = XST_FAILURE;
	XPm_Device *Device = (XPm_Device *)Node;

	PmDbg("State=%s, Event=%s\n\r", PmDevStates[Node->State], PmDevEvents[Event]);

	switch(Node->State)
	{
		case XPM_DEVSTATE_UNUSED:
			if (XPM_DEVEVENT_BRINGUP_ALL == Event) {
				Status = BringUp(Node);
			}
			break;
		case XPM_DEVSTATE_PWR_ON:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Device->WfPwrUseCnt == Device->Power->UseCount) {
					Node->State = XPM_DEVSTATE_CLK_ON;
					/* Enable clock */
					Status = SetClocks(Device, TRUE);
					if (XST_SUCCESS != Status) {
						break;
					}
					/* Todo: Start timer to poll the clock node */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll the power node */
				}
			} else {
				Status = XST_DEVICE_BUSY;
			}
			break;
		case XPM_DEVSTATE_CLK_ON:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if clock is enabled */
				if (1 /* Hack: Clock enabled */) {
					Node->State = XPM_DEVSTATE_RST_OFF;
					/* De-assert reset for peripheral devices */
					if (XPM_NODESUBCL_DEV_PERIPH ==
						NODESUBCLASS(Device->Node.Id)) {
						if (NULL != Device->Reset) {
							Status = Device->Reset->Ops->SetState(Device->Reset,
								PM_RESET_ACTION_RELEASE);
							if (XST_SUCCESS != Status) {
								break;
							}
						}
					}
					/* Todo: Start timer to poll the reset node */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll the clock node */
				}
			} else {
				Status = XST_DEVICE_BUSY;
			}
			break;
		case XPM_DEVSTATE_RST_OFF:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if reset is de-asserted */
				if (1 /* Hack: Reset de-asserted */) {
					XPm_RequiremntUpdate(Device->PendingReqm);
					Node->State = XPM_DEVSTATE_RUNNING;
					XPm_RequiremntUpdate(Device->PendingReqm);
					Device->PendingReqm = NULL;
				} else {
					/* Todo: Start timer to poll the reset node */
				}
			} else {
				Status = XST_DEVICE_BUSY;
			}
			break;
		case XPM_DEVSTATE_RUNNING:
			if (XPM_DEVEVENT_BRINGUP_ALL == Event) {
				Status = BringUp(Node);
			} else if (XPM_DEVEVENT_BRINGUP_CLKRST == Event) {
				Node->State = XPM_DEVSTATE_CLK_ON;
				/* Enable all clocks */
				Status = SetClocks(Device, TRUE);
				if (XST_SUCCESS != Status) {
					break;
				}
				/* Todo: Start timer to poll the clock node */
				/* Hack */
				Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
			} else if (XPM_DEVEVENT_SHUTDOWN == Event) {
				Status = XST_SUCCESS;
				Node->State = XPM_DEVSTATE_RST_ON;
				/* Assert reset for peripheral devices */
				if (XPM_NODESUBCL_DEV_PERIPH ==
					NODESUBCLASS(Device->Node.Id)) {
					if (NULL != Device->Reset) {
						Status = Device->Reset->Ops->SetState(Device->Reset,
							PM_RESET_ACTION_ASSERT);
						if (XST_SUCCESS != Status) {
							break;
						}
					}
				}
				/* Todo: Start timer to poll reset node */
				/* Hack */
				Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
			} else {
				/* Required by MISRA */
			}
			break;
		case XPM_DEVSTATE_RST_ON:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if reset is asserted */
				if (1 /* Hack: asserted */) {
					Node->State = XPM_DEVSTATE_CLK_OFF;
					/* Disable all clocks */
					Status = SetClocks(Device, FALSE);
					if (XST_SUCCESS != Status) {
						break;
					}
					/* Todo: Start timer to poll clock node */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll reset node */
				}
			}
			break;
		case XPM_DEVSTATE_CLK_OFF:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if clock is disabled */
				if (1 /* Hack: Clock disabled */) {
					Node->State = XPM_DEVSTATE_PWR_OFF;
					Device->WfPwrUseCnt = Device->Power->UseCount - 1U;
					Status = Device->Power->Node.HandleEvent(
						&Device->Power->Node, XPM_POWER_EVENT_PWR_DOWN);
					/* Todo: Start timer to poll power node use count */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll clock node */
				}
			}
			break;
		case XPM_DEVSTATE_PWR_OFF:
			if (XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Device->WfPwrUseCnt == Device->Power->UseCount) {
					if (Device->WfDealloc) {
						Device->PendingReqm->Allocated = 0;
						Device->WfDealloc = 0;
					}
					XPm_RequiremntUpdate(Device->PendingReqm);
					Device->PendingReqm = NULL;
					if (0 == IsRunning(Device)) {
						Node->State = XPM_DEVSTATE_UNUSED;
					} else {
						Node->State = XPM_DEVSTATE_RUNNING;
					}
				} else {
					/* Todo: Start timer to poll power node use count */
				}
			}
			break;
		default:
			break;
	}

	return Status;
}

static XStatus Request(XPm_Device *Device,
		XPm_Subsystem *Subsystem,
		u32 Capabilities, const u32 Latency, const u32 QoS)
{
	u32 Status = XST_FAILURE;
	XPm_Requirement *Reqm;

	if ((XPM_DEVSTATE_UNUSED != Device->Node.State) &&
		(XPM_DEVSTATE_RUNNING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		goto done;
	}

	if (1 == Reqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Todo: Check whether this device assigned to the subsystem */

	/* Todo: Check whether this device is sharable */

	/* Allocated device for the subsystem */
	Reqm->Allocated = 1;

	Status = Device->DeviceOps->SetRequirement(Device,
		Subsystem, Capabilities, Latency, QoS);

done:
	return Status;
}

static XStatus SetRequirement(XPm_Device *Device,
		XPm_Subsystem *Subsystem,
		u32 Capabilities, const u32 Latency, const u32 QoS)
{
	u32 Status = XST_FAILURE;

	if ((XPM_DEVSTATE_UNUSED != Device->Node.State) &&
		(XPM_DEVSTATE_RUNNING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Device->PendingReqm = FindReqm(Device, Subsystem);
	if (NULL == Device->PendingReqm) {
		goto done;
	}

	Device->PendingReqm->Next.Capabilities = Capabilities;
	Device->PendingReqm->Next.Latency = Latency;
	Device->PendingReqm->Next.QoS = QoS;

	if (0U != Capabilities) {
		if (0U == Device->PendingReqm->Curr.Capabilities) {
			/* This subsystem has no requirements on the device before, but now
			 * it wants to use it.  Bring up power, clocks and reset in case
			 * they were down.
			 */
			Status = Device->Node.HandleEvent((XPm_Node *)Device,
				XPM_DEVEVENT_BRINGUP_ALL);
		} else {
			/* This subsystem has been using the device but is changing the
			 * requirements.  Bring up clocks and reset in case they are
			 * down.
			 */
			Status = Device->Node.HandleEvent((XPm_Node *)Device,
				XPM_DEVEVENT_BRINGUP_CLKRST);
		}
	} else {
		/* This subsystem will not be using the device.  Trying here to shut
		 * down the device.  The power, clock and reset nodes should keep use
		 * count for each subsystem using the device.
		 */
		Status = Device->Node.HandleEvent((XPm_Node *)Device,
			XPM_DEVEVENT_SHUTDOWN);
	}

done:
	return Status;
}

static XStatus Release(XPm_Device *Device,
		XPm_Subsystem *Subsystem)
{
	u32 Status = XST_FAILURE;

	if ((XPM_DEVSTATE_UNUSED != Device->Node.State) &&
		(XPM_DEVSTATE_RUNNING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Device->PendingReqm = FindReqm(Device, Subsystem);
	if (NULL == Device->PendingReqm) {
		goto done;
	}

	if (0 == Device->PendingReqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

	Device->WfDealloc = 1;

	Status = Device->DeviceOps->SetRequirement(Device,
		Subsystem, 0, XPM_DEF_LATENCY, XPM_DEF_QOS);

done:
	return Status;
}

XStatus XPmDevice_Init(XPm_Device *Device,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode * Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	XPm_Requirement *Reqm;

	if (PmDevices[NODEINDEX(Id)] != NULL) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	Status = XPmNode_Init((XPm_Node *)Device,
		Id, XPM_DEVSTATE_UNUSED, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Reqm = (XPm_Requirement *)XPm_AllocBytes(sizeof(XPm_Requirement) * XPM_SUBSYSID_MAX);
	if (NULL == Reqm) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	for (i = 0; i < XPM_SUBSYSID_MAX; i++) {
		Status = XPmRequirement_Init(&Reqm[i], &PmSubsystems[i], Device);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Device->Power = Power;
	Device->Reset = Reset;
	Device->PendingReqm = NULL;
	Device->WfDealloc = 0;
	Device->WfPwrUseCnt = 0;

	Status = XPmDevice_AddClock(Device, Clock);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Device->Node.HandleEvent = HandleDeviceEvent;

	PmDeviceOps.Request = Request;
	PmDeviceOps.SetRequirement = SetRequirement;
	PmDeviceOps.Release = Release;
	Device->DeviceOps = &PmDeviceOps;

	PmDevices[NODEINDEX(Id)] = Device;
	PmNumDevices++;

done:
	return Status;
}

XStatus XPmDevice_SetPower(XPm_Device *Device, XPm_Power *Power)
{
	u32 Status = XST_FAILURE;

	if (NULL != Device->Power) {
		/* Cannot set power node again */
		goto done;
	}

	Device->Power = Power;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmDevice_AddClock(XPm_Device *Device, XPm_ClockNode *Clock)
{
	XStatus Status = XST_SUCCESS;
	XPm_ClockHandle *ClkHandle;

	if (NULL == Device) {
		Status = XST_FAILURE;
		goto done;
	}

	if (NULL == Clock) {
		goto done;
	}

	ClkHandle = (XPm_ClockHandle *)XPm_AllocBytes(sizeof(XPm_ClockHandle));
	if (NULL == ClkHandle) {
		Status = XST_BUFFER_TOO_SMALL;
                goto done;
	}

	ClkHandle->Clock = Clock;
	ClkHandle->Device = Device;

	/* Prepend the new handle to the device's clock handle list */
	ClkHandle->NextClock = Device->ClkHandles;
	Device->ClkHandles = ClkHandle;

	/* Prepend the new handle to the clock's device handle list */
	ClkHandle->NextDevice = Clock->ClkHandles;
	Clock->ClkHandles = ClkHandle;

done:
	return Status;
}

XStatus XPmDevice_SetReset(XPm_Device *Device, XPm_ResetNode *Reset)
{
	u32 Status = XST_FAILURE;

	if (NULL != Device->Reset) {
		/* Cannot set reset node again */
		goto done;
	}

	Device->Reset = Reset;

	Status = XST_SUCCESS;

done:
	return Status;
}

u8 XPmDevice_IsAllocated(u32 DeviceId, XPm_Subsystem *Subsystem)
{
	u8 IsAllocated = FALSE;
	XPm_Requirement *Reqm;
	XPm_Device *Device = PmDevices[DeviceId];

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (NULL != Subsystem) {
			if (Reqm->Subsystem != Subsystem) {
				Reqm = Reqm->NextSubsystem;
				continue;
			}
		}

		if (TRUE == Reqm->Allocated) {
			IsAllocated = TRUE;
			break;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return IsAllocated;
}

XPm_Device *XPmDevice_GetById(const u32 DeviceId)
{
	XPm_Device *Device = NULL;

	if (XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	Device = PmDevices[NODEINDEX(DeviceId)];
done:
	return Device;
}

XStatus XPmDevice_Request(const u32 TargetSubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS)
{
	u32 Status = XST_FAILURE;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;

	/* Todo: Check if policy allows this request */

	if (XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Device = PmDevices[NODEINDEX(DeviceId)];
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Device->Node.Id != DeviceId) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (TargetSubsystemId >= XPM_SUBSYSID_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem = &PmSubsystems[TargetSubsystemId];

	Status = Device->DeviceOps->Request(Device, Subsystem, Capabilities, Latency, QoS);

done:
	return Status;
}

XStatus XPmDevice_Release(const u32 SubsystemId, const u32 DeviceId)
{
	u32 Status = XST_FAILURE;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;

	/* Todo: Check if subsystem has permission */

	if (XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Device = PmDevices[NODEINDEX(DeviceId)];
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Device->Node.Id != DeviceId) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (SubsystemId >= XPM_SUBSYSID_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem = &PmSubsystems[SubsystemId];

	Status = Device->DeviceOps->Release(Device, Subsystem);

done:
	return Status;
}

XStatus XPmDevice_SetRequirement(const u32 SubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS)
{
	u32 Status = XST_FAILURE;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;

	/* Todo: Check if subsystem has permission */

	if (XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Device = PmDevices[NODEINDEX(DeviceId)];
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Device->Node.Id != DeviceId) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (SubsystemId >= XPM_SUBSYSID_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem = &PmSubsystems[SubsystemId];

	Status = Device->DeviceOps->SetRequirement(Device, Subsystem, Capabilities, Latency, QoS);

done:
	return Status;
}

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Device *Device;
	XPm_Requirement *Reqm;
	u8 ThisSubsystemCount = 0;
	u8 OtherSubsystemCount = 0;

	if (SubsystemId > XPM_SUBSYSID_MAX) {
		goto done;
	}

	Subsystem = &PmSubsystems[SubsystemId];

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		goto done;
	}

	/*
	 * Return the power state of the device.
	 */
	 if (NULL == Device->Power) {
	 	goto done;
	}
	DeviceStatus->Status = Device->Power->Node.State;

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if ((Subsystem == Reqm->Subsystem) &&
			(TRUE == Reqm->Allocated)) {
			ThisSubsystemCount++;
			DeviceStatus->Requirement = Reqm->Curr.Capabilities;
		} else if (TRUE == Reqm->Allocated) {
			OtherSubsystemCount++;
		} else {
			/* MISRA Requirement? */
		}
		Reqm = Reqm->NextSubsystem;
	}

	if ((0 == ThisSubsystemCount) && (0 == OtherSubsystemCount)) {
		DeviceStatus->Usage = 0;
	} else if ((1 == ThisSubsystemCount) && (0 == OtherSubsystemCount)) {
		DeviceStatus->Usage = 1;
	} else if ((0 == ThisSubsystemCount) && (0 != OtherSubsystemCount)) {
		DeviceStatus->Usage = 2;
	} else if ((0 != ThisSubsystemCount) && (0 != OtherSubsystemCount)) {
		DeviceStatus->Usage = 3;
	} else {
		/* XXX - xil_assert() - should not get here */
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmDevice_Alloc(u32 NumDevices)
{
	int Status = XST_SUCCESS;

	return Status;
}

XStatus XPmDevice_AddParent(u32 Id, u32 *Parents, u32 NumParents)
{
	XStatus Status = XST_SUCCESS;
	u32 i = 0;
	u32 DeviceIndex = NODEINDEX(Id);
	XPm_Device *DevPtr = XPmDevice_GetById(Id);

	if(DevPtr == NULL || DeviceIndex > MaxDevices || NumParents == 0)
	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for(i=0;i<NumParents;i++)
	{
		if(NODECLASS(Parents[i]) == XPM_NODECLASS_CLOCK)
		{
			XPm_ClockNode *Clk = XPmClock_GetById(Parents[i]);
			if (NULL == Clk) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = XPmDevice_AddClock(DevPtr, Clk);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		else if(NODECLASS(Parents[i]) == XPM_NODECLASS_RESET)
		{
			XPm_ResetNode *Rst;
			if(DevPtr->Reset == NULL)
				DevPtr->Reset = RstNodeList[NODEINDEX(Parents[i])];
			else
			{
				Rst = DevPtr->Reset;
				while(Rst->NextReset != NULL)
					Rst = Rst->NextReset;
				Rst->NextReset = RstNodeList[NODEINDEX(Parents[i])];
			}
		}
		else if(NODECLASS(Parents[i]) == XPM_NODECLASS_POWER)
		{
			if(DevPtr->Power != NULL)
			{
				Status = XST_INVALID_PARAM;
				goto done;
			}
			else
				DevPtr->Power = PmPowers[NODEINDEX(Parents[i])];
		}
		else
		{
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}
done:
	return Status;
}

XStatus XPmDevice_GetPermissions(XPm_Device *Device, u32 *PermissionMask)
{
	XStatus Status = XST_SUCCESS;
	XPm_Requirement *Reqm;
	u32 Idx;

	if ((NULL == Device) || (NULL == PermissionMask)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (TRUE == Reqm->Allocated) {
			for (Idx = 0; Idx < XPM_SUBSYSID_MAX; Idx++) {
				if (Reqm->Subsystem == &PmSubsystems[Idx]) {
					*PermissionMask |= (1 << Idx);
				}
			}
		}
		Reqm = Reqm->NextSubsystem;
	}

done:
	return Status;
}

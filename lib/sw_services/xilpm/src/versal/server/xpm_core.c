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

#include "xpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&Core->Device, Id, 0, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->DebugMode = 0;
	Core->ImageId = 0;
	Core->Ipi = IpiCh;
	Core->CoreOps = Ops;
	Core->PwrUpLatency = 0;
	Core->PwrDwnLatency = 0;

done:
	return Status;
}

int XPmCore_StoreResumeAddr(XPm_Core *Core, u64 Address)
{
	int Status = XST_FAILURE;
	u32 Idx;

	/* Check for valid resume address */
	if (0U == (Address & 1ULL)) {
		PmErr("Invalid resume address\r\n");
		goto done;
	}

	/* Check for the version of the PsmToPlmEvent structure */
	if (PsmToPlmEvent->Version != PSM_TO_PLM_EVENT_VERSION) {
		PmErr("PSM-PLM are out of sync. Can't set resume address.\n\r");
		goto done;
	}

	for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
		/* Store the resume address to PSM reserved RAM location */
		if (ProcDevList[Idx] == Core->Device.Node.Id) {
			PsmToPlmEvent->ResumeAddress[Idx] = Address;
			break;
		}
	}
	if (Idx < ARRAY_SIZE(ProcDevList)) {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

int XPmCore_SetCPUIdleFlag(XPm_Core *Core, u32 CpuIdleFlag)
{
	int Status = XST_FAILURE;
	u32 Idx;

	/* PsmToPlmEvent version is already checked in StoreResumeAddr function */

	for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
		/* Store the CPU idle flag to PSM reserved RAM location */
		if (ProcDevList[Idx] == Core->Device.Node.Id) {
			PsmToPlmEvent->CpuIdleFlag[Idx] = CpuIdleFlag;
			break;
		}
	}
	if (Idx < ARRAY_SIZE(ProcDevList)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

int XPmCore_GetCPUIdleFlag(XPm_Core *Core, u32 *CpuIdleFlag)
{
	int Status = XST_FAILURE;
	u32 Idx;

	/* PsmToPlmEvent version is already checked in StoreResumeAddr function */

	for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
		/* Store the CPU idle flag to PSM reserved RAM location */
		if (ProcDevList[Idx] == Core->Device.Node.Id) {
			*CpuIdleFlag = PsmToPlmEvent->CpuIdleFlag[Idx];
			break;
		}
	}
	if (Idx < ARRAY_SIZE(ProcDevList)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	DISABLE_WAKE(Core->SleepMask);

	if (((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State) &&
	    (NULL != Core->Device.Power)) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set reset address */
	if (1U == SetAddress) {
		Status = XPmCore_StoreResumeAddr(Core, (Address | 1U));
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->CoreOps && NULL != Core->CoreOps->RestoreResumeAddr) {
		Status = Core->CoreOps->RestoreResumeAddr(Core);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if ((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State) {
		if (NULL != Core->Device.ClkHandles) {
			Status = XPmClock_Request(Core->Device.ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Status = XPm_DirectPwrUp(Core->Device.Node.Id);
	}

	/* Release reset for all resets attached to this core */
	Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;

done:
	return Status;
}

int XPmCore_AfterDirectWakeUp(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if ((u32)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Request(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	if ((u32)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		DISABLE_WFI(Core->SleepMask);
	}

	/* If parent is on, then only send sleep request */
	if ((Core->Device.Power->Parent->Node.State == (u8)XPM_POWER_STATE_ON) &&
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(Core->Device.Node.Id))) {
		/* Power down the core */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmCore_AfterDirectPwrDwn(Core);

done:
	return Status;
}

int XPmCore_AfterDirectPwrDwn(XPm_Core *Core)
{
	int Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Release(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;

done:
	return Status;
}

int XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	int Status = XST_SUCCESS;
	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	XPm_Power *Power;
	u32 Lat = 0;

	*Latency = 0;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		goto done;
	}

	*Latency += Core->PwrUpLatency;
	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		*Latency += Core->PwrDwnLatency;
		goto done;
	}

	Power = Core->Device.Power;
	if (NULL != Power) {
		Status = XPmPower_GetWakeupLatency(Power->Node.Id, &Lat);
		if (XST_SUCCESS == Status) {
			*Latency += Lat;
		}
	}

done:
	return Status;
}

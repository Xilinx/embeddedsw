/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"
#include "xpm_debug.h"
#include "xpm_requirement.h"
#include "xpm_subsystem.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmDevice_Init(&Core->Device, Id, 0, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_DEVICE_INIT;
		goto done;
	}

	Core->DebugMode = 0;
	Core->ImageId = 0;
	Core->Ipi = IpiCh;
	Core->CoreOps = Ops;
	Core->PwrUpLatency = 0;
	Core->PwrDwnLatency = 0;
	Core->isCoreUp = 0;
	Core->IsCoreIdleSupported = 0U;
	Core->PsmToPlmEvent_ProcIdx = (u8)PROC_DEV_MAX;

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Id)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Id))) {
		/* Find and store PsmToPlmEvent_ProcIdx in Core structure */
		for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
			if (ProcDevList[Idx] == Id) {
				Core->PsmToPlmEvent_ProcIdx = (u8)Idx;
				break;
			}
		}
		if (Idx >= ARRAY_SIZE(ProcDevList)) {
			DbgErr = XPM_INT_ERR_INVALID_PROC;
			Status = XST_FAILURE;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	const XPm_Power *Power;
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

XStatus XPmCore_SetCPUIdleFlag(const XPm_Core *Core, u32 CpuIdleFlag)
{
	XStatus Status = XST_FAILURE;

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		goto done;
	}

	/* Store the CPU idle flag to PSM reserved RAM location */
	PsmToPlmEvent->CpuIdleFlag[Core->PsmToPlmEvent_ProcIdx] = CpuIdleFlag;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmCore_StoreResumeAddr(const XPm_Core *Core, u64 Address)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Check for valid resume address */
	if (0U == (Address & 1ULL)) {
		DbgErr = XPM_INT_ERR_INVALID_RESUME_ADDR;
		goto done;
	}

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		DbgErr = XPM_INT_ERR_INVALID_PROC;
		goto done;
	}

	/* Store the resume address to PSM reserved RAM location */
	PsmToPlmEvent->ResumeAddress[Core->PsmToPlmEvent_ProcIdx] = Address;
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

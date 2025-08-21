/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_util.h"
#include "xpm_requirement.h"
#include "xpm_power.h"
#include "xpm_api.h"
#include "xpm_runtime_device.h"
#include "xpm_aiedevice.h"
#include "xpm_device_fsm.h"

static XStatus XPmRequirement_Init(XPm_Requirement *Reqm, XPm_Subsystem *Subsystem,
				XPm_Device *Device, u32 Flags,
				u32 PreallocCaps, u32 PreallocQoS)
{
	XStatus Status = XST_FAILURE;
	Reqm->Subsystem = Subsystem;
	Reqm->Device = Device;
	Reqm->Allocated = 0;
	Reqm->SetLatReq = 0;
	Reqm->Flags = (u16)(Flags & REG_FLAGS_MASK);
	Reqm->PreallocCaps = (u8)PreallocCaps;
	Reqm->PreallocQoS = PreallocQoS;

	Reqm->Curr.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Curr.Latency = XPM_MAX_LATENCY;
	Reqm->Curr.QoS = XPM_MAX_QOS;
	Reqm->Next.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Next.Latency = XPM_MAX_LATENCY;
	Reqm->Next.QoS = XPM_MAX_QOS;
	Reqm->IsPending = 0;
	/* Prepend Reqm to Subsystem->Requirements */
	LIST_PREPEND(Subsystem->Requirements, Reqm);

	/* Prepend Reqm to RuntimeOps->Requirements */
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		Status = XST_FAILURE;
		goto done;
	}
	/* Prepend Reqm to Device->Requirements */
	LIST_PREPEND(DevOps->Requirements, Reqm);
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device,
			   u32 Flags, u32 PreallocCaps, u32 PreallocQoS)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;
	if (Subsystem->Requirements == NULL) {
		Subsystem->Requirements = Make_XPm_RequirementList();
		if (Subsystem->Requirements == NULL) {
			PmErr("Can not allocate Memory for Subsystem Requirements\r\n");
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	XPmRuntime_DeviceOps* DevOps = XPmDevice_SetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		/** TODO: Create Error code for this */
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}

	if (DevOps->Requirements == NULL) {
		DevOps->Requirements = Make_XPm_RequirementList();
		if (DevOps->Requirements == NULL) {
			PmErr("Can not allocate Memory for Device Requirements\r\n");
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	Status = XPmDeviceFsm_Init(DevOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Reqm = (XPm_Requirement *)XPm_AllocBytesReqm(sizeof(XPm_Requirement));
	if (NULL == Reqm) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmRequirement_Init(Reqm, Subsystem, Device, Flags, PreallocCaps, PreallocQoS);

done:
	return Status;
}

void XPm_RequiremntUpdate(XPm_Requirement *Reqm)
{
	if(NULL != Reqm)
	{
		Reqm->Next.Capabilities = Reqm->Curr.Capabilities;
		Reqm->Next.Latency = Reqm->Curr.Latency;
		Reqm->Next.QoS = Reqm->Curr.QoS;
	}
}

void XPmRequirement_Clear(XPm_Requirement* Reqm)
{
	if(NULL != Reqm) {
		/* Clear flag - master is not using slave anymore */
		Reqm->Allocated = 0;
		Reqm->IsPending = 0;
		/* Release current and next requirements */
		Reqm->Curr.Capabilities = XPM_MIN_CAPABILITY;
		Reqm->Curr.Latency = XPM_MAX_LATENCY;
		Reqm->Curr.QoS = XPM_MAX_QOS;
		Reqm->Next.Capabilities = XPM_MIN_CAPABILITY;
		Reqm->Next.Latency = XPM_MAX_LATENCY;
		Reqm->Next.QoS = XPM_MAX_QOS;
	}
}

XStatus XPmRequirement_ReleaseFromAllSubsystem(XPm_Device* Device)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		if (1U != ReqmNode->Data->Allocated) {
			continue;
		}
		Status = XPmDevice_Release(ReqmNode->Data->Subsystem->Id, ReqmNode->Data->Device->Node.Id,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}

	}
done:
	return Status;
}


XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Reqm) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (RELEASE_ONE == Scope) {
		Status = XPmDevice_Release(Reqm->Subsystem->Id, Reqm->Device->Node.Id,
					   XPLMI_CMD_SECURE);
		goto done;
	}

	/*
	 * Release requirements of a device from all subsystems that are
	 * sharing the device.
	 */

	if (RELEASE_DEVICE == Scope) {
		Status = XPmRequirement_ReleaseFromAllSubsystem(Reqm->Device);
		goto done;
	}

	/**
	 * Release all device from given subsystem from the requirement
	 */
	LIST_FOREACH(Reqm->Subsystem->Requirements, ReqmNode) {
		XPm_Requirement* cReqm = ReqmNode->Data;
		if ((((RELEASE_ALL == Scope) && (1U == cReqm->Allocated)) ||
			 ((RELEASE_UNREQUESTED == Scope) && (0U == cReqm->Allocated))) &&
			 ((u32)XPM_NODETYPE_DEV_DDR != NODETYPE(cReqm->Device->Node.Id))) {
			Status = XPmDevice_Release(cReqm->Subsystem->Id, cReqm->Device->Node.Id,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Triggers the setting for scheduled requirements
 *
 * @param Subsystem	Subsystem which changed the state and whose scheduled
			requirements are triggered
 * @param Swap	Flag stating should current requirements be saved as next
 *
 * @note 	a) swap=false
 *		Set scheduled requirements of a subsystem without swapping
 *		current and next requirements - means the current requirements
 *		will be dropped. Upon every self suspend, subsystem has to
 *		explicitly re-request device requirements.
 *		b) swap=true
 *		Set scheduled requirements of a subsystem with swapping current
 *		and next requirements (swapping means the current requirements
 *		will be saved as next, and will be configured once subsystem
 *		wakes-up).
 *
 ****************************************************************************/
XStatus XPmRequirement_UpdateScheduled(const XPm_Subsystem *Subsystem, u32 Swap)
{
	XStatus Status = XST_FAILURE;
	XPm_ReqmInfo TempReq;
	LIST_FOREACH(Subsystem->Requirements, ReqmNode)
	{
		XPm_Requirement *Reqm = ReqmNode->Data;
		if (Reqm->Curr.Capabilities != Reqm->Next.Capabilities) {
			TempReq.Capabilities = Reqm->Next.Capabilities;
			TempReq.Latency = Reqm->Next.Latency;
			TempReq.QoS = Reqm->Next.QoS;

			if (1U == Swap) {
				Reqm->Next.Capabilities = Reqm->Curr.Capabilities;
				Reqm->Next.Latency = Reqm->Curr.Latency;
				Reqm->Next.QoS = Reqm->Curr.QoS;
			}

			Reqm->Curr.Capabilities = TempReq.Capabilities;
			Reqm->Curr.Latency = TempReq.Latency;
			Reqm->Curr.QoS = TempReq.QoS;

			Status = XPmDevice_UpdateStatus(Reqm->Device);
			if (XST_SUCCESS != Status) {
				PmErr("Updating %x\r\n", Reqm->Device->Node.Id);
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmRequirement_IsExclusive(const XPm_Requirement *Reqm)
{
	XStatus Status = XST_FAILURE;

	if (NULL == Reqm) {
		goto done;
	}

	if (1U != Reqm->Allocated) {
		goto done;
	}
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Reqm->Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		Status = XST_FAILURE;
		goto done;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		if (1 == ReqmNode->Data->Allocated) {
			goto done;
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}


XPm_Requirement* FindReqm(const XPm_Device *Device,const XPm_Subsystem *Subsystem)
{
	if (Device == NULL || Subsystem == NULL) {
		return NULL;
	}
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		if (!(IS_DEV_AIE(Device->Node.Id))) {
			PmErr("DeviceOps is NULL\r\n");
		}
		return NULL;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		XPm_Requirement *cReqm = ReqmNode->Data;
		if (NULL == cReqm) {
			PmErr("Not expecting null here \n\r");
			return NULL;
		}
		if (cReqm->Subsystem == Subsystem) {
			return cReqm;
		}
	}

	return NULL; // No matching requirement found
}

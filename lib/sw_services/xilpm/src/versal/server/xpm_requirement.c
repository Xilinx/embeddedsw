/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_util.h"
#include "xpm_requirement.h"
#include "xpm_power.h"
#include "xpm_api.h"

static void XPmRequirement_Init(XPm_Requirement *Reqm, XPm_Subsystem *Subsystem,
				XPm_Device *Device, u32 Flags, u32 AperPerm,
				u32 PreallocCaps, u32 PreallocQoS)
{
	/* Prepend to subsystem's device reqm list */
	Reqm->NextDevice = Subsystem->Requirements;
	Subsystem->Requirements = Reqm;
	Reqm->Subsystem = Subsystem;

	/* Prepend to device's subsystem reqm list */
	Reqm->NextSubsystem = Device->Requirements;
	Device->Requirements = Reqm;
	Reqm->Device = Device;

	Reqm->Allocated = 0;
	Reqm->SetLatReq = 0;
	Reqm->Flags = (u16)(Flags & REG_FLAGS_MASK);
	Reqm->AperPerm = AperPerm;
	Reqm->PreallocCaps = (u8)PreallocCaps;
	Reqm->PreallocQoS = PreallocQoS;

	Reqm->Curr.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Curr.Latency = XPM_MAX_LATENCY;
	Reqm->Curr.QoS = XPM_MAX_QOS;
	Reqm->Next.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Next.Latency = XPM_MAX_LATENCY;
	Reqm->Next.QoS = XPM_MAX_QOS;
}

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device,
			   u32 Flags, u32 AperPerm, u32 PreallocCaps,
			   u32 PreallocQoS)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;

	Reqm = (XPm_Requirement *)XPm_AllocBytes(sizeof(XPm_Requirement));
	if (NULL == Reqm) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	XPmRequirement_Init(Reqm, Subsystem, Device, Flags, AperPerm,
			    PreallocCaps, PreallocQoS);
	Status = XST_SUCCESS;

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
		/* Release current and next requirements */
		Reqm->Curr.Capabilities = XPM_MIN_CAPABILITY;
		Reqm->Curr.Latency = XPM_MAX_LATENCY;
		Reqm->Curr.QoS = XPM_MAX_QOS;
		Reqm->Next.Capabilities = XPM_MIN_CAPABILITY;
		Reqm->Next.Latency = XPM_MAX_LATENCY;
		Reqm->Next.QoS = XPM_MAX_QOS;
	}
}

XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *NextReqm = NULL;

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
		NextReqm = Reqm;
		while (NULL != NextReqm) {
			Status = XPmRequirement_Release(NextReqm, RELEASE_ONE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			NextReqm = NextReqm->NextSubsystem;
		}
		goto done;
	}

	while (NULL != Reqm) {
		if ((((RELEASE_ALL == Scope) && (1U == Reqm->Allocated)) ||
		     ((RELEASE_UNREQUESTED == Scope) && (0U == Reqm->Allocated))) &&
		     ((u32)XPM_NODETYPE_DEV_DDR != NODETYPE(Reqm->Device->Node.Id))) {
			Status = XPmDevice_Release(Reqm->Subsystem->Id, Reqm->Device->Node.Id,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
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
	XPm_Requirement *Reqm = Subsystem->Requirements;
	XPm_ReqmInfo TempReq;

	if (NULL == Reqm) {
		Status = XST_SUCCESS;
		goto done;
	}

	while (NULL != Reqm) {
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
		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmRequirement_IsExclusive(const XPm_Requirement *Reqm)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Next_Reqm;

	if (NULL == Reqm) {
		goto done;
	}

	if (1U != Reqm->Allocated) {
		goto done;
	}

	Next_Reqm = Reqm->NextSubsystem;
	while (NULL != Next_Reqm) {
		if (1U == Next_Reqm->Allocated) {
			goto done;
		}
		Next_Reqm = Next_Reqm->NextSubsystem;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

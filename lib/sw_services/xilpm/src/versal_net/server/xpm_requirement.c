/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_util.h"
#include "xpm_requirement.h"
#include "xpm_power.h"
#include "xpm_api.h"

static void XPmRequirement_Init(XPm_Requirement *Reqm, XPm_Subsystem *Subsystem,
				XPm_Device *Device, u32 Flags,
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
			   u32 Flags, u32 PreallocCaps, u32 PreallocQoS)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;

	Reqm = (XPm_Requirement *)XPm_AllocBytes(sizeof(XPm_Requirement));
	if (NULL == Reqm) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	XPmRequirement_Init(Reqm, Subsystem, Device, Flags, PreallocCaps, PreallocQoS);
	Status = XST_SUCCESS;

done:
	return Status;
}
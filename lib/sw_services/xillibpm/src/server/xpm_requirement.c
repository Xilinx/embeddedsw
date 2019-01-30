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

#include "xpm_requirement.h"
#include "xpm_power.h"
#include "xillibpm_api.h"

XStatus XPmRequirement_Init(XPm_Requirement *Reqm,
		XPm_Subsystem *Subsystem, XPm_Device *Device)
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
	Reqm->Curr.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Curr.Latency = XPM_MAX_LATENCY;
	Reqm->Curr.QoS = XPM_MAX_QOS;
	Reqm->Next.Capabilities = XPM_MIN_CAPABILITY;
	Reqm->Next.Latency = XPM_MAX_LATENCY;
	Reqm->Next.QoS = XPM_MAX_QOS;

	return XST_SUCCESS;
}

XStatus XPmRequirement_Add(XPm_Subsystem *Subsystem, XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;

	Reqm = (XPm_Requirement *)XPm_AllocBytes(sizeof(XPm_Requirement));
	if (NULL == Reqm) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmRequirement_Init(Reqm, Subsystem, Device);
done:
	return Status;
}

void XPm_RequiremntUpdate(XPm_Requirement *Reqm)
{
	Reqm->Curr.Capabilities = Reqm->Next.Capabilities;
	Reqm->Curr.Latency = Reqm->Next.Latency;
	Reqm->Curr.QoS = Reqm->Next.QoS;
}

void XPmRequirement_Clear(XPm_Requirement* Reqm)
{
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

XStatus XPmRequirement_Release(XPm_Requirement *Reqm, XPm_ReleaseScope Scope)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *NextReqm = NULL;

	if (RELEASE_ONE == Scope) {
		XPmRequirement_Clear(Reqm);
		Status = Reqm->Device->Node.HandleEvent((XPm_Node *)Reqm->Device,
							XPM_DEVEVENT_SHUTDOWN);
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
		if (((RELEASE_ALL == Scope) && (1 == Reqm->Allocated)) ||
		    ((RELEASE_UNREQUESTED == Scope) && (0 == Reqm->Allocated))) {
			XPmRequirement_Clear(Reqm);
			Status = Reqm->Device->Node.HandleEvent((XPm_Node *)Reqm->Device,
								XPM_DEVEVENT_SHUTDOWN);
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
XStatus XPmRequirement_UpdateScheduled(XPm_Subsystem *Subsystem, u32 Swap)
{
	XStatus Status = XST_SUCCESS;
	XPm_Requirement *Reqm = Subsystem->Requirements;
	XPm_ReqmInfo TempReq;

	while (NULL != Reqm) {
		if (Reqm->Curr.Capabilities != Reqm->Next.Capabilities) {
			TempReq.Capabilities = Reqm->Next.Capabilities;
			TempReq.Latency = Reqm->Next.Latency;
			TempReq.QoS = Reqm->Next.QoS;

			if (TRUE == Swap) {
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
				break;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	return Status;
}

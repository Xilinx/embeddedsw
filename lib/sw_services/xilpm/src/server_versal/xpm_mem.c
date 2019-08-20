/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
#include "xillibpm_defs.h"
#include "xplmi_dma.h"
#include "xpm_device.h"
#include "xpm_mem.h"
#include "xpm_rpucore.h"

#define XPM_TCM_BASEADDRESS_MODE_OFFSET	0x80000U

static const XPm_StateCap XPmMemDeviceStates[] = {
	{
		.State = XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = XPM_DEVSTATE_RUNNING,
		.Cap = PM_CAP_ACCESS | PM_CAP_CONTEXT,
	},
};

static const XPm_StateTran XPmMemDevTransitions[] = {
	{
		.FromState = XPM_DEVSTATE_RUNNING,
		.ToState = XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = XPM_DEVSTATE_UNUSED,
		.ToState = XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static void TcmEccInit(XPm_MemDevice *Tcm, u32 Mode)
{
	u32 Size = Tcm->EndAddress - Tcm->StartAddress;
	u32 Id = Tcm->Device.Node.Id;
	u32 Base = Tcm->StartAddress;

	if (XPM_DEVID_TCM_1_A == Id || XPM_DEVID_TCM_1_B == Id) {
		if (XPM_RPU_MODE_LOCKSTEP == Mode)
			Base -= XPM_TCM_BASEADDRESS_MODE_OFFSET;
	}
	if (Size) {
		XPlmi_EccInit(Base, Size);
	}
	return;
}

static XStatus HandleTcmDeviceState(XPm_Device* Device, u32 NextState)
{
	XStatus Status = XST_SUCCESS;
	XPm_Device *Rpu0Device = XPmDevice_GetById(XPM_DEVID_R50_0);
	XPm_Device *Rpu1Device = XPmDevice_GetById(XPM_DEVID_R50_1);
	u32 Id = Device->Node.Id;
	u32 Mode;

	switch (Device->Node.State) {
	case XPM_DEVSTATE_UNUSED:
		if (XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(&Device->Node);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			/* TCM is only accessible when the RPU is powered on and out of reset and is in halted state
			 * so bring up RPU too when TCM is requested*/
			XPm_RpuGetOperMode(XPM_DEVID_R50_0, &Mode);
			 if (XPM_RPU_MODE_SPLIT == Mode)
			 {
				if ((XPM_DEVID_TCM_0_A == Id ||
				     XPM_DEVID_TCM_0_B == Id) &&
				    (XPM_DEVSTATE_RUNNING !=
				     Rpu0Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu0Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
				if ((XPM_DEVID_TCM_1_A == Id ||
				     XPM_DEVID_TCM_1_B == Id) &&
				    (XPM_DEVSTATE_RUNNING !=
				     Rpu1Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu1Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			 }
			 if (XPM_RPU_MODE_LOCKSTEP == Mode)
			 {
				if ((XPM_DEVID_TCM_0_A == Id ||
				     XPM_DEVID_TCM_0_B == Id ||
				     XPM_DEVID_TCM_1_A == Id ||
				     XPM_DEVID_TCM_1_B == Id) &&
				     (XPM_DEVSTATE_RUNNING !=
				      Rpu0Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu0Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}
			/* Tcm should be ecc initialized */
			TcmEccInit((XPm_MemDevice *)Device, Mode);
		}
		break;
	case XPM_DEVSTATE_RUNNING:
		if (XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->Node.HandleEvent((XPm_Node *)Device,
							  XPM_DEVEVENT_SHUTDOWN);
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}

static const XPm_DeviceFsm XPmTcmDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleTcmDeviceState,
};

static XStatus HandleMemDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_SUCCESS;

	switch (Device->Node.State) {
	case XPM_DEVSTATE_UNUSED:
		if (XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(&Device->Node);
		}
		break;
	case XPM_DEVSTATE_RUNNING:
		if (XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->Node.HandleEvent((XPm_Node *)Device,
							  XPM_DEVEVENT_SHUTDOWN);
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmMemDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleMemDeviceState,
};

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset, u32 MemStartAddress,
		u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Type = NODETYPE(Id);

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;

	if (XPM_NODETYPE_DEV_TCM == Type) {
		MemDevice->Device.DeviceFsm = &XPmTcmDeviceFsm;
	} else {
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
	}

done:
	return Status;
}

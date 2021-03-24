/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_periph.h"
#include "xpm_gic_proxy.h"
#include "xpm_defs.h"

static struct XPm_PeriphOps GenericOps = {
	.SetWakeupSource = XPmGicProxy_WakeEventSet,
};

XStatus XPmPeriph_Init(XPm_Periph *Periph, u32 Id, u32 BaseAddress,
		       XPm_Power *Power, XPm_ClockNode *Clock,
		       XPm_ResetNode *Reset, u32 GicProxyMask,
		       u32 GicProxyGroup)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&Periph->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Periph->PeriphOps = &GenericOps;
	Periph->GicProxyMask = GicProxyMask;
	Periph->GicProxyGroup = GicProxyGroup;

done:
	return Status;
}


static const XPm_StateCap XPmVirtDev_VirtDevStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = XPM_MAX_CAPABILITY,
	},
};

static const XPm_StateTran XPmVirtDev_VirtDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus HandleVirtDevState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmVirtDevFsm = {
	DEFINE_DEV_STATES(XPmVirtDev_VirtDevStates),
	DEFINE_DEV_TRANS(XPmVirtDev_VirtDevTransitions),
	.EnterState = HandleVirtDevState,
};

XStatus XPmVirtDev_DeviceInit(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Device->DeviceFsm = &XPmVirtDevFsm;

done:
	return Status;
}

/*
 * Implementation for Healthy Boot Monitor
 */
static const XPm_StateCap XPmHbMonDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = PM_CAP_ACCESS,
	},
};

static const XPm_StateTran XPmHbMonDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus HandleHbMonDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_SUCCESS;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
		    //todo: Start the timer
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
		    //todo: stop the timer.
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmHbMonDeviceFsm = {
	DEFINE_DEV_STATES(XPmHbMonDeviceStates),
	DEFINE_DEV_TRANS(XPmHbMonDevTransitions),
	.EnterState = HandleHbMonDeviceState,
};

XStatus XPmHbMonDev_Init(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Device->DeviceFsm = &XPmHbMonDeviceFsm;

done:
	return Status;
}

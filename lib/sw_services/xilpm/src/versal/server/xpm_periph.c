/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_periph.h"
#include "xpm_gic_proxy.h"

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

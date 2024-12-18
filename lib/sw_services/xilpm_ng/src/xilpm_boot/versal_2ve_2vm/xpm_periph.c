/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_api.h"
#include "xpm_periph.h"
#include "xpm_defs.h"
#include "xplmi_err_common.h"
#include "xplmi_scheduler.h"
#include "xpm_regs.h"
#include "xpm_common.h"

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
	Periph->GicProxyMask = GicProxyMask;
	Periph->GicProxyGroup = GicProxyGroup;

done:
	return Status;
}

XStatus XPmHbMonDev_Init(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

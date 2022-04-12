/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xplmi_dma.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_rpucore.h"
#include "xpm_npdomain.h"
#include "xpm_debug.h"

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
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

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_TCM:
		/*TBD: add tcmfsm */
		MemDevice->Device.DeviceFsm = NULL;
		break;
	default:
		/*TBD: add device fsm */
		MemDevice->Device.DeviceFsm = NULL;
		break;
	}

done:
	return Status;
}

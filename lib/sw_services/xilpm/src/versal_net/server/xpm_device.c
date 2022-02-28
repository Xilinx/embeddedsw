/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_device.h"

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	//changed to support minimum boot time xilpm
	PmDbg("DeviceId %x\n",DeviceId);
	(void)SubsystemId;
	(void)DeviceId;
	(void)DeviceStatus;
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

}

/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_nodeid.h"
#include "xpm_plat_proc.h"

XStatus XPmPlatAddProcDevice(u32 DeviceId, u32 Ipi, const u32 *BaseAddr, const XPm_Power *Power)
{
	(void)DeviceId;
	(void)Ipi;
	(void)BaseAddr;
	(void)Power;
	// @TODO Add platform specific processor device code
	return XST_FAILURE;
}

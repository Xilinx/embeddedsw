/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_asucore.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_psm.h"

XStatus XPmAsuCore_Init(struct XPm_AsuCore *AsuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&AsuCore->Core, Id, Power, Clock, Reset, (u8)Ipi, NULL);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	AsuCore->AsuBaseAddr = BaseAddress[0];
	// @TODO Add code to assign register addres to AsuCore
	//XPmRpuCore_AssignRegAddr(AsuCore, Id, BaseAddress);

done:
	return Status;
}

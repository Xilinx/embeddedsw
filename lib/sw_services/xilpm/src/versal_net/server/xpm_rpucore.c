/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_psm.h"

XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	/*TBD: add rpuops*/
	Status = XPmCore_Init(&RpuCore->Core, Id, Power, Clock, Reset, (u8)Ipi,
			      NULL);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	RpuCore->RpuBaseAddr = BaseAddress[0];

	if (PM_DEV_RPU_A_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_0_PWR_CTRL_MASK;
	} else if (PM_DEV_RPU_A_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_1_PWR_CTRL_MASK;
	} else if (PM_DEV_RPU_B_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_0_PWR_CTRL_MASK;
	} else if (PM_DEV_RPU_B_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_1_PWR_CTRL_MASK;
	}

done:
	return Status;
}
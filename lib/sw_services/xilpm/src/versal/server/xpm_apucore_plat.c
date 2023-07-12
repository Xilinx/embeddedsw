/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"

XStatus XPmApuCore_AssignRegisterMask(XPm_ApuCore *ApuCore, const u32 Id)
{
	XStatus Status = XST_FAILURE;

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_0_CPUPWRDWNREQ_MASK;
		ApuCore->PcilPwrDwnReg = PSM_GLOBAL_REG + PSM_GLOBAL_APU_POWER_STATUS_INIT_OFFSET;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_1_CPUPWRDWNREQ_MASK;
		ApuCore->PcilPwrDwnReg = PSM_GLOBAL_REG + PSM_GLOBAL_APU_POWER_STATUS_INIT_OFFSET;
		Status = XST_SUCCESS;
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

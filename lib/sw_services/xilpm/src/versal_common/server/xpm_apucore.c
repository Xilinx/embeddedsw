/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"

static XStatus XPmApuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_WakeUp(Core, SetAddress, Address);
	if (XST_SUCCESS != Status) {
		PmErr("Status = %x\r\n", Status);
		goto done;
	}

done:
	return Status;
}

static XStatus XPmApuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_PwrDwn(Core);
	if (XST_SUCCESS != Status) {
		PmErr("Status = %x\r\n", Status);
	}

	return Status;
}

static struct XPm_CoreOps ApuOps = {
	.RequestWakeup = &XPmApuCore_WakeUp,
	.PowerDown = &XPmApuCore_PwrDwn,
};

XStatus XPmApuCore_Init(XPm_ApuCore *ApuCore,
	u32 Id,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmCore_Init(&ApuCore->Core, Id, Power, Clock, Reset, (u8)Ipi,
			      &ApuOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CORE_INIT;
		goto done;
	}

	ApuCore->FpdApuBaseAddr = BaseAddress[0];
	Status = XPmApuCore_AssignRegisterMask(ApuCore, Id);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

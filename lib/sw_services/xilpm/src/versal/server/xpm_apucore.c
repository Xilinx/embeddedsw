/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"

static int XPmApuCore_RestoreResumeAddr(XPm_Core *Core)
{
	int Status = XST_FAILURE;
	u32 AddrLow = (u32) (Core->ResumeAddr & 0xfffffffeULL);
	u32 AddrHigh = (u32) (Core->ResumeAddr >> 32ULL);
	XPm_ApuCore *ApuCore = (XPm_ApuCore *)Core;

	/* Check for valid resume address */
	if (0U == (Core->ResumeAddr & 1ULL)) {
		PmErr("Invalid resume address\r\n");
		goto done;
	}

	if ((u32)XPM_NODEIDX_DEV_ACPU_0 == NODEINDEX(Core->Device.Node.Id)) {
		PmOut32(ApuCore->FpdApuBaseAddr + APU_DUAL_RVBARADDR0L_OFFSET, AddrLow);
		PmOut32(ApuCore->FpdApuBaseAddr + APU_DUAL_RVBARADDR0H_OFFSET, AddrHigh);
		Status = XST_SUCCESS;
	} else if ((u32)XPM_NODEIDX_DEV_ACPU_1 == NODEINDEX(Core->Device.Node.Id)) {
		PmOut32(ApuCore->FpdApuBaseAddr + APU_DUAL_RVBARADDR1L_OFFSET, AddrLow);
		PmOut32(ApuCore->FpdApuBaseAddr + APU_DUAL_RVBARADDR1H_OFFSET, AddrHigh);
		Status = XST_SUCCESS;
	} else {
		Status = XST_INVALID_PARAM;
	}

	Core->ResumeAddr = 0ULL;

done:
	return Status;
}

static int XPmApuCore_HasResumeAddr(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	if (0U != (Core->ResumeAddr & 1ULL)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus XPmApuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;

	/* Set reset address */
	if (1U == SetAddress) {
		Core->ResumeAddr = Address | 1U;
	}

	Status = XPmCore_WakeUp(Core);
	if (XST_SUCCESS != Status) {
		PmErr("Core Wake Up failed, Status = %x\r\n", Status);
		goto done;
	}

	/* Release reset for all resets attached to this core */
	Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;

done:
	return Status;
}

static XStatus XPmApuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_PwrDwn(Core);

	return Status;
}

static struct XPm_CoreOps ApuOps = {
		.RestoreResumeAddr = XPmApuCore_RestoreResumeAddr,
		.HasResumeAddr = XPmApuCore_HasResumeAddr,
		.RequestWakeup = XPmApuCore_WakeUp,
		.PowerDown = XPmApuCore_PwrDwn,
};

XStatus XPmApuCore_Init(XPm_ApuCore *ApuCore,
	u32 Id,
	u32 Ipi,
	u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&ApuCore->Core, Id, Power, Clock, Reset, (u8)Ipi,
			      &ApuOps);

	ApuCore->FpdApuBaseAddr = BaseAddress[0];

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_0_CPUPWRDWNREQ_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_1_CPUPWRDWNREQ_MASK;
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

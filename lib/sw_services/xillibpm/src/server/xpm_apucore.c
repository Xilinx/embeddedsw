/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"

static int XPmApuCore_RestoreResumeAddr(XPm_Core *Core)
{
	int Status = XST_SUCCESS;
	u32 AddrLow = (u32) (Core->ResumeAddr & 0xfffffffeULL);
	u32 AddrHigh = (u32) (Core->ResumeAddr >> 32ULL);

	/* Check for valid resume address */
	if (0 == (Core->ResumeAddr & 1ULL)) {
		PmErr("Invalid resume address\r\n");
		Status = XST_FAILURE;
		goto done;
	}

	if (XPM_NODEIDX_DEV_ACPU_0 == NODEINDEX(Core->Device.Node.Id)) {
		PmOut32(Core->Device.Node.BaseAddress + APU_DUAL_RVBARADDR0L_OFFSET, AddrLow);
		PmOut32(Core->Device.Node.BaseAddress + APU_DUAL_RVBARADDR0H_OFFSET, AddrHigh);
	} else if (XPM_NODEIDX_DEV_ACPU_1 == NODEINDEX(Core->Device.Node.Id)) {
		PmOut32(Core->Device.Node.BaseAddress + APU_DUAL_RVBARADDR1L_OFFSET, AddrLow);
		PmOut32(Core->Device.Node.BaseAddress + APU_DUAL_RVBARADDR1H_OFFSET, AddrHigh);
	} else {
		Status = XST_INVALID_PARAM;
	}

	Core->ResumeAddr = 0ULL;

done:
	return Status;
}

static XStatus XPmApuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;

	/* Set reset address */
	if (1 == SetAddress) {
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

	Core->Device.Node.State = XPM_DEVSTATE_RUNNING;

done:
	return Status;
}

static XStatus XPmApuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_PwrDwn(Core, FPD_APU_PWRCTL_OFFSET);

	return Status;
}

struct XPm_CoreOps ApuOps = {
		.RestoreResumeAddr = XPmApuCore_RestoreResumeAddr,
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

	Status = XPmCore_Init(&ApuCore->Core,
		Id, BaseAddress,
		Power, Clock, Reset, Ipi, &ApuOps);

	if(NODEINDEX(Id) == XPM_NODEIDX_DEV_ACPU_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_0_CPUPWRDWNREQ_MASK;
	} else if(NODEINDEX(Id) == XPM_NODEIDX_DEV_ACPU_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_1_CPUPWRDWNREQ_MASK;
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

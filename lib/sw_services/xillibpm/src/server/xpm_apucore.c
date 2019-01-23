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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"

static XStatus XPmApuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_ApuCore *ApuCore = (XPm_ApuCore *)Core;
	u32 AddrLow;
	u32 AddrHigh;

	/* Set reset address */
	if (0 == SetAddress) {
		if (Core->ResumeAddr & 1ULL) {
			Address = Core->ResumeAddr;
		} else {
			PmErr("Invalid resume address\r\n");
			Status = XST_FAILURE;
			goto done;
		}
	}

	AddrLow = (u32) (Address & 0xfffffffeULL);
	AddrHigh = (u32) (Address >> 32ULL);

	PmOut32(ApuCore->Core.Device.Node.BaseAddress + APU_DUAL_RVBARADDR0L_OFFSET, AddrLow);
	PmOut32(ApuCore->Core.Device.Node.BaseAddress + APU_DUAL_RVBARADDR0H_OFFSET, AddrHigh);

	if (XPM_DEVSTATE_RUNNING != Core->Device.Node.State) {
		Status = XPmCore_WakeUp(Core);
		if (XST_SUCCESS != Status) {
			PmErr("Core Wake Up failed, Status = %x\r\n", Status);
			goto done;
		}
	}

	/* Release reset for all resets attached to this core */
	Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_RELEASE);

	Core->ResumeAddr = 0ULL;
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

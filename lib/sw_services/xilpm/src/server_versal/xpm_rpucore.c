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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_api.h"

XStatus XPmRpuCore_Halt(XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)Device;

	/* Put RPU in  halt state */
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_NCPUHALT_MASK,
		~XPM_RPU_NCPUHALT_MASK);

	/* Release reset for all resets attached to this core */
	Status = XPmDevice_Reset(&RpuCore->Core.Device, PM_RESET_ACTION_RELEASE);

	return Status;
}

static int XPmRpuCore_RestoreResumeAddr(XPm_Core *Core)
{
	int Status = XST_SUCCESS;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;
	u32 AddrLow = (u32) (Core->ResumeAddr & 0xffff0000ULL);

	/* Check for valid resume address */
	if (0 == (Core->ResumeAddr & 1ULL)) {
		PmErr("Invalid resume address\r\n");
		Status = XST_FAILURE;
		goto done;
	}

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_PROC_RPU_HIVEC_ADDR == AddrLow) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			XPM_RPU_VINITHI_MASK);
	} else {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			~XPM_RPU_VINITHI_MASK);
	}

	Core->ResumeAddr = 0ULL;

done:
	return Status;
}

static int XPmRpuCore_HasResumeAddr(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	if (0U != (Core->ResumeAddr & 1ULL)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus XPmRpuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

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

	/* Put RPU in running state from halt state */
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_NCPUHALT_MASK,
		XPM_RPU_NCPUHALT_MASK);

	Core->Device.Node.State = XPM_DEVSTATE_RUNNING;

done:
	return Status;
}

static XStatus XPmRpuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPmRpuCore_Halt((XPm_Device *)Core);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmCore_PwrDwn(Core);

done:
	return Status;
}

struct XPm_CoreOps RpuOps = {
		.RestoreResumeAddr = XPmRpuCore_RestoreResumeAddr,
		.HasResumeAddr = XPmRpuCore_HasResumeAddr,
		.RequestWakeup = XPmRpuCore_WakeUp,
		.PowerDown = XPmRpuCore_PwrDwn,
};


XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&RpuCore->Core, Id, Power, Clock, Reset, Ipi,
			      &RpuOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	RpuCore->RpuBaseAddr = BaseAddress[0];

	if (PM_DEV_RPU0_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_0_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_0_CPUPWRDWNREQ_MASK;
	} else {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_1_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_1_CPUPWRDWNREQ_MASK;
	}

done:
	return Status;
}

void XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	u32 Val;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	PmIn32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);
	Val &= XPM_RPU_SLSPLIT_MASK;
	if (0 == Val) {
		*Mode = XPM_RPU_MODE_LOCKSTEP;
	} else {
		*Mode = XPM_RPU_MODE_SPLIT;
	}
}

void XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	u32 Val;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);
	int Status;
	XPm_Subsystem *DefSubsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);

	PmIn32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);
	if (Mode == XPM_RPU_MODE_SPLIT) {
		Val |= XPM_RPU_SLSPLIT_MASK;
		Val &= ~XPM_RPU_TCM_COMB_MASK;
		Val &= ~XPM_RPU_SLCLAMP_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		Val &= ~XPM_RPU_SLSPLIT_MASK;
		Val |= XPM_RPU_TCM_COMB_MASK;
		Val |= XPM_RPU_SLCLAMP_MASK;
	} else {
		/* Required by MISRA */
	}

	PmOut32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);

	/* Add or remove R50_1 core in default subsystem according to its mode */
	Status = XPmDevice_IsRequested(PM_DEV_RPU0_0, PM_SUBSYS_DEFAULT);
	if ((XST_SUCCESS == Status) && (ONLINE == DefSubsystem->State)) {
		if (Mode == XPM_RPU_MODE_SPLIT) {
			XPmDevice_Request(PM_SUBSYS_DEFAULT, PM_DEV_RPU0_1,
					  PM_CAP_ACCESS, XPM_MAX_QOS);
		} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
			Status = XPmDevice_IsRequested(PM_DEV_RPU0_1,
						       PM_SUBSYS_DEFAULT);
			if (XST_SUCCESS == Status) {
				XPmDevice_Release(PM_SUBSYS_DEFAULT,
						  PM_DEV_RPU0_1);
			}
		}
	}
}

XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr)
{
	XStatus Status = XST_SUCCESS;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_RPU_BOOTMEM_LOVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			~XPM_RPU_VINITHI_MASK);
	} else if (XPM_RPU_BOOTMEM_HIVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			XPM_RPU_VINITHI_MASK);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config)
{
	XStatus Status = XST_SUCCESS;
	u32 Address;
	XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	Address = RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET;
	if (Config == XPM_RPU_TCM_SPLIT) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			~XPM_RPU_TCM_COMB_MASK);
	} else if (Config == XPM_RPU_TCM_COMB) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			XPM_RPU_TCM_COMB_MASK);
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

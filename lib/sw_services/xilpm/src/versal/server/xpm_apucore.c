/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_prot.h"

static XStatus XPmApuCore_ProtControl(const XPm_Requirement *Reqm,
				      u32 Enable)
{
	const XPm_ApuCore *Apu = (XPm_ApuCore *)Reqm->Device;

	return XPmProt_PpuControl(Reqm, Apu->FpdApuBaseAddr, Enable);
}

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
	.RequestWakeup = XPmApuCore_WakeUp,
	.PowerDown = XPmApuCore_PwrDwn,
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
	}

	/* Protection handler for APU cores */
	ApuCore->Core.Device.HandleProtection = &XPmApuCore_ProtControl;

	ApuCore->FpdApuBaseAddr = BaseAddress[0];

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_0_CPUPWRDWNREQ_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_PWR_CTRL_MASK;
		ApuCore->Core.PwrDwnMask = XPM_ACPU_1_CPUPWRDWNREQ_MASK;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

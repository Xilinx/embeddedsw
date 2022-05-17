/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_power.h"
#include "xpm_psfpdomain.h"

#define XPM_APU_MODE_MASK(ClusterId)		BIT(ClusterId)

XStatus XPmApuCore_Init(XPm_ApuCore *ApuCore,
	u32 Id,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/*TBD: add apuops*/
	Status = XPmCore_Init(&ApuCore->Core, Id, Power, Clock, Reset, (u8)Ipi,
			      NULL);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CORE_INIT;
	}

	ApuCore->FpdApuBaseAddr = BaseAddress[0];

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_0_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_1_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_2_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_3_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_0_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_1_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_2_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_3_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_0_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_1_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_2_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_3_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_0_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_1_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_2_PWR_CTRL_MASK;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_3_PWR_CTRL_MASK;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_ApuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	XStatus Status = XST_FAILURE;
	u32 Val;
	XPm_PsFpDomain *Fpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	u32 ClusterId = GET_APU_CLUSTER_ID(DeviceId);

	if (NULL == Fpd) {
		*Mode = XPM_INVAL_OPER_MODE;
		goto done;
	}

	PmIn32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL, Val);
	Val &= XPM_APU_MODE_MASK(ClusterId);
	if (0U == Val) {
		*Mode = XPM_APU_MODE_SPLIT;
	} else {
		*Mode = XPM_APU_MODE_LOCKSTEP;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPm_ApuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	XStatus Status = XST_FAILURE;
	u32 Val;
	XPm_PsFpDomain *Fpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	u32 ClusterId = GET_APU_CLUSTER_ID(DeviceId);

	if (NULL == Fpd) {
		goto done;
	}

	PmIn32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL, Val);
	if (Mode == XPM_APU_MODE_SPLIT) {
		Val &= ~(Mode << ClusterId);
	} else if (Mode == XPM_APU_MODE_LOCKSTEP) {
		Val |= (Mode << ClusterId);
	}
	PmOut32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL, Val);

	Status = XST_SUCCESS;
done:
	return Status;
}

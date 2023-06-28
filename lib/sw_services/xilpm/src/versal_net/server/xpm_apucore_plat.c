/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_power.h"
#include "xpm_psfpdomain.h"

#define XPM_APU_MODE_MASK(ClusterId)		BIT(ClusterId)

XStatus XPmApuCore_AssignRegisterMask(XPm_ApuCore *ApuCore, const u32 Id)
{
	XStatus Status = XST_FAILURE;

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_0_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_0_0_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_0_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_1_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_0_1_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_1_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_2_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_0_2_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_2_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_0_3_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_0_3_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_3_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_0_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_1_0_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_4_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_1_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_1_1_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_5_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_2_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_1_2_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_6_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_1_3_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_1_3_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_7_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_0_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_2_0_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_8_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_1_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_2_1_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_9_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_2_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_2_2_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_10_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_2_3_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_2_3_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_11_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_0) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_0_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_3_0_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_12_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_1) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_1_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_3_1_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_13_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_2) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_2_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_3_2_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_14_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_3) {
		ApuCore->Core.SleepMask = XPM_ACPU_3_3_PWR_CTRL_MASK;
		ApuCore->Core.WakeUpMask = XPM_ACPU_3_3_WAKEUP_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_15_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else {
		Status = XST_INVALID_PARAM;
	}

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

	PmIn32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL_OFFSET, Val);
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

	PmIn32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL_OFFSET, Val);
	if (Mode == XPM_APU_MODE_SPLIT) {
		Val &= ~(Mode << ClusterId);
	} else if (Mode == XPM_APU_MODE_LOCKSTEP) {
		Val |= (Mode << ClusterId);
	}
	PmOut32(Fpd->FpdSlcrBaseAddr + FPX_SLCR_APU_CTRL_OFFSET, Val);

	Status = XST_SUCCESS;
done:
	return Status;
}

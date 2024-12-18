/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_apucore.h"
#include "xpm_regs.h"
#include "xpm_debug.h"
#include "xpm_psfpdomain.h"
#include "xpm_versal_aiepg2_regs.h"
#define XPM_APU_MODE_MASK(ClusterId)		BIT(ClusterId)
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


static struct XPm_CoreOps ApuOps= {
	.RequestWakeup = XPmApuCore_WakeUp,
	.PowerDown = NULL
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
XStatus XPmApuCore_AssignRegisterMask(XPm_ApuCore *ApuCore, const u32 Id)
{
	XStatus Status = XST_FAILURE;

	if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_0) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU0_CORE0_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU0_CORE0_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_0_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_0_1) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU0_CORE1_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU0_CORE1_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_1_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_0) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU1_CORE0_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU1_CORE0_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_4_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_1_1) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU1_CORE1_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU1_CORE1_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_5_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_0) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU2_CORE0_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU2_CORE0_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_8_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_2_1) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU2_CORE1_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU2_CORE1_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_9_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_0) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU3_CORE0_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU3_CORE0_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_12_PWRDWN;
		ApuCore->Core.PwrDwnMask = APU_PCIL_CORE_PWRDWN_MASK;
		Status = XST_SUCCESS;
	} else if (NODEINDEX(Id) == (u32)XPM_NODEIDX_DEV_ACPU_3_1) {
		ApuCore->Core.SleepMask = PSXC_LPX_SLCR_APU3_CORE1_PWRDWN_MASK;
		ApuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP0_IRQ_APU3_CORE1_MASK;
		ApuCore->PcilPwrDwnReg = APU_PCIL_CORE_13_PWRDWN;
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
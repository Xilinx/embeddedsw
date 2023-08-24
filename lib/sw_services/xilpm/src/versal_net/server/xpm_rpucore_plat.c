/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_psm.h"
#include "xpm_pslpdomain.h"

void XPmRpuCore_AssignRegAddr(struct XPm_RpuCore *RpuCore, const u32 Id, const u32 *BaseAddress)
{
	RpuCore->RpuBaseAddr = BaseAddress[0];
	RpuCore->ClusterBaseAddr = BaseAddress[1];
	const XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	if (PM_DEV_RPU_A_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_0_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_A_0_WAKEUP_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + LPX_SLCR_RPU_PCIL_A0_ISR_OFFSET;
	} else if (PM_DEV_RPU_A_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_1_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_A_1_WAKEUP_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + LPX_SLCR_RPU_PCIL_A1_ISR_OFFSET;
	} else if (PM_DEV_RPU_B_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_0_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_B_0_WAKEUP_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + LPX_SLCR_RPU_PCIL_B0_ISR_OFFSET;
	} else if (PM_DEV_RPU_B_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_1_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_B_1_WAKEUP_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + LPX_SLCR_RPU_PCIL_B1_ISR_OFFSET;
	}
}

void XPm_PlatRpuSetOperMode(const struct XPm_RpuCore *RpuCore, const u32 Mode, u32 *Val)
{
	if (Mode == XPM_RPU_MODE_SPLIT) {
		*Val |= XPM_RPU_SLSPLIT_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		*Val &= ~XPM_RPU_SLSPLIT_MASK;
	} else {
		/* Required by MISRA */
	}
	PmOut32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, *Val);
}

XStatus XPm_PlatRpuBootAddrConfig(const struct XPm_RpuCore *RpuCore, const u32 BootAddr)
{
	if (0U == BootAddr) {
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK,
			XPM_RPU_TCMBOOT_MASK);
	} else {
		PmOut32(RpuCore->RpuBaseAddr + XPM_CORE_VECTABLE_OFFSET, BootAddr);
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, 0U);
	}

	return XST_SUCCESS;
}

u32 XPm_PlatRpuGetOperMode(const struct XPm_RpuCore *RpuCore)
{
	u32 Val;

	PmIn32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, Val);

	return Val;
}

void XPm_GetCoreId(u32 *Rpu0, u32 *Rpu1, const u32 DeviceId)
{
	if (PM_DEV_RPU_A_0 == DeviceId || PM_DEV_RPU_A_1 == DeviceId) {
		*Rpu0 = PM_DEV_RPU_A_0;
		*Rpu1 = PM_DEV_RPU_A_1;
	} else {
		*Rpu0 = PM_DEV_RPU_B_0;
		*Rpu1 = PM_DEV_RPU_B_1;
	}
}

void XPmRpuCore_SetTcmBoot(const u32 DeviceId, const u8 TcmBootFlag){
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);
	if(1U == TcmBootFlag){
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, XPM_RPU_TCMBOOT_MASK);
	} else {
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, ~XPM_RPU_TCMBOOT_MASK);
	}
}

XStatus XPm_PlatRpucoreHalt(XPm_Core *Core){
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

	if((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State){
		/* Put RPU in  halt state */
		/*don't do reset release because it is setting up PCIL ISR and we are enabling wake interrupt.
		 *So, it will trigger wake interrupt directly after reset release.
		 */
		XPM_RPU_CORE_HALT(RpuCore->ResumeCfg);

		Status = XST_SUCCESS;
	} else{
		Status = XPmRpuCore_Halt((XPm_Device *)Core);
	}

	return Status;
}
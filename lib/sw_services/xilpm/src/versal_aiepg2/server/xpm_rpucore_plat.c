/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_pslpdomain.h"

#define XPM_RPU_CPUHALT_MASK	BIT(0)

void XPmRpuCore_AssignRegAddr(struct XPm_RpuCore *RpuCore, const u32 Id, const u32 *BaseAddress)
{
	RpuCore->RpuBaseAddr = BaseAddress[0U];
	RpuCore->ClusterBaseAddr = BaseAddress[1U];
	const XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	if (PM_DEV_RPU_A_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_A0_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUA_CORE0_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_A0_ISR_OFFSET;
	} else if (PM_DEV_RPU_A_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_A1_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUA_CORE1_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_A1_ISR_OFFSET;
	} else if (PM_DEV_RPU_B_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_B0_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUB_CORE0_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_B0_ISR_OFFSET;
	} else if (PM_DEV_RPU_B_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_B1_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUB_CORE1_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_B1_ISR_OFFSET;
	} else if (PM_DEV_RPU_C_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_C0_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUC_CORE0_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_C0_ISR_OFFSET;
	} else if (PM_DEV_RPU_C_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_C1_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUC_CORE1_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_C1_ISR_OFFSET;
	} else if (PM_DEV_RPU_D_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_D0_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUD_CORE0_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_D0_ISR_OFFSET;
	} else if (PM_DEV_RPU_D_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_D1_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUD_CORE1_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_D1_ISR_OFFSET;
	} else if (PM_DEV_RPU_E_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_E0_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUE_CORE0_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_E0_ISR_OFFSET;
	} else if (PM_DEV_RPU_E_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = PSXC_LPX_SLCR_RPU_E1_POWER_DWN_MASK;
		RpuCore->Core.WakeUpMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUE_CORE1_MASK;
		RpuCore->PcilIsr = PsLpd->LpdSlcrBaseAddr + PSXC_LPX_SLCR_RPU_PCIL_E1_ISR_OFFSET;
	}
}

XStatus XPm_PlatRpuSetOperMode(const struct XPm_RpuCore *RpuCore, const u32 Mode, u32 *Val)
{
	XStatus Status = XST_FAILURE;
	if (XPM_RPU_MODE_SPLIT == Mode) {
		*Val |= XPM_RPU_SLSPLIT_MASK;
	} else if (XPM_RPU_MODE_LOCKSTEP == Mode) {
		*Val &= ~XPM_RPU_SLSPLIT_MASK;
	} else {
		/* Required by MISRA */
	}

	/*skip if the mode is already set*/
	if (Mode != (Xil_In32(RpuCore->ClusterBaseAddr) & 0x1U)) {

		/*assert reset before changing mode*/
		Status = XPmDevice_Reset((XPm_Device *)RpuCore, PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		PmOut32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, *Val);
		Status = XPmDevice_Reset((XPm_Device *)RpuCore, PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
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
	} else if (PM_DEV_RPU_B_0 == DeviceId || PM_DEV_RPU_B_1 == DeviceId) {
		*Rpu0 = PM_DEV_RPU_B_0;
		*Rpu1 = PM_DEV_RPU_B_1;
	} else if (PM_DEV_RPU_C_0 == DeviceId || PM_DEV_RPU_C_1 == DeviceId) {
		*Rpu0 = PM_DEV_RPU_C_0;
		*Rpu1 = PM_DEV_RPU_C_1;
	} else if (PM_DEV_RPU_D_0 == DeviceId || PM_DEV_RPU_D_1 == DeviceId) {
		*Rpu0 = PM_DEV_RPU_D_0;
		*Rpu1 = PM_DEV_RPU_D_1;
	} else if (PM_DEV_RPU_E_0 == DeviceId || PM_DEV_RPU_E_1 == DeviceId) {
		*Rpu0 = PM_DEV_RPU_E_0;
		*Rpu1 = PM_DEV_RPU_E_1;
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
	u32 Reg = 0;

	if((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State){
		/* Put RPU in  halt state */
		/*don't do reset release because it is setting up PCIL ISR and we are enabling wake interrupt.
		 *So, it will trigger wake interrupt directly after reset release.
		 */
		XPM_RPU_CORE_HALT(RpuCore->ResumeCfg);

		Status = XST_SUCCESS;
	} else if (0U == (Reg & Core->Device.Power->PwrStatMask)) {
		/*skip halt if the core is powered down*/
		Status = XST_SUCCESS;
	} else {
		Status = XPmRpuCore_Halt((XPm_Device *)Core);
	}

	return Status;
}

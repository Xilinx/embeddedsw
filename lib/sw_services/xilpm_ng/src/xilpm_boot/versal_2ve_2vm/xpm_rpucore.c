/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_pslpdomain.h"
#include "xpm_mem.h"

static XStatus SetResetState(const XPm_RpuCore *Core, u32 Value);
static XStatus XPm_PlatRpucoreHalt(u32 CoreId);

XStatus XPmRpuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

	Status = XPmCore_WakeUp(Core, SetAddress, Address);
	if (XST_SUCCESS != Status) {
		PmErr("Status = %x\r\n", Status);
		goto done;
	}

	/* Put RPU in running state from halt state */
	XPM_RPU_CORE_RUN(RpuCore->ResumeCfg);

 done:
	return Status;
 }

XStatus XPmRpuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPm_PlatRpucoreHalt(Core->Device.Node.Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmCore_PwrDwn(Core);

done:
	return Status;
}


XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&RpuCore->Core, Id, Power, Clock, Reset, (u8)Ipi);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	XPmRpuCore_AssignRegAddr(RpuCore, Id, BaseAddress);

done:
	return Status;
}

XStatus XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	XStatus Status = XST_FAILURE;
	u32 Val;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);
	if (NULL == RpuCore) {
		*Mode = XPM_INVAL_OPER_MODE;
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Val = XPm_PlatRpuGetOperMode(RpuCore);
	Val &= XPM_RPU_SLSPLIT_MASK;
	if (0U == Val) {
		*Mode = XPM_RPU_MODE_LOCKSTEP;
	} else {
		*Mode = XPM_RPU_MODE_SPLIT;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	u32 Val;
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if (NULL == RpuCore)  {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Val = XPm_PlatRpuGetOperMode(RpuCore);

	Status = XPm_PlatRpuSetOperMode(RpuCore, Mode, &Val);
	if (XST_SUCCESS != Status)  {
		PmErr("Error while setting operation mode\n");
		goto done;
	}

	Status = XPm_PlatRpucoreHalt(DeviceId);

done:
	return Status;
}

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
		PmOut32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, *Val);
	}
	Status = XST_SUCCESS;

	return Status;
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
	if (NULL == RpuCore) {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		return;
	}
	XStatus Status = XST_FAILURE;
	if(1U == TcmBootFlag){
		/* This TCM boot flag indicate that we are using TCM for this specific core*/
		/* So we go ahead Halt the core and release reset to make sure TCM is accessible*/
		Status = XPmRpuCore_ResetAndHalt(DeviceId);
		if (XST_SUCCESS != Status) {
			return;
		}
		Status = XPmRpuCore_ReleaseReset(DeviceId);
		if (XST_SUCCESS != Status) {
			return;
		}
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, XPM_RPU_TCMBOOT_MASK);
	} else {
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, ~XPM_RPU_TCMBOOT_MASK);
	}
}

static XStatus XPm_PlatRpucoreHalt(u32 CoreId) {
	XStatus Status = XST_FAILURE;
	/* Type check */
	if (XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(CoreId)) {
		Status = XST_INVALID_PARAM;
		PmErr("Expecting RPUCore but get 0x%x\r\n", CoreId);
		goto done;
	}

	XPm_Core *Core = (XPm_Core*) XPmDevice_GetById(CoreId);
	if (NULL == Core) {
		PmErr("RPU Core Node is NULL\r\n");
		goto done;
	}
	/* Safely cast to RpuCore since Type check passed */
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

	if((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State){
		/* Put RPU in  halt state */
		/*don't do reset release because it is setting up PCIL ISR and we are enabling wake interrupt.
		 *So, it will trigger wake interrupt directly after reset release.
		 */
		XPM_RPU_CORE_HALT(RpuCore->ResumeCfg);

		Status = XST_SUCCESS;
	} else {
		Status = XPmRpuCore_ResetAndHalt(CoreId);
		if (XST_SUCCESS != Status) {
			PmErr("Error while halting RPU core\r\n");
			goto done;
		}
		Status = XPmRpuCore_ReleaseReset(CoreId);
		if (XST_SUCCESS != Status) {
			PmErr("Error while releasing reset for RPU core\r\n");
			goto done;
		}

	}
done:
	return Status;
}
static XStatus SetResetState(const XPm_RpuCore *Core, u32 Value)
{
	XStatus Status = XST_FAILURE;
	/* Safe to cast because Core is a device */
	XPm_Device *Device = (XPm_Device *)Core;
	if (NULL == Core) {
		PmErr("RPU Core Node is NULL\r\n");
		goto done;
	}
	XPm_ResetHandle *CurHandle = Device->RstHandles;
	while(NULL != CurHandle) {
		XPm_ResetNode *Reset = CurHandle->Reset;
		if (NULL == Reset) {
			PmErr("RPU Core Reset node is NULL\r\n");
			goto done;
		}
		u32 RpuRstAddr = Reset->Node.BaseAddress;
		u32 Mask = BITNMASK(Reset->Shift, Reset->Width);
		if (0U == Value) {
			XPm_RMW32(RpuRstAddr, Mask, 0U);
		} else {
			XPm_RMW32(RpuRstAddr, Mask, Mask);
		}
		CurHandle = CurHandle->NextReset;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * XPmRpuCore_ResetAndHalt - Resets and halts the specified RPU core.
 *
 * @CoreId: The ID of the RPU core to reset and halt.
 *
 * This function performs the following steps:
 * 1. Retrieves the RPU core object using the provided CoreId.
 * 2. Checks if the retrieved object is valid and of the correct type.
 * 3. Asserts the reset state for the RPU core.
 * 4. Halts the RPU core.
 *
 * Return:
 * XST_SUCCESS if the operation is successful.
 * XST_FAILURE if the RPU core object could not be retrieved.
 * XST_INVALID_PARAM if the provided CoreId is not of the RPU core type.
 */
XStatus XPmRpuCore_ResetAndHalt(u32 CoreId)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(CoreId);
	if (NULL == RpuCore) {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", CoreId);
		goto done;
	}
	if (XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(CoreId)) {
		Status = XST_INVALID_PARAM;
		PmErr("Expecting RPUCore but get 0x%x\r\n", CoreId);
		goto done;
	}
	Status = SetResetState(RpuCore, 1U);
	if (XST_SUCCESS != Status) {
		PmErr("Error while asserting reset\r\n");
		goto done;
	}
	XPM_RPU_CORE_HALT(RpuCore->ResumeCfg);

	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * XPmRpuCore_ReleaseReset - Release the reset state of an RPU core.
 *
 * @CoreId: The ID of the RPU core to be released from reset.
 *
 * This function releases the reset state of the specified RPU core.
 * It first retrieves the RPU core object using the provided CoreId.
 * If the core object is not found, it logs an error and returns failure.
 * It then checks if the node type of the core is RPU. If not, it logs an error
 * and returns an invalid parameter status.
 * Finally, it attempts to release the reset state of the core. If successful,
 * it returns success; otherwise, it logs an error and returns failure.
 *
 * Return: XST_SUCCESS if the reset state is successfully released,
 *         XST_FAILURE if the core object is not found,
 *         XST_INVALID_PARAM if the node type is not RPU,
 *         or an error status if releasing the reset state fails.
 */
XStatus XPmRpuCore_ReleaseReset(u32 CoreId)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *Core = (XPm_RpuCore *)XPmDevice_GetById(CoreId);
	if (NULL == Core) {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", CoreId);
		goto done;
	}

	/* Check NODETYPE to make sure it is RPU*/
	if (XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(CoreId)) {
		Status = XST_INVALID_PARAM;
		PmErr("Expecting RPUCore but get 0x%x\r\n", CoreId);
		goto done;
	}

	Status = SetResetState(Core, 0U);
	if (XST_SUCCESS != Status) {
		PmErr("Error while releasing reset of RpuCore 0x%x\r\n", CoreId);
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

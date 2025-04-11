/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_core.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_apucore.h"
#include "xpm_power_core.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	(void)IpiCh;

	Status = XPmDevice_Init(&Core->Device, Id, 0, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_DEVICE_INIT;
		goto done;
	}
	Core->CoreOps = Ops;
	Core->isCoreUp = 0;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;

	if (1U == Core->isCoreUp) {
		Status = XPM_ERR_WAKEUP;
		goto done;
	}
	XPmDevice_BringUp((XPm_Device*)Core);
	DisableWake(Core);

	/* Set reset address */
	if (1U == SetAddress) {
		Status = XPmCore_StoreResumeAddr(Core, (Address | 1U));
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	if (((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State)
	    && (NULL != Core->Device.ClkHandles)) {
		/** TODO: Implement Request clock */
		PmWarn("Request clock is not yet implemented!\n");

	}

	/* Reset will be released at a part of direct power up sequence */
	Status = XPm_DirectPwrUp(Core->Device.Node.Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->isCoreUp = 1;

done:
	return Status;
}
static void EnableWake(const struct XPm_Core *Core)
{
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Core->Device.Node.Id)) {
		ENABLE_WAKE0(Core->WakeUpMask);
	} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) {
		ENABLE_WAKE1(Core->WakeUpMask);
	} else {
		/* Required for MISRA */
	}
}
XStatus XPmCore_AfterDirectPwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if (Core->isCoreUp == 0U) {
		PmInfo("Core is already down 0x%x, State: 0x%x, skipping\n", Core->Device.Node.Id, Core->Device.Node.State);
		Status = XST_SUCCESS;
		goto done;
	}

	/** Release Core clocks */
	Status = XPmCore_SetClock(Core->Device.Node.Id, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	PmInfo("After direct power down for 0x%x, State: 0x%x\n", Core->Device.Node.Id, Core->Device.Node.State);

	/**
	 * Enabling of wake interrupt is removed from PSM direct power down
	 * sequence so enable wakeup interrupt here.
	 */
	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		EnableWake(Core);
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;
	Core->isCoreUp = 0;
	Status = XST_SUCCESS;

done:
	if (Status != XST_SUCCESS) {
		PmErr("Core Id=0x%x 0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	if ((u32)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		DISABLE_WFI(Core->SleepMask);
	}

	/* If parent is on, then only send sleep request */
	if ((Core->Device.Power->Parent->Node.State == (u8)XPM_POWER_STATE_ON) &&
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(Core->Device.Node.Id))) {
		PmInfo("Going in --> Power down core 0x%x\r\n", Core->Device.Node.Id);
		/* Power down the core */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmCore_AfterDirectPwrDwn(Core);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to process pending force power down for 0x%x: 0x%x\n", Core->Device.Node.Id, Status);
		goto done;
	}

done:
	if (Status != XST_SUCCESS) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

void DisableWake(const struct XPm_Core *Core)
{
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Core->Device.Node.Id)) {
		DISABLE_WAKE0(Core->WakeUpMask);
	} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) {
		const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

		DISABLE_WAKE1(Core->WakeUpMask);

		/*Clear PWRDWN EN and Pwr Ctrl Status as direct power down will not execute in forcepower down case*/
		/*example: for RPU A0 this RpuCore->PcilIsr + LPX_SLCR_RPU_PCIL_X_PWRDWN_OFFSET = 0xEB4200C0*/
		if ((PM_DEV_RPU_A_0 == Core->Device.Node.Id) ||
		    (PM_DEV_RPU_A_1 == Core->Device.Node.Id) ||
		    (PM_DEV_RPU_B_0 == Core->Device.Node.Id) ||
		    (PM_DEV_RPU_B_1 == Core->Device.Node.Id)) {
			PmOut32(RpuCore->PcilIsr + LPX_SLCR_RPU_PCIL_AB_PWRDWN_OFFSET, 0x0U);
		} else {
			PmOut32(RpuCore->PcilIsr + LPX_SLCR_RPU_PCIL_CDE_PWRDWN_OFFSET, 0x0U);
		}

		CLEAR_PWRCTRL1(Core->SleepMask);
		PmOut32(RpuCore->PcilIsr + LPX_SLCR_RPU_PCIL_X_IDS_OFFSET, 0x1U);

	} else {
		/* Required for MISRA */
	}
}
static XStatus SetResumeAddr(XPm_Core *Core, u64 Address)
{
	Core->ResumeAddr = Address;
	return XST_SUCCESS;
}
XStatus XPmCore_StoreResumeAddr(XPm_Core *Core, u64 Address)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	/* Check for valid resume address */
	/* TODO: Check if this is correct way to validate address */
	if (0U == (Address & 1ULL)) {
		DbgErr = XPM_INT_ERR_INVALID_RESUME_ADDR;
		goto done;
	}

	if (NULL == Core) {
		DbgErr = XPM_INT_ERR_INVALID_PROC;
		goto done;
	}

	XSECURE_REDUNDANT_CALL(Status, StatusTmp, SetResumeAddr, Core, Address);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		goto done;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmCore_HasResumeAddr(const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u64 ResumeAddr;

	if (NULL == Core) {
		goto done;
	}

	ResumeAddr = Core->ResumeAddr;
	if (0U != (ResumeAddr & 1ULL)) {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

XStatus XPmCore_SetCPUIdleFlag(const XPm_Core *Core, u32 CpuIdleFlag)
{
	(void)Core;
	(void)CpuIdleFlag;
	return XST_SUCCESS;
}

XStatus XPmCore_GetCPUIdleFlag(const XPm_Core *Core, u32 *CpuIdleFlag)
{
	(void)Core;
	(void)CpuIdleFlag;
	return XST_SUCCESS;
}

XStatus __attribute__((weak,noinline)) XPmCore_SetClock(u32 CoreId, u32 Enable)
{
	(void)CoreId;
	(void)Enable;
	return XST_SUCCESS;
}

/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_core.h"
#include "xpm_notifier.h"
#include "xpm_rpucore.h"
#include "xpm_debug.h"

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
		/* Power down the core */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmCore_AfterDirectPwrDwn(Core);

done:
	return Status;
}

XStatus XPmCore_AfterDirectPwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Release(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

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
	/* Send notification about core state change */
	XPmNotifier_Event(Core->Device.Node.Id, (u32)EVENT_STATE_CHANGE);

done:
	return Status;
}

XStatus ResetAPUGic(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Acpu0PwrNode = XPmPower_GetById(PM_POWER_ACPU_0);
	const XPm_Power *Acpu1PwrNode = XPmPower_GetById(PM_POWER_ACPU_1);
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);

	if (((PM_DEV_ACPU_0 == DeviceId) || (PM_DEV_ACPU_1 == DeviceId)) &&
	    (NULL != Acpu0PwrNode) && (NULL != Acpu1PwrNode) &&
	    (NULL != FpdPwrNode) &&
	    ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu0PwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu1PwrNode->Node.State)) {
		Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC,
					     (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
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

XStatus XPmCore_StoreResumeAddr(XPm_Core *Core, u64 Address)
{
	XStatus Status = XST_FAILURE;
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

	/* Store the resume address */
	Core->ResumeAddr = Address;
	Status = XST_SUCCESS;

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

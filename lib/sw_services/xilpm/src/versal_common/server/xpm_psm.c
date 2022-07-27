/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xil_util.h"
#include "xpm_regs.h"
#include "xpm_psm.h"
#include "xpm_psm_api.h"
#include "xplmi_generic.h"

static struct PsmToPlmEvent_t PsmToPlmEvent_bkp = {0};
static u32 Is_PsmPoweredDown = 0U;

static XStatus XPmPsm_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	const XPm_Psm *Psm = (XPm_Psm *)Core;
	u32 CRLBaseAddress = Psm->CrlBaseAddr;
	const u32 CopySize = sizeof(PsmToPlmEvent_bkp);

	if (1U == Core->isCoreUp) {
		Status = XPM_ERR_WAKEUP;
		goto done;
	}

	/* Set reset address */
	if (1U == SetAddress) {
		if (0U != Address) {
			PmWarn("Handoff address is not used for PSM.\r\n");
		}
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Assert wakeup bit to Wakeup PSM */
	PmRmw32(CRLBaseAddress + CRL_PSM_RST_MODE_OFFSET, XPM_PSM_WAKEUP_MASK, XPM_PSM_WAKEUP_MASK);

	/* Wait for PSMFW to initialize */
	Status = XPm_PollForMask(GLOBAL_CNTRL(Psm->PsmGlobalBaseAddr),
				 PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
				 XPM_MAX_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_GetPsmToPlmEventAddr();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (1U == Is_PsmPoweredDown) {
		/* Restore the context of reserved PSM RAM memory */
		Status = Xil_SMemCpy((void *)PsmToPlmEvent, CopySize, &PsmToPlmEvent_bkp, CopySize, CopySize);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Check for the version of the PsmToPlmEvent structure */
	if (PsmToPlmEvent->Version != PSM_TO_PLM_EVENT_VERSION) {
		PmErr("PSM-PLM are out of sync. Can't process PSM event\n\r");
		goto done;
	} else {
		Status = XST_SUCCESS;
		Core->isCoreUp = 1;
	}

	/*
	 * PSM toggles between running and sleeping too frequently. Clear PSM
	 * wakeup bit to put it into sleep state while idle.
	 */
	PmRmw32(CRLBaseAddress + CRL_PSM_RST_MODE_OFFSET, XPM_PSM_WAKEUP_MASK, 0U);

	Status = XPM_SET_PROC_LIST_PLAT;
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPsm_PowerDown(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;
	(void)Core;
	const u32 CopySize = sizeof(PsmToPlmEvent_bkp);

	if ((u8)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Store the context of reserved PSM RAM memory */
	Status = Xil_SMemCpy(&PsmToPlmEvent_bkp, CopySize, (void *)PsmToPlmEvent, CopySize, CopySize);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Add PSM specific power down sequence if any */

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Is_PsmPoweredDown = 1U;

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;
	Core->isCoreUp = 0;
	Status = XST_SUCCESS;

done:
	return Status;
}

static struct XPm_CoreOps PsmOps = {
	.RequestWakeup = XPmPsm_WakeUp,
	.PowerDown = XPmPsm_PowerDown,
};

XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Psm->Core, PM_DEV_PSM_PROC, Power, Clock, Reset,
			      (u8)Ipi, &PsmOps);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	Psm->PsmGlobalBaseAddr = BaseAddress[0];
	Psm->CrlBaseAddr = BaseAddress[1];
done:
	return Status;
}

u32 XPmPsm_FwIsPresent(void)
{
        u32 Reg = 0U;
        const XPm_Psm *Psm;
        const XPm_Power *Lpd = XPmPower_GetById(PM_POWER_LPD);

        Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
        if ((NULL == Psm) || ((u8)XPM_POWER_STATE_ON != Lpd->Node.State)) {
                goto done;
        }

        PmIn32(GLOBAL_CNTRL(Psm->PsmGlobalBaseAddr), Reg);
        if (PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK ==
                (Reg & PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)) {
                Reg = 1U;
        }

done:
        return Reg;
}

void XPmPsm_RegWrite(const u32 Offset, const u32 Value)
{
	const XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	PmOut32(Psm->PsmGlobalBaseAddr + Offset, Value);

done:
	return;
}

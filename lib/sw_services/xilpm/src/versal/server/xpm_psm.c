/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xpm_regs.h"
#include "xpm_psm.h"

#define GLOBAL_CNTRL(BASE)	((BASE) + PSM_GLOBAL_CNTRL)
#define PWR_UP_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRUP_EN)
#define PWR_UP_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRUP_TRIG)
#define PWR_DN_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRDWN_EN)
#define PWR_DN_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRDWN_TRIG)
#define PWR_DN_STAT(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRDWN_STAT)
#define SLEEP_EN(BASE)		((BASE) + PSM_GLOBAL_PWR_CTRL_EN)
#define SLEEP_DIS(BASE)		((BASE) + PSM_GLOBAL_PWR_CTRL_DIS)
#define SLEEP_TRIG(BASE)	((BASE) + PSM_GLOBAL_PWR_CTRL_TRIG)
#define PWR_STAT(BASE)		((BASE) + PSM_GLOBAL_PWR_STATE)

static XStatus XPmPsm_WakeUp(XPm_Core *Core, u32 SetAddress,
			  u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_Psm *Psm = (XPm_Psm *)Core;
	u32 CRLBaseAddress = Psm->CrlBaseAddr;

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

done:
	return Status;
}

static struct XPm_CoreOps PsmOps = {
		.RestoreResumeAddr = NULL,
		.HasResumeAddr = NULL,
		.RequestWakeup = XPmPsm_WakeUp,
		.PowerDown = NULL,
};

XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Psm->Core, PM_DEV_PSM_PROC, Power, Clock, Reset,
			      (u8)Ipi, &PsmOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Psm->PsmGlobalBaseAddr = BaseAddress[0];
	Psm->CrlBaseAddr = BaseAddress[1];
done:
	return Status;
}

XStatus XPmPsm_SendPowerUpReq(u32 BitMask)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	XPm_Psm *Psm;

	PmDbg("BitMask=0x%08X\n\r", BitMask);

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* Check if already powered up */
	PmIn32(PWR_STAT(Psm->PsmGlobalBaseAddr), Reg);
	if (BitMask == (Reg & BitMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmOut32(PWR_UP_TRIG(Psm->PsmGlobalBaseAddr), BitMask);
	PmOut32(PWR_UP_EN(Psm->PsmGlobalBaseAddr), BitMask);
	do {
		PmIn32(PWR_STAT(Psm->PsmGlobalBaseAddr), Reg);
	} while ((Reg & BitMask) != BitMask);

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPsm_SendPowerDownReq(u32 BitMask)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	XPm_Psm *Psm;
	XPm_Power *Ocm0 = XPmPower_GetById(PM_POWER_OCM_0);
	XPm_Power *Ocm1 = XPmPower_GetById(PM_POWER_OCM_1);
	XPm_Power *Ocm2 = XPmPower_GetById(PM_POWER_OCM_2);
	XPm_Power *Ocm3 = XPmPower_GetById(PM_POWER_OCM_3);

	/*
	 * As per EDT-995988, Getting the SLV error from power down
	 * island even when Dec error disabled
	 *
	 * OCM gives SLVERR response when a powered-down bank is
	 * accessed, even when Response Error is disabled. Error occurs
	 * only for a narrow access (< 64 bits). Skip OCM power down as
	 * workaround.
	 */
	if ((BitMask == Ocm0->Node.BaseAddress) ||
	    (BitMask == Ocm1->Node.BaseAddress) ||
	    (BitMask == Ocm2->Node.BaseAddress) ||
	    (BitMask == Ocm3->Node.BaseAddress)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmDbg("BitMask=0x%08X\n\r", BitMask);

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* Check if already powered down */
	PmIn32(PWR_STAT(Psm->PsmGlobalBaseAddr), Reg);
	if (0U == (Reg & BitMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmOut32(PWR_DN_TRIG(Psm->PsmGlobalBaseAddr), BitMask);
	PmOut32(PWR_DN_EN(Psm->PsmGlobalBaseAddr), BitMask);
	do {
		PmIn32(PWR_DN_STAT(Psm->PsmGlobalBaseAddr), Reg);
	} while (0U != (Reg & BitMask));

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPmPsm_FwIsPresent(void)
{
	u32 Reg = 0U;
	XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	PmIn32(GLOBAL_CNTRL(Psm->PsmGlobalBaseAddr), Reg)
	if (PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK ==
		(Reg & PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)) {
		Reg = 1U;
	}

done:
	return Reg;
}

void XPmPsm_RegWrite(const u32 Offset, const u32 Value)
{
	XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	PmOut32(Psm->PsmGlobalBaseAddr + Offset, Value);

done:
	return;
}

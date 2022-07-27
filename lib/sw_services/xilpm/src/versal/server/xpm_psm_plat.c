/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"
#include "xil_util.h"
#include "xpm_regs.h"
#include "xpm_psm.h"

#define PWR_UP_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRUP_EN)
#define PWR_UP_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRUP_TRIG)
#define PWR_DN_EN(BASE)		((BASE) + PSM_GLOBAL_REQ_PWRDWN_EN)
#define PWR_DN_TRIG(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRDWN_TRIG)
#define PWR_DN_STAT(BASE)	((BASE) + PSM_GLOBAL_REQ_PWRDWN_STAT)
#define PWR_STAT(BASE)		((BASE) + PSM_GLOBAL_PWR_STATE)

XStatus XPmPsm_SendPowerUpReq(u32 BitMask)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	const XPm_Psm *Psm;

	PmDbg("BitMask=0x%08X\n\r", BitMask);

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		PmErr("PSMFW is not present\r\n");
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
	const XPm_Psm *Psm;
	const XPm_Power *Ocm0 = XPmPower_GetById(PM_POWER_OCM_0);
	const XPm_Power *Ocm1 = XPmPower_GetById(PM_POWER_OCM_1);
	const XPm_Power *Ocm2 = XPmPower_GetById(PM_POWER_OCM_2);
	const XPm_Power *Ocm3 = XPmPower_GetById(PM_POWER_OCM_3);

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

/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_psm.h"
#include "xil_types.h"
#include "xpm_common.h"
#include "xpm_regs.h"

XStatus XPmPsm_SendPowerUpReq(XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	const XPm_Psm *Psm;

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
	PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	if (Power->PwrStatMask == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}


	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPsm_SendPowerDownReq(XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	const XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* Check if already powered down */
	PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	if (0U == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}


	Status = XST_SUCCESS;

done:
	return Status;
}

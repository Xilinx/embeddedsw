/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_psm.h"
#include "xil_types.h"
#include "xpm_common.h"
#include "xpm_regs.h"

static u8 IsCorePowerNode(XPm_Power *Power)
{
	u8 Val = 0U;
	u32 NodeIdx = NODEINDEX(Power->Node.Id);

	if ((NodeIdx >= (u32)XPM_NODEIDX_POWER_ACPU_0_0) &&
	    (NodeIdx <= (u32)XPM_NODEIDX_POWER_RPU_B_1)) {
		Val = 1U;
	}

	return Val;
}

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
	if ((Power->PwrStatMask == (Reg & Power->PwrStatMask)) || (1U == IsCorePowerNode(Power))) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrUpEnOffset + REQ_PWRUP_INT_TRIG_OFFSET, Power->PwrUpMask);
	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrUpEnOffset, Power->PwrUpMask);
	Status = XPm_PollForMask(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Power->PwrStatMask,
				 XPM_POLL_TIMEOUT);

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

	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrDwnEnOffset + REQ_PWRDWN_INT_TRIG_OFFSET, Power->PwrDwnMask);
	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrDwnEnOffset, Power->PwrDwnMask);
	Status = XPm_PollForZero(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Power->PwrStatMask,
				 XPM_POLL_TIMEOUT);

done:
	return Status;
}

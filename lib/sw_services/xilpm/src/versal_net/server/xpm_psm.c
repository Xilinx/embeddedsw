/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_psm.h"
#include "xil_types.h"
#include "xpm_common.h"
#include "xpm_regs.h"

#define GLOBAL_CNTRL(BASE)	((BASE) + PSMX_GLOBAL_CNTRL)

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
	PmIn32(Power->PwrStatReg, Reg);
	if (Power->PwrStatMask == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmOut32(Power->PwrUpEnReg + REQ_PWRUP_INT_TRIG_OFFSET, Power->PwrUpMask);
	PmOut32(Power->PwrUpEnReg, Power->PwrUpMask);
	do {
		PmIn32(Power->PwrStatReg, Reg);
	} while ((Reg & Power->PwrStatMask) != Power->PwrStatMask);

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
	PmIn32(Power->PwrStatReg, Reg);
	if (0U == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmOut32(Power->PwrDwnEnReg + REQ_PWRDWN_INT_TRIG_OFFSET, Power->PwrDwnMask);
	PmOut32(Power->PwrDwnEnReg, Power->PwrDwnMask);
	do {
		PmIn32(Power->PwrStatReg, Reg);
	} while (0U != (Reg & Power->PwrStatMask));

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPmPsm_FwIsPresent(void)
{
	u32 Reg = 0U;
	//changed to support minimum boot time xilpm
	PmIn32(GLOBAL_CNTRL(PSM_GLOBAL_REG_BASEADDR), Reg);//read PSM_GLOBAL_CNTRL data
	if (PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK ==
		(Reg & PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)) {
		Reg = 1U;
	}
	PmDbg("Reg %d\n",Reg);
	return Reg;
}

XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	/*TBD: add psmops*/
	Status = XPmCore_Init(&Psm->Core, PM_DEV_PSM_PROC, Power, Clock, Reset,
			      (u8)Ipi, NULL);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	Psm->PsmGlobalBaseAddr = BaseAddress[0];
	Psm->CrlBaseAddr = BaseAddress[1];
done:
	return Status;
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

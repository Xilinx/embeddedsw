/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_psm.h"
#include "xil_types.h"
#include "xpm_common.h"
#include "xpm_regs.h"

#define GLOBAL_CNTRL(BASE)	((BASE) + PSM_GLOBAL_CNTRL)

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
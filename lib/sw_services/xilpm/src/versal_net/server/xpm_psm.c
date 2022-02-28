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

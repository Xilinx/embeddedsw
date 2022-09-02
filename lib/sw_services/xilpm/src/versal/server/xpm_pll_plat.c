/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pll_plat.h"
#include "xpm_psm.h"
#include "xpm_regs.h"

void XPm_PllClearLockError(const XPm_PllClockNode* Pll)
{
	const XPm_Psm *Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL != Psm) {
		if (PM_CLK_APU_PLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_APLL_LOCK_MASK);
		} else if (PM_CLK_RPU_PLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_RPLL_LOCK_MASK);
		} else {
			/* Required due to MISRA */
		}
	}
}

void XPmClockPll_PlatReset(const XPm_PllClockNode *Pll)
{
	u32 ControlReg = Pll->ClkNode.Node.BaseAddress;
	u32 PlatformVersion = XPm_GetPlatformVersion();

	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    ((u32)PLATFORM_VERSION_SILICON_ES1 != PlatformVersion)) {
		/*
		 * The value of the CRX.XPLL_REG3.CP_RES_H must be set
		 * to 0x1 while the PLL is in reset for ES2 and forward
		 */
		u32 Reg;
		if ((u32)XPM_NODEIDX_CLK_PMC_PLL ==
		    NODEINDEX(Pll->ClkNode.Node.Id)) {
			Reg = ((ControlReg & (0xFFFFFF00U)) +
					PPLL_REG3_OFFSET);
		} else {
			Reg = ((ControlReg & (0xFFFFFF00U)) +
					(Pll->Topology->PllReg3Offset));
		}
		XPm_RMW32(Reg, BITNMASK(PLL_REG3_CP_RES_H_SHIFT,
					PLL_REG3_CP_RES_H_WIDTH),
				0x1UL << PLL_REG3_CP_RES_H_SHIFT);
	}
}

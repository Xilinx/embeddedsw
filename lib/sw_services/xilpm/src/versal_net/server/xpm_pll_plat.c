/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
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
		if (PM_CLK_APLL1 == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_APLL1_LOCK_MASK);
		} else if (PM_CLK_APLL2 == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_APLL2_LOCK_MASK);
		} else if (PM_CLK_RPLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_RPLL_LOCK_MASK);
		} else if (PM_CLK_FLXPLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_FLXPLL_LOCK_MASK);
		} else {
			/* Required due to MISRA */
		}
	}
}

void XPmClockPll_PlatReset(XPm_PllClockNode* Pll)
{
	(void)Pll;
}

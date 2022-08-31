/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_clock.h"

#define VERSAL_MAX_CLK_IDX		(0x7CU)

/* TODO: update this logic to return maximum node index for versal automatically instead of hard coding */
XStatus XPmClock_GetNumClocks(u32 *Resp)
{
	*Resp = VERSAL_MAX_CLK_IDX;

	return XST_SUCCESS;
}

void XPmClock_SetPlClockAsReadOnly(void)
{
	XPm_ClockNode *Clk = NULL;
	u32 Idx, Enable = 0U;
	XStatus Status = XST_FAILURE;
	const u32 PlClocksList[] = {
		PM_CLK_PMC_PL0_REF,
		PM_CLK_PMC_PL1_REF,
		PM_CLK_PMC_PL2_REF,
		PM_CLK_PMC_PL3_REF,
	};

	for (Idx = 0U; Idx < ARRAY_SIZE(PlClocksList); Idx++) {
		Clk = XPmClock_GetById(PlClocksList[Idx]);
		if (NULL != Clk) {
			/* Mark only enabled PL clocks */
			Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						       (u32)TYPE_GATE, &Enable);
			if ((XST_SUCCESS == Status) && (1U == Enable)) {
				Clk->Flags |= CLK_FLAG_READ_ONLY;
			}
		}
	}
}

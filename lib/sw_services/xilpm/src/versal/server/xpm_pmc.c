/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_pmc.h"

XStatus XPmPmc_Init(XPm_Pmc *Pmc, u32 DevcieId, u32 Ipi, const u32 *BaseAddress,
		    XPm_Power *Power,  XPm_ClockNode *Clock,
		    XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Pmc->Core, DevcieId, Power, Clock, Reset, (u8)Ipi,
			      NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc->PmcIouSlcrBaseAddr = BaseAddress[0];
	Pmc->PmcGlobalBaseAddr = BaseAddress[1];
	Pmc->PmcAnalogBaseAddr = BaseAddress[2];
done:
	return Status;
}

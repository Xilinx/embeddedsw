/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "sleep.h"
#include "xpm_common.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_debug.h"
#include "xpm_device.h"

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, NULL);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	/*TBD: Clear NPD section of PMC RAM register reserved for houseclean disable */

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

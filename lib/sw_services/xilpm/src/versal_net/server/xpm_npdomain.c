/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
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


static XStatus NpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	Status = XST_SUCCESS;
	return Status;
}


static XStatus NpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	Status = XST_SUCCESS;
	return Status;
}

static const struct XPm_PowerDomainOps NpdOps = {
	.InitStart = NpdInitStart,
	.InitFinish = NpdInitFinish
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

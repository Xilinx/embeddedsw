/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_vcudomain.h"
#include "xpm_debug.h"

static XStatus VcuInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding VCU support to xilpm */
	return XST_SUCCESS;
}

static XStatus VcuInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding VCU support to xilpm */
	return XST_SUCCESS;
}

static struct XPm_PowerDomainOps VcuOps = {
	.InitStart = VcuInitStart,
	.InitFinish = VcuInitFinish
};

XStatus XPmVcuDomain_Init(XPm_VcuDomain *VcuDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const struct XPm_PowerDomainOps *Ops = NULL;

	if (PM_POWER_VCU == Id) {
		/* VCU: Ops */
		Ops = &VcuOps;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	Status = XPmPowerDomain_Init(&VcuDomain->Domain, Id, BaseAddress, Parent, Ops);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

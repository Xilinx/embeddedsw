/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aie.h"
#include "xpm_debug.h"

static XStatus Aie2InitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding AIE support to xilpm */
	return XST_SUCCESS;
}

static XStatus Aie2InitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding AIE support to xilpm */
	return XST_SUCCESS;
}

static struct XPm_PowerDomainOps AieOps = {
	.InitStart = Aie2InitStart,
	.InitFinish = Aie2InitFinish
};

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const struct XPm_PowerDomainOps *Ops = NULL;

	(void)Args;
	(void)NumArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding AIE support to xilpm */

	if (PM_POWER_ME2 == Id) {
		/* AIE2: Ops */
		Ops = &AieOps;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	Status = XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress,
			Parent, Ops);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

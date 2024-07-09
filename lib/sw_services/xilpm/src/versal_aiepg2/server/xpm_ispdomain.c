/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_ispdomain.h"
#include "xpm_debug.h"

static XStatus IspInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding ISP support to xilpm */
	return XST_SUCCESS;
}

static XStatus IspInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding ISP support to xilpm */
	return XST_SUCCESS;
}

static struct XPm_PowerDomainOps IspOps = {
	.InitStart = IspInitStart,
	.InitFinish = IspInitFinish
};

XStatus XPmIspDomain_Init(XPm_IspDomain *IspDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const struct XPm_PowerDomainOps *Ops = NULL;

	if (PM_POWER_ISP == Id) {
		/* ISP: Ops */
		Ops = &IspOps;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	Status = XPmPowerDomain_Init(&IspDomain->Domain, Id, BaseAddress, Parent, Ops);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_cpmdomain.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_power.h"
#include "sleep.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_pldomain.h"
#include "xpm_psm_api.h"
#include "xpm_api.h"
static XStatus Cpm5nInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	Status = XST_SUCCESS;
	return Status;
}

static XStatus Cpm5nInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	Status = XST_SUCCESS;
	return Status;
}
static const struct XPm_PowerDomainOps Cpm5nOps = {
		.InitStart = Cpm5nInitStart,
		.InitFinish = Cpm5nInitFinish
};

XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, &Cpm5nOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (4U <= OtherBaseAddressesCnt) {
		CpmDomain->CpmSlcrBaseAddr = OtherBaseAddresses[0];
		CpmDomain->CpmSlcrSecureBaseAddr = OtherBaseAddresses[1];
		CpmDomain->CpmPcsrBaseAddr = OtherBaseAddresses[2];
		CpmDomain->CpmCrCpmBaseAddr = OtherBaseAddresses[3];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/*TBD: Clear CPM section of PMC RAM register reserved for houseclean disable */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

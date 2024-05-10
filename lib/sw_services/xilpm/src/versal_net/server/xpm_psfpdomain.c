/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_rail.h"


static XStatus FpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccintPsfpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSFP);

	/* Check vccint_fpd first to make sure power is on */
	Status = XPmPower_CheckPower(VccintPsfpRail,
				     PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Perform VID adjustment */
	Status = XPmRail_AdjustVID((XPm_Rail *)VccintPsfpRail);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VID_ADJUST;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus FpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static struct XPm_PowerDomainOps FpdOps = {
	.InitStart = FpdInitStart,
	.InitFinish = FpdInitFinish
};

XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent,  const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PsFpd->Domain, Id, BaseAddress, Parent, &FpdOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (1U <= OtherBaseAddressCnt) {
		PsFpd->FpdSlcrBaseAddr = OtherBaseAddresses[0];
		PsFpd->FpdSlcrSecureBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}


done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

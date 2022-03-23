/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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


XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, NULL);
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

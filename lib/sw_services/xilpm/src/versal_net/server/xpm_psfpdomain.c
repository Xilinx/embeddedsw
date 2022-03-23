/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_debug.h"

XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent,  const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PsFpd->Domain, Id, BaseAddress, Parent, NULL);
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

	/*TBD: disable housecleaning */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

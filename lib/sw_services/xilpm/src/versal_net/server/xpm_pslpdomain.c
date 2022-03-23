/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pslpdomain.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xplmi.h"

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PsLpd->Domain, Id, BaseAddress, Parent, NULL);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	PsLpd->LpdBisrFlags = 0;

	/* Make sure enough base addresses are being passed */
	if (3U <= OtherBaseAddressesCnt) {
		PsLpd->LpdIouSlcrBaseAddr = OtherBaseAddresses[0];
		PsLpd->LpdSlcrBaseAddr = OtherBaseAddresses[1];
		PsLpd->LpdSlcrSecureBaseAddr = OtherBaseAddresses[2];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/*TBD: disable housecleaning*/

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

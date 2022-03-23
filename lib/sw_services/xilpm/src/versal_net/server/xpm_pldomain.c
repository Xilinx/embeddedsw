/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_debug.h"

XStatus XPmPlDomain_RetriggerPlHouseClean(void)
{
	//changed to support minimum boot time xilpm
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

}

XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, const u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PlDomain->Domain, Id, BaseAddress, Parent, NULL);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (2U <= OtherBaseAddressCnt) {
		PlDomain->CfuApbBaseAddr = OtherBaseAddresses[0];
		PlDomain->Cframe0RegBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/*TBD: Clear PLD section of PMC RAM register reserved for houseclean disable */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
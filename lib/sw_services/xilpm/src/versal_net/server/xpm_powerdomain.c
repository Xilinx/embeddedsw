/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_psm.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_err.h"

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    const struct XPm_PowerDomainOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	PowerDomain->DomainOps = Ops;
	if (NULL != Parent) {
		PowerDomain->Parents[0] = Parent->Node.Id;
	}

	/*TBD: Set houseclean disable mask to default */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPowerDomain_AddParent(u32 Id, const u32 *ParentNodes, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PowerD;
	XPm_PowerDomain *ParentPowerD;
	u32 i, j;

	PowerD = (XPm_PowerDomain *)XPmPower_GetById(Id);

	if (NULL == PowerD) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if ((MAX_POWERDOMAINS - 1U) < NumParents) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		PowerD->Parents[i + 1U] = ParentNodes[i];

		/* Add Id as child of each parent node */
		ParentPowerD = (XPm_PowerDomain *)XPmPower_GetById(ParentNodes[i]);
		if (NULL == ParentPowerD) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		for (j = 0U; j < MAX_POWERDOMAINS; j++) {
			if (ParentPowerD->Children[j] != 0U) {
				continue;
			}

			ParentPowerD->Children[j] = Id;
			break;
		}

		if (MAX_POWERDOMAINS == j) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
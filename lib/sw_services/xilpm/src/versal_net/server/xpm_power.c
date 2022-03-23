/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_bisr.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psm.h"
#include "sleep.h"
#include "xpm_debug.h"
#include "xpm_err.h"

static XPm_Power *PmPowers[XPM_NODEIDX_POWER_MAX];
static u32 PmNumPowers;

XPm_Power *XPmPower_GetById(u32 Id)
{
	XPm_Power *Power = NULL;
	u32 NodeClass = NODECLASS(Id);
	u32 NodeIndex = NODEINDEX(Id);

	if (((u32)XPM_NODECLASS_POWER == NodeClass) &&
	    ((u32)XPM_NODEIDX_POWER_MAX > NodeIndex)) {
		Power = PmPowers[NodeIndex];
		/* Validate power node ID is same as given ID. */
		if ((NULL != Power) && (Id != Power->Node.Id)) {
			Power = NULL;
		}
	}

	return Power;
}

static XStatus SetPowerNode(u32 Id, XPm_Power *PwrNode)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != PwrNode) && ((u32)XPM_NODEIDX_POWER_MAX > NodeIndex)) {
		PmPowers[NodeIndex] = PwrNode;
		PmNumPowers++;
		Status = XST_SUCCESS;
	}

	return Status;
}

XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Todo: Uncomment this after integrating with CDO handler */
	if (NULL != XPmPower_GetById(Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	XPmNode_Init(&Power->Node, Id, (u8)XPM_POWER_STATE_OFF, BaseAddress);

	Power->Parent = Parent;
	/*TBD: add PowerEvent */
	Power->HandleEvent = NULL;
	Power->UseCount = 0;
	Power->WfParentUseCnt = 0;
	Power->PwrDnLatency = 0;
	Power->PwrUpLatency = 0;
	Status = SetPowerNode(Id, Power);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPower_AddParent(u32 Id, const u32 *Parents, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power;
	XPm_Power *PowerParent;
	u32 i;

	Power = XPmPower_GetById(Id);

	if (NULL == Power) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		if ((u32)XPM_NODESUBCL_POWER_DOMAIN ==
		    NODESUBCLASS(Parents[i])) {
			PowerParent = XPmPower_GetById(Parents[i]);
			if (NULL == PowerParent) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			Power->Parent = PowerParent;
		}
	}

	if ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(Id)) {
		(void)XPmPowerDomain_AddParent(Id, Parents, NumParents);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
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
#include "xpm_domain_iso.h"
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

XStatus XPm_PowerUpLPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for LPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnLPD(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for LPD */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for FPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnFPD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for FPD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpPLD(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for PLD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnPLD(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for PLD */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnCPM5N(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for CPM5N */
	(void)Node;
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpNoC(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for NOC */
	(void)Node;
	Status = XST_SUCCESS;

        return Status;
}

XStatus XPm_PowerDwnNoC(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for NOC */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerDwnHnicx(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power down sequence for HNICX */
	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_PowerUpHnicx(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Add Power up sequence for HNICX */
	Status = XST_SUCCESS;

        return Status;
}
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	const struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	switch (Function) {
	case (u32)FUNC_INIT_START:
		PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_INITIALIZING;
		if ((NULL != Ops) && (NULL != Ops->InitStart)) {
			Status = Ops->InitStart(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_START;
				goto done;
			}
		}

		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_INIT_FINISH:
		if ((u8)XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
			PmWarn("[INIT_FINISH]Skip. PwrDomain 0x%X is in wrong state 0x%X\n\r", \
				PwrDomain->Power.Node.Id, PwrDomain->Power.Node.State);
			Status = XST_SUCCESS;
			goto done;
		}
		if ((NULL != Ops) && (NULL != Ops->InitFinish)) {
			Status = Ops->InitFinish(PwrDomain, Args, NumArgs);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
				goto done;
			}
			PwrDomain->Power.Node.State = (u8)XPM_POWER_STATE_ON;
		}

		Status = XPmDomainIso_ProcessPending();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_ISO;
			goto done;
		}

		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

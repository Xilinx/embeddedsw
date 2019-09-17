/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pmcdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_prot.h"

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePmcDomainEvent(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("State=%d, Event=%d\n\r", Node->State, Event);

	switch (Node->State)
	{
		case (u8)XPM_POWER_STATE_ON:
			if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
			} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				Power->UseCount--;
			} else {
				/* Required by MISRA */
			}
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function configures xppu for PMC or PMC_NPI
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus PmcXppuCtrl(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 XppuNodeId, Enable;

	if(NumOfArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XppuNodeId = Args[0];
	Enable = Args[1];

	if ((u32)XPM_NODECLASS_PROTECTION != NODECLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_PROT_XPPU != NODESUBCLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((1U == Enable) && (3U == NumOfArgs)) {
		Status = XPmProt_XppuEnable(XppuNodeId, Args[2]);
	} else {
		Status = XPmProt_XppuDisable(XppuNodeId);
	}

done:
	return Status;
}

static struct XPm_PowerDomainOps PmcOps = {
	.XppuCtrl = PmcXppuCtrl,
};

XStatus XPmPmcDomain_Init(XPm_PmcDomain *PmcDomain, u32 Id)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPowerDomain_Init(&PmcDomain->Domain, Id, 0x00000000, NULL, &PmcOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmcDomain->Domain.Power.Node.State = (u8)XPM_POWER_STATE_ON;
	PmcDomain->Domain.Power.UseCount = 1;

	HandlePowerEvent = PmcDomain->Domain.Power.HandleEvent;
	PmcDomain->Domain.Power.HandleEvent = HandlePmcDomainEvent;

done:
	return Status;
}

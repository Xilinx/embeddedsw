/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pmcdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_prot.h"

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePmcDomainEvent(XPm_Node *Node, u32 Event)
{
	u32 Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("State=%d, Event=%d\n\r", Node->State, Event);

	switch (Node->State)
	{
		case XPM_POWER_STATE_ON:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				Power->UseCount++;
			} else if (XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				Power->UseCount--;
			} else {
				/* Required by MISRA */
			}
			break;
		default:
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
	XStatus Status = XST_SUCCESS;
	u32 XppuNodeId, Enable;

	if(NumOfArgs < 2) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XppuNodeId = Args[0];
	Enable = Args[1];

	if (XPM_NODECLASS_PROTECTION != NODECLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (XPM_NODESUBCL_PROT_XPPU != NODESUBCLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if(Enable && (NumOfArgs==3))
		Status = XPmProt_XppuEnable(XppuNodeId, Args[2]);
	else
		Status = XPmProt_XppuDisable(XppuNodeId);

done:
	return Status;
}

struct XPm_PowerDomainOps PmcOps = {
	.XppuCtrl = PmcXppuCtrl,
};

XStatus XPmPmcDomain_Init(XPm_PmcDomain *PmcDomain, u32 Id)
{
	XPmPowerDomain_Init(&PmcDomain->Domain, Id, 0x00000000, NULL, &PmcOps);

	PmcDomain->Domain.Power.Node.State = XPM_POWER_STATE_ON;
	PmcDomain->Domain.Power.UseCount = 1;

	HandlePowerEvent = PmcDomain->Domain.Power.Node.HandleEvent;
	PmcDomain->Domain.Power.Node.HandleEvent =
		HandlePmcDomainEvent;

	return XST_SUCCESS;
}

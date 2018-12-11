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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pslpdomain.h"

static XStatus (*HandlePowerEvent)(XPm_Node *Node, u32 Event);

static XStatus HandlePsLpDomainEvent(XPm_Node *Node, u32 Event)
{
	u32 Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;
	XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)Node;

	PmDbg("State=%s, Event=%s\n\r",
		PmPowerStates[Node->State], PmPowerEvents[Event]);

	switch (Node->State)
	{
		case XPM_POWER_STATE_PWR_UP_SELF:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: Power node is up */) {
					Node->State = XPM_POWER_STATE_LBIST;
					/* Todo: Write to PSM request register */
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_LBIST:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: LBIST is done */) {
					Node->State = XPM_POWER_STATE_SCAN_CLEAR;
					/* Todo: Write to PSM request register */
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_SCAN_CLEAR:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: Scan Clear is done */) {
					Node->State = XPM_POWER_STATE_BISR;
					/* Todo: Write to PSM request register */
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_BISR:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: BISR is done */) {
					Node->State = XPM_POWER_STATE_MIST;
					/* Todo: Write to PSM request register */
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_MIST:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: MIST is done */) {
					Node->State = XPM_POWER_STATE_ON;
					PsLpd->Domain.Power.UseCount++;
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_ON:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				/* Use base class handler */
				Status = HandlePowerEvent(Node, Event);
			} else if (XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				if (1 == Power->UseCount) {
					Node->State = XPM_POWER_STATE_PWR_DOWN_SELF;
					/* Todo: Power down LPD */
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Use base class handler */
					Status = HandlePowerEvent(Node, Event);
				}
			} else {
				/* Required by MISRA */
			}
			break;
		case XPM_POWER_STATE_PWR_DOWN_SELF:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check LPD power state */
				if (1 /* Hack: LPD is down */) {
					Power->UseCount--;
					if (NULL != Power->Parent) {
						Node->State = XPM_POWER_STATE_PWR_DOWN_PARENT;
						Power->WfParentUseCnt = Power->Parent->UseCount - 1;
						Status = Power->Parent->Node.HandleEvent(
							&Power->Parent->Node, XPM_POWER_EVENT_PWR_DOWN);
						/* Todo: Start timer to poll the parent node */
						/* Hack */
						Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
					} else {
						Node->State = XPM_POWER_STATE_OFF;
					}
				} else {
					/* Todo: Restart timer to poll parent node */
				}
			}
			break;
		default:
			Status = HandlePowerEvent(Node, Event);
			break;
	}

	return Status;
}

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd,
	u32 Id, u32 BaseAddress, XPm_Power *Parent)
{
	XPmPowerDomain_Init(&PsLpd->Domain,
		Id, BaseAddress, Parent);

	HandlePowerEvent = PsLpd->Domain.Power.Node.HandleEvent;
	PsLpd->Domain.Power.Node.HandleEvent =
		HandlePsLpDomainEvent;

	return XST_SUCCESS;
}

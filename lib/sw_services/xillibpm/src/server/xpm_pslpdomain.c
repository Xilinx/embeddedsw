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

/****************************************************************************/
/**
 * @brief  This function executes scan clear sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
int XPmPsLpDomain_ScanClearLpd(void)
{
	int Status = XST_FAILURE;

	/* Trigger Scan clear on LPD/LPD_IOU */
	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		(PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK),
		(PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK));

	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_DONE,
				 (PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_PASS,
				 (PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes LBIST sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
int XPmPsLpDomain_LpdLBIST(void)
{
	int Status = XST_FAILURE;

	/* Trigger LBIST on LPD */
	PmRmw32(PMC_ANALOG_LBIST_ENABLE,
		(PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		 PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK),
		(PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		 PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK));

	Status = XPm_PollForMask(PMC_ANALOG_LBIST_DONE,
				 (PMC_ANALOG_LBIST_DONE_LPD_MASK |
				  PMC_ANALOG_LBIST_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes BISR sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
int XPmPsLpDomain_LpdBISR(void)
{
	int Status = XST_FAILURE;

	PmRmw32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_TRIGGER_MASK,
		LPD_SLCR_BISR_TRIGGER_MASK);

	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS,
				 (LPD_SLCR_BISR_DONE_GLOBAL_MASK |
				  LPD_SLCR_BISR_DONE_1_MASK |
				  LPD_SLCR_BISR_DONE_0_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS,
				 (LPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  LPD_SLCR_BISR_PASS_1_MASK |
				  LPD_SLCR_BISR_PASS_0_MASK),
				 XPM_POLL_TIMEOUT);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes MBIST sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
int XPmPsLpDomain_LpdMBIST(void)
{
	int Status = XST_FAILURE;
	u32 RegValue;

	PmRmw32(PMC_ANALOG_OD_MBIST_RST,
		(PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_MASK));

	PmRmw32(PMC_ANALOG_OD_MBIST_SETUP,
		(PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK));

	PmRmw32(PMC_ANALOG_OD_MBIST_PG_EN,
		(PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK));

	Status = XPm_PollForMask(PMC_ANALOG_OD_MBIST_DONE,
				 (PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK|
				  PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK |
				  PMC_ANALOG_OD_MBIST_DONE_LPD_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmIn32(PMC_ANALOG_OD_MBIST_GOOD, RegValue);

	if ((PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK) != RegValue) {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

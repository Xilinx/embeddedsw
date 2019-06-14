/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#include "xpm_power.h"
#include "xpm_powerdomain.h"
#include "xpm_psm.h"


static XPm_Power *PmPowers[XPM_NODEIDX_POWER_MAX];
static u32 PmNumPowers;

XPm_Power *XPmPower_GetById(u32 Id)
{
	XPm_Power *Power = NULL;
	u32 NodeClass = NODECLASS(Id);
	u32 NodeIndex = NODEINDEX(Id);

	if ((XPM_NODECLASS_POWER == NodeClass) &&
	    (XPM_NODEIDX_POWER_MAX > NodeIndex)) {
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
	u32 Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != PwrNode) && (XPM_NODEIDX_POWER_MAX > NodeIndex)) {
		PmPowers[NodeIndex] = PwrNode;
		PmNumPowers++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SendPowerUpReq(XPm_Node *Node)
{
	u32 Status = XST_SUCCESS;

	if (XPM_POWER_STATE_ON == Node->State)
		goto done;

	if (XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		Status = XPmPsm_SendPowerUpReq(Node->BaseAddress);
	} else {
		PmInfo("Request to power up domain %x\r\n",Node->Id);
		switch (NODEINDEX(Node->Id)) {
		case XPM_NODEIDX_POWER_LPD:
			Status = XPm_PowerUpLPD(Node);
			break;
		case XPM_NODEIDX_POWER_FPD:
			Status = XPm_PowerUpFPD(Node);
			break;
		case XPM_NODEIDX_POWER_NOC:
			Status = XPm_PowerUpNoC(Node);
			break;
		case XPM_NODEIDX_POWER_ME:
			Status = XPm_PowerUpME(Node);
			break;
		case XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerUpPLD(Node);
			break;
		case XPM_NODEIDX_POWER_CPM:
			Status = XPm_PowerUpCPM(Node);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
		}
	}
done:
	return Status;
}

static XStatus SendPowerDownReq(XPm_Node *Node)
{
	u32 Status = XST_SUCCESS;

	if (XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		Status = XPmPsm_SendPowerDownReq(Node->BaseAddress);
	} else {
		PmInfo("Request to power down domain %x\r\n",Node->Id);
		switch (NODEINDEX(Node->Id)) {
		case XPM_NODEIDX_POWER_LPD:
			Status = XPm_PowerDwnLPD();
			break;
		case XPM_NODEIDX_POWER_FPD:
			Status = XPm_PowerDwnFPD(Node);
			break;
		case XPM_NODEIDX_POWER_NOC:
			Status = XPm_PowerDwnNoC();
			break;
		case XPM_NODEIDX_POWER_ME:
			Status = XPm_PowerDwnME();
			break;
		case XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerDwnPLD();
			break;
		case XPM_NODEIDX_POWER_CPM:
			Status = XPm_PowerDwnCPM();
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
		}
	}

	return Status;
}

static XStatus HandlePowerEvent(XPm_Node *Node, u32 Event)
{
	u32 Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("State=%s, Event=%s\n\r",
		PmPowerStates[Node->State], PmPowerEvents[Event]);

	switch (Node->State)
	{
		case XPM_POWER_STATE_STANDBY:
		case XPM_POWER_STATE_OFF:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				if (NULL != Power->Parent) {
					Node->State = XPM_POWER_STATE_PWR_UP_PARENT;
					Power->WfParentUseCnt = Power->Parent->UseCount + 1;
					Status = Power->Parent->Node.HandleEvent(
						&Power->Parent->Node, XPM_POWER_EVENT_PWR_UP);
					/* Todo: Start timer to poll parent node */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Write to PSM power up request register */
					SendPowerUpReq(Node);
					Node->State = XPM_POWER_STATE_PWR_UP_SELF;
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				}
			}
			break;
		case XPM_POWER_STATE_PWR_UP_PARENT:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Power->WfParentUseCnt == Power->Parent->UseCount) {
					Power->WfParentUseCnt = 0;
					/* Write to PSM power up request register */
					SendPowerUpReq(Node);
					Node->State = XPM_POWER_STATE_PWR_UP_SELF;
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll parent state */
				}
			}
			break;
		case XPM_POWER_STATE_PWR_UP_SELF:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: Power node is up */) {
					Node->State = XPM_POWER_STATE_ON;
					Power->UseCount++;
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_ON:
			if (XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				if (NULL != Power->Parent) {
					Status = Power->Parent->Node.HandleEvent(
						&Power->Parent->Node, XPM_POWER_EVENT_PWR_UP);
				}
				Power->UseCount++;
			} else if (XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				if (1 == Power->UseCount) {
					Node->State = XPM_POWER_STATE_PWR_DOWN_SELF;
					/* Write to PSM power down request register */
					SendPowerDownReq(Node);
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Node->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					Power->UseCount--;
					if (NULL != Power->Parent) {
						Status = Power->Parent->Node.HandleEvent(
							&Power->Parent->Node, XPM_POWER_EVENT_PWR_DOWN);
					}
				}
			} else {
				/* Required by MISRA */
			}
			break;
		case XPM_POWER_STATE_PWR_DOWN_SELF:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (1 /* Hack: Power node is down */) {
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
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case XPM_POWER_STATE_PWR_DOWN_PARENT:
			if (XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Power->WfParentUseCnt == Power->Parent->UseCount) {
					Node->State = XPM_POWER_STATE_OFF;
					Power->WfParentUseCnt = 0;
				} else {
					/* Todo: Restart timer to poll parent state */
				}
			}
			break;
		default:
			break;
	}

	return Status;
}

XStatus XPmPower_GetStatus(const u32 SubsystemId, const u32 DeviceId, XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power;

	/* Warning Fix */
	(void)SubsystemId;

	Power = XPmPower_GetById(DeviceId);
	if (NULL == Power) {
		goto done;
	}

	DeviceStatus->Status = Power->Node.State;
	DeviceStatus->Usage = 0;
	DeviceStatus->Requirement = 0;
	Status = XST_SUCCESS;

done:
	return Status;
}
XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PowerDomain;

	/* Todo: Uncomment this after integrating with CDO handler */
	if (NULL != XPmPower_GetById(Id)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	XPmNode_Init(&Power->Node,
		Id, (u32)XPM_POWER_STATE_OFF, BaseAddress);

	Power->Parent = Parent;
	Power->NextPeer = NULL;
	Power->Node.HandleEvent = HandlePowerEvent;
	Power->UseCount = 0;
	Power->WfParentUseCnt = 0;
	Power->PwrDnLatency = 0;
	Power->PwrUpLatency = 0;

	if ((NULL != Parent) && (XPM_NODESUBCL_POWER_DOMAIN ==
				 NODESUBCLASS(Parent->Node.Id))) {
		PowerDomain = (XPm_PowerDomain *)Parent;
		Power->NextPeer = PowerDomain->Children;
		PowerDomain->Children = Power;
	}

	Status = SetPowerNode(Id, Power);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPower_AddParent(u32 Id, u32 *Parents, u32 NumParents)
{
	int Status = XST_SUCCESS;
	XPm_Power *Power;
	XPm_Power *PowerParent;
	u32 i;

	Power = XPmPower_GetById(Id);

	if (NULL == Power) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++) {
		PowerParent = XPmPower_GetById(Parents[i]);
		if (NULL == PowerParent) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/* Todo: Handle more than one parent */
		Power->Parent = PowerParent;
	}

done:
	return Status;
}

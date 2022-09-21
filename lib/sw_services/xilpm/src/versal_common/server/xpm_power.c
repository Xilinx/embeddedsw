/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_notifier.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_pll.h"
#include "xpm_psm.h"
#include "xpm_requirement.h"
#include "xpm_debug.h"

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

/****************************************************************************/
/**
 * @brief	Get handle to requested power node by "only" Node INDEX
 *
 * @param PwrIndex	Power Node Index
 *
 * @return	Pointer to requested XPm_Power, NULL otherwise
 *
 * @note	Requires ONLY Node Index
 *
 * Caller should be _careful_ while using this function as it skips the checks
 * for validating the class, subclass and type of the power before and after
 * retrieving the node from the database. Use this only where it is absolutely
 * necessary, otherwise use XPmPower_GetById() which is more strict
 * and requires 'complete' Node ID for retrieving the handle.
 *
 ****************************************************************************/
static XPm_Power *XPmPower_GetByIndex(const u32 PwrIndex)
{
	XPm_Power *Power = NULL;

	/* Validate power node index is less than maximum power node index. */
	if ((u32)XPM_NODEIDX_POWER_MAX > PwrIndex) {
		Power = PmPowers[PwrIndex];
		/* Validate power node index is same as given index. */
		if ((NULL != Power) &&
		    (PwrIndex != NODEINDEX(Power->Node.Id))) {
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

static void ResetPowerDomainOpFlags(XPm_Power *PwrNode)
{
	XPm_PowerDomain *PwrDomain = (XPm_PowerDomain *)PwrNode;
	u32 SubClass = NODESUBCLASS(PwrNode->Node.Id);

	/*
	 * Reset domain init flags, so operations can be performed
	 * Note:
	 *   Call this function only through the Power node FSM
	 *   when the state of a node is powered off (~ usecount == 0)
	 */
	if ((u32)XPM_NODESUBCL_POWER_DOMAIN == SubClass) {
		PwrDomain->InitFlag = 0U;
	}
}

static XStatus SendPowerUpReq(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	if ((u8)XPM_POWER_STATE_ON == Node->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		Status = XPmPower_SendIslandPowerUpReq(Node);
	} else {
		PmDbg("Request to power up domain %x\r\n",Node->Id);
		switch (NODEINDEX(Node->Id)) {
		case (u32)XPM_NODEIDX_POWER_LPD:
			Status = XPm_PowerUpLPD(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_FPD:
			Status = XPm_PowerUpFPD(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_NOC:
			Status = XPm_PowerUpNoC(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerUpPLD(Node);
			break;
		default:
			Status = XPmPower_PlatSendPowerUpReq(Node);
			break;
		}
	}
done:
	return Status;
}

static XStatus SendPowerDownReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	u32 Idx;

	if ((u32)XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		Status = XPmPower_SendIslandPowerDwnReq(Node);
	} else {
		/* Put the PLL in suspended mode */
		for (Idx = (u32)XPM_NODEIDX_CLK_MIN; Idx < (u32)XPM_NODEIDX_CLK_MAX; Idx++) {
			Clk = XPmClock_GetByIdx(Idx);
			if ((NULL != Clk) && (ISPLL(Clk->Node.Id)) &&
			    (Node->Id == Clk->PwrDomain->Node.Id)) {
				Status = XPmClockPll_Suspend((XPm_PllClockNode *)Clk);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}

		PmDbg("Request to power down domain %x\r\n",Node->Id);
		switch (NODEINDEX(Node->Id)) {
		case (u32)XPM_NODEIDX_POWER_LPD:
			Status = XPm_PowerDwnLPD();
			break;
		case (u32)XPM_NODEIDX_POWER_FPD:
			Status = XPm_PowerDwnFPD(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_NOC:
			Status = XPm_PowerDwnNoC();
			break;
		case (u32)XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerDwnPLD(Node);
			break;
		default:
			Status = XPmPower_PlatSendPowerDownReq(Node);
			break;
		}
	}

done:
	return Status;
}

static XStatus XPmPower_PowerOffState(XPm_Node *Node, u32 Event, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
		Status = XST_SUCCESS;
		if (NULL != Power->Parent) {
			Node->State = (u8)XPM_POWER_STATE_PWR_UP_PARENT;
			Power->WfParentUseCnt = Power->Parent->UseCount + 1U;
			Status = Power->Parent->HandleEvent(&Power->Parent->Node,
							    XPM_POWER_EVENT_PWR_UP);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_PWR_PARENT_UP;
				goto done;
			}
			/* Todo: Start timer to poll parent node */
			/* Hack */
			Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
		} else {
			/* Write to PSM power up request register */
			Status = SendPowerUpReq(Node);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_PSM_PWR_UP;
				goto done;
			}
			Node->State = (u8)XPM_POWER_STATE_PWR_UP_SELF;
			/* Todo: Start timer to poll PSM status register */
			/* Hack */
			Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
		}
	}
done:
	return Status;
}

static XStatus XPmPower_PowerUpParentState(XPm_Node *Node, u32 Event, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_TIMER == Event) {
		Status = XST_SUCCESS;
		if (Power->WfParentUseCnt == Power->Parent->UseCount) {
			Power->WfParentUseCnt = 0;
			/* Write to PSM power up request register */
			Status = SendPowerUpReq(Node);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_PSM_PWR_UP;
				goto done;
			}
			Node->State = (u8)XPM_POWER_STATE_PWR_UP_SELF;
			/* Todo: Start timer to poll PSM status register */
			/* Hack */
			Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
		} else {
			/* Todo: Restart timer to poll parent state */
		}
	}
done:
	return Status;
}

static XStatus XPmPower_PowerUpSelfState(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_TIMER == Event) {
		Status = XST_SUCCESS;
		/* Todo: Read PSM status register */
		if (TRUE /* Hack: Power node is up */) {
			Node->State = (u8)XPM_POWER_STATE_ON;
			XPmNotifier_Event(Node->Id, (u32)EVENT_STATE_CHANGE);
			Power->UseCount++;
		} else {
			/* Todo: Restart timer to poll PSM */
		}
	}

	return Status;
}

static XStatus XPmPower_PowerOnState(XPm_Node *Node, u32 Event, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
		Status = XST_SUCCESS;
		/**
		 * Here UseCount 0 with state ON indicates that power
		 * is ON (CDO is loaded) but no child node is requested.
		 * So power parent needs to be requested since power node
		 * UseCount is changing from 0 to 1
		 */
		if ((0U == Power->UseCount) && (NULL != Power->Parent)) {
			Status = Power->Parent->HandleEvent(&Power->Parent->Node,
							    XPM_POWER_EVENT_PWR_UP);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_PWR_PARENT_UP;
				goto done;
			}
		}
		Power->UseCount++;
	} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Event) {
		Status = XST_SUCCESS;
		if (1U == Power->UseCount) {
			/* Write to PSM power down request register */
			Status = SendPowerDownReq(Node);
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_PSM_PWR_DWN;
				goto done;
			}
			Node->State = (u8)XPM_POWER_STATE_PWR_DOWN_SELF;
			/* Todo: Start timer to poll PSM status register */
			/* Hack */
			Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
		} else if (1U < Power->UseCount) {
			Power->UseCount--;
		} else {
			/* Required by MISRA */
		}
	} else {
		/* Required by MISRA */
	}
done:
	return Status;
}

static XStatus XPmPower_PowerDwnSelfState(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_TIMER == Event) {
		Status = XST_SUCCESS;
		/* Todo: Read PSM status register */
		if (TRUE /* Hack: Power node is down */) {
			if (1U == Power->UseCount) {
				Power->UseCount--;
			}
			if (NULL != Power->Parent) {
				Node->State = (u8)XPM_POWER_STATE_PWR_DOWN_PARENT;
				Power->WfParentUseCnt = (u8)(Power->Parent->UseCount - 1U);
				Status = Power->Parent->HandleEvent(&Power->Parent->Node,
								    XPM_POWER_EVENT_PWR_DOWN);
				/* Todo: Start timer to poll the parent node */
				/* Hack */
				Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
			} else {
				Node->State = (u8)XPM_POWER_STATE_OFF;
				ResetPowerDomainOpFlags(Power);
				XPmNotifier_Event(Node->Id, (u32)EVENT_STATE_CHANGE);
			}
		} else {
			/* Todo: Restart timer to poll PSM */
		}
	}

	return Status;
}

static XStatus XPmPower_PowerDwnParentState(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	if ((u32)XPM_POWER_EVENT_TIMER == Event) {
		Status = XST_SUCCESS;
		if (Power->WfParentUseCnt == Power->Parent->UseCount) {
			Node->State = (u8)XPM_POWER_STATE_OFF;
			Power->WfParentUseCnt = 0;
			ResetPowerDomainOpFlags(Power);
			XPmNotifier_Event(Node->Id, (u32)EVENT_STATE_CHANGE);
		} else {
			/* Todo: Restart timer to poll parent state */
		}
	}

	return Status;
}

static XStatus PowerEvent(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Power = (XPm_Power *)Node;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmDbg("[IN] Id:0x%x, UseCount:%d, State=%x, Event=%x\r\n",
			Node->Id, Power->UseCount, Node->State, Event);

	switch (Node->State)
	{
		case (u8)XPM_POWER_STATE_STANDBY:
		case (u8)XPM_POWER_STATE_OFF:
			Status = XPmPower_PowerOffState(Node, Event, &DbgErr);
			break;
		case (u8)XPM_POWER_STATE_PWR_UP_PARENT:
			Status = XPmPower_PowerUpParentState(Node, Event, &DbgErr);
			break;
		case (u8)XPM_POWER_STATE_PWR_UP_SELF:
			Status = XPmPower_PowerUpSelfState(Node, Event);
			break;
		case (u8)XPM_POWER_STATE_ON:
			Status = XPmPower_PowerOnState(Node, Event, &DbgErr);
			break;
		case (u8)XPM_POWER_STATE_PWR_DOWN_SELF:
			Status = XPmPower_PowerDwnSelfState(Node, Event);
			break;
		case (u8)XPM_POWER_STATE_PWR_DOWN_PARENT:
			Status = XPmPower_PowerDwnParentState(Node, Event);
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	PmDbg("[OUT] Id:0x%x, UseCount:%d, State=%x, Event=%x\r\n",
			Node->Id, Power->UseCount, Node->State, Event);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPower_GetStatus(const u32 SubsystemId, const u32 DeviceId, XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Power;

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
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Todo: Uncomment this after integrating with CDO handler */
	if (NULL != XPmPower_GetById(Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	XPmNode_Init(&Power->Node, Id, (u8)XPM_POWER_STATE_OFF, BaseAddress);

	Power->Parent = Parent;
	Power->HandleEvent = PowerEvent;
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

XStatus XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Power *Power = XPmPower_GetById(DeviceId);
	const XPm_Power *Parent;

	*Latency = 0;
	if (NULL == Power) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON == Power->Node.State) {
		goto done;
	}

	*Latency += Power->PwrUpLatency;

	Parent = Power->Parent;

	/* Account latencies of parents if a parent is down */
	while (NULL != Parent) {
		if ((u8)XPM_POWER_STATE_ON == Parent->Node.State) {
			break;
		}

		*Latency += Parent->PwrUpLatency;
		Parent = Parent->Parent;
	}

done:
	return Status;
}

XStatus XPmPower_ForcePwrDwn(u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power;
	const XPm_Device *Device;
	const XPm_Core *Core;
	u32 i;

	if ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(NodeId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/*
	 * PMC power domain can not be powered off.
	 */
	if ((u32)XPM_NODEIDX_POWER_PMC == NODEINDEX(NodeId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/**
	 * Check if any power domain is ON and its parent is requested power
	 * domain, then explicitly power down those such power domains.
	 */
	for (i = (u32)XPM_NODEIDX_POWER_PMC; i < (u32)XPM_NODEIDX_POWER_MAX; i++) {
		Power = XPmPower_GetByIndex(i);
		if ((NULL == Power) || (NULL == Power->Parent) ||
		    (Power->Parent->Node.Id != NodeId) ||
		    ((u8)XPM_POWER_STATE_ON != Power->Node.State) ||
		    ((u32)XPM_NODESUBCL_POWER_DOMAIN !=
		    NODESUBCLASS(Power->Node.Id))) {
			continue;
		}

		Status = XPmPower_ForcePwrDwn(Power->Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/**
	 * This is a special use case where child power domain is ON but no
	 * device of that power domain is requested. So use count of child power
	 * domain is 0. Also parent power domain usecount does not consider this
	 * child power domain as no device is requested.
	 *
	 * So to force power domain in this case, increment child and parent
	 * both power domain and call power down of child power domain which
	 * powers off child power domain without affecting usecount.
	 */
	Power = XPmPower_GetById(NodeId);
	if ((NULL != Power) && (0U == Power->UseCount) &&
	    ((u8)XPM_POWER_STATE_ON == Power->Node.State)) {
		Status = Power->HandleEvent(&Power->Node, (u32)XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = Power->HandleEvent(&Power->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/*
	 * Release devices belonging to the power domain.
	 */
	for (i = 1; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
		/*
		 * Note: XPmDevice_GetByIndex() assumes that the caller is
		 * responsible for validating the Node ID attributes other than
		 * node index.
		 */
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) ||
		    ((u32)XPM_DEVSTATE_UNUSED == Device->Node.State)) {
			continue;
		}

		/*
		 * Check power topology of this device to identify if it belongs
		 * to the power domain.
		 */
		Power = Device->Power;
		while (NULL != Power) {
			if (NodeId == Power->Node.Id) {
				/* Disable the direct wake in case of force power down */
				if ((u32)XPM_NODESUBCL_DEV_CORE ==
				    NODESUBCLASS(Device->Node.Id)) {
					Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
					if (NULL != Core) {
						DisableWake(Core);
					}
				}

				Status = XPmRequirement_Release(Device->Requirements,
								RELEASE_DEVICE);
				if (XST_SUCCESS != Status) {
					Status = XPM_PM_INVALID_NODE;
					goto done;
				}
			}
			Power = Power->Parent;
		}
	}

done:
	return Status;
}

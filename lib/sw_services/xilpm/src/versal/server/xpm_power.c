/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_bisr.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psm.h"
#include "sleep.h"
#include "xpm_rpucore.h"
#include "xpm_pll.h"

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


static XStatus PowerUpXram(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *Device;
	u32 XramSlcrAddress, PwrCtlAddress, PwrStatusAddress, RegVal;
	u32 BitMask;

	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);
	if (NULL == Device) {
		goto done;
	}
	XramSlcrAddress = Device->Node.BaseAddress;
	BitMask = Node->BaseAddress;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_XRAM_0:
	case (u32)XPM_NODEIDX_POWER_XRAM_1:
	case (u32)XPM_NODEIDX_POWER_XRAM_2:
	case (u32)XPM_NODEIDX_POWER_XRAM_3:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK0_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK0_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_4:
	case (u32)XPM_NODEIDX_POWER_XRAM_5:
	case (u32)XPM_NODEIDX_POWER_XRAM_6:
	case (u32)XPM_NODEIDX_POWER_XRAM_7:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK1_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK1_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_8:
	case (u32)XPM_NODEIDX_POWER_XRAM_9:
	case (u32)XPM_NODEIDX_POWER_XRAM_10:
	case (u32)XPM_NODEIDX_POWER_XRAM_11:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK2_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK2_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_12:
	case (u32)XPM_NODEIDX_POWER_XRAM_13:
	case (u32)XPM_NODEIDX_POWER_XRAM_14:
	case (u32)XPM_NODEIDX_POWER_XRAM_15:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_UP_BANK3_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK3_OFFSET;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if already power up */
	RegVal = XPm_In32(PwrStatusAddress);
	if ((RegVal & BitMask) == BitMask) {
		goto done;
	}

	/* Enable power state for selected bank */
	PmRmw32(PwrCtlAddress, BitMask, BitMask);

	/* Poll for power status to set */
	Status = XPm_PollForMask(PwrStatusAddress, BitMask, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO: Wait for power to ramp up */
	usleep(1);

	/* TODO: Set chip enable bit */

done:
	return Status;
}

static XStatus PowerDwnXram(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *Device;
	u32 XramSlcrAddress, PwrCtlAddress, PwrStatusAddress, RegVal;
	u32 BitMask;

	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);
	if (NULL == Device) {
		goto done;
	}
	XramSlcrAddress = Device->Node.BaseAddress;
	BitMask = Node->BaseAddress;

	switch (NODEINDEX(Node->Id)) {
	case (u32)XPM_NODEIDX_POWER_XRAM_0:
	case (u32)XPM_NODEIDX_POWER_XRAM_1:
	case (u32)XPM_NODEIDX_POWER_XRAM_2:
	case (u32)XPM_NODEIDX_POWER_XRAM_3:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK0_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK0_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_4:
	case (u32)XPM_NODEIDX_POWER_XRAM_5:
	case (u32)XPM_NODEIDX_POWER_XRAM_6:
	case (u32)XPM_NODEIDX_POWER_XRAM_7:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK1_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK1_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_8:
	case (u32)XPM_NODEIDX_POWER_XRAM_9:
	case (u32)XPM_NODEIDX_POWER_XRAM_10:
	case (u32)XPM_NODEIDX_POWER_XRAM_11:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK2_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK2_OFFSET;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_XRAM_12:
	case (u32)XPM_NODEIDX_POWER_XRAM_13:
	case (u32)XPM_NODEIDX_POWER_XRAM_14:
	case (u32)XPM_NODEIDX_POWER_XRAM_15:
		PwrCtlAddress = XramSlcrAddress + XRAM_SLCR_PWR_DWN_BANK3_OFFSET;
		PwrStatusAddress = XramSlcrAddress + XRAM_SLCR_PWR_STATUS_BANK3_OFFSET;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if already power up */
	RegVal = XPm_In32(PwrStatusAddress);
	if ((RegVal & BitMask) == 0U) {
		goto done;
	}

	/* TODO: Clear chip enable bit */

	/* Disable power state for selected bank */
	PmRmw32(PwrCtlAddress, BitMask, ~BitMask);

	/* Poll for power status to clear */
	Status = XPm_PollForZero(PwrStatusAddress, BitMask, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}


done:
	return Status;
}


static XStatus SendPowerUpReq(XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON == Node->State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		if ((u32)XPM_NODETYPE_POWER_ISLAND_XRAM == NODETYPE(Node->Id)) {
			Status = PowerUpXram(Node);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			Status = XPmPsm_SendPowerUpReq(Node->BaseAddress);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
		}
		/*
		 * For S80 ES1, there is a bug in LPD which requires the
		 * LPD_INT and RPU power domain signals needs to be
		 * asserted, to prevent repair vector corruption(EDT-993543).
		 * To fix this bug rerun LPD BISR whenever the RPU power
		 * island is powered down and brought up again.
		 */
		if ((PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
		    (PLATFORM_VERSION_SILICON == Platform) &&
		    ((u32)XPM_NODEIDX_POWER_RPU0_0 == NODEINDEX(Node->Id)) &&
		    (0U == (LPD_BISR_DONE & LpDomain->LpdBisrFlags))) {
			if (0U != (LPD_BISR_DATA_COPIED & LpDomain->LpdBisrFlags)) {
				Status = XPmBisr_TriggerLpd();
			} else {
				Status = XPmBisr_Repair(LPD_TAG_ID);
			}
		}
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
		case (u32)XPM_NODEIDX_POWER_ME:
			Status = XPm_PowerUpME(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerUpPLD(Node);
			break;
		case (u32)XPM_NODEIDX_POWER_CPM:
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
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	u32 Idx;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(Node->Id)) {
		if ((u32)XPM_NODETYPE_POWER_ISLAND_XRAM == NODETYPE(Node->Id)) {
                        Status = PowerDwnXram(Node);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
                } else {
			Status = XPmPsm_SendPowerDownReq(Node->BaseAddress);
                        if (XST_SUCCESS != Status) {
                                goto done;
                        }
                }

		if ((u32)XPM_NODEIDX_POWER_RPU0_0 == NODEINDEX(Node->Id)) {
			LpDomain->LpdBisrFlags &= (u8)(~(LPD_BISR_DONE));
			/*
			 * Clear and Disable RPU internal reg comparators to prevent
			 * PSM_GLOBAL_REG.PSM_ERR1_STATUS.rpu_ls from erroring out
			 */
			 Status = XPm_RpuRstComparators(PM_DEV_RPU0_0);

			 if (Status != XST_SUCCESS) {
				PmErr("Unable to reset RPU Comparators\n\r");
				goto done;
			 }
		}
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
		case (u32)XPM_NODEIDX_POWER_ME:
			Status = XPm_PowerDwnME();
			break;
		case (u32)XPM_NODEIDX_POWER_PLD:
			Status = XPm_PowerDwnPLD();
			break;
		case (u32)XPM_NODEIDX_POWER_CPM:
			Status = XPm_PowerDwnCPM();
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
		}
	}

done:
	return Status;
}

static XStatus HandlePowerEvent(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = (XPm_Power *)Node;

	PmDbg("Id:0x%x, UseCount:%d, State=%x, Event=%x\n\r",
				Node->Id, Power->UseCount, Node->State, Event);

	switch (Node->State)
	{
		case (u8)XPM_POWER_STATE_STANDBY:
		case (u8)XPM_POWER_STATE_OFF:
			if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				if (NULL != Power->Parent) {
					Node->State = (u8)XPM_POWER_STATE_PWR_UP_PARENT;
					Power->WfParentUseCnt = Power->Parent->UseCount + 1U;
					Status = Power->Parent->HandleEvent(
						 &Power->Parent->Node, XPM_POWER_EVENT_PWR_UP);
					/* Todo: Start timer to poll parent node */
					/* Hack */
					Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Write to PSM power up request register */
					Status = SendPowerUpReq(Node);
					if (XST_SUCCESS != Status) {
						break;
					}
					Node->State = (u8)XPM_POWER_STATE_PWR_UP_SELF;
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				}
			}
			break;
		case (u8)XPM_POWER_STATE_PWR_UP_PARENT:
			if ((u32)XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Power->WfParentUseCnt == Power->Parent->UseCount) {
					Power->WfParentUseCnt = 0;
					/* Write to PSM power up request register */
					Status = SendPowerUpReq(Node);
					if (XST_SUCCESS != Status) {
						break;
					}
					Node->State = (u8)XPM_POWER_STATE_PWR_UP_SELF;
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					/* Todo: Restart timer to poll parent state */
				}
			}
			break;
		case (u8)XPM_POWER_STATE_PWR_UP_SELF:
			if ((u32)XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (TRUE /* Hack: Power node is up */) {
					Node->State = (u8)XPM_POWER_STATE_ON;
					Power->UseCount++;
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case (u8)XPM_POWER_STATE_ON:
			if ((u32)XPM_POWER_EVENT_PWR_UP == Event) {
				Status = XST_SUCCESS;
				if (NULL != Power->Parent) {
					Status = Power->Parent->HandleEvent(
						 &Power->Parent->Node, XPM_POWER_EVENT_PWR_UP);
				}
				Power->UseCount++;
			} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Event) {
				Status = XST_SUCCESS;
				if (1U == Power->UseCount) {
					/* Write to PSM power down request register */
					Status = SendPowerDownReq(Node);
					if (XST_SUCCESS != Status) {
						break;
					}
					Node->State = (u8)XPM_POWER_STATE_PWR_DOWN_SELF;
					/* Todo: Start timer to poll PSM status register */
					/* Hack */
					Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
				} else {
					Power->UseCount--;
					if (NULL != Power->Parent) {
						Status = Power->Parent->HandleEvent(
							 &Power->Parent->Node, XPM_POWER_EVENT_PWR_DOWN);
					}
				}
			} else {
				/* Required by MISRA */
			}
			break;
		case (u8)XPM_POWER_STATE_PWR_DOWN_SELF:
			if ((u32)XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Read PSM status register */
				if (TRUE /* Hack: Power node is down */) {
					Power->UseCount--;
					if (NULL != Power->Parent) {
						Node->State = (u8)XPM_POWER_STATE_PWR_DOWN_PARENT;
						Power->WfParentUseCnt = Power->Parent->UseCount - 1U;
						Status = Power->Parent->HandleEvent(
							 &Power->Parent->Node, XPM_POWER_EVENT_PWR_DOWN);
						/* Todo: Start timer to poll the parent node */
						/* Hack */
						Status = Power->HandleEvent(Node, XPM_POWER_EVENT_TIMER);
					} else {
						Node->State = (u8)XPM_POWER_STATE_OFF;
					}
				} else {
					/* Todo: Restart timer to poll PSM */
				}
			}
			break;
		case (u8)XPM_POWER_STATE_PWR_DOWN_PARENT:
			if ((u32)XPM_POWER_EVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Power->WfParentUseCnt == Power->Parent->UseCount) {
					Node->State = (u8)XPM_POWER_STATE_OFF;
					Power->WfParentUseCnt = 0;
				} else {
					/* Todo: Restart timer to poll parent state */
				}
			}
			break;
		default:
			Status = XST_FAILURE;
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

	XPmNode_Init(&Power->Node, Id, (u8)XPM_POWER_STATE_OFF, BaseAddress);

	Power->Parent = Parent;
	Power->NextPeer = NULL;
	Power->HandleEvent = HandlePowerEvent;
	Power->UseCount = 0;
	Power->WfParentUseCnt = 0;
	Power->PwrDnLatency = 0;
	Power->PwrUpLatency = 0;

	if ((NULL != Parent) && ((u32)XPM_NODESUBCL_POWER_DOMAIN ==
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
		PowerParent = XPmPower_GetById(Parents[i]);
		if (NULL == PowerParent) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/* Todo: Handle more than one parent */
		Power->Parent = PowerParent;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

int XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	int Status = XST_SUCCESS;
	XPm_Power *Power = XPmPower_GetById(DeviceId);
	XPm_Power *Parent;

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

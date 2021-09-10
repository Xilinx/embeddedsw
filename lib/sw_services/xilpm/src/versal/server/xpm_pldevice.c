/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_pldevice.h"
#include "xpm_debug.h"
#include "xpm_defs.h"
#include "xpm_aiedevice.h"

#define PWR_DOMAIN_UNUSED_BITMASK		0U
#define PWR_DOMAIN_NOC_BITMASK			BIT(0)
#define PWR_DOMAIN_PL_BITMASK			BIT(1)
#define MAX_PWR_DOMAIN_BITMASK			(PWR_DOMAIN_NOC_BITMASK |\
						  PWR_DOMAIN_PL_BITMASK)
#define NOT_INITIALIZED			0xFFU

typedef struct {
	const u8 BitMask;
	const u32 NodeId;
} XPm_NodeIdBitMap;

static const XPm_NodeIdBitMap PmPwrBitMap[] = {
	{
		.BitMask = PWR_DOMAIN_NOC_BITMASK,
		.NodeId = PM_POWER_NOC,
	}, {
		.BitMask = PWR_DOMAIN_PL_BITMASK,
		.NodeId = PM_POWER_PLD,
	},
};

static XStatus Pld_SetBitPwrBitMask(u8 *BitMask, const u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(PmPwrBitMap); ++i) {
		if (PmPwrBitMap[i].NodeId == NodeId) {
			*BitMask |= PmPwrBitMap[i].BitMask;
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

static XStatus Pld_UnsetBitPwrBitMask(u8 *BitMask, const u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	u32 i;

	for (i = 0; i < ARRAY_SIZE(PmPwrBitMap); ++i) {
		if (PmPwrBitMap[i].NodeId == NodeId) {
			*BitMask &= (u8)(~(PmPwrBitMap[i].BitMask));
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Check if a certain PLD Device is a valid device. Qualification
 * 		for a device to be called valid is:
 * 		- Should have a structure allocated
 * 		- Should have a valid existing parent (to prevent broken trees,
 * 		  as only 1 tree can exist). Exception is for PLD_0 because
 * 		  it's the root of the tree
 *
 * @param PlDevice	PlDevice whose validity needs to be checked
 *
 * @return XStatus	Returns XST_SUCCESS or print appropriate error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmPlDevice_IsValidPld(const XPm_PlDevice *PlDevice)
{
	XStatus Status = XST_FAILURE;
	const XPm_PlDevice *Parent;

	if (NULL == PlDevice) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Parent = PlDevice->Parent;
	if ((NULL == Parent) &&
	    ((u32)XPM_NODEIDX_DEV_PLD_0 != NODEINDEX(PlDevice->Device.Node.Id))) {
		goto done;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Release a PlDevice's children from PMC Subsystem and unlink
 *
 * @param PlDevice	PlDevice whose children need to released and unlinked
 *
 * @return	XStatus	Returns XST_SUCCESS or appropriate error code
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus Pld_ReleaseChildren(XPm_PlDevice *PlDevice)
{
	XStatus Status = XST_FAILURE;
	XPm_PlDevice *PldChild;
	XPm_PlDevice *PldToUnlink;
	XPm_AieDevice *AieToUnlink;

	if (NULL == PlDevice) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	PldChild = PlDevice->Child;

	while (NULL != PldChild) {
		if (NULL != PldChild->Child) {
			Status = Pld_ReleaseChildren(PldChild);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		PldChild->WfPowerBitMask = PWR_DOMAIN_UNUSED_BITMASK;
		PldChild->Device.Node.State = (u8)XPM_DEVSTATE_INITIALIZING;
		Status = XPmDevice_Release(PM_SUBSYS_PMC, PldChild->Device.Node.Id,
					   XPLMI_CMD_SECURE);
		if(XST_SUCCESS != Status) {
			goto done;
		}

		PldToUnlink = PldChild;
		PldChild = PldChild->NextPeer;
		PldToUnlink->Parent = NULL;
		PldToUnlink->NextPeer = NULL;
	}

	PlDevice->Child = NULL;
	AieToUnlink = PlDevice->AieDevice;
	if (NULL != AieToUnlink) {
		AieToUnlink->Parent = NULL;
		PlDevice->AieDevice = NULL;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static const XPm_StateCap PlDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = (u32)XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_INITIALIZING,
		.Cap = (u32)PM_CAP_ACCESS,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = (u32)PM_CAP_ACCESS,
	},
};

static const XPm_StateTran PlDeviceTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_INITIALIZING,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_INITIALIZING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_INITIALIZING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_INITIALIZING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus Pld_HandlePowerEvent(u8 *BitMask, u32 PwrNodeId, u32 Action)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_Power *Power = NULL;

	if (NOT_INITIALIZED == Action) {
		Status = XST_SUCCESS;
		goto done;
	}

	Power = XPmPower_GetById(PwrNodeId);
	if (NULL == Power) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	Status = Power->HandleEvent(&Power->Node, Action);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_INVALID_EVENT;
		goto done;
	}

	if ((u32)XPM_POWER_EVENT_PWR_UP == Action) {
		Status = Pld_SetBitPwrBitMask(BitMask, PwrNodeId);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_SET_BIT;
		}
	} else if ((u32)XPM_POWER_EVENT_PWR_DOWN == Action) {
		Status = Pld_UnsetBitPwrBitMask(BitMask, PwrNodeId);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_UNSET_BIT;
		}
	} else {
		DbgErr = XPM_INT_ERR_INVALID_EVENT;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;

}

/****************************************************************************/
/**
 * @brief	Manage power domain dependency for PLD
 *
 * @param PlDevice	PlDevice nose whose power domain dependency needs to be
 *			managed
 *
 * @return	XStatus	Returns XST_SUCCESS or appropriate error code
 *
 * @note	Power Domain dependencies for PLD are dynamic in nature. Based
 *		on its changes in power domain dependency generate necessary
 *		events for power domain nodes
 *
 ****************************************************************************/
static XStatus Pld_ManagePower(XPm_PlDevice *PlDevice)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u8 PlPowerBitMask;
	u8 PlWfPowerBitMask;
	u8 BitMask;
	u32 PwrNodeId;
	u32 i;
	u32 PwrEvtAction = NOT_INITIALIZED;

	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	PlPowerBitMask = PlDevice->PowerBitMask;
	PlWfPowerBitMask = PlDevice->WfPowerBitMask;

	if (MAX_PWR_DOMAIN_BITMASK < PlWfPowerBitMask) {
		DbgErr = XPM_INT_ERR_PLDEVICE_INVALID_BITMASK;
		goto done;
	}

	for (i = 0; i < ARRAY_SIZE(PmPwrBitMap); ++i) {
		BitMask = PmPwrBitMap[i].BitMask;
		PwrNodeId = PmPwrBitMap[i].NodeId;

		if ((0U == (PlPowerBitMask & BitMask)) &&
		  (BitMask == (PlWfPowerBitMask & BitMask))) {
			PwrEvtAction = (u32)XPM_POWER_EVENT_PWR_UP;
		} else if ((BitMask == (PlPowerBitMask & BitMask)) &&
			   (0U == (PlWfPowerBitMask & BitMask))) {
			PwrEvtAction = (u32)XPM_POWER_EVENT_PWR_DOWN;
		} else {
			continue;
		}

		Status = Pld_HandlePowerEvent(&PlPowerBitMask, PwrNodeId, PwrEvtAction);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_PWR_MANAGE;
			goto done;
		}
	}

	if (PlPowerBitMask != PlDevice->PowerBitMask) {
		PlDevice->PowerBitMask  = PlPowerBitMask;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief	State machine for PL Device state management
 *
 * @param Device	Device structure whose states need to be managed
 * @param NextState	Desired next state for the Device in consideration
 *
 * @return	XStatus	Returns XST_SUCCESS or appropriate error code
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus HandlePlDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_PlDevice *PlDevice;
	const XPm_PlDevice *Parent = NULL;
	u8 CurrState;

	if (NULL == Device) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_DEV_PL != NODESUBCLASS(Device->Node.Id)) {
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
		goto done;
	}

	PlDevice = (XPm_PlDevice *)Device;
	Parent = PlDevice->Parent;

	/* Every PLD has a Parent except PLD0 */
	if ((NULL == Parent) && (PM_DEV_PLD_0 != PlDevice->Device.Node.Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PLDEVICE_PARENT;
		goto done;
	}

	CurrState = Device->Node.State;

	if ((u8)NextState == CurrState) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * Parent PLD state should be in running or initializing state before
	 * any activity on current PLD, except PLD0 as it does not have parent.
	 */
	if ((NULL != Parent) &&
	  ((u8)XPM_DEVSTATE_RUNNING != Parent->Device.Node.State) &&
	  ((u8)XPM_DEVSTATE_INITIALIZING != Parent->Device.Node.State)) {
		DbgErr = XPM_INT_ERR_INVALID_PLDEVICE_PARENT_STATE;
		goto done;
	}

	PmDbg ("ID=0x%x FromState=0x%x ToState=0x%x\n\r", PlDevice->Device.Node.Id,
		   CurrState, NextState);
	switch (CurrState) {
		case (u8)XPM_DEVSTATE_UNUSED:
			if ((u8)XPM_DEVSTATE_INITIALIZING == NextState) {
				DbgErr = XPM_INT_ERR_PLDEVICE_UNUSED_TO_INIT_EVT;
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_STATE_TRANS;
			}
			break;
		case (u8)XPM_DEVSTATE_RUNNING:
			if ((u8)XPM_DEVSTATE_INITIALIZING == NextState) {
				DbgErr = XPM_INT_ERR_PLDEVICE_RUNNING_TO_INIT_EVT;
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_STATE_TRANS;
			}
			break;
		case (u8)XPM_DEVSTATE_INITIALIZING:
			if ((u8)XPM_DEVSTATE_RUNNING == NextState) {
				DbgErr = XPM_INT_ERR_PLDEVICE_INIT_TO_RUNNING_EVT;
				Status = XST_SUCCESS;
			} else if ((u8)XPM_DEVSTATE_UNUSED == NextState) {
				DbgErr = XPM_INT_ERR_PLDEVICE_INIT_TO_UNUSED_EVT;
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_STATE_TRANS;
			}
			break;
		default:
			DbgErr = XPM_INT_ERR_INVALID_STATE;
			break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Pld_ManagePower(PlDevice);

	if (XST_SUCCESS == Status) {
		Device->Node.State = (u8)NextState;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const XPm_DeviceFsm XPmPlDeviceFsm = {
	DEFINE_DEV_STATES(PlDeviceStates),
	DEFINE_DEV_TRANS(PlDeviceTransitions),
	.EnterState = HandlePlDeviceState,
};

/****************************************************************************/
/**
 * @brief  Start Node initialization for PlDevice
 *
 * @param  Args: Arguments for PlDevice
 * @param  NumArgs: Number of arguments for PlDevice
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Arguments consist of Power Domain Node Ids that PlDevice depends on
 *
 ****************************************************************************/
static XStatus PlInitStart(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_FUNC_INIT_START;
	const XPm_PlDevice *Parent;
	u32 i;

	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	Parent = PlDevice->Parent;
	if ((NULL == Parent) && (PM_DEV_PLD_0 != PlDevice->Device.Node.Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PLDEVICE_PARENT;
		goto done;
	}

	/* RM for a PLD cannot run if parent is in unused or initializing state */
	if ((NULL != Parent) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Parent->Device.Node.State)) {
		DbgErr = XPM_INT_ERR_RUN_PARENT_IMAGE_FIRST;
		goto done;
	}

	PlDevice->WfPowerBitMask = PWR_DOMAIN_UNUSED_BITMASK;

	for (i = 0; i < NumArgs; ++i) {
		Status = Pld_SetBitPwrBitMask(&PlDevice->WfPowerBitMask, Args[i]);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_SET_BIT;
			goto done;
		}
	}

	/*
	 * Sufficient to only check for Unused state before Requesting PLD as:
	 * 1. PLD has been newly created by add_node (hence state is unused) and
	 *    needs to be requested
	 * 2. PLD has been powered down and released, therefore in unused state
	 */
	if ((u8)XPM_DEVSTATE_UNUSED == PlDevice->Device.Node.State) {
		Status = XPmDevice_Request(PM_SUBSYS_PMC, PlDevice->Device.Node.Id,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_REQUEST;
			goto done;
		}
	} else if ((u8)XPM_DEVSTATE_RUNNING == PlDevice->Device.Node.State) {
		Status = XPmDevice_ChangeState(&PlDevice->Device,
			   (u32)XPM_DEVSTATE_INITIALIZING);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_CHANGE_STATE;
			goto done;
		}
	} else {
		DbgErr = XPM_INT_ERR_INVALID_STATE;
		goto done;
	}

	/*
	 * PL Init Start indicates that a new RM or static image has been
	 * re/loaded. We must get rid of any existing child nodes if at all
	 * there are any, since new topology can be formed with those
	 */
	Status = Pld_ReleaseChildren(PlDevice);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLDEVICE_UNLINK_FAIL;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Finish Node initialization for PlDevice
 *
 * @param  Args: Arguments for PlDevice
 * @param  NumArgs: Number of arguments for PlDevice
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Arguments consist of Power Domain Node Ids that PlDevice depends on
 *
 ****************************************************************************/
static XStatus PlInitFinish(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
	const XPm_PlDevice *Parent;
	u32 i;

	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/*
	 * InitFinish should only be executed if device is currently in initial-
	 * izing state
	 */
	if ((u8)XPM_DEVSTATE_INITIALIZING != PlDevice->Device.Node.State) {
		DbgErr = XPM_INT_ERR_INVALID_STATE;
		goto done;
	}

	Parent = PlDevice->Parent;
	if ((NULL == Parent) && (PM_DEV_PLD_0 != PlDevice->Device.Node.Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PLDEVICE_PARENT;
		goto done;
	}

	/* RM for a PLD cannot run if parent is in unused or initializing state */
	if ((NULL != Parent) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Parent->Device.Node.State)) {
		DbgErr = XPM_INT_ERR_RUN_PARENT_IMAGE_FIRST;
		goto done;
	}

	PlDevice->WfPowerBitMask = PWR_DOMAIN_UNUSED_BITMASK;

	for (i = 0; i < NumArgs; ++i) {
		Status = Pld_SetBitPwrBitMask(&PlDevice->WfPowerBitMask, Args[i]);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_SET_BIT;
			goto done;
		}
	}

	if (PWR_DOMAIN_UNUSED_BITMASK == PlDevice->WfPowerBitMask) {
		Status = XPmDevice_Release(PM_SUBSYS_PMC, PlDevice->Device.Node.Id,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_RELEASE;
		}
	} else {
		Status = XPmDevice_ChangeState(&PlDevice->Device, (u32)XPM_DEVSTATE_RUNNING);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_CHANGE_STATE;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PldInitNodeOps PldOps = {
	.InitStart = PlInitStart,
	.InitFinish = PlInitFinish,
};

/****************************************************************************/
/**
 * @brief  Initialize PLDevice node base class
 *
 * @param  PlDevice: Pointer to an uninitialized PlDevice struct
 * @param  PldId: Node Id assigned to a PlDevice node
 * @param  BaseAddress: Baseaddress that is passed from topology
 * @param  Power: Power Node dependency. Will no longer be used when PlDevice
 *		   topology is activated
 * @param  Clock: Clocks that PlDevice is dependent on. No longer in use as
 *		   it's managed by CDO
 * @param  Reset: PlDevice reset dependency. No longer in use as it's managed
 *		   by CDO
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmPlDevice_Init(XPm_PlDevice *PlDevice,
		u32 PldId,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	PlDevice->Parent = NULL;
	PlDevice->NextPeer = NULL;
	PlDevice->Child = NULL;
	PlDevice->AieDevice = NULL;
	PlDevice->PowerBitMask = (u8)0x0U;
	PlDevice->WfPowerBitMask = (u8)0x0U;
	PlDevice->Ops = &PldOps;

	Status = XPmDevice_Init(&PlDevice->Device, PldId, BaseAddress, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLDEVICE_INIT;
		goto done;
	}

	PlDevice->Device.DeviceFsm = &XPmPlDeviceFsm;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Get PLD parent for an AIE/PLD Device
 *
 * @param  NodeId: Node Id assigned to a device node
 * @param  Resp: Pointer to the output data
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmPlDevice_GetParent(u32 NodeId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	const XPm_PlDevice *Parent = NULL;

	Device = XPmDevice_GetById(NodeId);
	if (NULL == Device) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(NodeId)) {
		Parent = ((XPm_AieDevice *)Device)->Parent;
	} else if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(NodeId)) {
		/* For PLD0 Node parent returned value will be 0U */
		Parent = ((XPm_PlDevice *)Device)->Parent;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
		goto done;
	}

	if (NULL != Parent) {
		*Resp = Parent->Device.Node.Id;
	} else {
		*Resp = 0U;
	}

	Status = XST_SUCCESS;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

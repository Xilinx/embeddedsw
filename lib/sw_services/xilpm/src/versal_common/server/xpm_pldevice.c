/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_pldevice.h"
#include "xpm_debug.h"
#include "xpm_defs.h"
#include "xpm_requirement.h"

#define PWR_DOMAIN_UNUSED_BITMASK		0U
#define PWR_DOMAIN_NOC_BITMASK			BIT(0)
#define PWR_DOMAIN_PL_BITMASK			BIT(1)
#ifdef VERSAL_NET
#define PWR_DOMAIN_HNIC_BITMASK			BIT(2)
#define MAX_PWR_DOMAIN_BITMASK			(PWR_DOMAIN_NOC_BITMASK |\
						  PWR_DOMAIN_PL_BITMASK |\
						  PWR_DOMAIN_HNIC_BITMASK)
#else
#define PWR_DOMAIN_ME_BITMASK			BIT(2)
#define PWR_DOMAIN_ME2_BITMASK			BIT(3)
#define MAX_PWR_DOMAIN_BITMASK			(PWR_DOMAIN_NOC_BITMASK |\
						  PWR_DOMAIN_PL_BITMASK |\
						  PWR_DOMAIN_ME_BITMASK |\
						  PWR_DOMAIN_ME2_BITMASK)
#endif
#define NOT_INITIALIZED			0xFFU

/* NoC clock gating definitions */
#define MAX_NOC_CLOCK_BITS			256U
#define PREVIOUS_ARRAY_MASK			0x0002U
#define CURRENT_ARRAY_MASK			0x0001U
static u32 NocClkPrevUnionBits[MAX_NOC_CLOCK_ARRAY_SIZE];
static u32 NocClkCurrUnionBits[MAX_NOC_CLOCK_ARRAY_SIZE];

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
#ifdef VERSAL_NET
	{
		.BitMask = PWR_DOMAIN_HNIC_BITMASK,
		.NodeId = PM_POWER_HNICX,
	},
#else
	{
		.BitMask = PWR_DOMAIN_ME_BITMASK,
		.NodeId = PM_POWER_ME,
	},
	{
		.BitMask = PWR_DOMAIN_ME2_BITMASK,
		.NodeId = PM_POWER_ME2,
	},
#endif
};

#ifdef XSEM_NPISCAN_EN
static void (*SemCbHandler)(u32 DeviceId);
static inline void XPmPlDevice_TriggerSemHandler(u32 DeviceId)
{
	if (NULL != SemCbHandler) {
		SemCbHandler(DeviceId);
	}
}

/****************************************************************************/
/**
 * @brief  Set XilSEM callback handler to clear the PLD node descriptors
 *
 * @param  Handler: Pointer to XilSEM callback handler
 *
 * @return None
 *
 * @note None
 *
 ****************************************************************************/
void XPmPlDevice_SetSemCallback(void (*Handler)(u32 DeviceId))
{
	if (NULL == SemCbHandler) {
		SemCbHandler = Handler;
	}
}
#else
static inline void XPmPlDevice_TriggerSemHandler(u32 DeviceId)
{
	(void)DeviceId;
}

void XPmPlDevice_SetSemCallback(void (*Handler)(u32 DeviceId))
{
	(void)Handler;
}
#endif /* XSEM_NPISCAN_EN */

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

static XStatus Pld_ReleaseMemCtrlr(XPm_PlDevice *PlDevice)
{
	XStatus Status = XST_FAILURE;
	u32 McCount = PlDevice->MemCtrlrCount;

	for (u8 i = 0; i < McCount; ++i) {
		if ((NULL == PlDevice->MemCtrlr[i]) ||
		    ((u8)XPM_DEVSTATE_RUNNING != PlDevice->MemCtrlr[i]->Device.Node.State)) {
			continue;
		}
		Status = XPmDevice_Release(PM_SUBSYS_PMC,
				PlDevice->MemCtrlr[i]->Device.Node.Id,
				XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_DEVICE_RELEASE;
			goto done;
		}
		/* Remove link between PLD <-> DDRMC */
		PlDevice->MemCtrlr[i]->PlDevice = NULL;
		PlDevice->MemCtrlr[i] = NULL;
		PlDevice->MemCtrlrCount--;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Check if a certain PLD Device is a valid device. Qualification
 *		for a device to be called valid is:
 *		- Should have a structure allocated
 *		- Should have a valid existing parent (to prevent broken trees,
 *		  as only 1 tree can exist). Exception is for PLD_0 because
 *		  it's the root of the tree
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

		/* Trigger Xilsem handler to release child PLD nodes */
		XPmPlDevice_TriggerSemHandler(PldChild->Device.Node.Id);

		/* Release any DDRMCs linked to this PLD */
		Status = Pld_ReleaseMemCtrlr(PldChild);
		if (XST_SUCCESS != Status) {
			goto done;
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
	XPmPlDevice_ReleaseAieDevice(PlDevice);

	/* Release any DDRMCs linked to this PLD */
	Status = Pld_ReleaseMemCtrlr(PlDevice);

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
		PlDevice->PowerBitMask	= PlPowerBitMask;
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
	.EnterState = &HandlePlDeviceState,
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

	/* Trigger Xilsem handler to clear previous descriptors */
	XPmPlDevice_TriggerSemHandler(PlDevice->Device.Node.Id);

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
		/* Release DDRMCs linked to this PLD */
		Status = Pld_ReleaseMemCtrlr(PlDevice);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_MEM_CTRLR_RELEASE;
			goto done;
		}
		/* Release PLD */
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

static XStatus XPm_AddDDRMemRegnForDefaultSubsystem(const XPm_MemCtrlrDevice *MCDev)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = PM_SUBSYS_DEFAULT;
	XPm_Subsystem *Subsystem;
	u32 DeviceId;
	u64 Address;
	u64 Size;

	DeviceId = MCDev->Device.Node.Id;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
		u32 MemRegnIndx = XPmDevice_GetMemRegnCount();
		DeviceId = MEMREGN_DEVID(MemRegnIndx);
		Address = MCDev->Region[Cnt].Address;
		Size = MCDev->Region[Cnt].Size;

		PmDbg("DeviceId: (0x%x) MemRegnDeviceId: (0x%x)\r\n", MCDev->Device.Node.Id, DeviceId);
		Status = XPm_AddMemRegnDevice(DeviceId, Address, Size);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPmRequirement_Add(Subsystem, XPmDevice_GetById(DeviceId),
			(u32)REQUIREMENT_FLAGS(1U,
			(u32)REQ_ACCESS_SECURE_NONSECURE,
			(u32)REQ_NO_RESTRICTION),
			(u32)PM_CAP_ACCESS, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;

}

/****************************************************************************/
/**
 * @brief  DDRMC mapping/annotation for PlDevice
 *
 * @param  Args: Arguments for PlDevice
 * @param  NumArgs: Number of arguments for PlDevice
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note This command is used to annotate the DDRMC device nodes defined by
 * topology with additional user specified information. A two way link is
 * established between a PLD and DDRMC device. Each DDRMC device links to
 * exactly one PLD, however each PLD can link to one or more DDRMC devices.
 *
 ****************************************************************************/
static XStatus PldMemCtrlrMap(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_MemCtrlrDevice *MCDev = NULL;
	u32 Offset = 2U;

	if ((6U != NumArgs) && (10U != NumArgs)) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	/*
	 * Current PLD or one of parents from the tree must be in an initializing state
	 *
	 * This is because DDR modeling information for a given PLD can be present in
	 * its own RM (partial) or a default RM present in one its parent RM or static image.
	 */
	if ((u8)XPM_DEVSTATE_INITIALIZING != PlDevice->Device.Node.State) {
		const XPm_PlDevice *Parent = PlDevice->Parent;
		while (NULL != Parent) {
			if ((u8)XPM_DEVSTATE_INITIALIZING == Parent->Device.Node.State) {
				Status = XST_SUCCESS;
				break;
			}
			Parent = Parent->Parent;
		}
		/*
		 * We can end up here in either of the two failure cases:
		 *  1. No parent in the hierarchy is initializing
		 *  2. PLD0 which is the root node is not initializing
		 */
		if (NULL == Parent) {
			Status = XPM_ERR_DEVICE_STATUS;
			DbgErr = XPM_INT_ERR_INVALID_STATE;
			goto done;
		}
	}

	/* Lookup DDRMC device based on provided address in args */
	for (u32 i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
	/*
	 * - This block expands the base address check to new DDRMC nodes,
	 *   skipping device nodes with type other than XPM_NODETYPE_DEV_DDR.
	 * - To add more DDRMC nodes, define new min and max macros for those
	 *   nodes and add a block similar to the one below.
	 */
#ifdef XPM_NODEIDX_DEV_DDRMC_MAX_INT_1
		if (XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 + 1 == i) {
			i = XPM_NODEIDX_DEV_DDRMC_MIN_INT_2;
		}
#endif
		MCDev = (XPm_MemCtrlrDevice *)XPmDevice_GetById(DDRMC_DEVID(i));
		if ((NULL != MCDev) &&
		    (Args[0U] == MCDev->Device.Node.BaseAddress)) {
			Status = XST_SUCCESS;
			break;
		}
	}
	if (XST_SUCCESS != Status) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_PLDEVICE_FUNC_MEM_CTRLR_MAP;
		goto done;
	}

	/* Link DDRMC to PLD */
	if (NULL == MCDev->PlDevice) {
		MCDev->PlDevice = PlDevice;
	} else {
		/* Error if previously linked to another PLD */
		Status = XST_DEVICE_BUSY;
		DbgErr = XPM_INT_ERR_PLDEVICE_FUNC_MEM_CTRLR_MAP;
		goto done;
	}

	/* Link PLD to DDRMC */
	if (PlDevice->MemCtrlrCount < ARRAY_SIZE(PlDevice->MemCtrlr)) {
		PlDevice->MemCtrlr[PlDevice->MemCtrlrCount] = MCDev;
		PlDevice->MemCtrlrCount++;
	} else {
		/* Fatal error if not enough space */
		Status = XST_BUFFER_TOO_SMALL;
		DbgErr = XPM_INT_ERR_PLDEVICE_FUNC_MEM_CTRLR_MAP;
		goto done;
	}

	/* DDRMC address region count */
	MCDev->RegionCount = (u8)(Args[1U] & 0xFFU);

	/* Ensure RegionCount does not exceed array bounds */
	if (MCDev->RegionCount > ARRAY_SIZE(MCDev->Region)) {
		Status = XST_FAILURE;
		DbgErr = XPM_INT_ERR_OUT_OF_RANGE;
		goto done;
	}

	/* DDRMC interleave size and index */
	MCDev->IntlvSize = (u8)((Args[1U] >> 8U) & 0xFFU);
	MCDev->IntlvIndex = (u8)((Args[1U] >> 16U) & 0xFU);

	/* Annotate DDRMC regions */
	for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
		MCDev->Region[Cnt].Address = ((u64)Args[Offset + 1U] << 32U) | (u64)(Args[Offset]);
		MCDev->Region[Cnt].Size = ((u64)Args[Offset + 3U] << 32U) | (u64)(Args[Offset + 2U]);
		Offset += 4U;
	}

	/* Request DDRMC for this PLD */
	Status = XPmDevice_Request(PM_SUBSYS_PMC, MCDev->Device.Node.Id,
			(u32)PM_CAP_ACCESS, XPM_MAX_QOS, XPLMI_CMD_SECURE);
	if (XST_SUCCESS != Status) {
		Status = XPM_ERR_DEVICE_REQ;
		DbgErr = XPM_INT_ERR_PLDEVICE_MEM_CTRLR_REQUEST;
		goto done;
	}
	if (1U == XPm_GetOverlayCdoFlag()) {
		Status = XPm_AddDDRMemRegnForDefaultSubsystem(MCDev);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PldInitNodeOps PldOps = {
	.InitStart = &PlInitStart,
	.InitFinish = &PlInitFinish,
	.MemCtrlrMap = &PldMemCtrlrMap,
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

	/* Initialize Noc Clock BitArray to 0 */
	for (u32 i = 0U; i < MAX_NOC_CLOCK_ARRAY_SIZE; i++) {
		PlDevice->NocClockEnablement[i] = 0U;
	}

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
		XPmPlDevice_GetAieParent(Device, &Parent);
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

/****************************************************************************/
/**
 * @brief  Save the NoC clock enablement for the given PLD partition and
 *	    update the global current and previous union bit arrays
 *
 * @param  PlDevice: Pointer to PLD partition
 * @param  Args: Argument buffer with BitArray words
 * @param  NumArgs: Number of BitArray words
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmPlDevice_NocClkEnable(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	const XPm_PlDevice *Device;
	u32 i;

	if (MAX_NOC_CLOCK_ARRAY_SIZE < NumArgs)	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Save local copy of bit array to PLD partition */
	for (i = 0U; i < NumArgs; i++) {
		PlDevice->NocClockEnablement[i] = Args[i];
	}

	/*
	 * Copy the current union bit array into the previous union bit array
	 * and clear the current union bit array. The current union bit array is
	 * cleared in case an enablement is switching from 1 to 0.
	 */
	for (i = 0U; i < MAX_NOC_CLOCK_ARRAY_SIZE; i++) {
		NocClkPrevUnionBits[i] = NocClkCurrUnionBits[i];
		NocClkCurrUnionBits[i] = 0U;
	}

	/*
	 * The current union bit array has been cleared to handle cases where
	 * a clock enablement bit has transitioned from 1 to 0. To create a new
	 * current union bit array, OR all partition Bit Arrays.
	 */
	for (i = 0U; i < (u32)XPM_NODEIDX_DEV_PLD_MAX; i++) {
		Device = (XPm_PlDevice *)XPmDevice_GetPlDeviceByIndex(i);
		/* Only update the current union for active PLD partitions */
		if ((NULL == Device) ||
			(((u8)XPM_DEVSTATE_INITIALIZING != Device->Device.Node.State) &&
			((u8)XPM_DEVSTATE_RUNNING != Device->Device.Node.State))) {
			continue;
		}

		/* OR all words for this partition with the current union bit array */
		for (u32 Idx = 0U; Idx < MAX_NOC_CLOCK_ARRAY_SIZE; Idx++) {
			NocClkCurrUnionBits[Idx] |= Device->NocClockEnablement[Idx];
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Check the current and previous NoC clock enablement specified by
 *	    BitArrayIdx argument. If the enablement matches the State argument
 *	    the processing will continue. Otherwise the processing will break
 *	    out of the block.
 *
 * @param  Cmd: Pointer to the CDO command
 * @param  BitArrayIdx: Index into the bit array which represents a specific
 *	    NoC clock switch
 * @param  State: The state which is checked against the previous and current
 *	    clock enablement
 * @param  Mask: Mask which indicates if the current or previous enablement
 *	    states will be checked
 * @param  Level: Level of block nesting to break out of
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmPlDevice_IfNocClkEnable(XPlmi_Cmd *Cmd, u32 BitArrayIdx, u16 State,
		u16 Mask, u32 Level)
{
	XStatus Status = XST_FAILURE;
	u8 PreviousState;
	u8 CurrentState;
	u32 PreviousWord, CurrentWord;
	u32 Bit, Index;

	if (MAX_NOC_CLOCK_BITS <= BitArrayIdx) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Get the previous and current states from State argument.
	 * Bit 0 of State is the current state
	 * Bit 1 of State is the previous state
	 */
	PreviousState = (u8)((State >> 1U) & 0x1U);
	CurrentState = (u8)(State & 0x1U);

	/*
	 * BitArrayIdx is used to calculate the Index of the word in the current
	 * and previous union arrays as well as the respective bit in that word.
	 * Using the calculated Index, retrieve the word from the current and
	 * previous union arrays.
	 */
	Index = BitArrayIdx / 32U;
	Bit = BitArrayIdx % 32U;
	PreviousWord = NocClkPrevUnionBits[Index];
	CurrentWord = NocClkCurrUnionBits[Index];

	/*
	 * This block is used to check if the clock enablement matches the state
	 * passed by the CDO command. If the states do not match then
	 * skip CDO processing to the end of the block.
	 */
	if (((PREVIOUS_ARRAY_MASK == (PREVIOUS_ARRAY_MASK & Mask)) &&
		(PreviousState != ((PreviousWord >> Bit) & 1U))) ||
		((CURRENT_ARRAY_MASK == (CURRENT_ARRAY_MASK & Mask)) &&
		(CurrentState != ((CurrentWord >> Bit) & 1U)))) {

		/*
		 * If the block condition passes this means one of two things:
		 * - There is no clock state transition -> from on(1) to off(0) or vice
		 *   versa
		 * - The clock is already enabled or disabled -> in cases where the
		 *   mask is included, it is checked if a clock is/was (curr/prev)
		 *   enabled/disabled (1/0). No transition is checked, i.e. there is no
		 *   check on both previous and current arrays.
		 * This indicates there is no requirement to enable or disable the
		 * clocks. Therefore, the CDO block should not continue. Call this
		 * function to break out of the block.
		 */
		Status = XPlmi_GetJumpOffSet(Cmd, Level);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

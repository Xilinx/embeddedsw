/******************************************************************************
 * Copyright (c) 2021 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpm_access.h"
#include "xpm_common.h"
#include "xpm_api.h"
#include "xpm_device.h"
#include "xpm_subsystem.h"
#include "xplmi.h"

static XPm_RegNode *PmRegnodes;
static XPm_NodeAccess *PmNodeAccessTable;

/* Match found in the "Node Access Table" */
typedef struct XPm_NodeAccessMatch {
	XPm_NodeAccess *Entry;
	XPm_NodeAper *Aper;
} XPm_NodeAccessMatch;

static XStatus XPmAccess_LookupEntry(u32 NodeId, u32 Offset,
				     XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccess *Entry = PmNodeAccessTable;

	/* Check for a matching entry with given node id */
	while (NULL != Entry) {
		if (Entry->Id == NodeId) {
			/* Matching entry found, Look for aperture containing given offset */
			XPm_NodeAper *Aper = Entry->Aperture;
			while (NULL != Aper) {
				if ((Offset >=  Aper->Offset) &&
				    (Offset < (Aper->Offset + (Aper->Size * 4U)))) {
					/* Found matching aperture */
					Match->Entry = Entry;
					Match->Aper = Aper;
					Status = XST_SUCCESS;
					break;
				}
				Aper = Aper->NextAper;
			}
			/* Stop the search at first node match */
			break;
		}
		Entry = Entry->NextNode;
	}

	return Status;
}

static XStatus XPmAccess_CheckRequirement(u32 SubsystemId, u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_RegNode *Regnode;
	u32 SubsysIdx = NODEINDEX(SubsystemId);

	/**
	 * NOTE:
	 *  No need for checking if requirement is allocated to the caller subsystem;
	 *  mere presence of a requirement on the given node from caller subsystem
	 *  is enough to pass the access criteria.
	 *
	 *  Therefore, simply check if the caller has requirement (aka permission)
	 *  to access the given node.
	 */
	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		/* Check power parent status */
		Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		/* Check if caller subsystem has a req on this node */
		if (NULL == XPmDevice_FindRequirement(DeviceId, SubsystemId)) {
			Status = XST_FAILURE;
			goto done;
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Regnode = PmRegnodes;
		while (NULL != Regnode) {
			if (DeviceId == Regnode->Id) {
				/* Check if caller subsystem has a req on this node */
				if (0U != (Regnode->Requirements & BIT32(SubsysIdx))) {
					Status = XST_SUCCESS;
				}
				break;
			}
			Regnode = Regnode->NextRegnode;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus XPmAccess_BaseHandler(pm_ioctl_id Op, XPm_NodeAccessTypes AccessType)
{
	XStatus Status = XPM_PM_NO_ACCESS;

	switch (AccessType) {
	case ACCESS_ANY_RO:
	case ACCESS_SEC_RO:
	case ACCESS_SEC_NS_SUBSYS_RO:
	case ACCESS_SEC_SUBSYS_RO:
		/* When access is set to RO, only RO operation is allowed */
		if (IOCTL_READ_REG == Op) {
			Status = XST_SUCCESS;
		}
		break;
	case ACCESS_ANY_RW:
	case ACCESS_SEC_RW:
	case ACCESS_SEC_NS_SUBSYS_RW:
	case ACCESS_SEC_SUBSYS_RW:
		/* When access is set to RW, either RO/WR operations are allowed */
		Status = XST_SUCCESS;
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

static XStatus XPmAccess_AnyHandler(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				    XPm_NodeAccessTypes AccessType,
				    const XPm_NodeAccessMatch *const Match)
{
	(void)SubsystemId;
	(void)CmdType;
	(void)Match;

	return XPmAccess_BaseHandler(Op, AccessType);
}

static XStatus XPmAccess_SecHandler(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				    XPm_NodeAccessTypes AccessType,
				    const XPm_NodeAccessMatch *const Match)
{
	(void)SubsystemId;
	(void)Match;

	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_BaseHandler(Op, AccessType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if incoming command is Secure */
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;

done:
	return Status;
}

static XStatus XPmAccess_NSecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					   u32 CmdType,
					   XPm_NodeAccessTypes AccessType,
					   const XPm_NodeAccessMatch *const Match)
{
	(void)CmdType;

	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_BaseHandler(Op, AccessType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if caller subsystem has access to this entry */
	Status = XPmAccess_CheckRequirement(SubsystemId, Match->Entry->Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

static XStatus XPmAccess_SecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					  u32 CmdType,
					  XPm_NodeAccessTypes AccessType,
					  const XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_NSecSubsysHandler(SubsystemId, Op, CmdType,
					     AccessType, Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if incoming command is Secure */
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;

done:
	return Status;
}

static XStatus XPmAccess_EnforcePolicy(u32 SubsystemId, pm_ioctl_id IoctlId,
				       u32 CmdType,
				       const XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XPM_PM_NO_ACCESS;
	XPm_NodeAccessTypes AccessType = (XPm_NodeAccessTypes) Match->Aper->Access;

	/* Handlers for access policy checking */
	static const struct XPm_NodeAccessHandler {
		XStatus (*Handler)(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				   XPm_NodeAccessTypes AccessType,
				   const XPm_NodeAccessMatch *const Match);
	} AccessPolicy[ACCESS_TYPE_MAX] = {
		[ACCESS_ANY_RO] = { .Handler = XPmAccess_AnyHandler },
		[ACCESS_ANY_RW] = { .Handler = XPmAccess_AnyHandler },
		[ACCESS_SEC_RO] = { .Handler = XPmAccess_SecHandler },
		[ACCESS_SEC_RW] = { .Handler = XPmAccess_SecHandler },
		[ACCESS_SEC_NS_SUBSYS_RO] = { .Handler = XPmAccess_NSecSubsysHandler },
		[ACCESS_SEC_NS_SUBSYS_RW] = { .Handler = XPmAccess_NSecSubsysHandler },
		[ACCESS_SEC_SUBSYS_RO] = { .Handler = XPmAccess_SecSubsysHandler },
		[ACCESS_SEC_SUBSYS_RW] = { .Handler = XPmAccess_SecSubsysHandler },
	};

	switch (AccessType) {
	case ACCESS_ANY_RO:
	case ACCESS_ANY_RW:
	case ACCESS_SEC_RO:
	case ACCESS_SEC_RW:
	case ACCESS_SEC_NS_SUBSYS_RO:
	case ACCESS_SEC_NS_SUBSYS_RW:
	case ACCESS_SEC_SUBSYS_RO:
	case ACCESS_SEC_SUBSYS_RW:
		if (NULL != AccessPolicy[AccessType].Handler) {
			Status = AccessPolicy[AccessType].Handler(SubsystemId,
					IoctlId, CmdType, AccessType, Match);
		} else {
			Status = XPM_PM_NO_ACCESS;
		}
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_NO_FEATURE;
		break;
	}

	return Status;
}

static XStatus XPmAccess_IsAllowed(u32 SubsystemId, u32 DeviceId,
				   pm_ioctl_id IoctlId,
				   u32 Offset, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccessMatch Match = { NULL, NULL };

	/**
	 * Check sanity of given offset:
	 *  - Max offset width must be NODE_APER_OFFSET_BIT_FIELD_SIZE
	 *  - Offset must be aligned on a word boundary
	 */
	if ((0U != (Offset & ~NODE_APER_OFFSET_MASK)) ||
	    (0U != (Offset & (sizeof(int) - 1U)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmAccess_LookupEntry(DeviceId, Offset, &Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	PmDbg("Matching Entry Found:\r\n");
	PmDbg("    Id: 0x%08x\r\n", Match.Entry->Id);
	PmDbg("    Offset: 0x%x, Size: 0x%x, Access 0x%x\r\n",
			Match.Aper->Offset,
			Match.Aper->Size,
			Match.Aper->Access);

	Status = XPmAccess_EnforcePolicy(SubsystemId, IoctlId, CmdType, &Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	PmDbg("Access policy successfully enforced\r\n");

done:
	return Status;
}


static XStatus XPmAccess_CheckParent(u32 DeviceId, u32 *BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Base = 0U;
	const XPm_Device *Device;
	const XPm_RegNode *Regnode;

	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		/* Check power parent status */
		Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		if ((u32)XPM_POWER_STATE_ON != Device->Power->Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Get device base */
		Status = XPm_GetDeviceBaseAddr(DeviceId, &Base);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Regnode = PmRegnodes;
		while (NULL != Regnode) {
			if (DeviceId == Regnode->Id) {
				/* Get base for a regnode */
				Base = Regnode->BaseAddress;
				/* Check parent power domain state */
				if ((u32)XPM_POWER_STATE_ON == Regnode->Power->Node.State) {
					Status = XST_SUCCESS;
				}
				break;
			}
			Regnode = Regnode->NextRegnode;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		Base = 0U;
		break;
	}

done:
	*BaseAddress = Base;
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL read action handler (Reads given offset - 1 32-bit word)
 *
 * @param  SubsystemId: Subsystem Id
 * @param  DeviceId: Device Id (Device, Regnode etc.)
 * @param  IoctlId: IOCTL Id (Rd/Wr etc.)
 * @param  Offset: Offset to write to in base address of the given node
 * @param  Count: Count (Must be 1 for now, may change in future)
 * @param  CmdType: Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_ReadReg(u32 SubsystemId, u32 DeviceId,
			  pm_ioctl_id IoctlId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u32 DataIn = 0U;
	(void)Count;
	(void)Response;

	Status = XPmAccess_IsAllowed(SubsystemId, DeviceId, IoctlId,
				     Offset, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmAccess_CheckParent(DeviceId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmDbg("RD @ (0x%x + 0x%x)\r\n", BaseAddress, Offset);

	PmIn32((BaseAddress + Offset), DataIn);
	*Response = DataIn;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL read action handler (Mask writes to given offset - 1 32-bit word)
 *
 * @param  SubsystemId: Subsystem Id
 * @param  DeviceId: Device Id (Device, Regnode etc.)
 * @param  IoctlId: IOCTL Id (Rd/Wr etc.)
 * @param  Offset: Offset to write to in base address of the given node
 * @param  Mask: Mask to be applied to Value
 * @param  Value: Data to be written to given offset
 * @param  CmdType: Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_MaskWriteReg(u32 SubsystemId, u32 DeviceId,
			       pm_ioctl_id IoctlId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	(void)Mask;
	(void)Value;

	Status = XPmAccess_IsAllowed(SubsystemId, DeviceId, IoctlId,
				     Offset, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmAccess_CheckParent(DeviceId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmDbg("RMW M:0x%x V:0x%x @ (0x%x + 0x%x)\r\n",
			Mask, Value, BaseAddress, Offset);

	PmRmw32((BaseAddress + Offset), Mask, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add a new node access entry to the "Node Access Table"
 *
 * @param  NodeEntry: Pointer to an uninitialized XPm_NodeAccess struct
 * @param  Args: command arguments
 * @param  NumArgs: Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_UpdateTable(XPm_NodeAccess *NodeEntry,
			      const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAper *NodeApertures = NULL;

	/* SET_NODE_ACCESS <NodeId: Arg0> <Arg 1,2> <Arg 3,4> ... */
	for (u32 i = 1U; i < NumArgs; i += 2U) {
		XPm_NodeAper *Aper = (XPm_NodeAper *)XPm_AllocBytes(sizeof(XPm_NodeAper));
		if (NULL == Aper) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		/* Setup aperture */
		Aper->Offset = NODE_APER_OFFSET(Args[i]);
		Aper->Size = (u8)NODE_APER_SIZE(Args[i]);
		Aper->Access = NODE_APER_ACCESS(Args[i + 1U]);

		/* Add new aperture entry to the list */
		Aper->NextAper = NodeApertures;
		NodeApertures = Aper;
	}
	NodeEntry->Aperture = NodeApertures;

	/* Add new node entry to the access table */
	NodeEntry->NextNode = PmNodeAccessTable;
	PmNodeAccessTable = NodeEntry;

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Print "Node Access Table" and "Regnodes" table to console
 *
 * @param  None
 *
 * @return None
 *
 * @note This is only meant for debugging; in a usual case, this will get
 * optimized during link time since there are no references to it
 *
 ****************************************************************************/
void XPmAccess_PrintTable(void)
{
	const XPm_NodeAccess *Table = PmNodeAccessTable;
	const XPm_NodeAper *Aper = NULL;
	const XPm_RegNode *Regnode = PmRegnodes;

	while (NULL != Regnode) {
		PmDbg("Id: 0x%08x, Base: 0x%08x, Req: 0x%08x, Power: 0x%08x\r\n",
				Regnode->Id, Regnode->BaseAddress,
				Regnode->Requirements,
				Regnode->Power->Node.Id);
		Regnode = Regnode->NextRegnode;
	}

	while (NULL != Table) {
		PmDbg("Id: 0x%08x\r\n", Table->Id);
		Aper = Table->Aperture;
		while (NULL != Aper) {
			PmDbg("    Offset: 0x%x, Size: 0x%x, Access 0x%x\r\n",
					Aper->Offset, Aper->Size, Aper->Access);
			Aper = Aper->NextAper;
		}
		Table = Table->NextNode;
	}
}

/****************************************************************************/
/**
 * @brief  Add requirements on a regnode from different subsystems
 *
 * @param  SubsystemId: Node Id assigned to a Subsystem
 * @param  RegnodeId: Node Id assigned to a Regnode
 *
 * @return XST_SUCCESS if successful, XST_DEVICE_NOT_FOUND otherwise
 *
 * @note   SubsystemId must be validated on caller side
 *
 ****************************************************************************/
XStatus XPmAccess_AddRegnodeRequirement(u32 SubsystemId, u32 RegnodeId)
{
	XStatus Status = XST_DEVICE_NOT_FOUND;
	u32 SubsysIdx = NODEINDEX(SubsystemId);
	XPm_RegNode *Regnode = PmRegnodes;

	while (NULL != Regnode) {
		if (RegnodeId == Regnode->Id) {
			Regnode->Requirements |= BIT32(SubsysIdx);
			Status = XST_SUCCESS;
			break;
		}
		Regnode = Regnode->NextRegnode;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize RegNode and add to database
 *
 * @param  RegNode: Pointer to an uninitialized XPm_RegNode struct
 * @param  NodeId: Node Id assigned to a Regnode
 * @param  BaseAddress: Baseaddress of given RegNode
 * @param  Power: Power parent dependency
 *
 * @return None
 *
 * @note None
 *
 ****************************************************************************/
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power)
{
	RegNode->Id = NodeId;
	RegNode->BaseAddress = BaseAddress;
	RegNode->Power = Power;
	RegNode->Requirements = 0U;

	/* Add to list of regnodes */
	RegNode->NextRegnode = PmRegnodes;
	PmRegnodes = RegNode;
}

/******************************************************************************
 * Copyright (c) 2021 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpm_access.h"
#include "xpm_common.h"

static XPm_RegNode *PmRegnodes;
static XPm_NodeAccess *PmNodeAccessTable;

XStatus XPmAccess_ReadReg(u32 SubsystemId, u32 DeviceId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType)
{
	(void)SubsystemId;
	(void)DeviceId;
	(void)Offset;
	(void)Count;
	(void)Response;
	(void)CmdType;

	/* TODO */
	return XST_SUCCESS;
}

XStatus XPmAccess_MaskWriteReg(u32 SubsystemId, u32 DeviceId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType)
{
	(void)SubsystemId;
	(void)DeviceId;
	(void)Offset;
	(void)Mask;
	(void)Value;
	(void)CmdType;

	/* TODO */
	return XST_SUCCESS;
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
	for (u32 i = 1; i < NumArgs; i += 2)
	{
		XPm_NodeAper *Aper = XPm_AllocBytes(sizeof(XPm_NodeAper));
		if (NULL == Aper) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		/* Setup aperture */
		Aper->Offset = NODE_APER_OFFSET(Args[i]);
		Aper->Size = NODE_APER_SIZE(Args[i]);
		Aper->Access = NODE_APER_ACCESS(Args[i + 1]);

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

	/* Add to list of regnodes */
	RegNode->NextRegnode = PmRegnodes;
	PmRegnodes = RegNode;
}

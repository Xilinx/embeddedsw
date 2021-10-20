/******************************************************************************
 * Copyright (c) 2021 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpm_access.h"

static XPm_RegNode *PmRegnodes;

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

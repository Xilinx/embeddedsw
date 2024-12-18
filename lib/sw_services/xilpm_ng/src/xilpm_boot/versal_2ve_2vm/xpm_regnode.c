/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_regnode.h"
static XPm_RegNode *PmRegnodes;

XPm_RegNode* XPmRegNode_GetNodes(void)
{
	return PmRegnodes;
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
void XPmRegNode_Init(XPm_RegNode *RegNode,
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

/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"

void XPmNode_Init(XPm_Node *Node, u32 Id, u8 State, u32 BaseAddress)
{
	PmDbg("Node Init: Type=%d, Id=%d, State=%d, BaseAddress=0x%08X\n\r",
		NODETYPE(Id), Id, State, BaseAddress);

	Node->Id = Id;
	Node->State = State;
	Node->BaseAddress = BaseAddress;
	Node->Flags = 0;
	Node->LatencyMarg = XPM_MAX_LATENCY;
}

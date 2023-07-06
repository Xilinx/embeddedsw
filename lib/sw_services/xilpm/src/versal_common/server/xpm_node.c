/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#ifdef VERSAL_NET
#define MAX_NUM_NODE 1000
static XPm_Node* AllNodes[MAX_NUM_NODE]= {NULL};
static u32 NumNodes = 0;
#endif
void XPmNode_Init(XPm_Node *Node, u32 Id, u8 State, u32 BaseAddress)
{
	PmDbg("Node Init: Type=%d, Id=%d, State=%d, BaseAddress=0x%08X\n\r",
		NODETYPE(Id), Id, State, BaseAddress);

	Node->Id = Id;
	Node->State = State;
	Node->BaseAddress = BaseAddress;
	Node->Flags = 0;
	Node->LatencyMarg = XPM_MAX_LATENCY;
#ifdef VERSAL_NET
	if (NumNodes >= MAX_NUM_NODE){
		PmErr("Error: Too many Node !\n\r");
	} else {
		AllNodes[NumNodes++] = Node;
	}
#endif
}
#ifdef VERSAL_NET

XPm_Node* XPmNode_GetNodeAt(u32 Index)
{
	if (Index > NumNodes){
		return NULL;
	}
	return AllNodes[Index];
}

inline u32 XPmNode_GetNumNodes()
{
	return NumNodes;
}
#endif
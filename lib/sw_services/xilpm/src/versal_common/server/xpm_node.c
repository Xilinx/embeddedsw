/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#ifdef VERSAL_NET
#ifndef VERSAL_2VE_2VM
#include "xpm_update.h"
#endif
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
#ifndef VERSAL_2VE_2VM
	XPmUpdate_AllNodes_Add(Node);
#endif
#endif
}

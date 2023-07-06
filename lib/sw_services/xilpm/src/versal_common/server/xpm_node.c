/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_node.h"
#ifdef VERSAL_NET
#define MAX_NUM_NODE 1000
static XPm_Node* AllNodes[MAX_NUM_NODE]= {NULL};
static u32 NumNodes = 0;

void XPmNode_Init(XPm_Node *Node, u32 Id, u8 State, u32 BaseAddress, SaveRestoreHandler_t Handler)
#else
void XPmNode_Init(XPm_Node *Node, u32 Id, u8 State, u32 BaseAddress)
#endif
{
	PmDbg("Node Init: Type=%d, Id=%d, State=%d, BaseAddress=0x%08X\n\r",
		NODETYPE(Id), Id, State, BaseAddress);

	Node->Id = Id;
	Node->State = State;
	Node->BaseAddress = BaseAddress;
	Node->Flags = 0;
	Node->LatencyMarg = XPM_MAX_LATENCY;
#ifdef VERSAL_NET
	Node->SaveRestoreHandler = Handler;
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

/**
 * @brief This function save node's dynamic data during PLM update.
 *
 * @param NodeData Node data to be saved.
 * @param SavedData location of data that is saved
 * @return XST_SUCCESS if node data is successfully saved; error code otherwise
 */
XStatus XPmNode_SaveNode(XPm_Node* NodeData, u32** SavedData)
{
	XStatus Status = XST_FAILURE;
	/* Allocate 2 words for ID and Info */
	*SavedData = (u32*)XPmUpdate_DynAllocBytes(2*sizeof(u32));
	if (NULL == *SavedData)
	{
		Status = XPM_UPDATE_BUFFER_TOO_SMALL;
		goto done;
	}

	SAVED_DATA_SET_ID((*SavedData), NodeData->Id);
	u32* StartDataAddr =(u32*)XPmUpdate_GetDynFreeBytes();
	SaveStruct(Status, done, ((NodeData->LatencyMarg)<<16) | ((NodeData->State)<<8) | (NodeData->Flags));
	/* Calculate and set size */
	SAVED_DATA_SET_SIZE((*SavedData),((((u32)XPmUpdate_GetDynFreeBytes()) - (u32)StartDataAddr)>>2));

	Status = XST_SUCCESS;
done:
	if (XST_SUCCESS != Status) {
		PmErr("Error:[%s %d] NodeId=0x%x Status = 0x%x\n\r", __func__, __LINE__, NodeData->Id, Status);
		*SavedData = NULL;
	}
	return Status;
}

XStatus XPmNode_RestoreNode(u32* SavedData, XPm_Node* NodeData, u32** NextSavedData)
{
	XStatus Status = XST_FAILURE;

	if (NULL == SavedData)
	{
		Status = XPM_UPDATE_SAVED_DATA_NULL;
		goto done;
	}

	if (NULL == NodeData)
	{
		Status = XPM_UPDATE_NODE_DATA_NULL;
		goto done;
	}

	if (NodeData->Id != SAVED_DATA_ID(SavedData))
	{
		Status = XPM_UPDATE_ID_MISMATCH;
		PmErr("Error: ID mismatch SaveDataId = 0x%x NodeId = 0x%x \n\r", SAVED_DATA_ID(SavedData), NodeData->Id);
		goto done;
	}

	*NextSavedData = &(SavedData[XPM_SAVED_DATA_RETORE_OFFSET]);
	u32 tmp = 0U;
	RestoreStruct((*NextSavedData), tmp);
	NodeData->LatencyMarg = (((u16)(tmp >> 16)) & 0xFFFFU);
	NodeData->State = (tmp >>8) & 0xFFU;
	NodeData->Flags = (tmp) & 0xFFU;

	Status = XST_SUCCESS;
done:
	if (XST_SUCCESS != Status) {
		PmErr("Error:[%s %d] Status = 0x%x\n\r", __func__, __LINE__, Status);
		*NextSavedData = NULL;
	}
	return Status;
}
#endif
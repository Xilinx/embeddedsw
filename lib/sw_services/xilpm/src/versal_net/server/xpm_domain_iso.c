/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_domain_iso.h"
#include "xpm_debug.h"

static XPm_Iso* XPmDomainIso_List[XPM_NODEIDX_ISO_MAX];

/****************************************************************************/
/**
 * @brief	Isolation Node initializing function
 * @param NodeId	Node unique identification number
 * @param Mask		Mask of bits field that control isolation
 * @param IsPsmLocal	A non-zero value indicate if register control isolation
 * 			is belong to PSM local registers
 * @param Polarity	Isolation control polarity
 * @param Dependencies	List of power domain node id that this isolation control
 * 			depends on.
 * @param NumDependencies	Number of dependencies.
 *
 * @return	XST_SUCCESS or error code.
 *
 * @note	None
 *
 ****************************************************************************/

XStatus XPmDomainIso_NodeInit(u32 NodeId, u32 BaseAddress, u32 Mask, \
	u8 IsPsmLocal, u8 Polarity, const u32* Dependencies, u32 NumDependencies)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIdx = NODEINDEX(NodeId);
	if (NodeIdx >= (u32)XPM_NODEIDX_ISO_MAX) {
		goto done;
	}

	XPm_Iso *IsoNode = (XPm_Iso*)XPm_AllocBytes(sizeof(XPm_Iso));
	if (IsoNode == NULL) {
		goto done;
	}

	XPmNode_Init(&IsoNode->Node, NodeIdx, (u8)PM_ISOLATION_ON, BaseAddress);
	IsoNode->IsPsmLocal = IsPsmLocal;
	IsoNode->Mask = Mask;
	IsoNode->Polarity = Polarity;

	for (u32 i =0; i < NumDependencies; i++) {
		IsoNode->Dependencies[i] = Dependencies[i];
	}

	XPmDomainIso_List[NodeIdx] = IsoNode;
	Status = XST_SUCCESS;

done:
	return Status;
}

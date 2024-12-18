/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_common.h"
#include "xplmi.h"
#include "xpm_alloc.h"


static XPm_ResetNode *RstNodeList[(u32)XPM_NODEIDX_RST_MAX];
static const u32 MaxRstNodes = (u32)XPM_NODEIDX_RST_MAX;
static u32 PmNumResets;

u32 UserAssertPsSrst = 0U;


static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Rst) && ((u32)XPM_NODEIDX_RST_MAX > NodeIndex)) {
		RstNodeList[NodeIndex] = Rst;
		PmNumResets++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static void XPmReset_Init(XPm_ResetNode *Rst, u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, const u32* Parents)
{
	u32 i = 0;
	(void)ResetType;
	XPmNode_Init(&Rst->Node, Id, (u8)XPM_RST_STATE_ASSERTED, 0);

	Rst->Node.BaseAddress = ControlReg;
	Rst->Shift = Shift;
	Rst->Width = Width;

	for (i=0; i<NumParents; i++) {
		Rst->Parents[i] = (u16)(NODEINDEX(Parents[i]));
	}
}

XStatus XPmReset_AddNode(u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, const u32* Parents)
{
	XStatus Status = XST_FAILURE;
	u32 SubClass = NODESUBCLASS(Id);
	XPm_ResetNode *Rst = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((NULL != XPmReset_GetById(Id)) ||
	    (NumParents > MAX_RESET_PARENTS)) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (INVALID_RST_SUBCLASS(SubClass)) {
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rst = (XPm_ResetNode *)XPm_AllocBytes(sizeof(XPm_ResetNode));
	if (Rst == NULL) {
		DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	XPmReset_Init(Rst, Id, ControlReg, Shift, Width, ResetType, NumParents, Parents);

	Status = SetResetNode(Id, Rst);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SET_RESET_NODE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XPm_ResetNode* XPmReset_GetById(u32 ResetId)
{
	u32 ResetIndex = NODEINDEX(ResetId);
	XPm_ResetNode *Rst = NULL;

	if ((NODECLASS(ResetId) != (u32)XPM_NODECLASS_RESET) ||
	    (ResetIndex >= MaxRstNodes)) {
		goto done;;
	}

	Rst = RstNodeList[ResetIndex];

	/* Check that Reset Node's ID is same as given ID or not. */
	if ((NULL != Rst) && (ResetId != Rst->Node.Id)) {
		Rst = NULL;
	}

done:
	return Rst;
}

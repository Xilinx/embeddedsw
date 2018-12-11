/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpm_reset.h"
#include "xpm_domain_iso.h"

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action);
static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action);
static u32 Reset_GetStatusCommon(XPm_ResetNode *Rst);

XPm_ResetNode *RstNodeList[XPM_NODEIDX_RST_MAX];
u32 MaxRstNodes=XPM_NODEIDX_RST_MAX;
u32 PmNumResets=0;

static XPm_ResetOps ResetOps[] = {
	[XPM_RSTOPS_GENRERIC] = {
			.SetState = Reset_AssertCommon,
			.GetState = Reset_GetStatusCommon,
	},
	[XPM_RSTOPS_CUSTOM] = {
			.SetState = Reset_AssertCustom,
			.GetState = Reset_GetStatusCommon,
	},
};

static XStatus XPmReset_Init(XPm_ResetNode *Rst, u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, u32* Parents)
{
	u32 Status = XST_SUCCESS, i = 0;

	Status = XPmNode_Init(&Rst->Node, Id, (u32)XPM_RST_STATE_ASSERTED, 0);

	Rst->Node.BaseAddress = ControlReg;
	Rst->Shift = Shift;
	Rst->Width = Width;
	Rst->Ops = &ResetOps[ResetType];

	for (i=0; i<NumParents; i++) {
		Rst->Parents[i] = Parents[i];
	}
	
	return Status;
}
XStatus XPmReset_AddNode(u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, u32* Parents)
{
	int Status = XST_SUCCESS;
	u32 SubClass = NODESUBCLASS(Id);
	u32 ResetIndex = NODEINDEX(Id);
	XPm_ResetNode *Rst = NULL;

	if (RstNodeList[ResetIndex] != 0 || ResetIndex > MaxRstNodes || NumParents > MAX_RESET_PARENTS) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (SubClass != XPM_NODETYPE_RESET_PERIPHERAL && SubClass != XPM_NODETYPE_RESET_POR && SubClass != XPM_NODETYPE_RESET_DBG && SubClass != XPM_NODETYPE_RESET_SRST) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rst = XPm_AllocBytes(sizeof(XPm_ResetNode));
	if (Rst == NULL) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmReset_Init(Rst, Id, ControlReg, Shift, Width, ResetType, NumParents, Parents);

	RstNodeList[ResetIndex] = Rst;
	PmNumResets++;

done:
	return Status;
}

XPm_ResetNode* XPmReset_GetById(u32 ResetId)
{
	u32 ResetIndex = NODEINDEX(ResetId);;
	u32 SubClass = NODESUBCLASS(ResetId);

	if (NODECLASS(ResetId) != XPM_NODECLASS_RESET) {
		return NULL;
	}
	if (SubClass != XPM_NODETYPE_RESET_PERIPHERAL && SubClass != XPM_NODETYPE_RESET_POR && SubClass != XPM_NODETYPE_RESET_DBG && SubClass != XPM_NODETYPE_RESET_SRST) {
		return NULL;
	}

	if (ResetIndex >= MaxRstNodes) {
		return NULL;
	}

	return RstNodeList[ResetIndex];
}

static XStatus ResetPulsePsOnly(XPm_ResetNode *Rst)
{
	u32 Status = XST_SUCCESS;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Block LPD-PL interfaces */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-NoC interfaces */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_SOC);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-PMC interfaces */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_LPD);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_LPD_DFX);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-PL interfaces */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_PL);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_PL_TEST);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-NoC interfaces */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_SOC);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Assert PS System Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);

	/* TODO: Wait for some time */

	/* Release PS System Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0);

done:
	return Status;
}

static XStatus ResetPulseLpd(XPm_ResetNode *Rst)
{
	u32 Status = XST_SUCCESS;
	//u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* This parameter is required as per the prototype */
	(void)(Rst);

	/* TODO: TBD */

	return Status;
}

XStatus (*const Reset_Pulse[])(XPm_ResetNode *Rst) = {
	[XPM_NODEIDX_RST_PS_SRST - XPM_NODEIDX_RST_MIN] = &ResetPulsePsOnly,
	[XPM_NODEIDX_RST_LPD - XPM_NODEIDX_RST_MIN] = &ResetPulseLpd,
};

static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action)
{
	u32 Status = XST_SUCCESS;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;

	switch (Action) {
	case PM_RESET_ACTION_RELEASE:
		XPm_RMW32(ControlReg, Mask, 0);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		break;
	case PM_RESET_ACTION_ASSERT:
		XPm_RMW32(ControlReg, Mask, Mask);
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		break;
	case PM_RESET_ACTION_PULSE:
		Reset_Pulse[NODEINDEX(Rst->Node.Id)](Rst);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	};

	return Status;
}

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action)
{
	u32 Status = XST_SUCCESS;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;

	switch (Action) {
	case PM_RESET_ACTION_RELEASE:
		XPm_RMW32(ControlReg, Mask, 0);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		break;
	case PM_RESET_ACTION_ASSERT:
		XPm_RMW32(ControlReg, Mask, Mask);
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		break;
	case PM_RESET_ACTION_PULSE:
		XPm_RMW32(ControlReg, Mask, Mask);
		//Wait for xms ??
		XPm_RMW32(ControlReg, Mask, 0);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	};

	return Status;
}

XStatus XPmReset_AssertbyId(u32 ResetId, const u32 Action)
{
	XStatus Status;
	XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (!Rst) {
		Status = Rst->Ops->SetState(Rst, Action);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

static u32 Reset_GetStatusCommon(XPm_ResetNode *Rst)
{
	u32 ResetStatus = 0U;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	if ((XPm_Read32(Rst->Node.BaseAddress) & Mask) == Mask) {
		ResetStatus = 1U;
	}

	return ResetStatus;
}

/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#include "xpm_reset.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action);
static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action);
static u32 Reset_GetStatusCommon(XPm_ResetNode *Rst);
static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst);

static XPm_ResetNode *RstNodeList[XPM_NODEIDX_RST_MAX];
static const u32 MaxRstNodes = XPM_NODEIDX_RST_MAX;
static u32 PmNumResets;

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

static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst)
{
	u32 Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Rst) && (XPM_NODEIDX_RST_MAX > NodeIndex)) {
		RstNodeList[NodeIndex] = Rst;
		PmNumResets++;
		Status = XST_SUCCESS;
	}

	return Status;
}

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
	XPm_ResetNode *Rst = NULL;

	if (NULL != XPmReset_GetById(Id) || NumParents > MAX_RESET_PARENTS) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	switch (SubClass) {
	case XPM_NODETYPE_RESET_PERIPHERAL:
	case XPM_NODETYPE_RESET_POR:
	case XPM_NODETYPE_RESET_DBG:
	case XPM_NODETYPE_RESET_SRST:
		break;

	default:
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rst = XPm_AllocBytes(sizeof(XPm_ResetNode));
	if (Rst == NULL) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmReset_Init(Rst, Id, ControlReg, Shift, Width, ResetType, NumParents, Parents);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetResetNode(Id, Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

XPm_ResetNode* XPmReset_GetById(u32 ResetId)
{
	u32 ResetIndex = NODEINDEX(ResetId);;
	XPm_ResetNode *Rst = NULL;

	if ((NODECLASS(ResetId) != XPM_NODECLASS_RESET) ||
	    (ResetIndex >= MaxRstNodes)) {
		return NULL;
	}

	Rst = RstNodeList[ResetIndex];

	/* Check that Reset Node's ID is same as given ID or not. */
	if ((NULL != Rst) && (ResetId != Rst->Node.Id)) {
		Rst = NULL;
	}

	return Rst;
}

static XStatus PsOnlyResetAssert(XPm_ResetNode *Rst)
{
	u32 i, Status = XST_SUCCESS;
	const u32 PsDomainIds[] = { LPD_NODEID, FPD_NODEID };
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Block LPD-PL interfaces */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-NoC interfaces */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_SOC, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-PMC interfaces */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD_DFX, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-PL interfaces */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-NoC interfaces */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_FPD_SOC, TRUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/*
	 * TODO: Use XPm_ForcePowerdown() API here to force LPD and FPD
	 * power and device nodes to power down, this will handle the
	 * use count and release of device requirements, gracefully.
	 *
	 * At present, Debugging is in progress for the issue where
	 * JTAG link is lost between XSDB and target device if this API is used.
	 *
	 * Therefore, following workaround is needed until debugging is done.
	 */
	for (i = 0; i < ARRAY_SIZE(PsDomainIds); i++) {
		XPm_Power *Power = NULL, *Child = NULL;

		Power = XPmPower_GetById(PsDomainIds[i]);
		if (NULL == Power) {
			Status = XST_FAILURE;
			goto done;
		}

		Child = ((XPm_PowerDomain *)Power)->Children;
		while (Child != NULL) {
			Child->Node.State = XPM_POWER_STATE_OFF;
			Child = Child->NextPeer;
		}
		Power->Node.State = XPM_POWER_STATE_OFF;
	}

	/* Assert PS System Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);

done:
	return Status;
}

static XStatus PsOnlyResetRelease(XPm_ResetNode *Rst)
{
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Release PS System Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0);

	return XST_SUCCESS;
}

static XStatus PsOnlyResetPulse(XPm_ResetNode *Rst)
{
	u32 Status = XST_SUCCESS;

	/* Assert PS System Reset */
	Status = PsOnlyResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release PS System Reset */
	Status = PsOnlyResetRelease(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

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

struct ResetCustomOps {
	XStatus (*const ActionAssert)(XPm_ResetNode *Rst);
	XStatus (*const ActionRelease)(XPm_ResetNode *Rst);
	XStatus (*const ActionPulse)(XPm_ResetNode *Rst);
};

static const struct ResetCustomOps Reset_Custom[] = {
	[XPM_NODEIDX_RST_PS_SRST - XPM_NODEIDX_RST_MIN] = {
		.ActionAssert = &PsOnlyResetAssert,
		.ActionRelease = &PsOnlyResetRelease,
		.ActionPulse = &PsOnlyResetPulse,
	},
	[XPM_NODEIDX_RST_LPD - XPM_NODEIDX_RST_MIN] = {
		.ActionPulse = &ResetPulseLpd,
	},
};

static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action)
{
	u32 Status = XST_FAILURE;
	u32 Id = NODEINDEX(Rst->Node.Id);
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;

	switch (Action) {
	case PM_RESET_ACTION_RELEASE:
		if (Reset_Custom[Id].ActionRelease) {
			Status = Reset_Custom[Id].ActionRelease(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, 0);
		}
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	case PM_RESET_ACTION_ASSERT:
		if (Reset_Custom[Id].ActionAssert) {
			Status = Reset_Custom[Id].ActionAssert(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, Mask);
		}
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		Status = XST_SUCCESS;
		break;
	case PM_RESET_ACTION_PULSE:
		if (Reset_Custom[Id].ActionPulse) {
			Status = Reset_Custom[Id].ActionPulse(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	};

done:
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

	if (Rst) {
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

int XPmReset_CheckPermissions(XPm_Subsystem *Subsystem, u32 ResetId)
{
	int Status = XST_FAILURE;
	u32 DevId;
	XPm_ResetHandle *DevHandle;
	XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (NULL == Rst) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DevHandle = Rst->RstHandles;
	while (NULL != DevHandle) {
		DevId = DevHandle->Device->Node.Id;
		if (XPM_DEVSTATE_RUNNING == DevHandle->Device->Node.State) {
			Status = XPmDevice_CheckPermissions(Subsystem, DevId);
			if (XST_SUCCESS == Status) {
				goto done;
			}
		}
		DevHandle = DevHandle->NextDevice;
	}

done:
	return Status;
}

int XPmReset_SystemReset()
{
	int Status = XST_SUCCESS;

	/* TODO: Confirm if idling is required here or not */

	Status = XPmReset_AssertbyId(SRST_RSTID(XPM_NODEIDX_RST_PMC),
				     PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

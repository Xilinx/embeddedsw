/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All rights reserved.
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

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action);
static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action);
static u32 Reset_GetStatusCommon(const XPm_ResetNode *Rst);
static u32 Reset_GetStatusCustom(const XPm_ResetNode *Rst);
static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst);

static const u32 PermissionResets[] = {
	PM_RST_PMC_POR,
	PM_RST_PMC,
	PM_RST_SYS_RST_1,
	PM_RST_SYS_RST_2,
	PM_RST_SYS_RST_3,
	PM_RST_PL_POR,
	PM_RST_NOC_POR,
	PM_RST_PL_SRST,
	PM_RST_NOC,
	PM_RST_NPI,
	PM_RST_PL0,
	PM_RST_PL1,
	PM_RST_PL2,
	PM_RST_PL3,
	PM_RST_ADMA,
};

static XPm_ResetNode *RstNodeList[(u32)XPM_NODEIDX_RST_MAX];
static const u32 MaxRstNodes = (u32)XPM_NODEIDX_RST_MAX;
static u32 PmNumResets;

u32 UserAssertPsSrst = 0U;

static XPm_ResetOps ResetOps[XPM_RSTOPS_MAX] = {
	[XPM_RSTOPS_GENRERIC] = {
			.SetState = &Reset_AssertCommon,
			.GetState = &Reset_GetStatusCommon,
	},
	[XPM_RSTOPS_CUSTOM] = {
			.SetState = &Reset_AssertCustom,
			.GetState = &Reset_GetStatusCustom,
	},
};

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

	XPmNode_Init(&Rst->Node, Id, (u8)XPM_RST_STATE_ASSERTED, 0);

	Rst->Node.BaseAddress = ControlReg;
	Rst->Shift = Shift;
	Rst->Width = Width;
	Rst->Ops = &ResetOps[ResetType];

	for (i=0; i<NumParents; i++) {
		Rst->Parents[i] = (u16)(NODEINDEX(Parents[i]));
	}
}

void XPmReset_MakeCpmPorResetCustom(void)
{
	XPm_ResetNode *CpmRst = XPmReset_GetById(PM_RST_CPM_POR);

	if (NULL != CpmRst) {
		CpmRst->Ops = &ResetOps[XPM_RSTOPS_CUSTOM];
	}
}

void XPmReset_MakeAdmaResetCustom(void)
{
	XPm_ResetNode *AdmaRst = XPmReset_GetById(PM_RST_ADMA);

	if (NULL != AdmaRst) {
		AdmaRst->Ops = &ResetOps[XPM_RSTOPS_CUSTOM];
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

	Rst = XPm_AllocBytes(sizeof(XPm_ResetNode));
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

static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;
	const struct ResetCustomOps *Ops = GetResetCustomOps(Rst->Node.Id);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	switch (Action) {
	case (u32)PM_RESET_ACTION_RELEASE:
		if ((NULL != Ops) && (NULL != Ops->ActionRelease)) {
			Status = Ops->ActionRelease(Rst);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_CUSTOM_RESET_RELEASE;
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, 0);
		}
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_ASSERT:
		if ((NULL != Ops) && (NULL != Ops->ActionAssert)) {
			Status = Ops->ActionAssert(Rst);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_CUSTOM_RESET_ASSERT;
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, Mask);
		}
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_PULSE:
		if ((NULL != Ops) && (NULL != Ops->ActionPulse)) {
			Status = Ops->ActionPulse(Rst);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_CUSTOM_RESET_PULSE;
				goto done;
			}
			Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		}
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		break;
	};

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;

	switch (Action) {
	case (u32)PM_RESET_ACTION_RELEASE:
		XPm_RMW32(ControlReg, Mask, 0);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_ASSERT:
		XPm_RMW32(ControlReg, Mask, Mask);
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_PULSE:
		XPm_RMW32(ControlReg, Mask, Mask);
		//Wait for xms ??
		XPm_RMW32(ControlReg, Mask, 0);
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	};

	return Status;
}

XStatus XPmReset_AssertbyId(u32 ResetId, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (NULL != Rst) {
		Status = Rst->Ops->SetState(Rst, Action);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

static u32 Reset_GetStatusCustom(const XPm_ResetNode *Rst)
{
	const struct ResetCustomOps *Ops = GetResetCustomOps(Rst->Node.Id);
	u32 ResetStatus = 0U;

	if ((NULL != Ops) && (NULL != Ops->GetStatus)) {
		ResetStatus = Ops->GetStatus();
	} else {
		ResetStatus = Reset_GetStatusCommon(Rst);
	}

	return ResetStatus;
}

static u32 Reset_GetStatusCommon(const XPm_ResetNode *Rst)
{
	u32 ResetStatus = 0U;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	if ((XPm_Read32(Rst->Node.BaseAddress) & Mask) == Mask) {
		ResetStatus = 1U;
	}

	return ResetStatus;
}

/*
 * These resets are specifically allowed and none else for permissions
 * policy as these are the most common cases for users.
 *
 * The list of nodes may have elements removed or added in the future.
 */
XStatus XPmReset_IsPermissionReset(const u32 ResetId)
{
	u32 Index;
	XStatus Status = XPM_PM_NO_ACCESS;

	for (Index = 0; Index < (ARRAY_SIZE(PermissionResets)); Index++) {
		if (ResetId == PermissionResets[Index]) {
			Status = XST_SUCCESS;
			goto done;
		}
	}

done:
	return Status;
}

/*
 * In the case of default subsystem with a pre-defined set of requirements,
 * add entire list of resets that are allowed to have permissions to the
 * default subsystem.
 */
XStatus XPmReset_AddPermForGlobalResets(const XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	u32 Index, Flags;
	XPm_ResetNode *Rst = NULL;
	Flags = (1UL << RESET_PERM_SHIFT_NS) | (1UL << RESET_PERM_SHIFT_S);

	for (Index = 0U; Index < (ARRAY_SIZE(PermissionResets)); Index++) {
		Rst = XPmReset_GetById( PermissionResets[Index]  );
		if (NULL != Rst) {
			Status = XPmReset_AddPermission(Rst, Subsystem, Flags);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

done:
	return Status;
}

XStatus XPmReset_IsOperationAllowed(const u32 SubsystemId,
				    const XPm_ResetNode *Rst,
				    const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Have Target check if Host can enact the operation */
	if ((XPLMI_CMD_SECURE == CmdType) &&
	    (0U != (Rst->AllowedSubsystems & ((u32)1U << SUBSYS_TO_S_BITPOS(SubsystemId))))) {
		Status = XST_SUCCESS;
	} else if (0U != (Rst->AllowedSubsystems & ((u32)1U << SUBSYS_TO_NS_BITPOS(SubsystemId)))) {
		Status = XST_SUCCESS;
	} else {
		Status = XPM_PM_NO_ACCESS;
	}

done:
	return Status;
}

XStatus XPmReset_AddPermission(XPm_ResetNode *Rst,
			       const XPm_Subsystem *Subsystem,
			       const u32 Operations)
{
	XStatus Status = XST_FAILURE;

	/* PMC and default subsystem can always enact operations */
	if ((NULL == Subsystem) || (PM_SUBSYS_PMC == Subsystem->Id)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmReset_IsPermissionReset(Rst->Node.Id);
	if (XST_SUCCESS != Status) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rst->AllowedSubsystems |=  PERM_BITMASK(Operations, RESET_PERM_SHIFT_NS,
						SUBSYS_TO_NS_BITPOS(Subsystem->Id));
	Rst->AllowedSubsystems |=  PERM_BITMASK(Operations, RESET_PERM_SHIFT_S,
						SUBSYS_TO_S_BITPOS(Subsystem->Id));

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmReset_CheckPermissions(const XPm_Subsystem *Subsystem, u32 ResetId)
{
	XStatus Status = XST_FAILURE;
	u32 DevId;
	const XPm_ResetHandle *DevHandle;
	const XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (NULL == Rst) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DevHandle = Rst->RstHandles;
	while (NULL != DevHandle) {
		DevId = DevHandle->Device->Node.Id;
		if ((u32)XPM_DEVSTATE_RUNNING == DevHandle->Device->Node.State) {
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

XStatus XPmReset_SystemReset(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Confirm if idling is required here or not */

	Status = XPmReset_PlatSystemReset();
	if (XST_SUCCESS != Status) {
		PmWarn("Error %d in XPmReset_PlatSystemReset()\r\n", Status);
	}

	Status = XPmReset_AssertbyId(PM_RST_PMC, (u32)PM_RESET_ACTION_ASSERT);

	return Status;
}

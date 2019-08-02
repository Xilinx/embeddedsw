/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_reset.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_common.h"

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action);
static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action);
static u32 Reset_GetStatusCommon(XPm_ResetNode *Rst);
static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst);

static XPm_ResetNode *RstNodeList[(u32)XPM_NODEIDX_RST_MAX];
static const u32 MaxRstNodes = (u32)XPM_NODEIDX_RST_MAX;
static u32 PmNumResets;

static XPm_ResetOps ResetOps[XPM_RSTOPS_MAX] = {
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

static void XPmReset_Init(XPm_ResetNode *Rst, u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, u32* Parents)
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

XStatus XPmReset_AddNode(u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, u32* Parents)
{
	XStatus Status = XST_FAILURE;
	u32 SubClass = NODESUBCLASS(Id);
	XPm_ResetNode *Rst = NULL;

	if (NULL != XPmReset_GetById(Id) || NumParents > MAX_RESET_PARENTS) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	switch (SubClass) {
	case (u32)XPM_NODETYPE_RESET_PERIPHERAL:
	case (u32)XPM_NODETYPE_RESET_POR:
	case (u32)XPM_NODETYPE_RESET_DBG:
	case (u32)XPM_NODETYPE_RESET_SRST:
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Rst = XPm_AllocBytes(sizeof(XPm_ResetNode));
	if (Rst == NULL) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	XPmReset_Init(Rst, Id, ControlReg, Shift, Width, ResetType, NumParents, Parents);

	Status = SetResetNode(Id, Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

XPm_ResetNode* XPmReset_GetById(u32 ResetId)
{
	u32 ResetIndex = NODEINDEX(ResetId);
	XPm_ResetNode *Rst = NULL;

	if ((NODECLASS(ResetId) != (u32)XPM_NODECLASS_RESET) ||
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
	XStatus Status = XST_FAILURE;
	u32 i;
	const u32 PsDomainIds[] = { PM_POWER_LPD, PM_POWER_FPD };
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/*
	 * Prevent LPD access
	 */
	XPlmi_ResetLpdInitialized();

	/* Block LPD-PL interfaces */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL, TRUE_PENDING_REMOVE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_PL_TEST, TRUE_PENDING_REMOVE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-NoC interfaces */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_SOC, TRUE_VALUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block LPD-PMC interfaces */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD, TRUE_VALUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD_DFX, TRUE_VALUE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-PL interfaces */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL, TRUE_PENDING_REMOVE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_PL_TEST, TRUE_PENDING_REMOVE);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	/* Block FPD-NoC interfaces */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_FPD_SOC, TRUE_VALUE);
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
			Child->Node.State = (u8)XPM_POWER_STATE_OFF;
			Child = Child->NextPeer;
		}
		Power->Node.State = (u8)XPM_POWER_STATE_OFF;
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
	XStatus Status = XST_FAILURE;

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
	XStatus Status = XST_FAILURE;
	//u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* This parameter is required as per the prototype */
	(void)(Rst);

	/* TODO: TBD */

	Status = XST_SUCCESS;

	return Status;
}

static XStatus AieResetAssert(XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	XPm_Device *AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Unlock the AIE PCSR register to allow register writes */
	XPmAieDomain_UnlockPcsr(AieDev->Node.BaseAddress);

	/* Set array or shim reset bit in mask register */
	XPm_RMW32((AieDev->Node.BaseAddress) + NPI_PCSR_MASK_OFFSET, Mask, Mask);

	/* Write to control register to assert reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);

	/* Re-lock the AIE PCSR registers for protection */
	XPmAieDomain_LockPcsr(AieDev->Node.BaseAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieResetRelease(XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	XPm_Device *AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Unlock the AIE PCSR register to allow register writes */
	XPmAieDomain_UnlockPcsr(AieDev->Node.BaseAddress);

	/* Set array or shim reset bit in mask register */
	XPm_RMW32((AieDev->Node.BaseAddress) + NPI_PCSR_MASK_OFFSET, Mask, Mask);

	/* Write to control register to release reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0U);

	/* Re-lock the AIE PCSR registers for protection */
	XPmAieDomain_LockPcsr(AieDev->Node.BaseAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieResetPulse(XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	/* Assert AIE reset */
	Status = AieResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release AIE reset */
	Status = AieResetRelease(Rst);

done:
	return Status;
}

static const struct ResetCustomOps {
	u32 ResetIdx;
	XStatus (*const ActionAssert)(XPm_ResetNode *Rst);
	XStatus (*const ActionRelease)(XPm_ResetNode *Rst);
	XStatus (*const ActionPulse)(XPm_ResetNode *Rst);
} Reset_Custom[] = {
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_PS_SRST,
		.ActionAssert = &PsOnlyResetAssert,
		.ActionRelease = &PsOnlyResetRelease,
		.ActionPulse = &PsOnlyResetPulse,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_LPD,
		.ActionPulse = &ResetPulseLpd,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_AIE_ARRAY,
		.ActionAssert = &AieResetAssert,
		.ActionRelease = &AieResetRelease,
		.ActionPulse = &AieResetPulse,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_AIE_SHIM,
		.ActionAssert = &AieResetAssert,
		.ActionRelease = &AieResetRelease,
		.ActionPulse = &AieResetPulse,
	},
};

static const struct ResetCustomOps *GetResetCustomOps(u32 ResetId)
{
	u16 i;

	for (i = 0; i < ARRAY_SIZE(Reset_Custom); i++) {
		if (Reset_Custom[i].ResetIdx == NODEINDEX(ResetId)) {
			return &Reset_Custom[i];
		}
	}
	return NULL;
}

static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	u32 ControlReg = Rst->Node.BaseAddress;
	const struct ResetCustomOps *Ops = GetResetCustomOps(Rst->Node.Id);

	switch (Action) {
	case (u32)PM_RESET_ACTION_RELEASE:
		if ((NULL != Ops) && (NULL != Ops->ActionRelease)) {
			Status = Ops->ActionRelease(Rst);
			if (XST_SUCCESS != Status) {
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

int XPmReset_SystemReset(void)
{
	int Status = XST_FAILURE;

	/* TODO: Confirm if idling is required here or not */

	/*
	 * For, ES1, When NPI_REF clock is used a source for SYSMON, SRST hangs
	 * at ROM stage (EDT-994792). So, switch to IRO CLK as source of
	 * SYSMON_REF_CLK before issuing SRST.
	 *
	 * There is no need to set original parent of SYSMON_REF_CLK explicitly,
	 * as after SRST, CDO will set it to default value.
	 */
	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		u16 i;
		XPm_OutClockNode *Clk;
		Clk = (XPm_OutClockNode *)XPmClock_GetById(PM_CLK_SYSMON_REF);
		if (NULL == Clk) {
			PmWarn("SYSMON_REF_CLK not found\r\n");
			goto assert_reset;
		}

		/* Find parent index of IRO_DIV2 for SYSMON_REF_CLK */
		for (i = 0; i < Clk->ClkNode.NumParents; i++) {
			if (NODEINDEX(PM_CLK_MUXED_IRO_DIV2)
			    == Clk->Topology.MuxSources[i]) {
				break;
			}
		}

		if (i == Clk->ClkNode.NumParents) {
			PmWarn("IRO_DIV2 not found as source of SYSMON_REF_CLK\r\n");
			goto assert_reset;
		}

		/* Disable clock before changing parent */
		(void)XPmClock_SetGate(Clk, 0);
		Status = XPmClock_SetParent(Clk, i);
		if (XST_SUCCESS == Status) {
			PmWarn("Failed to change parent of SYSMON_REF_CLK\r\n");
		}
		(void)XPmClock_SetGate(Clk, 1);

	}
assert_reset:
	Status = XPmReset_AssertbyId(PM_RST_PMC, (u32)PM_RESET_ACTION_ASSERT);

	return Status;
}

/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_cpmdomain.h"
#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_common.h"
#include "xplmi.h"

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action);
static XStatus Reset_AssertCustom(XPm_ResetNode *Rst, const u32 Action);
static u32 Reset_GetStatusCommon(const XPm_ResetNode *Rst);
static u32 Reset_GetStatusCustom(const XPm_ResetNode *Rst);
static XStatus SetResetNode(u32 Id, XPm_ResetNode *Rst);

static XPm_ResetNode *RstNodeList[(u32)XPM_NODEIDX_RST_MAX];
static const u32 MaxRstNodes = (u32)XPM_NODEIDX_RST_MAX;
static u32 PmNumResets;

u32 UserAssertPsSrst = 0U;

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
};

static XPm_ResetOps ResetOps[XPM_RSTOPS_MAX] = {
	[XPM_RSTOPS_GENRERIC] = {
			.SetState = Reset_AssertCommon,
			.GetState = Reset_GetStatusCommon,
	},
	[XPM_RSTOPS_CUSTOM] = {
			.SetState = Reset_AssertCustom,
			.GetState = Reset_GetStatusCustom,
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

	switch (SubClass) {
	case (u32)XPM_NODETYPE_RESET_PERIPHERAL:
	case (u32)XPM_NODETYPE_RESET_POR:
	case (u32)XPM_NODETYPE_RESET_DBG:
	case (u32)XPM_NODETYPE_RESET_SRST:
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
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

static XStatus PsOnlyResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const XPm_Subsystem *DefaultSubsys = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);

	const XPm_Power *LpdPower = XPmPower_GetById(PM_POWER_LPD);
	if (NULL == LpdPower) {
		Status = XST_FAILURE;
		goto done;
	}

	if (LpdPower->UseCount > 1U) {
		/**
		 * LPD UseCount more then 1 indicates that PS only reset is
		 * not called from LPD power down routine. So power down the
		 * LPD gracefully which in turn performs PS only reset.
		 */
		if (PM_RST_PS_SRST == Rst->Node.Id) {
			/* Set UserAssertPsSrst flag to skip PS-POR and LPD rail handling for PS-SRST */
			UserAssertPsSrst = 1U;
		}
		Status = XPm_ForcePowerdown(PM_SUBSYS_PMC, PM_POWER_LPD, 0U,
					    XPLMI_CMD_SECURE, 0U);
		UserAssertPsSrst = 0U;
		if (Status != XST_SUCCESS) {
			PmErr("Error %d in Powerdown of LPD %d\r\n", Status);
			goto done;
		}
		/**
		 * Change default subsystem state to POWERED_OFF as all
		 * processors are powered off after force power down LPD.
		 */
		if ((NULL != DefaultSubsys) &&
		    ((u8)ONLINE == DefaultSubsys->State)) {
			Status = XPmSubsystem_SetState(PM_SUBSYS_DEFAULT,
						       (u32)POWERED_OFF);
		}
	} else {
		/**
		 * LPD UseCount value 1 or less indicates that PS only reset
		 * is called from LPD power down routine which means that all
		 * isolation dependency is taken care in FPD and LPD power down
		 * itself and no need to take care in this routine.
		 */
		/* Assert PS System Reset */
		XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

static XStatus PsOnlyResetRelease(const XPm_ResetNode *Rst)
{
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);

	/* Release PS System Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0);

	return XST_SUCCESS;
}

static XStatus PsOnlyResetPulse(const XPm_ResetNode *Rst)
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

static XStatus PlPorResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		goto done;
	}

	/* Disable PUDC_B pin to allow PL_POR to toggle */
	XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
			PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
			PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

	/* Assert PL POR Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus PlPorResetRelease(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		goto done;
	}

	/* Release PL POR Reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0);

	/* Reset to allow PUDC_B pin to function */
	XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
			PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
			~PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus PlPorResetPulse(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	/* Assert PL POR Reset */
	Status = PlPorResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release PL POR Reset */
	Status = PlPorResetRelease(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}


static XStatus ResetPulseLpd(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	/* This parameter is required as per the prototype */
	(void)(Rst);

	/* TODO: TBD */

	Status = XST_SUCCESS;

	return Status;
}

static XStatus AieResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	const XPm_Device *AieDev = XPmDevice_GetById(PM_DEV_AIE);
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

	/* Wait for reset to propagate (1us) */
	usleep(1U);

	/* Re-lock the AIE PCSR registers for protection */
	XPmAieDomain_LockPcsr(AieDev->Node.BaseAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieResetRelease(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;

	const XPm_Device *AieDev = XPmDevice_GetById(PM_DEV_AIE);
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

	/* Wait for reset to propagate (1us) */
	usleep(1U);

	/* Re-lock the AIE PCSR registers for protection */
	XPmAieDomain_LockPcsr(AieDev->Node.BaseAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieResetPulse(const XPm_ResetNode *Rst)
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

static XStatus CpmResetSetState(const u32 State)
{
	XStatus Status = XST_FAILURE;
	const XPm_CpmDomain *Cpm;
	u32 CpmPcsrReg;
	u32 Platform =  XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u32 IdCode = XPm_GetIdCode();

	/* CPM POR register is not available for ES1 platforms so skip */
	/* NOTE: This is verified on XCVC1902 */
	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 != PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
		if (NULL == Cpm) {
			Status = XST_FAILURE;
			goto done;
		}

		XPmCpmDomain_UnlockPcsr(Cpm->CpmPcsrBaseAddr);

		/* TODO: Remove this when topology have CPM reset register */
		CpmPcsrReg = Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET;
		if (XPM_RST_STATE_DEASSERTED == State) {
			PmRmw32(CpmPcsrReg, CPM_POR_MASK, 0U);
		} else {
			PmRmw32(CpmPcsrReg, CPM_POR_MASK, CPM_POR_MASK);
		}

		XPmCpmDomain_LockPcsr(Cpm->CpmPcsrBaseAddr);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus CpmResetAssert(const XPm_ResetNode *Rst)
{
	(void)Rst;

	return CpmResetSetState(XPM_RST_STATE_ASSERTED);
}

static XStatus CpmResetRelease(const XPm_ResetNode *Rst)
{
	(void)Rst;

	return CpmResetSetState(XPM_RST_STATE_DEASSERTED);
}

static XStatus CpmResetPulse(const XPm_ResetNode *Rst)
{
	(void)Rst;
	XStatus Status = XST_FAILURE;

	Status = CpmResetSetState(XPM_RST_STATE_ASSERTED);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = CpmResetSetState(XPM_RST_STATE_DEASSERTED);

done:
	return Status;
}

static const struct ResetCustomOps {
	u32 ResetIdx;
	XStatus (*const ActionAssert)(const XPm_ResetNode *Rst);
	XStatus (*const ActionRelease)(const XPm_ResetNode *Rst);
	XStatus (*const ActionPulse)(const XPm_ResetNode *Rst);
} Reset_Custom[] = {
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_PS_SRST,
		.ActionAssert = &PsOnlyResetAssert,
		.ActionRelease = &PsOnlyResetRelease,
		.ActionPulse = &PsOnlyResetPulse,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_PS_POR,
		.ActionAssert = &PsOnlyResetAssert,
		.ActionRelease = &PsOnlyResetRelease,
		.ActionPulse = &PsOnlyResetPulse,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_PL_POR,
		.ActionAssert = &PlPorResetAssert,
		.ActionRelease = &PlPorResetRelease,
		.ActionPulse = &PlPorResetPulse,
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
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_CPM_POR,
		.ActionAssert = &CpmResetAssert,
		.ActionRelease = &CpmResetRelease,
		.ActionPulse = &CpmResetPulse,
	},
};

static const struct ResetCustomOps *GetResetCustomOps(u32 ResetId)
{
	u16 i;
	const struct ResetCustomOps *RstCustomStatus = NULL;

	for (i = 0; i < ARRAY_SIZE(Reset_Custom); i++) {
		if (Reset_Custom[i].ResetIdx == NODEINDEX(ResetId)) {
			RstCustomStatus = &Reset_Custom[i];
			break;
		}
	}
	return RstCustomStatus;
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

static u32 GetCpmPorResetStatus(void)
{
	u32 ResetStatus = XPM_RST_STATE_DEASSERTED;
	const XPm_CpmDomain *Cpm;
	u32 PcrValue;
	u32 Platform =  XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u32 IdCode = XPm_GetIdCode();

	/* CPM POR register is not available for ES1 platforms so skip */
	/* NOTE: This is verified on XCVC1902 */
	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 != PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
		if (NULL == Cpm) {
			PmErr("CPM Domain is not present\r\n");
			goto done;
		}

		XPmCpmDomain_UnlockPcsr(Cpm->CpmPcsrBaseAddr);

		/* TODO: Remove this when topology have CPM reset register */
		PmIn32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET, PcrValue);
		if (0U == (PcrValue & CPM_POR_MASK)) {
			ResetStatus = XPM_RST_STATE_DEASSERTED;
		} else {
			ResetStatus = XPM_RST_STATE_ASSERTED;
		}

		XPmCpmDomain_LockPcsr(Cpm->CpmPcsrBaseAddr);
	}

done:
	return ResetStatus;
}

static u32 Reset_GetStatusCustom(const XPm_ResetNode *Rst)
{
	u32 ResetStatus = 0U;

	if (PM_RST_CPM_POR == Rst->Node.Id) {
		ResetStatus = GetCpmPorResetStatus();
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
	Flags = (1U << RESET_PERM_SHIFT_NS) | (1U << RESET_PERM_SHIFT_S);

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
	u32 PlatformVersion = 0x0U;

	/* TODO: Confirm if idling is required here or not */

	/*
	 * For, ES1, When NPI_REF clock is used a source for SYSMON, SRST hangs
	 * at ROM stage (EDT-994792). So, switch to IRO CLK as source of
	 * SYSMON_REF_CLK before issuing SRST.
	 *
	 * There is no need to set original parent of SYSMON_REF_CLK explicitly,
	 * as after SRST, CDO will set it to default value.
	 */
	PlatformVersion = XPm_GetPlatformVersion();
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    ((u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
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
		if (XST_SUCCESS != Status) {
			PmWarn("Failed to change parent of SYSMON_REF_CLK\r\n");
		}
		(void)XPmClock_SetGate(Clk, 1);

	}
assert_reset:
	Status = XPmReset_AssertbyId(PM_RST_PMC, (u32)PM_RESET_ACTION_ASSERT);

	return Status;
}

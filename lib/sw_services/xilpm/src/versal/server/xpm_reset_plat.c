/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_cpmdomain.h"
#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_common.h"
#include "xplmi.h"

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
		Status = XPmPower_ForcePwrDwn(PM_POWER_LPD);
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
	XPm_UnlockPcsr(AieDev->Node.BaseAddress);

	/* Set array or shim reset bit in mask register */
	XPm_RMW32((AieDev->Node.BaseAddress) + NPI_PCSR_MASK_OFFSET, Mask, Mask);

	/* Write to control register to assert reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, Mask);

	/* Wait for reset to propagate (1us) */
	usleep(1U);

	/* Re-lock the AIE PCSR registers for protection */
	XPm_LockPcsr(AieDev->Node.BaseAddress);

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
	XPm_UnlockPcsr(AieDev->Node.BaseAddress);

	/* Set array or shim reset bit in mask register */
	XPm_RMW32((AieDev->Node.BaseAddress) + NPI_PCSR_MASK_OFFSET, Mask, Mask);

	/* Write to control register to release reset */
	XPm_RMW32(Rst->Node.BaseAddress, Mask, 0U);

	/* Wait for reset to propagate (1us) */
	usleep(1U);

	/* Re-lock the AIE PCSR registers for protection */
	XPm_LockPcsr(AieDev->Node.BaseAddress);

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
	u32 Platform =	XPm_GetPlatform();
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

		XPm_UnlockPcsr(Cpm->CpmPcsrBaseAddr);

		/* TODO: Remove this when topology have CPM reset register */
		CpmPcsrReg = Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET;
		if (XPM_RST_STATE_DEASSERTED == State) {
			PmRmw32(CpmPcsrReg, CPM_POR_MASK, 0U);
		} else {
			PmRmw32(CpmPcsrReg, CPM_POR_MASK, CPM_POR_MASK);
		}

		XPm_LockPcsr(Cpm->CpmPcsrBaseAddr);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AdmaResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;

	Device = XPmDevice_GetById(PM_DEV_ADMA_0);
	if (NULL == Device) {
		goto done;
	}

#if (defined(XILPM_ZDMA_0) || defined(XILPM_ZDMA_1) || defined(XILPM_ZDMA_2) || \
	defined(XILPM_ZDMA_3) || defined(XILPM_ZDMA_4) || defined(XILPM_ZDMA_5) || \
	defined(XILPM_ZDMA_6) || defined(XILPM_ZDMA_7))
	(void)Rst;
	Status = NodeZdmaIdle(0U, Device->Node.BaseAddress);
#else
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;

	XPm_RMW32(ControlReg, Mask, Mask);

	Status = XST_SUCCESS;
#endif

done:
	return Status;
}

static XStatus AdmaResetPulse(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;

	Status = AdmaResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPm_RMW32(ControlReg, Mask, 0U);

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

static u32 GetCpmPorResetStatus(void)
{
	u32 ResetStatus = XPM_RST_STATE_DEASSERTED;
	const XPm_CpmDomain *Cpm;
	u32 PcrValue;
	u32 Platform =	XPm_GetPlatform();
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

		XPm_UnlockPcsr(Cpm->CpmPcsrBaseAddr);

		/* TODO: Remove this when topology have CPM reset register */
		PmIn32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET, PcrValue);
		if (0U == (PcrValue & CPM_POR_MASK)) {
			ResetStatus = XPM_RST_STATE_DEASSERTED;
		} else {
			ResetStatus = XPM_RST_STATE_ASSERTED;
		}

		XPm_LockPcsr(Cpm->CpmPcsrBaseAddr);
	}

done:
	return ResetStatus;
}

static const struct ResetCustomOps Reset_Custom[] = {
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
		.GetStatus = &GetCpmPorResetStatus,
	},
	{
		.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA,
		.ActionAssert = &AdmaResetAssert,
		.ActionPulse = &AdmaResetPulse,
	},
};

const void *GetResetCustomOps(u32 ResetId)
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

XStatus XPmReset_PlatSystemReset(void)
{
	XStatus Status = XST_FAILURE;
	u32 PlatformVersion = 0x0U;

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
			goto done;
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
			goto done;
		}

		/* Disable clock before changing parent */
		(void)XPmClock_SetGate(Clk, 0);
		Status = XPmClock_SetParent(Clk, i);
		if (XST_SUCCESS != Status) {
			PmWarn("Failed to change parent of SYSMON_REF_CLK\r\n");
		}
		(void)XPmClock_SetGate(Clk, 1);

	}
	if (1U == XPm_In32(XPLMI_RTCFG_RST_PL_POR_WA)){
		(void)XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_ASSERT);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_cpmdomain.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_power.h"
#include "sleep.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_pldomain.h"
#include "xpm_psm_api.h"
#include "xpm_rail.h"
#include "xpm_api.h"

#define XPM_HC_CPM_OPS			0U
#define XPM_HC_CPM5_OPS			1U
#define XPM_CPM_OPS_MAX			2U

/* Define CPM5_GTYP device */
#define XPM_NODEIDX_DEV_GTYP_CPM5_MIN		XPM_NODEIDX_DEV_GTYP_CPM5_0
#define XPM_NODEIDX_DEV_GTYP_CPM5_MAX		XPM_NODEIDX_DEV_GTYP_CPM5_3

static u32 GtyAddresses[XPM_NODEIDX_DEV_GTYP_CPM5_MAX -
			XPM_NODEIDX_DEV_GTYP_CPM5_MIN + 1] = {0};

static XStatus CpmInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XStatus PslpRailPwrSts = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	const XPm_CpmDomain *Cpm = (XPm_CpmDomain *)PwrDomain;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 PlatformVersion;
	u32 DisableMask;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccintPslpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSLP);
	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	/* Check LPD and PL power rails first to make sure power is on */
	PslpRailPwrSts = XPmPower_CheckPower(VccintPslpRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK);
	IntRailPwrSts = XPmPower_CheckPower(VccintRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	AuxRailPwrSts =  XPmPower_CheckPower(VccauxRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	if ((XST_SUCCESS != PslpRailPwrSts) || (XST_SUCCESS != IntRailPwrSts) ||
	    (XST_SUCCESS != AuxRailPwrSts)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
		PmErr("PslpRailPwrSts: %d IntRailPwrSts: %d AuxRailPwrSts: %d\r\n",
		       PslpRailPwrSts, IntRailPwrSts, AuxRailPwrSts);
		goto done;
	}

	/* Remove isolation to allow scan_clear on CPM */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM_DFX, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM_DFX_ISO;
		goto done;
	}

	PlatformVersion = XPm_GetPlatformVersion();
	/* CPM POR control is not valid for ES1 platforms so skip. It is taken care by hw */
	if(!((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	     ((u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)))
	{
		/* Remove POR for CPM */
		/*Status = XPmReset_AssertbyId(PM_RST_CPM_POR,
				     (u32)PM_RESET_ACTION_RELEASE);*/

		/*TODO: Topology is not passing cpm reset register
		right now, so hard coded for now */
		XPmCpmDomain_UnlockPcsr(Cpm->CpmPcsrBaseAddr);
		PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_ECO_OFFSET, 0);
		XPmCpmDomain_LockPcsr(Cpm->CpmPcsrBaseAddr);
	}

	/* Get houseclean disable mask */
	DisableMask = XPm_In32(PM_HOUSECLEAN_DISABLE_REG_1) >> HOUSECLEAN_CPM_SHIFT;

	/* Set Houseclean Mask */
	PwrDomain->HcDisableMask |= DisableMask;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5InitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumofArgs)
{
	XStatus Status = XPM_ERR_INIT_START;
	XStatus PslpRailPwrSts = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;

	u32 i;
	const XPm_Device* Device = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DisableMask;

	/* This function does not use any args */
	(void)Args;
	(void)NumofArgs;

	const XPm_Rail *VccintPslpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSLP);
	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);
	const XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_RAM);

	/* Check LPD and PL power rails first to make sure power is on */
	PslpRailPwrSts = XPmPower_CheckPower(VccintPslpRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK);
	IntRailPwrSts = XPmPower_CheckPower(VccintRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	AuxRailPwrSts =  XPmPower_CheckPower(VccauxRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	RamRailPwrSts =  XPmPower_CheckPower(VccRamRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);

	if ((XST_SUCCESS != PslpRailPwrSts) || (XST_SUCCESS != IntRailPwrSts) ||
	    (XST_SUCCESS != AuxRailPwrSts) || (XST_SUCCESS != RamRailPwrSts)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_STATE;
		PmErr("PslpRailPwrSts: %d IntRailPwrSts: %d AuxRailPwrSts: %d RamRailPwrSts: %d\r\n",
		       PslpRailPwrSts, IntRailPwrSts, AuxRailPwrSts, RamRailPwrSts);
		goto done;
	}

	/* Remove isolation between CPM5 and LPD */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM5_DFX_ISO;
		goto done;
	}

	/* Remove POR for CPM5 */
	/* lpd_cpm5_por_n reset maps to PM_RST_OCM2_POR */
	Status = XPmReset_AssertbyId(PM_RST_OCM2_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	/* Initialize Array with GTYP Base Addresses*/
	for (i = 0U; i < ARRAY_SIZE(GtyAddresses); ++i) {
		Device = XPmDevice_GetById(GT_DEVID((u32)XPM_NODEIDX_DEV_GTYP_CPM5_MIN + i));
		if (NULL != Device) {
			GtyAddresses[i] = Device->Node.BaseAddress;
		}
	}

	/* Get houseclean disable mask */
	DisableMask = XPm_In32(PM_HOUSECLEAN_DISABLE_REG_1) >> HOUSECLEAN_CPM_SHIFT;

	/* Set Houseclean Mask */
	PwrDomain->HcDisableMask |= DisableMask;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus CpmInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Send a CCIX_EN IPI to PSM if its a valid CPM CCIX design */
	Status = XPm_CCIXEnEvent(PwrDomain->Power.Node.Id);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to send CCIX_EN IPI to PSM, Return: 0x%x\r\n", Status);
	}

	return Status;
}

static XStatus CpmScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	/* This function does not use the args */
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	/* ScanClear is not supported till CPM4 so skip this */
	return XST_SUCCESS;
}

static XStatus Cpm5ScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XPM_ERR_SCAN_CLR;
	const XPm_CpmDomain *Cpm = (XPm_CpmDomain *)PwrDomain;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegVal;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Unlock PCSR */
	XPmCpmDomain_UnlockPcsr(Cpm->CpmPcsrBaseAddr);

	/*
	 * De-assert the HOLDSTATE bit and release open the clock gates in CPM
	 */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET),
		      CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK,
		      CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK;
		goto done;
	}

	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET, 0U);

	if (HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK)) {
		PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Run scan clear on CPM */
		PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
		/* Check that the register value written properly or not! */
		PmChkRegMask32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET),
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK,
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK;
			goto done;
		}

		PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET,
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
		/* Check that the register value written properly or not! */
		PmChkRegMask32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET),
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK,
				CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_PCR;
			goto done;
		}

		/* Wait for Scan Clear do be done */
		Status = XPm_PollForMask(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PSR_OFFSET,
				CPM_PCSR_PSR_SCAN_CLEAR_DONE_MASK,
				XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			goto done;
		}

		/* Check if Scan Clear Passed */
		RegVal = XPm_In32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PSR_OFFSET);
		if ((RegVal & (u32)CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK) !=
				(u32)CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK) {
			DbgErr = XPM_INT_ERR_SCAN_PASS;
			goto done;
		}
	} else {
		/* ScanClear is skipped */
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/* Assert the HOLDSTATE bit and release open the clock gates in CPM */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET),
		      CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK,
		      CPM_PCSR_MASK_SCAN_CLEAR_HOLDSTATE_WEN_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK;
		goto done;
	}

	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET,
		CPM_PCSR_PCR_HOLDSTATE_MASK);

	/* Disable writes to PCR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		0x0);

	/* Remove isolation between CPM5 and LPD */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM5, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM5_ISO;
	}

	/* Enable CPM CPM_TOPSW_REF clock */
	Status = XPm_SetClockState(PM_SUBSYS_PMC, PM_CLK_CPM_TOPSW_REF, 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CPM_TOPSW_REF_CLK_ENABLE;
	}

done:
	/* Lock PCSR */
	XPmCpmDomain_LockPcsr(Cpm->CpmPcsrBaseAddr);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


static XStatus CpmBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Bisr */
		Status = XPmBisr_Repair(CPM_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BISR_REPAIR;
		}

	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	Status = XST_SUCCESS;

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5Bisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Bisr on CPM5 PD*/
		Status = XPmBisr_Repair(CPM5_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_CPM5_BISR_REPAIR;
			goto done;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	for (i = 0; i < ARRAY_SIZE(GtyAddresses); ++i) {
		if (0U == GtyAddresses[i]) {
			continue;
		}

		/* De-assert InitCtrl */
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(GtyAddresses[i] + GTY_PCSR_MASK_OFFSET,
			GTY_PCSR_INITCTRL_MASK);
		PmOut32(GtyAddresses[i] + GTY_PCSR_CONTROL_OFFSET, 0U);
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1U);
	}

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for CPM5_GTYP\r\n");

		/* Bisr on GTYP_CPM5 */
		Status = XPmBisr_Repair(CPM5_GTYP_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_CPM5_GTYP_BISR_REPAIR;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus CpmMbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	const XPm_CpmDomain *Cpm = (XPm_CpmDomain *)PwrDomain;
	u32 RegValue;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK == (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK)) {
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	/* Unlock Writes */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_WPROT0_OFFSET, 0);

	/* Trigger Mbist */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_RST;
		goto fail;
	}

	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_SETUP;
		goto fail;
	}

	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_PGEN;
		goto fail;
	}

	/* Wait till its done */
	Status = XPm_PollForMask(Cpm->CpmSlcrSecureBaseAddr +
				 CPM_SLCR_SECURE_OD_MBIST_DONE_OFFSET,
				 0xFF, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto fail;
	}

	/* Check status */
	PmIn32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_GO_OFFSET,
	       RegValue);
	if (0xFFU != (RegValue & 0xFFU)) {
		DbgErr = XPM_INT_ERR_MBIST_GOOD;
		Status = XST_FAILURE;
		goto fail;
	}

	/* Unwrite trigger bits */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET, 0x0);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET, 0x0);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET, 0x0);

fail:
	/* Lock Writes */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_WPROT0_OFFSET, 1);

done:
	XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

static XStatus Cpm5GtypMbist(u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddress + GTY_PCSR_MASK_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5_GTY_PCSR_MASK;
		goto done;
	}

	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET,
		GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddress + GTY_PCSR_CONTROL_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5_GTY_MEM_CLEAR_TRIGGER_MASK;
		goto done;
	}

	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET,
	GTY_PCSR_STATUS_MEM_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto done;
	}

	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET,
	GTY_PCSR_STATUS_MEM_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS_TIMEOUT;
		goto done;
	}

	/* Unwrite Trigger bits */
	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, 0);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5MbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XPM_ERR_MBIST_CLR;
	volatile XStatus StatusTmp = XPM_ERR_MBIST_CLR;
	const XPm_CpmDomain *Cpm = (XPm_CpmDomain *)PwrDomain;
	u32 RegValue, i;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK == (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK)) {
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	/* Disable write protection */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_WPROTS_OFFSET, 0);

	/* Trigger MBIST for all controllers */
	/* This step can be broken down into stages to reduce power
	 * consumption. However, clear action is performed in parallel by
	 * MBIST Controllers */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET,
		CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET),
		      CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_MASK,
		      CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5MBISTCLR_SLCRSECU_MBIST_TRIGGER;
		goto done;
	}

	/* Poll for done */
	/* If trigger action is performed in stages, then break down this step */
	Status = XPm_PollForMask(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_DONE_OFFSET,
				 CPM5_SLCR_SECURE_OD_MBIST_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto done;
	}

	/* Check Status */
	PmIn32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_PASSOUT_OFFSET,
		RegValue);
	if ((CPM5_SLCR_SECURE_OD_MBIST_PASSOUT_MASK & RegValue) !=
		CPM5_SLCR_SECURE_OD_MBIST_PASSOUT_MASK) {
		DbgErr = XPM_INT_ERR_MBIST_PASS;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite trigger bit and enable write protection */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
			CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET, 0x0);

	/* Enable write protection */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
			CPM5_SLCR_SECURE_WPROTS_OFFSET, 0x1);

	for (i = 0U; i < ARRAY_SIZE(GtyAddresses); ++i) {
		if (0U == GtyAddresses[i]) {
			continue;
		}
		XPmPlDomain_UnlockGtyPcsr(GtyAddresses[i]);
		/* Mbist */
		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Cpm5GtypMbist), (GtyAddresses[i]));
		XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
		/* Required for redundancy */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_MBIST;
			XPmPlDomain_LockGtyPcsr(GtyAddresses[i]);
			goto done;
		}
		XPmPlDomain_LockGtyPcsr(GtyAddresses[i]);
	}

	if (ARRAY_SIZE(GtyAddresses) != i) {
		Status = XST_FAILURE;
		DbgErr = XPM_INT_ERR_CPM5_MBIST_LOOP;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const struct XPm_PowerDomainOps CpmOps[XPM_CPM_OPS_MAX] = {
	[XPM_HC_CPM_OPS] = {
		.InitStart = CpmInitStart,
		.InitFinish = CpmInitFinish,
		.ScanClear = CpmScanClear,
		.Bisr = CpmBisr,
		.Mbist = CpmMbistClear,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
			     BIT16(FUNC_INIT_FINISH) |
			     BIT16(FUNC_SCAN_CLEAR) |
			     BIT16(FUNC_BISR) |
			     BIT16(FUNC_MBIST_CLEAR))
	},
	[XPM_HC_CPM5_OPS] = {
		.InitStart = Cpm5InitStart,
		.InitFinish = CpmInitFinish,
		.ScanClear = Cpm5ScanClear,
		.Bisr = Cpm5Bisr,
		.Mbist = Cpm5MbistClear,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
			     BIT16(FUNC_INIT_FINISH) |
			     BIT16(FUNC_SCAN_CLEAR) |
			     BIT16(FUNC_BISR) |
			     BIT16(FUNC_MBIST_CLEAR))
	},
};

XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const struct XPm_PowerDomainOps *Ops = NULL;

	if (Id == PM_POWER_CPM) {
		Ops = &CpmOps[XPM_HC_CPM_OPS];
	} else if (Id == PM_POWER_CPM5) {
		Ops = &CpmOps[XPM_HC_CPM5_OPS];
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	Status = XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, Ops);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (4U <= OtherBaseAddressesCnt) {
		CpmDomain->CpmSlcrBaseAddr = OtherBaseAddresses[0];
		CpmDomain->CpmSlcrSecureBaseAddr = OtherBaseAddresses[1];
		CpmDomain->CpmPcsrBaseAddr = OtherBaseAddresses[2];
		CpmDomain->CpmCrCpmBaseAddr = OtherBaseAddresses[3];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/* Hack because topology is not passing correct info */
	if (Id == PM_POWER_CPM5) {
		CpmDomain->CpmSlcrBaseAddr = CPM5_SLCR_BASEADDR;
		CpmDomain->CpmSlcrSecureBaseAddr = CPM5_SLCR_SECURE_BASEADDR;
		CpmDomain->CpmCrCpmBaseAddr = CPM5_CRX_BASEADDR;
	}

	/* Clear CPM section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_1, PM_HOUSECLEAN_DISABLE_CPM_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

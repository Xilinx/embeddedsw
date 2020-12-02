/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
#include "xpm_psm_api.h"

#define XPM_HC_CPM_OPS			0U
#define XPM_HC_CPM5_OPS			1U
#define XPM_CPM_OPS_MAX			2U

/* Define CPM5_GTYP device */
#define XPM_NODEIDX_DEV_GTYP_CPM5_MIN		XPM_NODEIDX_DEV_GTYP_CPM5_0
#define XPM_NODEIDX_DEV_GTYP_CPM5_MAX		XPM_NODEIDX_DEV_GTYP_CPM5_3

static u32 GtyAddresses[XPM_NODEIDX_DEV_GTYP_CPM5_MAX -
			XPM_NODEIDX_DEV_GTYP_CPM5_MIN + 1] = {0};

static XStatus CpmInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_CpmDomain *Cpm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 PlatformVersion;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
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
	if(!(PLATFORM_VERSION_SILICON == XPm_GetPlatform() &&
	     (u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))
	{
		/* Remove POR for CPM */
		/*Status = XPmReset_AssertbyId(PM_RST_CPM_POR,
				     (u32)PM_RESET_ACTION_RELEASE);*/

		/*TODO: Topology is not passing cpm reset register
		right now, so hard coded for now */
	        PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_ECO_OFFSET, 0);
	        PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, 1);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5InitStart(u32 *Args, u32 NumofArgs)
{
	XStatus Status = XPM_ERR_INIT_START;
	XPm_CpmDomain *Cpm;
	u32 i;
	XPm_Device* Device = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use any args */
	(void)Args;
	(void)NumofArgs;

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM5);
	if (NULL == Cpm) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* Remove POR for CPM5 */
	/* lpd_cpm5_por_n reset maps to PM_RST_OCM2_POR */
	Status = XPmReset_AssertbyId(PM_RST_OCM2_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	/* Remove isolation between CPM5 and LPD */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_CPM5_DFX, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_CPM5_DFX_ISO;
		goto done;
	}

	/* Initialize Array with GTYP Base Addresses*/
	for (i = 0; i < ARRAY_SIZE(GtyAddresses); ++i) {
		Device = XPmDevice_GetById(GT_DEVID((u32)XPM_NODEIDX_DEV_GTYP_CPM5_MIN + i));
		if (NULL != Device) {
			GtyAddresses[i] = Device->Node.BaseAddress;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus CpmInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Send a CCIX_EN IPI to PSM if its a valid CPM CCIX design */
	Status = XPm_CCIXEnEvent();
	if (XST_SUCCESS != Status) {
		PmErr("Failed to send CCIX_EN IPI to PSM, Return: 0x%x\r\n",
		      Status);
	}
	return Status;
}

static XStatus CpmScanClear(u32 *Args, u32 NumOfArgs)
{
	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* ScanClear is not supported till CPM4 so skip this */
	return XST_SUCCESS;
}

static XStatus Cpm5ScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XPM_ERR_SCAN_CLR;
	XPm_CpmDomain *Cpm;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM5);
	if (NULL == Cpm) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* Unlock PCSR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Run scan clear on CPM */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET),
		      CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK,
		      CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK;
		goto done;
	}

	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET,
		CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET),
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
	Status = XPm_PollForMask(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PSR_OFFSET,
				 CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_PASS_TIMEOUT;
		goto done;
	}

	/* Disable writes to PCR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		0x0);

	/* Lock PCSR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, 1);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


static XStatus CpmBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Bisr */
	Status = XPmBisr_Repair(CPM_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_REPAIR;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Cpm5Bisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Bisr on CPM5 PD*/
	Status = XPmBisr_Repair(CPM5_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CPM5_BISR_REPAIR;
		goto done;
	}

	/* Bisr on GTYP_CPM5 */
	Status = XPmBisr_Repair(CPM5_GTYP_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CPM5_GTYP_BISR_REPAIR;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus CpmMbistClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_CpmDomain *Cpm;
	u32 RegValue;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock Writes */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_WPROT0_OFFSET, 0);

	/* Trigger Mbist */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_RST;
		goto done;
	}

	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_SETUP;
		goto done;
	}

	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET, 0xFF);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET),
			  0xFF, 0xFF, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_PGEN;
		goto done;
	}

	/* Wait till its done */
	Status = XPm_PollForMask(Cpm->CpmSlcrSecureBaseAddr +
				 CPM_SLCR_SECURE_OD_MBIST_DONE_OFFSET,
				 0xFF, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto done;
	}

	/* Check status */
	PmIn32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_GO_OFFSET,
	       RegValue);
	if (0xFFU != (RegValue & 0xFFU)) {
		DbgErr = XPM_INT_ERR_MBIST_GOOD;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite trigger bits */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET, 0x0);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET, 0x0);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET, 0x0);

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
	PmChkRegRmw32((BaseAddress + GTY_PCSR_MASK_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_CPM5_GTY_PCSR_MASK;
		goto done;
	}

	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET,
		GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((BaseAddress + GTY_PCSR_CONTROL_OFFSET),
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

static XStatus Cpm5MbistClear(u32 *Args, u32 NumOfArgs)
{
	volatile XStatus Status = XPM_ERR_MBIST_CLR;
	volatile XStatus StatusTmp = XPM_ERR_MBIST_CLR;
	XPm_CpmDomain *Cpm;
	u32 RegValue, i;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM5);
	if (NULL == Cpm) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* Disable write protection */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_WPROTS_OFFSET, 0);

	/* Trigger MBIST for all controllers */
	/* This step can be broken down into stages to reduce power
	 * consumption. However, clear action is performed in parallel by
	 * MBIST Controllers */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET,
		CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET),
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

	/* Cleanup Operation. Unwrite trigger bit and enable write protection */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_OD_MBIST_TRIGGER_OFFSET,
		0x0);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM5_SLCR_SECURE_WPROTS_OFFSET,
		0x1);

	for (i = 0; 0U != GtyAddresses[i]; ++i) {
		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		/* Mbist */
		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (Cpm5GtypMbist), (GtyAddresses[i]));
		XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
		/* Required for redundancy */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_MBIST;
			PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
			goto done;
		}

		PmOut32(GtyAddresses[i] + GTY_PCSR_LOCK_OFFSET, 1);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PowerDomainOps CpmOps[XPM_CPM_OPS_MAX] = {
	[XPM_HC_CPM_OPS] = {
		.InitStart = CpmInitStart,
		.InitFinish = CpmInitFinish,
		.ScanClear = CpmScanClear,
		.Bisr = CpmBisr,
		.Mbist = CpmMbistClear,
	},
	[XPM_HC_CPM5_OPS] = {
		.InitStart = Cpm5InitStart,
		.InitFinish = CpmInitFinish,
		.ScanClear = Cpm5ScanClear,
		.Bisr = Cpm5Bisr,
		.Mbist = Cpm5MbistClear,
	},
};

XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	struct XPm_PowerDomainOps *Ops = NULL;

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

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

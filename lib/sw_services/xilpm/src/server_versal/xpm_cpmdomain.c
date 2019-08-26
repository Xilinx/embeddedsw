/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_cpmdomain.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_power.h"

static XStatus CpmInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	XPm_CpmDomain *Cpm;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Remove isolation to allow scan_clear on CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM_DFX, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* CPM POR control is not valid for ES1 platforms so skip. It is taken care by hw */
	if(!(PLATFORM_VERSION_SILICON == Platform && PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))
	{
		/* Remove POR for CPM */
		/*Status = XPmReset_AssertbyId(PM_RST_CPM_POR,
				     PM_RESET_ACTION_RELEASE);*/

		/*TODO: Topology is not passing cpm reset register
		right now, so hard coded for now */
	        PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
		PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_ECO_OFFSET, 0);
	        PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, 1);
	}
done:
	return Status;
}

static XStatus CpmInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	return Status;
}

static XStatus CpmScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	XPm_CpmDomain *Cpm;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Scan clear should be skipped for ES1 platforms */
	if ((PLATFORM_VERSION_SILICON != Platform) || (PLATFORM_VERSION_SILICON == Platform && PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		Status = XST_SUCCESS;
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock PCSR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Run scan clear on CPM */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET,
		CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	Status = XPm_PollForMask(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PSR_OFFSET,
				 CPM_PCSR_PSR_SCAN_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PSR_OFFSET,
				 CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Pulse CPM POR */
	Status = XPmReset_AssertbyId(PM_RST_CPM_POR,
				     PM_RESET_ACTION_PULSE);

	/* Unwrite trigger bits */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_MASK_OFFSET,
		CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
        PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_PCR_OFFSET, 0);

	/* Lock PCSR */
	PmOut32(Cpm->CpmPcsrBaseAddr + CPM_PCSR_LOCK_OFFSET, 1);
done:
	return Status;
}

static XStatus CpmBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Bisr */
	Status = XPmBisr_Repair(CPM_TAG_ID);

done:
	return Status;
}

static XStatus CpmMbistClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	XPm_CpmDomain *Cpm;
	u32 RegValue;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock Writes */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_WPROT0_OFFSET, 0);

	/* Trigger Mbist */
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_RESET_N_OFFSET, 0xFF);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_SETUP_OFFSET, 0xFF);
	PmOut32(Cpm->CpmSlcrSecureBaseAddr +
		CPM_SLCR_SECURE_OD_MBIST_PG_EN_OFFSET, 0xFF);

	/* Wait till its done */
	Status = XPm_PollForMask(Cpm->CpmSlcrSecureBaseAddr +
				 CPM_SLCR_SECURE_OD_MBIST_DONE_OFFSET,
				 0xFF, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check status */
	PmIn32(Cpm->CpmSlcrSecureBaseAddr + CPM_SLCR_SECURE_OD_MBIST_GO_OFFSET,
	       RegValue);
	if (0xFF != (RegValue & 0xFF)) {
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
        return Status;
}

struct XPm_PowerDomainOps CpmOps = {
	.InitStart = CpmInitStart,
	.InitFinish = CpmInitFinish,
	.ScanClear = CpmScanClear,
	.Bisr = CpmBisr,
	.Mbist = CpmMbistClear,
};

XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_SUCCESS;

	XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, &CpmOps);

	/* Make sure enough base addresses are being passed */
	if (4 <= OtherBaseAddressesCnt) {
		CpmDomain->CpmSlcrBaseAddr = OtherBaseAddresses[0];
		CpmDomain->CpmSlcrSecureBaseAddr = OtherBaseAddresses[1];
		CpmDomain->CpmPcsrBaseAddr = OtherBaseAddresses[2];
		CpmDomain->CpmCrCpmBaseAddr = OtherBaseAddresses[3];
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

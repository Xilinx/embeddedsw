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


static XStatus CpmInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Remove isolation to allow scan_clear on CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM_DFX, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* CPM POR control is not valid for ES1 platforms so skip. It is taken care by hw */
	if(!(PLATFORM_VERSION_SILICON == Platform && PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))
	{
		/* Remove POR for CPM */
		/*Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_RELEASE);*/

		/*TODO: Topology is not passing cpm reset register
		right now, so hard coded for now */
	        PmOut32(CPM_PCSR_LOCK, PCSR_UNLOCK_VAL);
		PmOut32(CPM_PCSR_ECO, 0);
	        PmOut32(CPM_PCSR_LOCK, 1);
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

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Scan clear should be skipped for ES1 platforms */
	if ((PLATFORM_VERSION_SILICON != Platform) || (PLATFORM_VERSION_SILICON == Platform && PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Unlock PCSR */
	PmOut32(CPM_PCSR_LOCK, PCSR_UNLOCK_VAL);

	/* Run scan clear on CPM */
	PmOut32(CPM_PCSR_MASK, CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	PmOut32(CPM_PCSR_PCR, CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_SCAN_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_SCAN_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Pulse CPM POR */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_PULSE);

	/* Unwrite trigger bits */
	PmOut32(CPM_PCSR_MASK, CPM_PCSR_PCR_SCAN_CLEAR_TRIGGER_MASK);
        PmOut32(CPM_PCSR_PCR, 0);

	/* Lock PCSR */
	PmOut32(CPM_PCSR_LOCK, 1);
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
	u32 RegValue;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Unlock Writes */
	PmOut32(CPM_SLCR_SECURE_WPROT0, 0);

	/* Trigger Mbist */
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_RESET_N, 0xFF);
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_SETUP, 0xFF);
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_PG_EN, 0xFF);

	/* Wait till its done */
	Status = XPm_PollForMask(CPM_SLCR_SECURE_OD_MBIST_DONE, 0xFF, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check status */
	PmIn32(CPM_SLCR_SECURE_OD_MBIST_GO, RegValue);
	if (0xFF != (RegValue & 0xFF)) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite trigger bits */
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_RESET_N, 0x0);
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_SETUP, 0x0);
	PmOut32(CPM_SLCR_SECURE_OD_MBIST_PG_EN, 0x0);

	/* Lock Writes */
	PmOut32(CPM_SLCR_SECURE_WPROT0, 1);
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
			   XPm_Power *Parent)
{
	XPmPowerDomain_Init(&CpmDomain->Domain, Id, BaseAddress, Parent, &CpmOps);
	return XST_SUCCESS;
}

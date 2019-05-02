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

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Remove isolation to allow scan_clear on CPM */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_CPM_DFX, FALSE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove POR for CPM */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_RELEASE);

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

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Unlock PCSR */
	PmOut32(CPM_PCSR_LOCK, PCSR_UNLOCK_VAL);

	/* Mbist */
	PmOut32(CPM_PCSR_MASK, CPM_PCSR_PCR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(CPM_PCSR_PCR, CPM_PCSR_PCR_MEM_CLEAR_TRIGGER_MASK);

	/* Poll for status */
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_MEM_CLEAR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPm_PollForMask(CPM_PCSR_PSR, CPM_PCSR_PSR_MEM_CLEAR_PASS_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Lock PCSR */
	PmOut32(CPM_PCSR_LOCK, 1);
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

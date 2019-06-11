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

#include "sleep.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_domain_iso.h"
#include "xpm_npdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"

static XStatus NpdPreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* TODO: Ensure power for NPI is on */

	/* Remove FPD-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove LPD-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* PL-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PL_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove VCCAUX-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove VCCRAM-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_SOC);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove PMC-NoC NPI domain isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_SOC_NPI);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static XStatus NpdPostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	return Status;
}

static XStatus NpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

#ifdef SPP_HACK
	XPm_Core *Pmc;
	u32 RegValue;

	Pmc = (XPm_Core *)PmDevices[XPM_NODEIDX_DEV_PMC_PROC];
	if (NULL == Pmc) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
		(PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
		PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK));

	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
		PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);

	usleep(200);

	PmIn32((Pmc->RegAddress[0] + PMC_GLOBAL_ERR1_STATUS_OFFSET),
	       RegValue);
	if (0 != (RegValue & (PMC_GLOBAL_ERR1_STATUS_NOC_TYPE_1_NCR_MASK |
			     PMC_GLOBAL_ERR1_STATUS_DDRMC_MC_NCR_MASK))) {
		Status = XST_FAILURE;
	}

done:
#endif
	return Status;
}

static XStatus NpdMbist(u32 *AddressList, u32 NumOfAddress)
{
	XStatus Status = XST_SUCCESS;

#ifdef SPP_HACK
	u32 RegValue;
	int i;

	/* Deassert PCSR Lock*/
	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PSCR_LOCK_OFFSET, 0);
	}

	/* TBD: Enable ILA clock for DDR blocks*/

	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PSCR_MASK_OFFSET, NPI_PSCR_CONTROL_MEM_CLEAR_TRIGGER_MASK)
		PmOut32(AddressList[i] + NPI_PSCR_CONTROL_OFFSET, NPI_PSCR_CONTROL_MEM_CLEAR_TRIGGER_MASK);
	}
	for(i=0; i<NumOfAddress; i++)
	{
		Status = XPm_PollForMask(AddressList[i] + NPI_PSCR_STATUS_OFFSET,
				 NPI_PSCR_STATUS_MEM_CLEAR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	for(i=0; i<NumOfAddress; i++)
	{
		PmIn32(AddressList[i] + NPI_PSCR_STATUS_OFFSET, RegValue);
		if (NPI_PSCR_STATUS_MEM_CLEAR_PASS_MASK !=
		    (RegValue & NPI_PSCR_STATUS_MEM_CLEAR_PASS_MASK)) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* TBD: Disable ILA clock for DDR blocks*/

	/* Assert PCSR Lock*/
	for(i=0; i<NumOfAddress; i++)
	{
		PmOut32(AddressList[i] + NPI_PSCR_LOCK_OFFSET, 1);
	}
done:
#else
	(void)AddressList;
	(void)NumOfAddress;
#endif
	return Status;
}

static XStatus NpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	Status = XPmBisr_Repair(DDRMC_TAG_ID);

	return Status;
}

struct XPm_PowerDomainOps NpdOps = {
	.PreHouseClean = NpdPreHouseclean,
	.PostHouseClean = NpdPostHouseclean,
	.ScanClear = NpdScanClear,
	.Mbist = NpdMbist,
	.Bisr = NpdBisr,
};

XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent)
{
	XPmPowerDomain_Init(&Npd->Domain, Id, BaseAddress, Parent, &NpdOps);

	return XST_SUCCESS;
}

/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
#include "xpm_pslpdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"

static XStatus LpdPreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* TODO: Check vccint_pslp first to make sure power is on */

	/* Remove PS_PL isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove PS_CPM domains isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_CPM_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_CPM);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove FP-SOC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove LP-SoC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove PS_PMC domains isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_LPD_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_LPD);
	if (Status != XST_SUCCESS)
		goto done;

	/*
	 * Release POR for PS-LPD
	 */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PS_POR),
				     PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static XStatus LpdPostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes scan clear sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Trigger Scan clear on LPD/LPD_IOU */
	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
		(PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK),
		(PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK));
	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_DONE,
				 (PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_PASS,
				 (PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes LBIST sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdLbist(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable LBIST isolation */
	PmRmw32(PMC_ANALOG_LBIST_ISOLATION_EN,
		(PMC_ANALOG_LBIST_ISOLATION_EN_LPD_MASK |
		 PMC_ANALOG_LBIST_ISOLATION_EN_LPD_RPU_MASK),
		(PMC_ANALOG_LBIST_ISOLATION_EN_LPD_MASK |
		 PMC_ANALOG_LBIST_ISOLATION_EN_LPD_RPU_MASK));

	/* Trigger LBIST on LPD */
	PmRmw32(PMC_ANALOG_LBIST_ENABLE,
		(PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		 PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK),
		(PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		 PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK));

	/* Release LBIST reset */
	PmRmw32(PMC_ANALOG_LBIST_RST_N,
		(PMC_ANALOG_LBIST_RST_N_LPD_MASK |
		 PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK),
		(PMC_ANALOG_LBIST_RST_N_LPD_MASK |
		 PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK));

	Status = XPm_PollForMask(PMC_ANALOG_LBIST_DONE,
				 (PMC_ANALOG_LBIST_DONE_LPD_MASK |
				  PMC_ANALOG_LBIST_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes BISR sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	Status = XPmBisr_Repair(LPD_TAG_ID);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes MBIST sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdMbist(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	u32 RegValue;

	PmRmw32(PMC_ANALOG_OD_MBIST_RST,
		(PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_RST_LPD_MASK));

	PmRmw32(PMC_ANALOG_OD_MBIST_SETUP,
		(PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK));

	PmRmw32(PMC_ANALOG_OD_MBIST_PG_EN,
		(PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK),
		(PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		 PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK));

	Status = XPm_PollForMask(PMC_ANALOG_OD_MBIST_DONE,
				 (PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK|
				  PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK |
				  PMC_ANALOG_OD_MBIST_DONE_LPD_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmIn32(PMC_ANALOG_OD_MBIST_GOOD, RegValue);

	if ((PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK) != RegValue) {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

struct XPm_PowerDomainOps LpdOps = {
	.PreHouseClean = LpdPreHouseclean,
	.PostHouseClean = LpdPostHouseclean,
	.ScanClear = LpdScanClear,
	.Mbist = LpdMbist,
	.Lbist = LpdLbist,
	.Bisr = LpdBisr,
};

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent)
{
	XPmPowerDomain_Init(&PsLpd->Domain, Id, BaseAddress, Parent, &LpdOps);

	return XST_SUCCESS;
}

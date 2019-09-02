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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_pslpdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_prot.h"
#include "xpm_regs.h"
#include "xpm_device.h"

static XStatus LpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_pslp first to make sure power is on */
	if (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK)) {
		/* TODO: Request PMC to power up VCCINT_LP rail and wait for the acknowledgement.*/
		goto done;
	}

	/* Remove PS_PMC domains isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD_DFX, FALSE);
	if (Status != XST_SUCCESS)
		goto done;

	/*
	 * Release POR for PS-LPD
	 */
	Status = XPmReset_AssertbyId(PM_RST_PS_POR,
				     PM_RESET_ACTION_RELEASE);
done:
	return Status;
}

static XStatus LpdPreBisrReqs()
{
	XStatus Status = XST_SUCCESS;

	/* Remove PMC LPD isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_PMC_LPD, FALSE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Release reset for PS SRST */
	Status = XPmReset_AssertbyId(PM_RST_PS_SRST,
				     PM_RESET_ACTION_RELEASE);
done:
	return Status;
}

static XStatus LpdInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	return Status;
}

static XStatus LpdHcComplete(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	(void)Args;
	(void)NumOfArgs;

	/* In case bisr and mbist are skipped */
	Status = LpdPreBisrReqs();
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove LPD SoC isolation */
	Status = XPmDomainIso_Control(XPM_NODEIDX_ISO_LPD_SOC, FALSE);
	if (Status != XST_SUCCESS)
		goto done;

	/* Copy sysmon data */
	XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PS_LPD], PM_POWER_LPD, 0);
done:
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
	if (XST_SUCCESS != Status) {
                goto done;
        }

	 /* unwrite trigger bits */
        PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
                (PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
                 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
                 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK), 0);

	/*
	 * Pulse PS POR
	 */
	Status = XPmReset_AssertbyId(PM_RST_PS_POR,
				     PM_RESET_ACTION_PULSE);
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
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u32 RegVal;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (NULL == EfuseCache) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Check if Lbist is enabled*/
	PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_MISC_CTRL_OFFSET, RegVal);
	if ((RegVal & EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK) != EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK) {
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

	if (XST_SUCCESS != Status) {
                goto done;
        }
	/* Unwrite trigger bits */
        PmRmw32(PMC_ANALOG_LBIST_ENABLE,
                (PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
                 PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK), 0);

	/*
	 * Pulse PS POR
	 */
	Status = XPmReset_AssertbyId(PM_RST_PS_POR,
				     PM_RESET_ACTION_PULSE);
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
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		Status = XST_FAILURE;
		goto done;
	}

	(void)Args;
	(void)NumOfArgs;

	/* Pre bisr requirements */
	Status = LpdPreBisrReqs();
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPmBisr_Repair(LPD_TAG_ID);
	if (XST_SUCCESS == Status) {
		LpDomain->LpdBisrFlags |= LPD_BISR_DONE;
	}

done:
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

	/* Pre bisr requirements - In case if Bisr is skipped */
	Status = LpdPreBisrReqs();
	if (Status != XST_SUCCESS)
		goto done;

	/* Release USB reset for LPD IOU Mbist to work*/
	Status = XPmReset_AssertbyId(PM_RST_USB_0,
				     PM_RESET_ACTION_RELEASE);

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
		goto done;
	}

	/* Unwrite bits after mem clear has finished */
	PmRmw32(PMC_ANALOG_OD_MBIST_RST,
                (PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
                 PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
                 PMC_ANALOG_OD_MBIST_RST_LPD_MASK), 0);

        PmRmw32(PMC_ANALOG_OD_MBIST_SETUP,
                (PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
                 PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
                 PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK),0);

        PmRmw32(PMC_ANALOG_OD_MBIST_PG_EN,
                (PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
                 PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
                 PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK),0);

done:
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function configures xppu for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdXppuCtrl(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;
	u32 XppuNodeId, Enable;

	if(NumOfArgs < 2) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XppuNodeId = Args[0];
	Enable = Args[1];

	if (XPM_NODECLASS_PROTECTION != NODECLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (XPM_NODESUBCL_PROT_XPPU != NODESUBCLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if(Enable && (NumOfArgs==3))
		Status = XPmProt_XppuEnable(XppuNodeId, Args[2]);
	else
		Status = XPmProt_XppuDisable(XppuNodeId);

done:
	return Status;
}

struct XPm_PowerDomainOps LpdOps = {
	.InitStart = LpdInitStart,
	.InitFinish = LpdInitFinish,
	.ScanClear = LpdScanClear,
	.Mbist = LpdMbist,
	.Lbist = LpdLbist,
	.Bisr = LpdBisr,
	.HcComplete = LpdHcComplete,
	.XppuCtrl = LpdXppuCtrl,
};

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_SUCCESS;

	XPmPowerDomain_Init(&PsLpd->Domain, Id, BaseAddress, Parent, &LpdOps);

	PsLpd->LpdBisrFlags = 0;

	/* Make sure enough base addresses are being passed */
	if (3 <= OtherBaseAddressesCnt) {
		PsLpd->LpdIouSlcrBaseAddr = OtherBaseAddresses[0];
		PsLpd->LpdSlcrBaseAddr = OtherBaseAddresses[1];
		PsLpd->LpdSlcrSecureBaseAddr = OtherBaseAddresses[2];
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

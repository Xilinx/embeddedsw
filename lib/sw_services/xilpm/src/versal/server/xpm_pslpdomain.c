/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pslpdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_board.h"
#include "xpm_prot.h"
#include "xpm_regs.h"
#include "xpm_device.h"

static XStatus LpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumOfArgs;

	/* Check vccint_pslp first to make sure power is on */
	if (XST_SUCCESS != XPmPower_CheckPower(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK)) {
		Status = XPmBoard_ControlRail(RAIL_POWER_UP, POWER_RAIL_LPD);
		if (XST_SUCCESS != Status) {
			PmErr("Control power rail for LPD failure during power up\r\n");
			goto done;
		}
	}

	/* Remove PS_PMC domains isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD_DFX, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Release POR for PS-LPD
	 */
	Status = XPmReset_AssertbyId(PM_RST_PS_POR, (u32)PM_RESET_ACTION_RELEASE);
done:
	return Status;
}

static XStatus LpdPreBisrReqs(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *XramDevice = NULL;
	XPm_ResetNode *XramRst = NULL;

	/* Remove PMC LPD isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release reset for PS SRST */
	Status = XPmReset_AssertbyId(PM_RST_PS_SRST, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release OCM2 (XRAM) SRST if XRAM exists */
	XramDevice = XPmDevice_GetById(PM_DEV_XRAM_0);
	if (NULL == XramDevice) {
		goto done;
	}

	/* Make sure SRST source is PS SRST */
	/* If PM_RST_XRAM_RST val is 0x0, XRAM_SRST = PS_SRST */
	/* else XRAM_SRST = PL_SRST */
	XramRst = XPmReset_GetById(PM_RST_XRAM);
	if (NULL == XramRst) {
		Status = XST_FAILURE;
		goto done;
	}

	if (XramRst->Ops->GetState(XramRst) == 0x0U) {
		Status = XPmReset_AssertbyId(PM_RST_OCM2_RST, (u32)PM_RESET_ACTION_RELEASE);
	} else {
		/* We shouldn't reach here. PL SRST is source for XRAM SRST */
		Status = XPM_ERR_RESET;
	}

done:
	return Status;
}

static XStatus LpdInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static XStatus LpdHcComplete(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumOfArgs;

	/* In case bisr and mbist are skipped */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Remove LPD SoC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Copy sysmon data */
	Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PS_LPD], PM_POWER_LPD, 0);
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
	XStatus Status = XST_FAILURE;

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
	Status = XPmReset_AssertbyId(PM_RST_PS_POR, (u32)PM_RESET_ACTION_PULSE);
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
	XStatus Status = XST_FAILURE;
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
	Status = XPmReset_AssertbyId(PM_RST_PS_POR, (u32)PM_RESET_ACTION_PULSE);
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
	XStatus Status = XST_FAILURE;
	XPm_Device *XramDevice = XPmDevice_GetById(PM_DEV_XRAM_0);
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		goto done;
	}

	(void)Args;
	(void)NumOfArgs;

	/* Pre bisr requirements */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmBisr_Repair(LPD_TAG_ID);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (NULL == XramDevice) {
		goto done;
	}

	Status = XPmBisr_Repair(XRAM_TAG_ID);

done:
	if (XST_SUCCESS == Status) {
		LpDomain->LpdBisrFlags |= LPD_BISR_DONE;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function executes MBIST sequence for XRAM
 *
 * @return XST_SUCCESS if successful else XPM_ERR_MBIST_CLR
 *
 ****************************************************************************/
static XStatus XramMbist(void)
{

	/* XRAM MBIST Sequence */
	/* There are 2 modes of memclear: (1) Unison Mode (2) Per Island Mode */
	/* Using Unison Mode */

	XStatus Status = XPM_ERR_MBIST_CLR;
	XPm_Device *Device = NULL;
	u32 BaseAddr, RegValue;

	Device = XPmDevice_GetById(PM_DEV_XRAM_0);
	if (NULL == Device) {
		/* device might not have XRAM IP, hence return success*/
		Status = XST_SUCCESS;
		goto done;
	}

	BaseAddr = Device->Node.BaseAddress;

	/* Unlock PCSR */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Write to Memclear Trigger */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, XRAM_MEM_CLEAR_TRIGGER_0_MASK);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, XRAM_MEM_CLEAR_TRIGGER_0_MASK);

	/* Poll for Memclear done */
	Status = XPm_PollForMask(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET,
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_0_MASK |
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_3_TO_1_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Memclear pass/fail status */
	PmIn32(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET, RegValue);
	if ((RegValue &
		(XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_0_MASK |
		XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_3_TO_1_MASK)) !=
		((XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_0_MASK |
		XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_3_TO_1_MASK))) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite the trigger bits */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, 0x0);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, 0x0);

	/* Lock PCSR */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, 0x0);

	Status = XST_SUCCESS;

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
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	u32 RegValue;

	/* Pre bisr requirements - In case if Bisr is skipped */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Release USB reset for LPD IOU Mbist to work*/
	Status = XPmReset_AssertbyId(PM_RST_USB_0, (u32)PM_RESET_ACTION_RELEASE);

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


	Status = XramMbist();

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
	XStatus Status = XST_FAILURE;
	u32 XppuNodeId, Enable;

	if (NumOfArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XppuNodeId = Args[0];
	Enable = Args[1];

	if ((u32)XPM_NODECLASS_PROTECTION != NODECLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_PROT_XPPU != NODESUBCLASS(XppuNodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((0U != Enable) && (3U == NumOfArgs)) {
		Status = XPmProt_XppuEnable(XppuNodeId, Args[2]);
	} else {
		Status = XPmProt_XppuDisable(XppuNodeId);
	}

done:
	return Status;
}

static struct XPm_PowerDomainOps LpdOps = {
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
	XStatus Status = XST_FAILURE;

	Status = XPmPowerDomain_Init(&PsLpd->Domain, Id, BaseAddress, Parent, &LpdOps);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PsLpd->LpdBisrFlags = 0;

	/* Make sure enough base addresses are being passed */
	if (3U <= OtherBaseAddressesCnt) {
		PsLpd->LpdIouSlcrBaseAddr = OtherBaseAddresses[0];
		PsLpd->LpdSlcrBaseAddr = OtherBaseAddresses[1];
		PsLpd->LpdSlcrSecureBaseAddr = OtherBaseAddresses[2];
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

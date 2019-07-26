/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pslpdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_prot.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_rail.h"

static XStatus LpdInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)Args;
	(void)NumOfArgs;

	XPm_Rail *VccintPslpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSLP);

	/* Check vccint_pslp first to make sure power is on */
	Status = XPmPower_CheckPower(VccintPslpRail,
						PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Remove PS_PMC domains isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD_DFX, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_LPD_DFX_ISO;
		goto done;
	}

	/*
	 * Release POR for PS-LPD
	 */
	Status = XPmReset_AssertbyId(PM_RST_PS_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PS_POR;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus LpdPreBisrReqs(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *XramDevice = NULL;
	XPm_ResetNode *XramRst = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Remove PMC LPD isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_LPD, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_LPD_ISO;
		goto done;
	}

	/* Release reset for PS SRST */
	Status = XPmReset_AssertbyId(PM_RST_PS_SRST, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PS_SRST;
		goto done;
	}

	/* Release OCM2 (XRAM) SRST if XRAM exists */
	XramDevice = XPmDevice_GetById(PM_DEV_XRAM_0);
	if (NULL == XramDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/* Make sure SRST source is PS SRST */
	/* If PM_RST_XRAM_RST val is 0x0, XRAM_SRST = PS_SRST */
	/* else XRAM_SRST = PL_SRST */
	XramRst = XPmReset_GetById(PM_RST_XRAM);
	if (NULL == XramRst) {
		DbgErr = XPM_INT_ERR_INVALID_RST;
		Status = XST_FAILURE;
		goto done;
	}

	if (XramRst->Ops->GetState(XramRst) == 0x0U) {
		Status = XPmReset_AssertbyId(PM_RST_OCM2_RST, (u32)PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_RELEASE;
		}
	} else {
		/* We shouldn't reach here. PL SRST is source for XRAM SRST */
		DbgErr = XPM_INT_ERR_RST_STATE;
		Status = XPM_ERR_RESET;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr;

	(void)Args;
	(void)NumOfArgs;

	/* In case bisr and mbist are skipped */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PRE_BISR_REQ;
		goto done;
	}

	/* Remove LPD SoC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_LPD_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LPD_SOC_ISO;
		goto done;
	}

	/* Copy sysmon data */
	SysmonAddr = XPm_GetSysmonByIndex((u32)XPM_NODEIDX_MONITOR_SYSMON_PS_LPD);
	if (0U == SysmonAddr) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Status = XPmPowerDomain_ApplyAmsTrim(SysmonAddr, PM_POWER_LPD, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AMS_TRIM;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegBitMask;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Trigger Scan clear on LPD/LPD_IOU */
	RegBitMask = ((u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		      (u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK);
	PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDLSCNCLR_TRIGGER;
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_DONE,
				 (PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_SCAN_CLEAR_PASS,
				 (PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
				  PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_PASS_TIMEOUT;
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
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PS_POR;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	u32 RegAddr;
	volatile u32 RegVal = 0U;
	volatile u32 RegValTmp = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegBitMask;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (NULL == EfuseCache) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Check if Lbist is enabled*/
	RegAddr = EfuseCache->Node.BaseAddress + EFUSE_CACHE_MISC_CTRL_OFFSET;
	RegVal = XPm_In32(RegAddr) & EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK;
	/* Required for redundancy */
	RegValTmp = XPm_In32(RegAddr) & EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK;
	u32 LocalRegVal = RegValTmp; /* Copy volatile to local to avoid MISRA */
	if ((EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK != RegVal) &&
	    (EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK != LocalRegVal)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable LBIST isolation */
	RegBitMask = ((u32)PMC_ANALOG_LBIST_ISOLATION_EN_LPD_MASK |
		      (u32)PMC_ANALOG_LBIST_ISOLATION_EN_LPD_RPU_MASK);
	PmRmw32(PMC_ANALOG_LBIST_ISOLATION_EN, RegBitMask, RegBitMask);
	/* Check that Lbist isolation Enabled */
	PmChkRegRmw32(PMC_ANALOG_LBIST_ISOLATION_EN, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDLBIST_ISO_EN;
		goto done;
	}

	/* Trigger LBIST on LPD */
	RegBitMask = ((u32)PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		      (u32)PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK);
	PmRmw32(PMC_ANALOG_LBIST_ENABLE, RegBitMask, RegBitMask);
	/* Check that Lbist triggered on LPD */
	PmChkRegRmw32(PMC_ANALOG_LBIST_ENABLE, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDLBIST_ENABLE;
		goto done;
	}

	/* Release LBIST reset */
	RegBitMask = ((u32)PMC_ANALOG_LBIST_RST_N_LPD_MASK |
		      (u32)PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK);
	PmRmw32(PMC_ANALOG_LBIST_RST_N, RegBitMask, RegBitMask);
	/* Check that Lbist reset released */
	PmChkRegRmw32(PMC_ANALOG_LBIST_RST_N, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDLBIST_RST_N;
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_LBIST_DONE,
				 (PMC_ANALOG_LBIST_DONE_LPD_MASK |
				  PMC_ANALOG_LBIST_DONE_LPD_RPU_MASK),
				 XPM_POLL_TIMEOUT);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_LBIST_DONE_TIMEOUT;
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
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PS_POR;
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	u16 DbgErr;

	if (NULL == LpDomain) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	(void)Args;
	(void)NumOfArgs;

	/* Pre bisr requirements */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PRE_BISR_REQ;
		goto done;
	}

	Status = XPmBisr_Repair(LPD_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_REPAIR;
		goto done;
	}

	if (NULL == XramDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	Status = XPmBisr_Repair(XRAM_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_XRAM_BISR_REPAIR;
	}

done:
	if (XST_SUCCESS == Status) {
		LpDomain->LpdBisrFlags |= LPD_BISR_DONE;
	} else {
		PmErr("0x%x\r\n", DbgErr);
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
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
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
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET),
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK,
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_XRAM_PCSR_MASK;
		goto done;
	}

	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, XRAM_MEM_CLEAR_TRIGGER_0_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32((BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET),
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK,
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_XRAM_MEM_CLEAR_TRIGGER_0_MASK;
		goto done;
	}

	/* Poll for Memclear done */
	Status = XPm_PollForMask(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET,
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_0_MASK |
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_3_TO_1_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto done;
	}

	/* Check Memclear pass/fail status */
	PmIn32(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET, RegValue);
	if ((RegValue &
		(XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_0_MASK |
		XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_3_TO_1_MASK)) !=
		((XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_0_MASK |
		XRAM_SLCR_PCSR_PSR_MEM_CLEAR_PASS_3_TO_1_MASK))) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS_TIMEOUT;
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
	XPm_PrintDbgErr(Status, DbgErr);
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
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegBitMask;

	(void)Args;
	(void)NumOfArgs;

	if (PLATFORM_VERSION_SILICON != XPm_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	u32 RegValue;

	/* Pre bisr requirements - In case if Bisr is skipped */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PRE_BISR_REQ;
		goto done;
	}

	/* Release USB reset for LPD IOU Mbist to work*/
	Status = XPmReset_AssertbyId(PM_RST_USB_0, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto done;
	}

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_RST_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_RST, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32(PMC_ANALOG_OD_MBIST_RST, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_RST;
		goto done;
	}

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_SETUP, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32(PMC_ANALOG_OD_MBIST_SETUP, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_SETUP;
		goto done;
	}

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_PG_EN, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegRmw32(PMC_ANALOG_OD_MBIST_PG_EN, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_PGEN;
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_OD_MBIST_DONE,
				 (PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK|
				  PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK |
				  PMC_ANALOG_OD_MBIST_DONE_LPD_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto done;
	}

	PmIn32(PMC_ANALOG_OD_MBIST_GOOD, RegValue);

	if ((PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK) != RegValue) {
		DbgErr = XPM_INT_ERR_MBIST_GOOD;
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

	/* Required for redundancy */
	XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (XramMbist));
	XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
		DbgErr = XPM_INT_ERR_XRAM_MBIST;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
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
	return XPmProt_CommonXppuCtrl(Args, NumOfArgs);
}

/****************************************************************************/
/**
 * @brief  This function configures xmpu for OCM
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdXmpuCtrl(u32 *Args, u32 NumOfArgs)
{
	return XPmProt_CommonXmpuCtrl(Args, NumOfArgs);
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
	.XmpuCtrl = LpdXmpuCtrl,
};

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressesCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PsLpd->Domain, Id, BaseAddress, Parent, &LpdOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
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
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

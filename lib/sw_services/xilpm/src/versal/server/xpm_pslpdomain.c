/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pslpdomain.h"
#include "xpm_domain_iso.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_rail.h"
#include "xplmi.h"

#define NUM_LPD_MIO		26U
#define LPD_IOU_SCLR_GPIO_MUX	0x40U

static XStatus LpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DisableMask;

	(void)Args;
	(void)NumOfArgs;

	const XPm_Rail *VccintPslpRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PSLP);

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

	/* Get houseclean disable mask */
	DisableMask = XPm_In32(PM_HOUSECLEAN_DISABLE_REG_1) >> HOUSECLEAN_LPD_SHIFT;

	/* Set Houseclean Mask */
	PwrDomain->HcDisableMask |= DisableMask;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus LpdPreBisrReqs(void)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *XramDevice = NULL;
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
	if (NULL != XramDevice) {
		Status = XPmReset_AssertbyId(PM_RST_OCM2_RST,
					     (u32)PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_RELEASE;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus LpdMioFlush(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	u32 SaveIouSclrMioSettings[NUM_LPD_MIO];
	u32 SaveData0, IsFlushed = 0U;
	u32 SaveDirm0, SaveTri0, SaveOen0;
	/* Read a RTCA register get the list of pins to flush */
	u32 LPD_MIO_0_FLUSH_MASK =  Xil_In32(XPLMI_RTCFG_MIO_WA_BANK_502_ADDR);
	/* Save GPIO reset state. */
	u32 SaveRst = Xil_In32(CRL_RST_GPIO);
	/* Deassert GPIO reset */
	/* Print a debug line */
	XPlmi_Printf(MIO_FLUSH_DEBUG,"Starting LPD MIO flush ...\n\r");
	/* Set all 26 MIO pins to GPIO */
	for (u32 i = 0U; i < NUM_LPD_MIO; i++){
		u32 PinAddr = LPD_IOU_SLCR_BASEADDR + (i << 2U);
		/* Saving all MIO mux settings to local memory */
		SaveIouSclrMioSettings[i] = Xil_In32(PinAddr);
		/* Inspect to skip certain pins that are not in the mask */
		IsFlushed = (LPD_MIO_0_FLUSH_MASK >> i ) & 1U;
		if (IsFlushed != 0U){
			/* Only set those pins that are in mask */
			XPm_Out32(PinAddr, LPD_IOU_SCLR_GPIO_MUX );
		}else{
			/* Skip those pins are not in mask */
			XPlmi_Printf(MIO_FLUSH_DEBUG,"##########Skipping pin %d ...\n\r",i);
		}
	}
	/* Saving TriState data */
	SaveTri0 = Xil_In32(LPD_IOU_SLCR_MIO_MST_TRI0);
	/* Saving OE data */
	SaveOen0 = Xil_In32(LPD_GPIO_OEN_0_ADDR);
	SaveDirm0 = Xil_In32(LPD_GPIO_DIRM_0_ADDR);
	/* Assert MST TRI0 and MST_TR1 */
	XPm_Out32(LPD_IOU_SLCR_MIO_MST_TRI0, SaveTri0 | LPD_MIO_0_FLUSH_MASK);
	/* save TX DATA */
	SaveData0 = Xil_In32(LPD_GPIO_DATA_0_ADDR);
	/* Deassert Tristate to allow LPD_GPIO controller to control the OEN */
	XPm_Out32(LPD_IOU_SLCR_MIO_MST_TRI0, SaveTri0 & (~LPD_MIO_0_FLUSH_MASK));
	/* Set all MIO to GPIO output */
	XPm_Out32(LPD_GPIO_DIRM_0_ADDR, SaveDirm0 | LPD_MIO_0_FLUSH_MASK);
	/* Set TX Data to zero */
	XPm_Out32(LPD_GPIO_DATA_0_ADDR, SaveData0 & (~LPD_MIO_0_FLUSH_MASK));
	/* Assert OE */
	XPm_Out32(LPD_GPIO_OEN_0_ADDR, SaveOen0 | LPD_MIO_0_FLUSH_MASK);
	/* Deassert OE */
	XPm_Out32(LPD_GPIO_OEN_0_ADDR, SaveOen0 & (~LPD_MIO_0_FLUSH_MASK));
	/* Restore OE */
	XPm_Out32(LPD_GPIO_OEN_0_ADDR, SaveOen0);
	/* Restore DIR */
	XPm_Out32(LPD_GPIO_DIRM_0_ADDR, SaveDirm0);
	/* Restore TX DATA */
	XPm_Out32(LPD_GPIO_DATA_0_ADDR, SaveData0);
	/* Restore TriState */
	XPm_Out32(LPD_IOU_SLCR_MIO_MST_TRI0, SaveTri0);
	/* Restore all MIO muxes */
	for (u32 i = 0U; i < NUM_LPD_MIO; i++){
		XPm_Out32(LPD_IOU_SLCR_BASEADDR + (i << 2U), SaveIouSclrMioSettings[i]);
	}
	/* Restore GPIO reset */
	Xil_Out32(CRL_RST_GPIO,SaveRst);
	/* Print debug message */
	XPlmi_Printf(MIO_FLUSH_DEBUG,"LPD flush MIO done.\n\r");
	/* Done and return. */
	return XST_SUCCESS;
}

static XStatus LpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static XStatus LpdHcComplete(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SysmonAddr;

	(void)PwrDomain;
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
static XStatus LpdScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegBitMask;
	u32 RegVal;

	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK)) {
		PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Trigger Scan clear on LPD/LPD_IOU */
		RegBitMask = ((u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
		      (u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK);
		PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER, RegBitMask, RegBitMask);
		/* Check that the register value written properly or not! */
		PmChkRegMask32(PMC_ANALOG_SCAN_CLEAR_TRIGGER, RegBitMask, RegBitMask, Status);
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

		RegVal = XPm_In32(PMC_ANALOG_SCAN_CLEAR_PASS);
		if ((RegVal & (u32)((u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
				(u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
				(u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK)) !=
				(u32)((u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK |
				(u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK |
				(u32)PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK)) {
			DbgErr = XPM_INT_ERR_SCAN_PASS;
			goto done;
		}

		/* unwrite trigger bits */
		PmRmw32(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
				(PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK |
                 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK |
                 PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK), 0);

	} else {
		/* ScanClear is skipped */
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

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
static XStatus LpdLbist(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u32 RegAddr;
	volatile u32 RegVal = 0U;
	volatile u32 RegValTmp = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RegBitMask;

	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_LBIST_MASK != (PwrDomain->HcDisableMask &
			HOUSECLEAN_DISABLE_LBIST_MASK)) {
		PmInfo("Triggering LBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

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
		PmChkRegMask32(PMC_ANALOG_LBIST_ISOLATION_EN, RegBitMask, RegBitMask, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_LPDLBIST_ISO_EN;
			goto done;
		}

		/* Trigger LBIST on LPD */
		RegBitMask = ((u32)PMC_ANALOG_LBIST_ENABLE_LPD_MASK |
		      (u32)PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK);
		PmRmw32(PMC_ANALOG_LBIST_ENABLE, RegBitMask, RegBitMask);
		/* Check that Lbist triggered on LPD */
		PmChkRegMask32(PMC_ANALOG_LBIST_ENABLE, RegBitMask, RegBitMask, Status);
		if (XPM_REG_WRITE_FAILED == Status) {
			DbgErr = XPM_INT_ERR_REG_WRT_LPDLBIST_ENABLE;
			goto done;
		}

		/* Release LBIST reset */
		RegBitMask = ((u32)PMC_ANALOG_LBIST_RST_N_LPD_MASK |
		      (u32)PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK);
		PmRmw32(PMC_ANALOG_LBIST_RST_N, RegBitMask, RegBitMask);
		/* Check that Lbist reset released */
		PmChkRegMask32(PMC_ANALOG_LBIST_RST_N, RegBitMask, RegBitMask, Status);
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
	} else {
		/* LBIST is skipped */
		PmInfo("Skipping LBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

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
static XStatus LpdBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *XramDevice = XPmDevice_GetById(PM_DEV_XRAM_0);
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)PwrDomain;
	u16 DbgErr;

	(void)Args;
	(void)NumOfArgs;

	/* Pre bisr requirements */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PRE_BISR_REQ;
		goto done;
	}

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
			HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

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
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
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
	const XPm_Device *Device = NULL;
	u32 BaseAddr, RegValue;

	Device = XPmDevice_GetById(PM_DEV_XRAM_0);
	if (NULL == Device) {
		/* device might not have XRAM IP, hence return success*/
		Status = XST_SUCCESS;
		goto done;
	}

	BaseAddr = Device->Node.BaseAddress;

	/* Unlock PCSR */
	XPmPsLpDomain_UnlockPcsr(BaseAddr);

	/* Write to Memclear Trigger */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, XRAM_MEM_CLEAR_TRIGGER_0_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET),
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK,
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_XRAM_PCSR_MASK;
		goto fail;
	}

	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, XRAM_MEM_CLEAR_TRIGGER_0_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET),
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK,
		      XRAM_MEM_CLEAR_TRIGGER_0_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_XRAM_MEM_CLEAR_TRIGGER_0_MASK;
		goto fail;
	}

	/* Poll for Memclear done */
	Status = XPm_PollForMask(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET,
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_0_MASK |
			XRAM_SLCR_PCSR_PSR_MEM_CLEAR_DONE_3_TO_1_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto fail;
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
		goto fail;
	}

	/* Unwrite the trigger bits */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, 0x0);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, 0x0);

	Status = XST_SUCCESS;

fail:
	/* Lock PCSR */
	XPmPsLpDomain_LockPcsr(BaseAddr);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus LpdMbistTrigger(u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;
	u32 RegBitMask;
	u32 RegValue;

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_RST_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_RST, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegMask32(PMC_ANALOG_OD_MBIST_RST, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_RST;
		goto done;
	}

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_SETUP, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegMask32(PMC_ANALOG_OD_MBIST_SETUP, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_SETUP;
		goto done;
	}

	RegBitMask = ((u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK |
		      (u32)PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK);
	PmRmw32(PMC_ANALOG_OD_MBIST_PG_EN, RegBitMask, RegBitMask);
	/* Check that the register value written properly or not! */
	PmChkRegMask32(PMC_ANALOG_OD_MBIST_PG_EN, RegBitMask, RegBitMask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_LPDMBIST_PGEN;
		goto done;
	}

	Status = XPm_PollForMask(PMC_ANALOG_OD_MBIST_DONE,
				 (PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK|
				  PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK |
				  PMC_ANALOG_OD_MBIST_DONE_LPD_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MBIST_DONE_TIMEOUT;
		goto done;
	}

	PmIn32(PMC_ANALOG_OD_MBIST_GOOD, RegValue);

	if ((PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK |
	     PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK) != RegValue) {
		*DbgErr = XPM_INT_ERR_MBIST_GOOD;
		Status = XST_FAILURE;
	}

done:
	return Status;
}

static void CleanupMemClearLpd(void)
{
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
}

/****************************************************************************/
/**
 * @brief  This function executes MBIST sequence for LPD
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
static XStatus LpdMbist(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_ClockNode *UsbClk, *Can0Clk, *Can1Clk;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	if (HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK == (PwrDomain->HcDisableMask &
			HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK)) {
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

	/* Pre bisr requirements - In case if Bisr is skipped */
	Status = LpdPreBisrReqs();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PRE_BISR_REQ;
		goto done;
	}

	UsbClk = XPmClock_GetById(PM_CLK_USB0_BUS_REF);
	Can0Clk = XPmClock_GetById(PM_CLK_CAN0_REF);
	Can1Clk = XPmClock_GetById(PM_CLK_CAN1_REF);
	if ((NULL == UsbClk) || (NULL == Can0Clk) || (NULL == Can1Clk)) {
		Status = XST_DEVICE_NOT_FOUND;
		DbgErr = XPM_INT_ERR_LPD_MBIST_CLK_NOT_FOUND;
		goto done;
	}

	/* Request USB clock as a dependency of LPD MBIST */
	Status = XPmClock_SetGate((XPm_OutClockNode *)UsbClk, 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_USB_CLK_ENABLE;
		goto done;
	}

	/* Release USB reset for LPD IOU Mbist to work*/
	Status = XPmReset_AssertbyId(PM_RST_USB_0, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_USB_RST_RELEASE;
		goto done;
	}

	/* Request CAN0 clock as a dependency of LPD MBIST */
	Status = XPmClock_SetGate((XPm_OutClockNode *)Can0Clk, 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CAN1_CLK_ENABLE;
		goto done;
	}

	/* Request CAN1 clock as a dependency of LPD MBIST */
	Status = XPmClock_SetGate((XPm_OutClockNode *)Can1Clk, 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CAN0_CLK_ENABLE;
		goto done;
	}

	Status = LpdMbistTrigger(&DbgErr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unwrite bits after mem clear has finished */
	CleanupMemClearLpd();

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

static const struct XPm_PowerDomainOps LpdOps = {
	.InitStart = LpdInitStart,
	.InitFinish = LpdInitFinish,
	.ScanClear = LpdScanClear,
	.Mbist = LpdMbist,
	.Lbist = LpdLbist,
	.Bisr = LpdBisr,
	.HcComplete = LpdHcComplete,
	.MioFlush = LpdMioFlush,
	/* Mask to indicate which Ops are present */
	.InitMask = (BIT16(FUNC_INIT_START) |
		     BIT16(FUNC_INIT_FINISH) |
		     BIT16(FUNC_SCAN_CLEAR) |
		     BIT16(FUNC_MBIST_CLEAR) |
		     BIT16(FUNC_LBIST) |
		     BIT16(FUNC_BISR) |
		     BIT16(FUNC_HOUSECLEAN_COMPLETE))
};

XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
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

	/* Clear LPD section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_1, PM_HOUSECLEAN_DISABLE_LPD_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

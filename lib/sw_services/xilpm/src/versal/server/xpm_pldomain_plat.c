/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_regs.h"
#include "xpm_reset.h"
#include "xpm_bisr.h"
#include "xpm_pmc.h"
#include "xpm_debug.h"
#include "xparameters.h"
#include "sleep.h"
#include "xpm_rail.h"

#include "xplmi_ssit.h"

#define XPM_NODEIDX_DEV_GT_MIN		XPM_NODEIDX_DEV_GT_0
/* Modify value of MAX_DEV_GT if we run out */
#define MAX_DEV_GT		52U

/* The current number of VDU device. Used to run housecleaning for each VDU */
#define XPM_NODEIDX_DEV_VDU_MIN     XPM_NODEIDX_DEV_VDU_0
#define XPM_NODEIDX_DEV_VDU_MAX     XPM_NODEIDX_DEV_VDU_3

/* The current number of BFR-B devices. Used to run housecleaning for each. */
#define XPM_NODEIDX_DEV_BFRB_MIN		XPM_NODEIDX_DEV_BFRB_0
#define XPM_NODEIDX_DEV_BFRB_MAX		XPM_NODEIDX_DEV_BFRB_11

/* The current number of ADC devices. Used to run housecleaning for each. */
#define XPM_NODEIDX_DEV_ADC_MIN		XPM_NODEIDX_DEV_ADC_0
#define XPM_NODEIDX_DEV_ADC_MAX		XPM_NODEIDX_DEV_ADC_3

/* The current number of DAC devices. Used to run housecleaning for each. */
#define XPM_NODEIDX_DEV_DAC_MIN		XPM_NODEIDX_DEV_DAC_0
#define XPM_NODEIDX_DEV_DAC_MAX		XPM_NODEIDX_DEV_DAC_3

#define PLHCLEAN_EARLY_BOOT 0U
#define PLHCLEAN_INIT_NODE  1U

/* If TRIM_CRAM[31:0]=0 (FUSE not programmed),
 * Use Dynamic read voltage and 4 Legs setting for keeper Bias */
#define CRAM_TRIM_RW_READ_VOLTAGE	0x08000B80U

/* Laguna housecleaning macros */
#define LAGUNA_WIDTH				117U
#define LAGUNA_HORIZONTAL_INTER_SLR_WIDTH	20U
#define SSIT_TOP_SLR_1 1U
#define SSIT_TOP_SLR_2 2U

#define XPM_RFCOM_HOUSECLEAN_LONG_WAIT_US 20U

static XCframe CframeIns={0}; /* CFRAME Driver Instance */
static XCfupmc CfupmcIns={0}; /* CFU Driver Instance */
static volatile u32 PlpdHouseCleanBypass = 0;
static volatile u32 PlpdHouseCleanBypassTmp = 0;
u32 HcleanDone = 0;

static XStatus XPmPlDomain_InitandHouseclean(u32 PollTimeOut);

static XStatus PldInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XPmPowerDomain_SecureEfuseTransfer(PM_POWER_PLD);

	return Status;
}

static XStatus PldGtyMbist(u32 BaseAddress, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddress + GTY_PCSR_MASK_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_GTY_PCSR_MASK;
		goto done;
	}

	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((BaseAddress + GTY_PCSR_CONTROL_OFFSET),
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK,
		      GTY_PCSR_MEM_CLEAR_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_GTY_MEM_CLEAR_TRIGGER_MASK;
		goto done;
	}
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET,
				GTY_PCSR_STATUS_MEM_CLEAR_DONE_MASK, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto done;
	}
	Status = XPm_PollForMask(BaseAddress + GTY_PCSR_STATUS_OFFSET,
				GTY_PCSR_STATUS_MEM_CLEAR_PASS_MASK, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS_TIMEOUT;
		goto done;
	}

done:
	/* Unwrite trigger bits */
	PmOut32(BaseAddress + GTY_PCSR_MASK_OFFSET, GTY_PCSR_MEM_CLEAR_TRIGGER_MASK);
	PmOut32(BaseAddress + GTY_PCSR_CONTROL_OFFSET, 0);
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus InitVduAddrArr(u32 *VduArrPtr, const u32 ArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	u32 i;
	u32 Idx = 0;

	if (NULL == VduArrPtr) {
		goto done;
	}

	for (i = (u32)XPM_NODEIDX_DEV_VDU_MIN; i <= (u32)XPM_NODEIDX_DEV_VDU_MAX; i++) {
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) || ((u32)XPM_NODETYPE_DEV_VDU != NODETYPE(Device->Node.Id))) {
			continue;
		}

	    if ((NULL == Device->Power) || ((u32)PM_POWER_PLD != Device->Power->Node.Id)) {
			continue;
		}

		if (Idx >= ArrLen) {
			DbgErr = XPM_INT_ERR_VDU_INIT;
			goto done;
		}

		VduArrPtr[Idx] = Device->Node.BaseAddress;
		Idx++;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to set/clear bits in VDU PCSR
 *
 * @param BaseAddress	BaseAddress of VDU device
 * @param Mask		Mask to be written into PCSR_MASK register
 * @param Value		Value to be written into PCSR_CONTROL register
 *
 * note:	Blind writes are not performed for VDU PCSR writes because
 *		VDU PCSR registers behave differently. There is a delay until
 *		some set bits are accurately updated in the registers.
 *
 * @return None
 *****************************************************************************/
static void VduPcsrWrite(u32 BaseAddress, u32 Mask, u32 Value)
{
	/* Write Mask to PCSR_MASK register */
	XPm_Out32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	/* Write Value to PCSR_CONTROL register */
	XPm_Out32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
}

static XStatus VduInit(u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Check for VDU power status */
	if (VDU_NPI_PCSR_STATUS_VDU_PWR_SUPPLY_MASK !=
			(XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) & VDU_NPI_PCSR_STATUS_VDU_PWR_SUPPLY_MASK)) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Release VDU internal POR */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_VDU_IPOR_MASK, 0U);

	/* Release DFX isolation gasket */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_ISO_2_VDU_PL_MASK, 0U);

	Status = XST_SUCCESS;

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddress);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus VduScanClear(u32 BaseAddress, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Trigger Scan Clear */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
			VDU_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);

	/* Delay is required after trigger ScanClear */
	/* TODO: Update delay based on HW team input */
	usleep(10000U);

	/* Wait for Scan Clear DONE */
	Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
				VDU_NPI_PCSR_STATUS_SCAN_CLEAR_DONE_MASK, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
		goto done;
	}

	/* Check Scan Clear PASS */
	if (VDU_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK !=
			(XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) & VDU_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK)) {
		DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddress);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus VduMbist(u32 BaseAddress, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Assert Mem Clear Trigger */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
					VDU_NPI_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);

	/* Wait for Mem Clear DONE */
	Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
				VDU_NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		goto done;
	}

	/* Check Mem Clear Pass */
	if (VDU_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
			(XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) & VDU_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
		DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite trigger bits */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddress);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void VduClearInterrupts(u32 BaseAddress)
{
	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/*
	 * VDU interrupts are unable to be masked until after INITSTATE is
	 * released, see EDT-1041183. Release INITSTATE so that VDU interrupts can
	 * be masked and cleared.
	 */
	VduPcsrWrite(BaseAddress, VDU_NPI_PCSR_MASK_INITSTATE, 0U);

	/* Mask VDU Interrupts */
	XPm_Out32((BaseAddress + VDU_NPI_REG_IDR0_OFFSET), VDU_INTERRUPTS_ALL_MASK);
	XPm_Out32((BaseAddress + VDU_NPI_REG_IDR1_OFFSET), VDU_INTERRUPTS_ALL_MASK);
	XPm_Out32((BaseAddress + VDU_NPI_REG_IDR2_OFFSET), VDU_INTERRUPTS_ALL_MASK);

	/* Clear VDU interrupt status */
	XPm_Out32((BaseAddress + VDU_NPI_REG_ISR_OFFSET), VDU_INTERRUPTS_ALL_MASK);

	/* Clear PMC NPI error bits */
	XPm_Out32((PMC_GLOBAL_BASEADDR + PMC_GLOBAL_ERR1_STATUS_OFFSET), PMC_GLOBAL_ERR1_STATUS_NPI_ALL_MASK);

	/* Lock PCSR */
	XPm_LockPcsr(BaseAddress);
}

static XStatus VduHouseClean(u32 PollTimeOut)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 VduAddresses[XPM_NODEIDX_DEV_VDU_MAX - XPM_NODEIDX_DEV_VDU_MIN + 1] = {0};
	u32 i;

	/* Initialize array with addresses for each VDU device */
	Status = InitVduAddrArr(VduAddresses, ARRAY_SIZE(VduAddresses));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Setup VDU for housecleaning and run ScanClear for each VDU device */
	for (i = 0U; i < ARRAY_SIZE(VduAddresses); i++) {
		if (0U == VduAddresses[i]) {
			continue;
		}

		/* Run setup for VDU */
		Status = VduInit(VduAddresses[i]);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VDU_INIT;
			goto done;
		}

		if (PM_HOUSECLEAN_CHECK(VDU, SCAN)) {
			PmInfo("Triggering ScanClear for VDU\r\n");

			/* Trigger scan clear */
			Status = VduScanClear(VduAddresses[i], PollTimeOut);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_VDU_SCAN_CLEAR;
				goto done;
			}
		}

		/*
		 * In ES1, unmasked VDU interrupts are triggering several PMC error
		 * bits, see EDT-1040224. As workaround, mask and clear the VDU
		 * interrupts then clear the PMC error bits.
		 */
		if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
		    (PLATFORM_VERSION_SILICON_ES1 == XPm_GetPlatformVersion()) &&
		    (XPM_PMC_TAP_IDCODE_SBFMLY_SV == (XPm_GetIdCode() & PMC_TAP_IDCODE_SBFMLY_MASK))) {
			VduClearInterrupts(VduAddresses[i]);
		}
	}

	if (PM_HOUSECLEAN_CHECK(VDU, BISR)) {
		PmInfo("Triggering BISR for VDU\r\n");

		/* Trigger VDU BISR */
		Status = XPmBisr_Repair2(VDU_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VDU_BISR_REPAIR;
			goto done;
		}
	}

	if (!(PM_HOUSECLEAN_CHECK(VDU, MBIST))) {
		PmInfo("Skipping MBIST for VDU\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

	/* For each VDU device run MBIST */
	for (i = 0U; i < ARRAY_SIZE(VduAddresses); i++) {
		if (0U == VduAddresses[i]) {
			continue;
		}

		/* Trigger MBIST */
		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (VduMbist), (VduAddresses[i]), (PollTimeOut));
		/* Copy volatile to local to avoid MISRA */
		XStatus LocalStatus = StatusTmp;
		/* Required for redundancy */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_VDU_MBIST;
			goto done;
		}
	}

	/*
	 * In ES2, GT_NCR bit is set immediately after ScanClear/MemClear is
	 * is triggered and cannot be masked, see EDT-1040224. As workaround,
	 * clear the GT_NCR error bit in PMC.
	 */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (PLATFORM_VERSION_SILICON_ES2 == XPm_GetPlatformVersion()) &&
	    (XPM_PMC_TAP_IDCODE_SBFMLY_SV == (XPm_GetIdCode() & PMC_TAP_IDCODE_SBFMLY_MASK))) {
		/* Clear PMC GT NCR error bit */
		XPm_Out32((PMC_GLOBAL_BASEADDR + PMC_GLOBAL_ERR1_STATUS_OFFSET), PMC_GLOBAL_ERR1_STATUS_GT_NCR_MASK);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void PldApplyTrim(u32 TrimType)
{
	u32 TrimVal;
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	Xuint128 VggTrim={0};
	const XPm_Device *EfuseCache;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, EfuseCache, XPm_Device, XPmDevice_GetById, PM_DEV_EFUSE_CACHE);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	/* Read the corresponding efuse registers for TRIM values */
	switch (TrimType)
	{
		/* Read VGG trim efuse registers */
		case XPM_PL_TRIM_VGG:
		{
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_0_OFFSET,
			       VggTrim.Word0);
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_1_OFFSET,
			       VggTrim.Word1);
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CFRM_VGG_2_OFFSET,
			       VggTrim.Word2);
			XCframe_VggTrim(&CframeIns, &VggTrim);
			Status = XST_SUCCESS;
		}
		break;
		/* Read CRAM trim efuse registers */
		case XPM_PL_TRIM_CRAM:
		{
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_CRAM_OFFSET,
			       TrimVal);

			Platform = XPm_GetPlatform();
			/* if eFUSE is not programmed,
			then set rw_read_voltages to 0.61V + 0.625V by writing */
			if ((TrimVal == 0U) && ((u32)PLATFORM_VERSION_SILICON == Platform)) {
				TrimVal = CRAM_TRIM_RW_READ_VOLTAGE;
			}
			XCframe_CramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
		}
		break;
		/* Read BRAM trim efuse registers */
		case XPM_PL_TRIM_BRAM:
		{
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_BRAM_OFFSET,
			       TrimVal);
			XCframe_BramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
		}
		break;
		/* Read URAM trim efuse registers */
		case XPM_PL_TRIM_URAM:
		{
			PmIn32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_TRIM_URAM_OFFSET,
			       TrimVal);
			XCframe_UramTrim(&CframeIns, TrimVal);
			Status = XST_SUCCESS;
		}
		break;
		default:
		{
			DbgErr = XPM_INT_ERR_INVALID_TRIM_TYPE;
			Status = XST_FAILURE;
			break;
		}
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return;
}

static void PldCfuLock(const XPm_PlDomain *Pld, u32 Enable)
{
	static u32 PrevLockState=1U;

	if (1U == Enable) {
		/* Lock CFU writes */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, PrevLockState);
	} else {
		PmIn32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, PrevLockState);
		/* Unlock CFU writes */
		PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_PROTECT_OFFSET, 0);
	}

	return;
}

static XStatus PldCfuInit(void)
{
	XStatus Status = XST_FAILURE;
	const XCfupmc_Config *Config;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U != CfupmcIns.IsReady) {
		DbgErr = XPM_INT_ERR_CFU_NOT_READY;
		Status = XST_SUCCESS;
		goto done;
	}
	/*
	 * Initialize the CFU driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XCfupmc_LookupConfig((u16)XPAR_XCFUPMC_0_DEVICE_ID);
#else
	Config = XCfupmc_LookupConfig(XPAR_XCFUPMC_0_BASEADDR);
#endif
	if (NULL == Config) {
		DbgErr = XPM_INT_ERR_DEVICE_LOOKUP;
		Status = XST_FAILURE;
		goto done;
	}

	Status = XCfupmc_CfgInitialize(&CfupmcIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_CFG_INIT;
		goto done;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCfupmc_SelfTest(&CfupmcIns);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_SELF_TEST;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
static XStatus PldCframeInit(void)
{
	XStatus Status = XST_FAILURE;
	const XCframe_Config *Config;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U != CframeIns.IsReady) {
		Status = XST_SUCCESS;
		goto done;
	}
	/*
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
#else
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_BASEADDR);
#endif
	if (NULL == Config) {
		DbgErr = XPM_INT_ERR_DEVICE_LOOKUP;
		Status = XST_FAILURE;
		goto done;
	}

	Status = XCframe_CfgInitialize(&CframeIns, Config, Config->BaseAddress);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CFG_INIT;
		goto done;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCframe_SelfTest(&CframeIns);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SELF_TEST;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus InitGtyAddrArr(u32 *GtArrPtr, const u32 ArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Idx = 0U;
	u32 i;
	const XPm_Device *Device;

	for (i = (u32)XPM_NODEIDX_DEV_GT_MIN; i < (u32)XPM_NODEIDX_DEV_MAX; ++i) {
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) ||
			((u32)XPM_NODETYPE_DEV_GT != NODETYPE(Device->Node.Id))) {
			continue;
		}

		if ((NULL == Device->Power) ||
			((u32)PM_POWER_PLD != Device->Power->Node.Id)) {
			continue;
		}

		if (Idx >= ArrLen) {
			DbgErr = XPM_INT_ERR_GTY_INIT;
			goto done;
		}

		GtArrPtr[Idx] = Device->Node.BaseAddress;
		++Idx;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus GtyHouseClean(const XPm_PlDomain *Pld, u32 PollTimeOut)
{
	volatile XStatus Status = XPM_ERR_HC_PL;
	volatile XStatus StatusTmp = XPM_ERR_HC_PL;
	volatile u32 i;
	u32 GtyAddrs[MAX_DEV_GT] = {0};
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	(void)Pld;

	/* Initialize array with GT addresses */
	Status = InitGtyAddrArr(GtyAddrs, ARRAY_SIZE(GtyAddrs));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_GTY_HC;
		goto done;
	}

	for (i = 0; i < ARRAY_SIZE(GtyAddrs); i++) {
		if (0U == GtyAddrs[i]) {
			continue;
		}
		Status = XPm_PollForMask(GtyAddrs[i] + GTY_PCSR_STATUS_OFFSET,
					GTY_PCSR_STATUS_HOUSECLEAN_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			PmErr("HOUSECLEAN_DONE poll failed for GT:0x%08X\n\r", GtyAddrs[i]);
			DbgErr = XPM_INT_ERR_GTY_HC;
			goto done;
		}

		XPm_UnlockPcsr(GtyAddrs[i]);
		/* Deassert INITCTRL */
		PmOut32(GtyAddrs[i] + GTY_PCSR_MASK_OFFSET,
			GTY_PCSR_INITCTRL_MASK);
		PmOut32(GtyAddrs[i] + GTY_PCSR_CONTROL_OFFSET, 0);
		XPm_LockPcsr(GtyAddrs[i]);
	}

	u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
	if ((0U == PlpdHouseCleanBypass) || (0U == LocalPlpdHCBypass)) {

		/* Run GTY BISR operations if houseclean disable mask not set */
		if (PM_HOUSECLEAN_CHECK(GT, BISR)) {
			/* Bisr repair - Bisr should be triggered only for Addresses for which repair
			* data is found and so not calling in loop. Trigger is handled in below routine
			* */
			Status = XPmBisr_Repair(GTY_TAG_ID);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_GTY_BISR_REPAIR;
				goto done;
			}

			Status = XPmBisr_Repair(GTM_TAG_ID);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_GTM_BISR_REPAIR;
				goto done;
			}

			Status = XPmBisr_Repair(GTYP_TAG_ID);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_GTYP_BISR_REPAIR;
				goto done;
			}
		}

		/* Run GTY MBIST operations if houseclean disable mask not set */
		if (PM_HOUSECLEAN_CHECK(GT, MBIST)) {
			for (i = 0; i < ARRAY_SIZE(GtyAddrs); i++) {
				if (0U == GtyAddrs[i]) {
					continue;
				}
				XPm_UnlockPcsr(GtyAddrs[i]);
				/* Mbist */
				XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (PldGtyMbist), (GtyAddrs[i]), (PollTimeOut));
				XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
				/* Required for redundancy */
				if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
					/* Gt Mem clear is found to be failing on some parts.
					Just print message and return not to break execution */
					PmErr("ERROR: GT Mem clear Failed for 0x%x\r\n", GtyAddrs[i]);
				}
				XPm_LockPcsr(GtyAddrs[i]);
			}

			if (i != ARRAY_SIZE(GtyAddrs)) {
				DbgErr = XPM_INT_ERR_GTY_MEM_CLEAR_LOOP;
				Status = XST_FAILURE;
				goto done;
			}
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus BfrbInit(const u32 *BfrbAddresses, const u32 ArrLen, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	u32 i = 0;

	Status = XPmPower_CheckPower(VccintRail,
			PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Deassert PWRDN for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(BfrbAddresses[i]);

		/* Deassert PWRDN */
		Status = XPm_PcsrWrite(BfrbAddresses[i], BFR_NPI_PCSR_MASK_PWRDN_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_RELEASE;
			goto done;
		}
	}

	/* SRAMS powered up for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Wait for SRAMS Powered Up */
		Status = XPm_PollForZero(BfrbAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					BFR_NPI_PCSR_STATUS_POWER_STATE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BFRB_POWER_STATE;
			goto done;
		}
	}

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(BfrbAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus BfrbMbist(const u32 *BfrbAddresses, const u32 ArrLen, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	/* Trigger Mem Clear for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(BfrbAddresses[i]);

		/* Assert Mem Clear Trigger */
		Status = XPm_PcsrWrite(BfrbAddresses[i], BFR_NPI_PCSR_MEM_CLEAR_TRIGGER_MASK,
				BFR_NPI_PCSR_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
			goto done;
		}
	}

	/* Wait for MemClear done and check for MemClear pass for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(BfrbAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					BFR_NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			goto done;
		}

		/* Check Mem Clear Pass */
		if (BFR_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
				(XPm_In32(BfrbAddresses[i] + NPI_PCSR_STATUS_OFFSET) & BFR_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* Set Status to SUCCESS in case BFR-B is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(BfrbAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
    return Status;
}

static XStatus BfrbScanClear(const u32 *BfrbAddresses, const u32 ArrLen, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	/* Trigger ScanClear for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(BfrbAddresses[i]);

		/* Trigger Scan Clear */
		Status = XPm_PcsrWrite(BfrbAddresses[i], BFR_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
				BFR_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER;
			goto done;
		}
	}

	/* Wait for ScanClear done and check ScanClear PASS for each BFRB */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(BfrbAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					BFR_NPI_PCSR_STATUS_SCAN_CLEAR_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			goto done;
		}

		/* Check Scan Clear PASS */
		if (BFR_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK !=
				(XPm_In32(BfrbAddresses[i] + NPI_PCSR_STATUS_OFFSET) & BFR_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* Set Status to SUCCESS in case BFR-B is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(BfrbAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
    return Status;
}

static XStatus BfrbInitFinish(const u32 *BfrbAddresses, const u32 ArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(BfrbAddresses[i]);

		/* Assert PWRDN */
		Status = XPm_PcsrWrite(BfrbAddresses[i], BFR_NPI_PCSR_MASK_PWRDN_MASK, BFR_NPI_PCSR_MASK_PWRDN_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_ASSERT;
			goto done;
		}

		/* Assert INITSTATE */
		Status = XPm_PcsrWrite(BfrbAddresses[i], BFR_NPI_PCSR_MASK_INITSTATE, BFR_NPI_PCSR_MASK_INITSTATE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BFRB_INITSTATE_ASSERT;
			goto done;
		}
	}

	/* Set Status to SUCCESS in case BFR-B is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == BfrbAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(BfrbAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus InitBfrbAddrArr(u32 *BfrbArrPtr, const u32 ArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	u32 i;
	u32 Idx = 0;

	if (NULL == BfrbArrPtr) {
		goto done;
	}

	for (i = (u32)XPM_NODEIDX_DEV_BFRB_MIN; i <= (u32)XPM_NODEIDX_DEV_BFRB_MAX; i++) {
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) || ((u32)XPM_NODETYPE_DEV_BFRB != NODETYPE(Device->Node.Id))) {
			continue;
		}

		if ((NULL == Device->Power) || ((u32)PM_POWER_PLD != Device->Power->Node.Id)) {
			continue;
		}

		if (Idx >= ArrLen) {
			DbgErr = XPM_INT_ERR_BFRB_INIT;
			goto done;
		}

		BfrbArrPtr[Idx] = Device->Node.BaseAddress;
		Idx++;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus BfrbHouseClean(u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BfrbAddresses[XPM_NODEIDX_DEV_BFRB_MAX - XPM_NODEIDX_DEV_BFRB_MIN + 1] = {0};

	/* Initialize array with addresses for each BFRB device */
	Status = InitBfrbAddrArr(BfrbAddresses, ARRAY_SIZE(BfrbAddresses));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Run setup for BFRB */
	Status = BfrbInit(BfrbAddresses, ARRAY_SIZE(BfrbAddresses), PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BFRB_INIT;
		goto done;
	}

	/* Run BFRB BISR */
	Status = XPmBisr_Repair2(BFRB_TAG_ID);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BFRB_BISR_REPAIR;
		goto done;
	}

	/* Run MBIST for each BFRB */
	XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (BfrbMbist),
			(BfrbAddresses), ARRAY_SIZE(BfrbAddresses), (PollTimeOut));
	/* Copy volatile to local to avoid MISRA */
	XStatus LocalStatus = StatusTmp;
	/* Required for redundancy */
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
		DbgErr = XPM_INT_ERR_BFRB_MBIST;
		goto done;
	}

	/* Run scan clear for each BFRB */
	Status = BfrbScanClear(BfrbAddresses, ARRAY_SIZE(BfrbAddresses), PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BFRB_SCAN_CLEAR;
		goto done;
	}

	/* Power down each BFRB to ensure no power leakage from unused instances */
	Status = BfrbInitFinish(BfrbAddresses, ARRAY_SIZE(BfrbAddresses));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_FUNC_INIT_FINISH;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus DacMbist(const u32 *DacAddresses, u32 ArrLen, u32 PollTimeOut){

	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	/* Trigger Mem Clear for each DAC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(DacAddresses[i]);

		/* Assert Mem Clear Trigger */
		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PCSR_MEM_CLEAR_TRIGGER_MASK,
				DAC_NPI_PCSR_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
			goto done;
		}
	}

	/* Wait for MemClear done and check for MemClear pass for each DAC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(DacAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					DAC_NPI_PCSR_STATUS_MEM_CLEAR_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			goto done;
		}

		/* Check Mem Clear Pass */
		if (DAC_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK !=
				(XPm_In32(DacAddresses[i] + NPI_PCSR_STATUS_OFFSET) & DAC_NPI_PCSR_STATUS_MEM_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}
	}
	/* Set Status to SUCCESS in case DAC is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(DacAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
    return Status;

}


static XStatus DacScanClear(const u32 *DacAddresses, u32 ArrLen, u32 PollTimeOut){

	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	/* Trigger ScanClear for each DAC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(DacAddresses[i]);

		/* set TEST_SAFE bit and trigger scan clear*/
		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PCSR_MASK_TEST_SAFE_MASK |
				DAC_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
				DAC_NPI_PCSR_MASK_TEST_SAFE_MASK |
				DAC_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER;
			goto done;
		}

	}

	/* Wait 20us */
	usleep(XPM_RFCOM_HOUSECLEAN_LONG_WAIT_US);

	/* Wait for ScanClear done and check ScanClear PASS for each DAC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(DacAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					DAC_NPI_PCSR_STATUS_SCAN_CLEAR_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			goto done;
		}

		/* Check Scan Clear PASS */
		if (DAC_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK !=
				(XPm_In32(DacAddresses[i] + NPI_PCSR_STATUS_OFFSET) & DAC_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}

		/* unset TEST_SAFE bit*/
		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PCSR_MASK_TEST_SAFE_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DAC_TEST_SAFE_UNSET;
			goto done;
		}
	}

	/* Set Status to SUCCESS in case DAC is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(DacAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
    return Status;

}

static XStatus AdcScanClear(const u32 *AdcAddresses, u32 ArrLen, u32 PollTimeOut){

	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0;

	/* Trigger ScanClear for each ADC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == AdcAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(AdcAddresses[i]);

		/* set TEST_SAFE bit and trigger scan clear*/
		Status = XPm_PcsrWrite(AdcAddresses[i], ADC_NPI_PCSR_MASK_TEST_SAFE_MASK |
				ADC_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
				ADC_NPI_PCSR_MASK_TEST_SAFE_MASK |
				ADC_NPI_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER;
			goto done;
		}

	}

	/* Wait 20us */
	usleep(XPM_RFCOM_HOUSECLEAN_LONG_WAIT_US);

	/* Wait for ScanClear done and check ScanClear PASS for each ADC */
	for (i = 0U; i < ArrLen; i++) {
		if (0U == AdcAddresses[i]) {
			continue;
		}

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(AdcAddresses[i] + NPI_PCSR_STATUS_OFFSET,
					ADC_NPI_PCSR_STATUS_SCAN_CLEAR_DONE_MASK, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			goto done;
		}

		/* Check Scan Clear PASS */
		if (ADC_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK !=
				(XPm_In32(AdcAddresses[i] + NPI_PCSR_STATUS_OFFSET) & ADC_NPI_PCSR_STATUS_SCAN_CLEAR_PASS_MASK)) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
			Status = XST_FAILURE;
			goto done;
		}

		/* unset TEST_SAFE bit */
		Status = XPm_PcsrWrite(AdcAddresses[i], ADC_NPI_PCSR_MASK_TEST_SAFE_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_ADC_TEST_SAFE_UNSET;
			goto done;
		}
	}

	/* Set Status to SUCCESS in case ADC is not present */
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == AdcAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(AdcAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
    return Status;


}

static XStatus InitAdcDacAddrArr(u32 *AdcAddresses, u32 *DacAddresses, const u32 AdcArrLen, const u32 DacArrLen)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	u32 i;
	u32 AdcIdx = 0, DacIdx = 0;

	if ((NULL == AdcAddresses) || (NULL == DacAddresses)){
		goto done;
	}

	for (i = (u32)XPM_NODEIDX_DEV_ADC_MIN; i <= (u32)XPM_NODEIDX_DEV_DAC_MAX; i++) {
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) || (((u32)XPM_NODETYPE_DEV_ADC != NODETYPE(Device->Node.Id))
			&& ((u32)XPM_NODETYPE_DEV_DAC != NODETYPE(Device->Node.Id)))) {
			continue;
		}

		if ((NULL == Device->Power) || ((u32)PM_POWER_PLD != Device->Power->Node.Id)) {
			continue;
		}

		if((u32)XPM_NODETYPE_DEV_ADC == NODETYPE(Device->Node.Id)){
			if (AdcIdx >= AdcArrLen){
				DbgErr = XPM_INT_ERR_ADC_INIT;
				goto done;
			}
			AdcAddresses[AdcIdx] = Device->Node.BaseAddress;
			AdcIdx++;
		}else {
			if (DacIdx >= DacArrLen){
				DbgErr = XPM_INT_ERR_DAC_INIT;
				goto done;
			}
			DacAddresses[DacIdx] = Device->Node.BaseAddress;
			DacIdx++;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/* Ub reset release for DAC*/
static XStatus DacUbEnable(const u32 *DacAddresses, u32 ArrLen){

	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 i = 0U;

	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Unlock PCSR */
		XPm_UnlockPcsr(DacAddresses[i]);

		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PSCR_CONTOL_RST_N_UBLAZE_RFDC_MASK,
				       ~DAC_NPI_PSCR_CONTOL_RST_N_UBLAZE_RFDC_MASK);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_RST_UBLAZE;
			goto done;
		}

		/*Turn on MB clocks */
		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PSCR_CONTOL_EN_UBLAZE_CLK_RFDC_MASK,
				       DAC_NPI_PSCR_CONTOL_EN_UBLAZE_CLK_RFDC_MASK);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_CLK_UBLAZE;
			goto done;
		}

		usleep(1);
		/*release reset*/
		Status = XPm_PcsrWrite(DacAddresses[i], DAC_NPI_PSCR_CONTOL_RST_N_UBLAZE_RFDC_MASK,
				       DAC_NPI_PSCR_CONTOL_RST_N_UBLAZE_RFDC_MASK);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_REL_RST_UBLAZE;
			goto done;
		}

		/*check SLEEP_UBLAZE*/
		if (DAC_NPI_PSCR_STATUS_SLEEP_UBLAZE_RFDC_MASK !=
			(XPm_In32(DacAddresses[i] + NPI_PCSR_STATUS_OFFSET) & DAC_NPI_PSCR_STATUS_SLEEP_UBLAZE_RFDC_MASK)) {
			DbgErr = XPM_INT_ERR_SLEEP_UBLAZE;
			goto done;
		}
	}
	Status = XST_SUCCESS;

done:
	for (i = 0U; i < ArrLen; i++) {
		if (0U == DacAddresses[i]) {
			continue;
		}

		/* Lock PCSR */
		XPm_LockPcsr(DacAddresses[i]);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AdcDacHouseClean(u32 SecLockDownInfo, u32 PollTimeOut)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DacAddresses[XPM_NODEIDX_DEV_DAC_MAX - XPM_NODEIDX_DEV_DAC_MIN + 1] = {0};
	u32 AdcAddresses[XPM_NODEIDX_DEV_ADC_MAX - XPM_NODEIDX_DEV_ADC_MIN + 1] = {0};

	/* Initialize array with addresses for each ADC/DAC devices */
	Status = InitAdcDacAddrArr(AdcAddresses, DacAddresses, ARRAY_SIZE(AdcAddresses), ARRAY_SIZE(DacAddresses));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*TBD: add Bisr support for DAC*/

	/* Run DacMbist if houseclean disable mask is not set*/
	if (PM_HOUSECLEAN_CHECK(DAC, MBIST)) {
		/* Run MBIST for each DAC */
		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (DacMbist),
				(DacAddresses), ARRAY_SIZE(DacAddresses), (PollTimeOut));
		/* Copy volatile to local to avoid MISRA */
		XStatus LocalStatus = StatusTmp;
		/* Required for redundancy */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_DAC_MBIST;
			XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
		}
	}

	/* Run DacScanClear if houseclean disable mask is not set*/
	if (PM_HOUSECLEAN_CHECK(DAC, SCAN)) {
		/* Run scan clear for each DAC */
		Status = DacScanClear(DacAddresses, ARRAY_SIZE(DacAddresses), PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DAC_SCAN_CLEAR;
			XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
		}
	}

	/* reset release Ub before loading the UB firmware into DAC UBLAZE data and inst memories*/
	Status = DacUbEnable(DacAddresses, ARRAY_SIZE(DacAddresses));
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_DAC_UB_ENABLE;
		XPM_GOTO_LABEL_ON_CONDITION(!IS_SECLOCKDOWN(SecLockDownInfo), done)
	}

	/* Run AdcScanClear if houseclean disable mask is not set*/
	if (PM_HOUSECLEAN_CHECK(ADC, SCAN)) {
		/* Run scan clear for each ADC */
		Status = AdcScanClear(AdcAddresses, ARRAY_SIZE(AdcAddresses), PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_ADC_SCAN_CLEAR;
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*
 * The top 2 SLRs on xcvp1902 require Laguna housecleaning which initializes
 * type-6 CLR to all one.
 * Each SLR has 6 CFRAME rows
 * Each CFRAME column has 96 CLE tiles (32 bit words), and 4 RCLK tiles.
 * Each CFRAME row has 117 Laguna columns
 */
static XStatus LagunaHouseclean(void)
{
	XStatus Status = XST_FAILURE;
	u32 FrameData[100U];
	u32 FrameAddr;
	u32 CframeRow;
	u32 CFrameAddr;
	u32 FdriAddr;
	u32 Idx;
	const XPm_PlDomain *Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);

	if (NULL == Pld) {
		goto done;
	}

	/*
	 * The first 33 frame tiles are non inter-SLR Laguna tiles.
	 * These tiles should be unaffected, so write 0
	 */
	for (Idx = 0U; Idx < 33U; Idx++) {
		FrameData[Idx] = 0U;
	}

	/*
	 * Remaining Laguna tiles below RCLK are inter-SLR vertical Laguna
	 * tiles, write 1 to all.
	 */
	for (; Idx < 48U; Idx++) {
		FrameData[Idx] = 0xFFFFFFFFU;
	}

	/* There are 4 RCLK tiles, tiles 48 - 51, set to default 0 */
	for (; Idx < 52U; Idx++) {
		FrameData[Idx] = 0U;
	}

	/* All remaining tiles above RCLK are inter-SLR vertical Laguna tiles */
	for (; Idx < 100U; Idx++) {
		FrameData[Idx] = 0xFFFFFFFFU;
	}

	/* Configure Frame address */
	FrameAddr = (u32)FRAME_BLOCK_TYPE_6 << (u32)CFRAME0_REG_FAR_BLOCKTYPE_SHIFT;
	/* Get CFRAME top row number */
	CframeRow = (XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET) &
		     (u32)CFU_APB_CFU_ROW_RANGE_NUM_MASK) - 1U;

	/* Get CFRAME Address */
	CFrameAddr = Pld->Cframe0RegBaseAddr + (XCFRAME_FRAME_OFFSET * CframeRow);

	/* Enable CFRAME Row */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_ROWON);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(300U);

	/* Enable write configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_WCFG);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(200U);

	/* Set Frame address register */
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x0U, FrameAddr);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0xCU, 0x0U);

	FdriAddr = CFRAME0_FDRI_BASEADDR + (XCFRAME_FRAME_OFFSET * CframeRow);

	/* Write 100 words of Frame data */
	for (Idx = 0U; Idx < 100U; Idx++) {
		XPm_Out32(FdriAddr, FrameData[Idx]);
		FdriAddr += 0x4U;
	}

	/*
	 * Use multi-frame write to set the remaining type-6 frames without
	 * resending data.
	 * NOTE: May need to make generic depending
	 */
	for (Idx = 1U; Idx < ((u32)LAGUNA_WIDTH - (u32)LAGUNA_HORIZONTAL_INTER_SLR_WIDTH); Idx++) {
		++FrameAddr;
		XPm_Out32((CFrameAddr + CFRAME_REG_FAR_MFW_OFFSET) + 0x0U, FrameAddr);
		XPm_Out32((CFrameAddr + CFRAME_REG_FAR_MFW_OFFSET) + 0x4U, 0x0U);
		XPm_Out32((CFrameAddr + CFRAME_REG_FAR_MFW_OFFSET) + 0x8U, 0x0U);
		XPm_Out32((CFrameAddr + CFRAME_REG_FAR_MFW_OFFSET) + 0xCU, 0x0U);

		/* nop delay */
		XPm_Wait(30U);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus PlHouseCleanEarlyBoot(u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;

	/* Enable ROWON */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST, XCFRAME_CMD_REG_ROWON);

	/* HCLEANR type 3,4,5,6 */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST, XCFRAME_CMD_REG_HCLEANR);

	/* HB BISR REPAIR */
	Status = XPmBisr_Repair(DCMAC_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_DCMAC_BISR_REPAIR;
		goto done;
	}
	Status = XPmBisr_Repair(HSC_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_HSC_BISR_REPAIR;
		goto done;
	}
	Status = XPmBisr_Repair(ILKN_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_ILKN_BISR_REPAIR;
		goto done;
	}
	Status = XPmBisr_Repair(MRMAC_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MRMAC_BISR_REPAIR;
		goto done;
	}
	Status = XPmBisr_Repair(SDFEC_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_SDFEC_BISR_REPAIR;
		goto done;
	}

	/* BRAM/URAM TRIM */
	PldApplyTrim(XPM_PL_TRIM_BRAM);
	PldApplyTrim(XPM_PL_TRIM_URAM);

	/* BRAM/URAM repair */
	Status = XPmBisr_Repair(BRAM_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_BRAM_BISR_REPAIR;
		goto done;
	}
	Status = XPmBisr_Repair(URAM_TAG_ID);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_URAM_BISR_REPAIR;
		goto done;
	}

	/* HCLEAN type 0,1,2 */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST, XCFRAME_CMD_REG_HCLEAN);

	/* Laguna housecleaning for xcvp1902 device on top 2 SLRs */
	if (PMC_TAP_IDCODE_DEV_SBFMLY_VP1902 == (XPm_GetIdCode() & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		/* Laguna Housecleaning sequence only applies to the top SLRs (SLR1 and SLR2) */
		if ((SSIT_TOP_SLR_1 == XPlmi_GetSlrIndex()) || (SSIT_TOP_SLR_2 == XPlmi_GetSlrIndex())) {
			Status = LagunaHouseclean();
			if (XST_SUCCESS != Status) {
				*DbgErr = XPM_INT_ERR_LAGUNA_HOUSECLEAN;
			}
		}
	}

done:
	return Status;
}

static XStatus PlHcScanClear(const XPm_PlDomain *Pld, u16 *DbgErr, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u32 PlatformVersion = XPm_GetPlatformVersion();

	/* PL scan clear / MBIST */
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
		CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET),
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_MASK;
		goto done;
	}

	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
		CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK);
	/* Check that the register value written properly or not! */
	PmChkRegMask32((Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET),
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK,
			CFU_APB_CFU_FGCR_SC_HBC_TRIGGER_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		*DbgErr = XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_FGCR;
		goto done;
	}

	/* Poll for status */
	XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for Hard Block Scan Clear / MBIST complete...", __func__);
	Status = XPm_PollForMask(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET,
				 CFU_APB_CFU_STATUS_SCAN_CLEAR_DONE_MASK,
				 PollTimeOut);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
		/** HACK: Continuing even if CFI SC is not completed for ES1 */
		if ((PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
			Status = XST_SUCCESS;
		} else {
			*DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			goto done;
		}
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");
	}

	/* Check if Scan Clear Passed */
	if ((XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_STATUS_OFFSET) &
			(u32)CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) !=
			(u32)CFU_APB_CFU_STATUS_SCAN_CLEAR_PASS_MASK) {
		XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: Hard Block Scan Clear / MBIST FAILED\r\n", __func__);
		/** HACK: Continuing even if CFI SC is not pass for ES1 */
		if ((PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
			Status = XST_SUCCESS;
		} else {
			*DbgErr = XPM_INT_ERR_SCAN_PASS;
			goto done;
		}
	}

#ifdef PLM_PRINT_PERF_PL
	XPlmi_Printf(DEBUG_GENERAL, "PL House Clean completed\n\r");
#endif

done:
	return Status;
}

static XStatus PlHouseClean(u32 TriggerTime, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	const XPm_PlDomain *Pld;
	u32 Value;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DeviceType;
	u32 RegAddr;


	if (PLHCLEAN_EARLY_BOOT == TriggerTime) {
		Status = PlHouseCleanEarlyBoot(&DbgErr);
	} else {

		Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
		if (NULL == Pld) {
			DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
			goto done;
		}

		/* Poll for house clean completion */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Waiting for PL HC complete....", __func__);
		Status = XPm_PollForMask(Pld->CfuApbBaseAddr +
						CFU_APB_CFU_STATUS_OFFSET,
					 CFU_APB_CFU_STATUS_HC_COMPLETE_MASK,
					 PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PL_HC_COMPLETE_TIMEOUT;
			goto done;
		}

		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		/* VGG TRIM */
		PldApplyTrim(XPM_PL_TRIM_VGG);

		/* CRAM TRIM */
		PldApplyTrim(XPM_PL_TRIM_CRAM);

		if (!(PM_HOUSECLEAN_CHECK(PLD, PLHC))) {
			PmInfo("Skipping PL Houseclean, power node 0x%x\r\n", Pld->Domain.Power.Node.Id);
			Status = XST_SUCCESS;
			goto done;
		}
		PmInfo("Running PL Houseclean, power node 0x%x\r\n", Pld->Domain.Power.Node.Id);

		/* LAGUNA REPAIR */
		/* Read PMC_TAP to check if device is SSIT device */
		RegAddr = PMC_TAP_BASEADDR + PMC_TAP_SLR_TYPE_OFFSET;
		DeviceType = PMC_TAP_SLR_TYPE_MASK & XPm_In32(RegAddr);

		if ((SLR_TYPE_INVALID != DeviceType) &&
		    (SLR_TYPE_MONOLITHIC_DEV != DeviceType)) {
#ifdef XCVP1902
			Status = XPmBisr_Repair2(VP1902_LAGUNA_TAG_ID);
#else
			Status = XPmBisr_Repair(LAGUNA_TAG_ID);
#endif
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_LAGUNA_REPAIR;
				goto done;
			}
		}

		/* There is no status for Bisr done in hard ip. But we must ensure
		 * BISR is complete before scan clear */
		 /*TBD - Wait for how long?? Wei to confirm with DFT guys */

		/* Fake read */
		/* each register is 128 bits long so issue 4 reads */
		XPlmi_Printf(DEBUG_INFO, "INFO: %s : CFRAME Fake Read...", __func__);
		PmIn32(Pld->Cframe0RegBaseAddr + 0U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 4U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 8U, Value);
		PmIn32(Pld->Cframe0RegBaseAddr + 12U, Value);
		XPlmi_Printf(DEBUG_INFO, "Done\r\n");

		Status = PlHcScanClear(Pld, &DbgErr, PollTimeOut);
	}

	/* Compilation warning fix */
	(void)Value;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus IsPlPowerUp(const XPm_Pmc *Pmc, u16 *DbgErr, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	u32 PlPowerUpTime=0;
	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	const XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_RAM);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	if (1U != HcleanDone) {
		while (TRUE) {
			IntRailPwrSts = XPmPower_CheckPower(VccintRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
			RamRailPwrSts =  XPmPower_CheckPower(VccRamRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
			AuxRailPwrSts =  XPmPower_CheckPower(VccauxRail,
					PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);

			if ((XST_SUCCESS == IntRailPwrSts) &&
			    (XST_SUCCESS == RamRailPwrSts) &&
			    (XST_SUCCESS == AuxRailPwrSts)) {
				break;
			}

			PlPowerUpTime++;
			if (PlPowerUpTime > PollTimeOut)
			{
				XPlmi_Printf(DEBUG_GENERAL, "ERROR: PL Power Up TimeOut\n\r");
				*DbgErr = XPM_INT_ERR_POWER_SUPPLY;
				/* TODO: Request PMC to power up all required rails and wait for the acknowledgement.*/
				goto done;
			}

			/** Wait for PL power up */
			usleep(10);
		}

		/* Skip PL release delay if using sysmon */
		if ((NULL != VccintRail) && (XPM_PGOOD_SYSMON == VccintRail->Source)) {
			PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET, 1U);
		} else {
			/* Delay is required to stabilize the voltage rails */
			usleep(250);
		}

		Status = XPmPlDomain_InitandHouseclean(PollTimeOut);
		if (XST_SUCCESS != Status) {
			*DbgErr = XPM_INT_ERR_DOMAIN_INIT_AND_HC;
			goto done;
		}
	}

done:
	return Status;
}


static XStatus PldInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	XStatus SStatus = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	XStatus SocRailPwrSts = XST_FAILURE;
	const XPm_PlDomain *Pld = (XPm_PlDomain *)PwrDomain;
	u32 Platform = XPm_GetPlatform();
	u32 IdCode = XPm_GetIdCode();
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 SecLockDownInfo = GetSecLockDownInfoFromArgs(Args, NumOfArgs);
	u32 PollTimeOut = GetPollTimeOut(SecLockDownInfo, XPM_POLL_TIMEOUT);
	const u32 secLockdownState = IS_SECLOCKDOWN(SecLockDownInfo);

	const XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_RAM);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);
	const XPm_Rail *VccSocRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_SOC);
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	/* Added redundant check to avoid the glitch attack */
	if ((secLockdownState != 0U) && (IS_SECLOCKDOWN(SecLockDownInfo) != 0U)) {
		HcleanDone = 0;
	}

	/*
	 * PL housecleaning requires CFU clock to run at a lower frequency for
	 * proper operation. Divide the CFU clock frequency by 2 to reduce the
	 * current frequency.
	 */
	XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (ReduceCfuClkFreq));
	/* Required for redundancy */
	XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
		goto done;
	}

	/* If PL power is still not up, return error as PLD can't
	   be initialized */
	Status = IsPlPowerUp(Pmc, &DbgErr, PollTimeOut);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * NOTE:
	 * VNPI output reset to VCCINT connected slaves is clamped at the wrong value.
	 * To work around this in XCVC1902, NPI_RESET should be asserted through the
	 * PL_SOC Isolation and then de-asserted after PL_SOC Isolation is removed
	 */
	if (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_NPI;
			Status = XPM_ERR_RESET;
			goto done;
		}
	}

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}
	 /* Remove PMC-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	if (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_NPI;
			Status = XPM_ERR_RESET;
			goto done;
		}
	}

	/* Unlock CFU writes */
	PldCfuLock(Pld, 0U);

	if(!IS_SECLOCKDOWN(SecLockDownInfo)) {
		u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
		if ((0U == PlpdHouseCleanBypass) || (0U == LocalPlpdHCBypass)) {
			Status = PlHouseClean(PLHCLEAN_INIT_NODE, PollTimeOut);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_PL_HC;
				goto fail;
			}
		}

		Status = PldCfuInit();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_CFU_INIT;
			goto fail;
		}
	}

	if ((PLATFORM_VERSION_SILICON == Platform) || (PLATFORM_VERSION_FCV == Platform)) {
		/*House clean GTY*/
		Status = GtyHouseClean(Pld, PollTimeOut);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GTY_HC;
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s : GTY HC failed", __func__);
		}
	}

	/* Run Houseclean sequence for VDU */
	Status = VduHouseClean(PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_VDU_HC;
		goto fail;
	}

	/* Run houseclean sequence for BFR-B */
	Status = BfrbHouseClean(PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BFRB_HC;
		goto fail;
	}

	/* Run houseclean sequence for ADC/DAC */
	Status = AdcDacHouseClean(SecLockDownInfo, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ADC_DAC_HC;
		goto fail;
	}

	/* Set init_complete */
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_MASK_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);
	PmOut32(Pld->CfuApbBaseAddr + CFU_APB_CFU_FGCR_OFFSET,
		CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Enable the global signals */
	XCfupmc_SetGlblSigEn(&CfupmcIns, CFUPMC_GLB_SIG_EN);

	RamRailPwrSts = XPmPower_CheckPower(VccRamRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);

	if ((XST_SUCCESS == RamRailPwrSts) &&
	    (XST_SUCCESS == AuxRailPwrSts)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VCCAUX_VCCRAM_ISO;
			goto fail;
		}
	}

	SocRailPwrSts =  XPmPower_CheckPower(VccSocRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
	if ((XST_SUCCESS == RamRailPwrSts) && (XST_SUCCESS == SocRailPwrSts)) {
		/* Remove vccaux-vccram domain isolation */
		Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_VCCRAM_SOC, FALSE_IMMEDIATE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_VCCRAM_SOC_ISO;
			goto fail;
		}
	} else {
		Status = XST_FAILURE;
	}

	XCfupmc_GlblSeqInit(&CfupmcIns);

fail:
	/* Lock CFU writes */
	PldCfuLock(Pld, 1U);

done:
	/* Restore the CFU clock frequency */
	SStatus = RestoreCfuClkFreq();
	if (XST_SUCCESS != SStatus) {
		if (XST_SUCCESS == Status) {
			Status = XST_FAILURE;
		}
		DbgErr = XPM_INT_ERR_CFU_CLK_DIVIDER;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
* @brief This function applies NPI, PL_POR Reset and Disables NPI Clock
*
* @param      None
*
* @return      XST_FAILURE if error / XST_SUCCESS if success
*
*****************************************************************************/
static XStatus AssertResetDisableClk(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Assert NPI Reset */
	Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_NPI;
		goto done;
	}

	/* Assert POR for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_POR;
		goto done;
	}

	/* Disable NPI Clock */
	Status = XPm_SetClockState(PM_SUBSYS_PMC, PM_CLK_NPI_REF, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_DIS_NPI_REF_CLK;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
* @brief This function de-asserts NPI, PL_POR Reset and enables NPI Clock
*
* @param      Pmc BaseAddress
*
* @param      Poll Time out
*
* @return      XST_FAILURE if error / XST_SUCCESS if success
*
*****************************************************************************/
static XStatus RemoveResetEnableClk(u32 PmcBaseAddress, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BaseAddress;

	/* Enable NPI Clock */
	Status = XPm_SetClockState(PM_SUBSYS_PMC, PM_CLK_NPI_REF, 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_EN_NPI_REF_CLK;
		goto done;
	}

	/* De-assert NPI Reset */
	Status = XPmReset_AssertbyId(PM_RST_NPI, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_NPI;
		goto done;
	}

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_POR;
		goto done;
	}

	/* Check for PL POR Status is de-asserted */
	BaseAddress = PmcBaseAddress + PMC_GLOBAL_PL_STATUS_OFFSET;

	Status = XPm_PollForMask(BaseAddress,
				 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
				 PollTimeOut);
	if(XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_POR_STATUS;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
* @brief This function applies GTY workaround
*
* @param   Pointer to XPmc device
*
* @param   Poll Time out
*
* @return  XST_FAILURE if error / XST_SUCCESS if success
*
*****************************************************************************/
static XStatus GtyWorkAround(const XPm_Pmc *Pmc, u32 PollTimeOut)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = AssertResetDisableClk();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ASSERT_RST_DIS_CLK;
		goto done;
	}

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	usleep(1);

	/* Remove PMC-SOC-NPI isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	/* Enable PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	usleep(1);

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	usleep(1);

	Status = RemoveResetEnableClk(Pmc->PmcGlobalBaseAddr, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_REMOVE_RST_EN_CLK;
		goto done;
	}

	/* Enable PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, TRUE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	usleep(1);

	Status = AssertResetDisableClk();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ASSERT_RST_DIS_CLK;
		goto done;
	}

	/* Remove PL-SOC isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PL_SOC, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SOC_ISO;
		goto done;
	}

	usleep(1);

	/* Remove PMC-SOC-NPI isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC_NPI, FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_NPI_ISO;
		goto done;
	}

	Status = RemoveResetEnableClk(Pmc->PmcGlobalBaseAddr, PollTimeOut);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_REMOVE_RST_EN_CLK;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;

}

/*****************************************************************************/
/**
* @brief This function initializes and performs housecleaning for PL domain
*
* @param       PollTimeOut
*
* @return      XST_FAILURE if error / XST_SUCCESS if success
*
*****************************************************************************/
static XStatus XPmPlDomain_InitandHouseclean(u32 PollTimeOut)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	XStatus IntRailPwrSts = XST_FAILURE;
	XStatus RamRailPwrSts = XST_FAILURE;
	XStatus AuxRailPwrSts = XST_FAILURE;
	volatile u32 PlatformType = 0xFFU;
	volatile u32 PlatformTypeTmp = 0xFFU;
	u32 PlatformVersion;
	const XPm_Pmc *Pmc;
	const XPm_PlDomain *Pld;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	const XPm_Rail *VccintRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_PL);
	const XPm_Rail *VccRamRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCINT_RAM);
	const XPm_Rail *VccauxRail = (XPm_Rail *)XPmPower_GetById(PM_POWER_VCCAUX);

	/* Skip if already done */
	if (0U != HcleanDone) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Proceed only if vccint, vccaux, vccint_ram is 1 */
	IntRailPwrSts = XPmPower_CheckPower(VccintRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
	RamRailPwrSts = XPmPower_CheckPower(VccRamRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
	AuxRailPwrSts = XPmPower_CheckPower(VccauxRail,
				PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
	if ((XST_SUCCESS != IntRailPwrSts) ||
	    (XST_SUCCESS != RamRailPwrSts) ||
	    (XST_SUCCESS != AuxRailPwrSts)) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		Status = XST_FAILURE;
		goto done;
	}

#ifdef PLM_PRINT_PERF_PL
	XPlmi_Printf(DEBUG_GENERAL, "PL supply status good\n\r");
#endif

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	PlatformVersion = XPm_GetPlatformVersion();
	PlatformType = XPm_GetPlatform();
	/* Required for redundancy */
	PlatformTypeTmp = XPm_GetPlatform();
	u32 LocalPlatformType = PlatformTypeTmp; /* Copy volatile to local to avoid MISRA */

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	/* Check if housecleaning needs to be bypassed */
	if (!(PM_HOUSECLEAN_CHECK(PLD, PLHC))) {
		PlpdHouseCleanBypass = 1;
		PlpdHouseCleanBypassTmp = 1;
		PmInfo("Enabling PL Houseclean bypass, power node 0x%x\r\n", Pld->Domain.Power.Node.Id);
	}

	/* Check for PL POR Status */
	Status = XPm_PollForMask(Pmc->PmcGlobalBaseAddr +
				 PMC_GLOBAL_PL_STATUS_OFFSET,
				 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
				 PollTimeOut);
	if(XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_POR_STATUS;
		goto done;
	}

	PmOut32(Pmc->PmcAnalogBaseAddr + PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET,
		PMC_ANLG_CFG_POR_CNT_SKIP_OFFSET_VAL_MASK);

	/* Workaround for GT MBIST/Memory Access/PCSR access issues */
	Status = GtyWorkAround(Pmc, PollTimeOut);
	if (XST_SUCCESS !=  Status) {
		DbgErr = XPM_INT_ERR_GT_WORKAROUND;
		goto done;
	}

	if ((PLATFORM_VERSION_SILICON == PlatformType) &&
	    (PLATFORM_VERSION_SILICON == LocalPlatformType) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/*
		 * There is a bug with ES1, due to which a small
		 * percent (<2%) of device may miss pl_por_b during power,
		 * which could result CFRAME wait up in wrong state. The work
		 * around requires to toggle PL_POR twice after PL supplies is
		 * up.
		 */

		// Disable PUDC_B pin to allow PL_POR to toggle
		XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

		/* Toggle PL POR */
		Status = XPmReset_AssertbyId(PM_RST_PL_POR, (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PL_POR;
			goto done;
		}

		/* Check for PL POR Status */
		/* This check is repeated due to ES1 workaround where PL POR is toggled again */
		Status = XPm_PollForMask(Pmc->PmcGlobalBaseAddr +
					 PMC_GLOBAL_PL_STATUS_OFFSET,
					 PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK,
					 PollTimeOut);
		if(XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PL_STATUS_TIMEOUT;
			goto done;
		}

		// Reset to allow PUDC_B pin to function
		XPm_RMW32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PUDC_B_OVERRIDE_OFFSET,
				PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK,
				~PMC_GLOBAL_PUDC_B_OVERRIDE_VAL_MASK);

		/*
		 * Clear sticky ERROR and interrupt status (They are not
		 * cleared by PL_POR). Otherwise, once ERROR/interrupt is
		 * enabled by PLM, PLM may behave incorrectly.
		 */
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_GIC_PROXY_BASE_OFFSET +
				GIC_PROXY_GROUP_OFFSET(3U) +
				GIC_PROXY_IRQ_STATUS_OFFSET,
				GICP3_CFRAME_SEU_MASK | GICP3_CFU_MASK);
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_ERR1_STATUS_OFFSET,
				PMC_GLOBAL_ERR1_STATUS_CFU_MASK |
				PMC_GLOBAL_ERR1_STATUS_CFRAME_MASK);
		XPm_Write32(Pmc->PmcGlobalBaseAddr +
				PMC_GLOBAL_ERR2_STATUS_OFFSET,
				PMC_GLOBAL_ERR2_STATUS_CFI_MASK |
				PMC_GLOBAL_ERR2_STATUS_CFRAME_SEU_CRC_MASK |
				PMC_GLOBAL_ERR2_STATUS_CFRAME_SEU_ECC_MASK);
	}


#ifdef PLM_PRINT_PERF_PL
	XPlmi_Printf(DEBUG_GENERAL, "PL POR B status good\n\r");
#endif

	/* Remove SRST for PL */
	Status = XPmReset_AssertbyId(PM_RST_PL_SRST, (u32)PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PL_SRST;
		goto done;
	}

	Status = PldCframeInit();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CFRAME_INIT;
		goto done;
	}

	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_PL_CFRAME,
					  FALSE_IMMEDIATE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_PL_CFRAME_ISO;
		goto done;
	}

	u32 LocalPlpdHCBypass = PlpdHouseCleanBypassTmp; /* Copy volatile to local to avoid MISRA */
	if ((0U == PlpdHouseCleanBypass) || (0U == LocalPlpdHCBypass)) {
		PmInfo("Running PL Houseclean, power node 0x%x\r\n", Pld->Domain.Power.Node.Id);

		XSECURE_TEMPORAL_IMPL((Status), (StatusTmp), (PlHouseClean), (PLHCLEAN_EARLY_BOOT),
			(PollTimeOut));
		/* Required for redundancy */
		XStatus LocalStatus = StatusTmp; /* Copy volatile to local to avoid MISRA */
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != LocalStatus)) {
			DbgErr = XPM_INT_ERR_PL_HC;
			goto done;
		}
	}

	/* Set the flag */
	HcleanDone = 1;

done:
	if (XPM_INT_ERR_POWER_SUPPLY != DbgErr) {
		XPm_PrintDbgErr(Status, DbgErr);
	}

	return Status;
}

static const struct XPm_PowerDomainOps PlDomainOps = {
	.InitStart = &PldInitStart,
	.InitFinish = &PldInitFinish,
	.PlHouseclean = NULL,
	/* Mask to indicate which Ops are present */
	.InitMask = (BIT16(FUNC_INIT_START) |
		     BIT16(FUNC_INIT_FINISH))
};

XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, const u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmPowerDomain_Init(&PlDomain->Domain, Id, BaseAddress, Parent, &PlDomainOps);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		goto done;
	}

	/* Make sure enough base addresses are being passed */
	if (2U <= OtherBaseAddressCnt) {
		PlDomain->CfuApbBaseAddr = OtherBaseAddresses[0];
		PlDomain->Cframe0RegBaseAddr = OtherBaseAddresses[1];
		Status = XST_SUCCESS;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_BASEADDR;
		Status = XST_FAILURE;
	}

	/* Clear PLD section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_2, PM_HOUSECLEAN_DISABLE_PLD_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPlDomain_RetriggerPlHouseClean(void)
{
	XStatus Status = XST_FAILURE;
	XStatus SStatus = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDomain *Pld;

	/*
	 * PL housecleaning requires CFU clock to run at a lower frequency for
	 * proper operation. Divide the CFU clock frequency by 2 to reduce the
	 * current frequency.
	 */
	Status = ReduceCfuClkFreq();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}
	HcleanDone = 0U;

	Status = XPmPlDomain_InitandHouseclean(XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unlock CFU writes */
	PldCfuLock(Pld, 0U);

	Status = PlHouseClean(PLHCLEAN_INIT_NODE, XPM_POLL_TIMEOUT);

	/* Lock CFU writes */
	PldCfuLock(Pld, 1U);

done:
	/* Restore the CFU clock frequency */
	SStatus = RestoreCfuClkFreq();
	if (XST_SUCCESS != SStatus) {
		if (XST_SUCCESS == Status) {
			Status = XST_FAILURE;
		}
		DbgErr = XPM_INT_ERR_CFU_CLK_DIVIDER;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

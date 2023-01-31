/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_debug.h"
#include "xpm_debug.h"
#include "xcframe.h"
#include "xpm_regs.h"

/* CFRAME Driver Instance */
static XCframe CframeIns = {0};
/**
 * If TRIM_CRAM[31:0]=0 (FUSE not programmed),
 * Use Dynamic read voltage and 4 Legs setting for keeper Bias
 */
#define CRAM_TRIM_RW_READ_VOLTAGE	0x08000B80U


static XStatus PldInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}

static XStatus PldInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	Status = XST_SUCCESS;

	return Status;
}
static const struct XPm_PowerDomainOps PlDomainOps = {
	.InitStart = PldInitStart,
	.InitFinish = PldInitFinish,
};

XStatus XPmPlDomain_RetriggerPlHouseClean(void)
{
	/**
	 * changed to support minimum boot time xilpm
	 * this service is not supported at boot time
	 */
	PmErr("unsupported service\n");
	return XST_FAILURE;

}

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

	/*TBD: Clear PLD section of PMC RAM register reserved for houseclean disable */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus PldCframeInit(void)
{
	XStatus Status = XST_FAILURE;
	XCframe_Config *Config;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (0U != CframeIns.IsReady) {
		Status = XST_SUCCESS;
		goto done;
	}

	/**
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
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

	/**
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

XStatus XPm_PldApplyTrim(u32 TrimType)
{
	u32 TrimVal;
	XStatus Status = XST_FAILURE;
	Xuint128 VggTrim={0};

	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 Platform;
	/* TODO: This cframe init should be called at init start node */
	PldCframeInit();
	/* Read the corresponding efuse registers for TRIM values */
	switch (TrimType){
	/* Read VGG trim efuse registers */
	case XPM_PL_TRIM_VGG:
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_CFRM_VGG_0_OFFSET,
			VggTrim.Word0);
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_CFRM_VGG_1_OFFSET,
			VggTrim.Word1);
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_CFRM_VGG_2_OFFSET,
			VggTrim.Word2);
		XCframe_VggTrim(&CframeIns, &VggTrim);
		Status = XST_SUCCESS;
		break;
	/* Read CRAM trim efuse registers */
	case XPM_PL_TRIM_CRAM:
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_CRAM_OFFSET,
			TrimVal);

		Platform = XPm_GetPlatform();
		/**
		 * if eFUSE is not programmed,
		 * then set rw_read_voltages to 0.61V + 0.625V by writing
		 */
		if ((0U == TrimVal) && ((u32)PLATFORM_VERSION_SILICON == Platform)) {
			TrimVal = CRAM_TRIM_RW_READ_VOLTAGE;
		}
		XCframe_CramTrim(&CframeIns, TrimVal);
		Status = XST_SUCCESS;
		break;
	/* Read BRAM trim efuse registers */
	case XPM_PL_TRIM_BRAM:
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_BRAM_OFFSET,
			TrimVal);
		XCframe_BramTrim(&CframeIns, TrimVal);
		Status = XST_SUCCESS;
		break;
	/* Read URAM trim efuse registers */
	case XPM_PL_TRIM_URAM:
		PmIn32(EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TRIM_URAM_OFFSET,
			TrimVal);
		XCframe_UramTrim(&CframeIns, TrimVal);
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_TRIM_TYPE;
		Status = XST_FAILURE;
		break;
	}
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

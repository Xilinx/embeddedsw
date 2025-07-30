/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*
 * File name: xpm_repair_me2ps.c
 * THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 * It is used for aie2ps module repair
 */

#include    "xpm_repair.h"

XStatus XPmRepair_Aie2p_s(u32 * EfuseTagAddr)
{
	XStatus Status = XST_FAILURE;
	u32 TagIdWd = *EfuseTagAddr++;
	u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
	u32 TagOptional = (TagIdWd & PMC_EFUSE_BISR_OPTIONAL_MASK) >> PMC_EFUSE_BISR_OPTIONAL_SHIFT;

	u64 BaseAddr =(u64)(VIVADO_ME_BASEADDR + ((u64)TagOptional << ME_BISR_EFUSE_OFFSET_SHIFT));    // aie2ps_pl_module_x_0 base address
	u64 BisrDataDestAddr = BaseAddr +(u64)(AIE2PS_PL_MODULE_0_0_BISR_CACHE_DATA0 - AIE2PS_PL_MODULE_0_0_BASEADDR);     // ME_PL_MODULE_X_0_BISR_cache_data0
	if(TagSize > 8U){ /*Sanity check incase user enters incorrect data size*/
		Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
		goto done;
	}
	/* Copy repair data */
	XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger Bisr Repair */
	swea((BaseAddr +(u64)(AIE2PS_PL_MODULE_0_0_BISR_CACHE_CTRL - AIE2PS_PL_MODULE_0_0_BASEADDR)), AIE2PS_PL_MODULE_0_0_BISR_CACHE_CTRL_TRIGGER_MASK);
	/* Wait for Bisr to finish */
	u64 RegAddr = BaseAddr + (AIE2PS_PL_MODULE_0_0_BISR_CACHE_STATUS - AIE2PS_PL_MODULE_0_0_BASEADDR);
	Status = XPm_PollForMask64(RegAddr, AIE2PS_PL_MODULE_0_0_BISR_CACHE_STATUS_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		Status = ERRSTS_REPAIR_TIMEOUT;
	} else {
		if ((lwea(RegAddr) & AIE2PS_PL_MODULE_0_0_BISR_CACHE_STATUS_PASS_MASK) == 0U) {
			Status = ERRSTS_REPAIR_FUNCTION_FAILED;
		}
	}
done:
	return Status;
}

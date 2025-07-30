/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*
 * File name: xpm_repair_vcu2.c
 * THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 * It is used for vcu2 module repair
 */

#include    "xpm_repair.h"

XStatus XPmRepair_Vcu2(u32 * EfuseTagAddr)
{
	XStatus Status = XST_FAILURE;
	u32 TagIdWd = *EfuseTagAddr++;
	u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
	u32 TagOptional = (TagIdWd & PMC_EFUSE_BISR_OPTIONAL_MASK) >> PMC_EFUSE_BISR_OPTIONAL_SHIFT;

	u32 RegAddrOffset = (VCU2_NPI_0_BISR_CACHE_DATA_SSSS_0 - VCU2_NPI_0_BASEADDR);

	u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);     // Module Base Address
	u64 BisrDataDestAddr = (u64)BaseAddr + (u64)RegAddrOffset;
	if (TagSize > 8U) { /*Sanity check incase user enters incorrect data size*/
		Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
		goto done;
	}
	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Copy repair data */
	XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(BaseAddr + NPI_PCSR_MASK_OFFSET, VCU2_NPI_0_MMD_NPI_PCSR_MASK_BISR_TRIGGER_MASK);
	XPm_Out32(BaseAddr + NPI_PCSR_CTRL_OFFSET, VCU2_NPI_0_MMD_NPI_PCSR_CONTROL_BISR_TRIGGER_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + NPI_PCSR_STATUS_OFFSET, VCU2_NPI_0_MMD_NPI_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		Status = ERRSTS_REPAIR_TIMEOUT;
		goto done;
	}

	/* Check for BISR pass */
	u32 RegValue = XPm_In32(BaseAddr + NPI_PCSR_STATUS_OFFSET);
	if ((RegValue & (u32)VCU2_NPI_0_MMD_NPI_PCSR_STATUS_BISR_PASS_MASK) == 0U) {
		Status = ERRSTS_REPAIR_FUNCTION_FAILED;
	} else {
		Status = XST_SUCCESS;
	}
done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);
	return Status;
}

/* ******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*  File name: xpm_repair_lpd_fpd_pmxc.c
 *  THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 *  It is used for PMXC device LPD and FPD repair
 */

#include "xpm_repair.h"

/* FPX repair */
#define NUM_OF_BISR_CACHE_DATA_REGIONS		5U
#define BISR_CACHE_SUB_SIZE			16U

XStatus XPmRepair_Lpd(u32 * EfuseTagAddr)
{
	XStatus Status = XST_FAILURE;
	u32 TagIdWd = *EfuseTagAddr++;
	u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
	if (TagSize > 16U) { /*Sanity check to verify that data size does not exceed register count*/
		Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
		goto done;
	}

	u32 BisrDataDestAddr = (u32)LPD_SLCR_BISR_CACHE_DATA_0;

	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, SET_ALLBITS_TO_ONE);
	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, 0U);
	XPmBisr_CopyStandard(EfuseTagAddr, TagSize, (u64)BisrDataDestAddr);

	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_1, LPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK, SET_ALLBITS_TO_ONE);
	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK, SET_ALLBITS_TO_ONE);

	/* Check if repair done */
	u32 RegValue = ( LPD_SLCR_BISR_CACHE_STATUS_DONE_GLOBAL_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_10_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_9_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_8_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_7_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_6_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_5_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_4_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_3_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_2_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_1_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_DONE_0_MASK);

	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS, RegValue, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		Status = ERRSTS_REPAIR_TIMEOUT;
		goto done;
	}

	/* Check if repair pass */
	RegValue =	( LPD_SLCR_BISR_CACHE_STATUS_PASS_GLOBAL_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_10_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_9_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_8_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_7_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_6_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_5_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_4_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_3_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_2_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_1_MASK
			| LPD_SLCR_BISR_CACHE_STATUS_PASS_0_MASK);

	if ((XPm_In32(LPD_SLCR_BISR_CACHE_STATUS) & RegValue) != RegValue) {
		Status =  ERRSTS_REPAIR_FUNCTION_FAILED;
	} else {
		Status = XST_SUCCESS;
	}
done:
	return Status;
}



XStatus XPmRepair_Fpd(u32 * EfuseTagAddr)
{
	XStatus Status = XST_FAILURE;
	u32 TagIdWd = *EfuseTagAddr++;
	u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
	if (TagSize > 70U) { /*Sanity check to verify that data size does not exceed register count*/
		Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
		goto done;
	}
	/* BISR register space is not continuous */
	const u32 BisrDataDestAddr[NUM_OF_BISR_CACHE_DATA_REGIONS] = { FPD_SLCR_BISR_CACHE_DATA_0,
		FPD_SLCR_BISR_CACHE_DATA_16,
		FPD_SLCR_BISR_CACHE_DATA_32,
		FPD_SLCR_BISR_CACHE_DATA_48,
		FPD_SLCR_BISR_CACHE_DATA_64
	};

	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, SET_ALLBITS_TO_ONE);
	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, 0U);

	/* Copy maximum 16 words per non-continuous BISR_CACHE register space*/
	u32 i = 0U;
	while (0U < TagSize) {
		u32 TempTagSize = (BISR_CACHE_SUB_SIZE <= TagSize) ? BISR_CACHE_SUB_SIZE : TagSize;
		XPmBisr_CopyStandard(EfuseTagAddr, TempTagSize, (u64)BisrDataDestAddr[i]);
		EfuseTagAddr += TempTagSize;	   // None issue for PMX, but may have issue for PMC
		TagSize -= TempTagSize;
		i++;
	}

	/* We write to all cache ctrl registers despite only half being used. Should not affect BISR repair. */
	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_1, FPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK, SET_ALLBITS_TO_ONE);
	/* Trigger BISR Repair */
	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK, SET_ALLBITS_TO_ONE);


	/* Poll for all done */
	/* PMXC FPD APU Clusters have 2 cores, half the registers are unused (15,16) */
	u32 RegValue = (FPD_SLCR_BISR_CACHE_STATUS1_DONE_20_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_DONE_19_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_DONE_18_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_DONE_17_MASK);

	Status = XPm_PollForMask(FPD_SLCR_BISR_CACHE_STATUS1, RegValue, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		Status = ERRSTS_REPAIR_TIMEOUT;
		goto done;
	}

	/* PMXC FPD APU Clusters have 2 cores, half the registers are unused (3,4, 7,8, 11,12) */
	RegValue = (FPD_SLCR_BISR_CACHE_STATUS0_DONE_14_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_13_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_10_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_9_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_6_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_5_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_2_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_1_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_0_MASK
		    | FPD_SLCR_BISR_CACHE_STATUS0_DONE_MASK);
	Status = XPm_PollForMask(FPD_SLCR_BISR_CACHE_STATUS0, RegValue, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		Status = ERRSTS_REPAIR_TIMEOUT;
		goto done;
	}

	/* Check if repair pass */
	/* PMXC FPD APU Clusters have 2 cores, half the registers are unused (15,16) */
	u32 BisrFullPassMask = (FPD_SLCR_BISR_CACHE_STATUS1_PASS_20_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_PASS_19_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_PASS_18_MASK
			| FPD_SLCR_BISR_CACHE_STATUS1_PASS_17_MASK );
	RegValue = XPm_In32(FPD_SLCR_BISR_CACHE_STATUS1);
	if ((RegValue & BisrFullPassMask) != BisrFullPassMask) {
		Status = ERRSTS_REPAIR_FUNCTION_FAILED;
		goto done;
	}

	/* PMXC FPD APU Clusters have 2 cores, half the registers are unused (3,4, 7,8, 11,12) */
	BisrFullPassMask = (FPD_SLCR_BISR_CACHE_STATUS0_PASS_14_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_13_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_10_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_9_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_6_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_5_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_2_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_1_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_0_MASK
			| FPD_SLCR_BISR_CACHE_STATUS0_PASS_MASK    );
	RegValue = XPm_In32(FPD_SLCR_BISR_CACHE_STATUS0);
	if ((RegValue & BisrFullPassMask) != BisrFullPassMask) {
		Status = ERRSTS_REPAIR_FUNCTION_FAILED;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

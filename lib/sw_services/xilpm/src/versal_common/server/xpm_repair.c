/******************************************************************************
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
#include "xpm_repair.h"

#define VDU_NPI_CACHE_DATA_REGISTER_OFFSET		0x104

/* VDU Repair */
#define VDU_PCSR_BISR_TRIGGER_MASK			0x02000000
#define VDU_PCSR_STATUS_BISR_DONE_MASK			0x00010000
#define VDU_PCSR_STATUS_BISR_PASS_MASK			0x00020000

/* FPx Repair */
#define NUM_OF_BISR_CACHE_DATA_REGIONS          5U
#define BISR_CACHE_SUB_SIZE                     16U

XStatus XPmRepair_Vdu(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u64 BisrDataDestAddr;
	u32 BaseAddr;

	BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr + (u64)VDU_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(BaseAddr + NPI_PCSR_MASK_OFFSET, VDU_PCSR_BISR_TRIGGER_MASK);
	XPm_Out32(BaseAddr + NPI_PCSR_CONTROL_OFFSET, VDU_PCSR_BISR_TRIGGER_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + NPI_PCSR_STATUS_OFFSET, VDU_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check for BISR PASS */
	RegValue = XPm_In32(BaseAddr + NPI_PCSR_STATUS_OFFSET);
	if ((RegValue & (u32)VDU_PCSR_STATUS_BISR_PASS_MASK) != (u32)VDU_PCSR_STATUS_BISR_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);
	return Status;
}

XStatus XPmRepair_Lpx(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u64 BisrDataDestAddr = (u64)LPD_SLCR_BISR_CACHE_DATA_0;
	u32 RegValue = 0U;

	/* Unused argument */
	(void)TagOptional;

	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK);
	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, 0U);
	*TagDataAddr = XPmBisr_CopyStandard((u32)EfuseTagAddr, TagSize, BisrDataDestAddr);

	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_1, LPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK, LPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK);
	XPm_RMW32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK, LPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK);

	/* Check if repair done */
	RegValue |= (LPD_SLCR_BISR_CACHE_STATUS_DONE_GLOBAL_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_DONE_4_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_DONE_3_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_DONE_2_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_DONE_1_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_DONE_0_MASK);

	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS, RegValue, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if repair pass */
	RegValue = 0U;
	RegValue |= (LPD_SLCR_BISR_CACHE_STATUS_PASS_GLOBAL_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_PASS_4_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_PASS_3_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_PASS_2_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_PASS_1_MASK
		     | LPD_SLCR_BISR_CACHE_STATUS_PASS_0_MASK);

	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS, RegValue, XPM_POLL_TIMEOUT);

done:
	return Status;
}

XStatus XPmRepair_Fpx(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue = 0U;
	u32 i = 0U;
	u32 TempTagSize;

	/* Unused argument */
	(void)TagOptional;

	/* BISR register space is not continuous */
	u64 BisrDataDestAddr[NUM_OF_BISR_CACHE_DATA_REGIONS] = { (u64)FPD_SLCR_BISR_CACHE_DATA_0,
								 (u64)FPD_SLCR_BISR_CACHE_DATA_16,
								 (u64)FPD_SLCR_BISR_CACHE_DATA_32,
								 (u64)FPD_SLCR_BISR_CACHE_DATA_48,
								 (u64)FPD_SLCR_BISR_CACHE_DATA_64 };

	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK);
	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK, 0U);

	/* Copy maximum 16 words to the non-continuous BISR_CACHE register space*/
	while (0U < TagSize)
	{
		TempTagSize = (TagSize >= BISR_CACHE_SUB_SIZE) ? BISR_CACHE_SUB_SIZE : TagSize;
		*TagDataAddr = XPmBisr_CopyStandard((u32)EfuseTagAddr, TempTagSize, BisrDataDestAddr[i]);
		EfuseTagAddr += (TempTagSize * 4U);
		TagSize -= TempTagSize;
		i++;
	}

	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_1, FPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK, FPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK);
	XPm_RMW32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK, FPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK);

	/* Check if repair pass */
	RegValue |= (FPD_SLCR_BISR_CACHE_STATUS1_PASS_20_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS1_PASS_19_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS1_PASS_18_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS1_PASS_17_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS1_PASS_16_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS1_PASS_15_MASK);

	Status = XPm_PollForMask(FPD_SLCR_BISR_CACHE_STATUS1, RegValue, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	RegValue = 0U;
	RegValue |= (FPD_SLCR_BISR_CACHE_STATUS0_PASS_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_14_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_13_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_12_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_11_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_10_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_9_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_8_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_7_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_6_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_5_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_4_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_3_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_2_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_1_MASK
		     | FPD_SLCR_BISR_CACHE_STATUS0_PASS_0_MASK);

	Status = XPm_PollForMask(FPD_SLCR_BISR_CACHE_STATUS0, RegValue, XPM_POLL_TIMEOUT);

done:
	return Status;
}

XStatus XPmRepair_Hnicx_Nthub(u32 EfuseTagAddr, u32 TagSize,
				u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u64 BisrDataDestAddr;

	/* Unused argument */
	(void)TagOptional;

	BisrDataDestAddr = HNICX_NPI_0_BISR_CACHE_DATA0;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(HNICX_NPI_0_BISR_CACHE_CNTRL, HNICX_NPI_0_BISR_CACHE_CNTRL_BISR_TRIGGER_NTHUB_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(HNICX_NPI_0_BISR_CACHE_STATUS, HNICX_NPI_0_BISR_CACHE_STATUS_BISR_DONE_NTHUB_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check for BISR pass */
	RegValue = XPm_In32(HNICX_NPI_0_BISR_CACHE_STATUS);
	if ((RegValue & (u32)HNICX_NPI_0_BISR_CACHE_STATUS_BISR_PASS_NTHUB_MASK) != (u32)HNICX_NPI_0_BISR_CACHE_STATUS_BISR_PASS_NTHUB_MASK) {
		Status = XST_FAILURE;
	}

done:
	return Status;
}

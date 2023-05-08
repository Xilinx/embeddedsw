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

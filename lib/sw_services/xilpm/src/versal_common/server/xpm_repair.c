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

/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
*****************************************************************************/
#include "xpm_repair.h"

#define VDU_NPI_CACHE_DATA_REGISTER_OFFSET              0x104

/* VDU Repair */
#define VDU_PCSR_BISR_TRIGGER_MASK                      0x02000000
#define VDU_PCSR_STATUS_BISR_DONE_MASK                  0x00010000
#define VDU_PCSR_STATUS_BISR_PASS_MASK                  0x00020000

/* BFR Repair */
#define BFR_NPI_CACHE_DATA_REGISTER_OFFSET		0x00000010U
#define BFR_PCSR_MASK_REGISTER_OFFSET			0x00000000U
#define BFR_PCSR_CONTROL_REGISTER_OFFSET		0x00000004U
#define BFR_PCSR_STATUS_REGISTER_OFFSET			0x00000008U
#define BFR_PCSR_STATUS_BISR_DONE_MASK			0x00800000U
#define BFR_PCSR_STATUS_BISR_PASS_MASK			0x01000000U
#define BFR_PCSR_STATUS_POWER_STATE_BIT_ZERO_MASK	0x00080000U
#define BFR_PCSR_CONTROL_BISR_TRIGGER_MASK		0x01000000U
#define BFR_PCSR_CONTROL_PWRDN_MASK			0x00004000U
#define BFR_PCSR_CONTROL_INITSTATE_MASK			0x00000040U

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

XStatus XPmRepair_Bfrb(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	u64 BisrDataDestAddr = BaseAddr + (u64)BFR_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Deassert PWRDN */
	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_CONTROL_PWRDN_MASK);
	XPm_RMW32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET,
			BFR_PCSR_CONTROL_PWRDN_MASK, 0U);

	Status = XPm_PollForMask(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_POWER_STATE_BIT_ZERO_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_CONTROL_INITSTATE_MASK);
	XPm_RMW32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET,
			BFR_PCSR_CONTROL_INITSTATE_MASK, 1U);

	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_CONTROL_INITSTATE_MASK);
	XPm_RMW32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET,
			BFR_PCSR_CONTROL_INITSTATE_MASK, 0U);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_CONTROL_BISR_TRIGGER_MASK);
	XPm_RMW32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET,
			BFR_PCSR_CONTROL_BISR_TRIGGER_MASK,
			BFR_PCSR_CONTROL_BISR_TRIGGER_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_BISR_DONE_MASK,
			BFR_PCSR_STATUS_BISR_DONE_MASK);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_BISR_PASS_MASK,
			BFR_PCSR_STATUS_BISR_PASS_MASK);
done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);
	return Status;
}

/******************************************************************************
 * Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
#include "xpm_repair.h"

u32 XPmBisr_RepairBram(u32 EfuseTagAddr, u32 TagSize)
{
	const XPm_PlDomain *Pld;
	u32 TagRow = 0U;
	u32 TagData;
	u32 TagDataAddr;
	u32 CframeRowAddr;
	u32 BramRepairRow;
	u32 BramRepairCol;
	u32 BramRepairIndex;
	u32 BramRepairVal;
	u32 BramExtendedRepair[4];
	u32 BramRepairWord;
	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;

	TagDataAddr = EfuseTagAddr + 4U;

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		/* Return negative address, caller can identify error */
		TagDataAddr = ~0U;
		goto done;
	}

	if (NULL == EfuseCache) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);

	while (TagRow < TagSize) {
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		/* break down TAG into components */
		BramRepairRow = (TagData & CFRM_BRAM_REPAIR_ROW_MASK) >> CFRM_BRAM_REPAIR_ROW_SHIFT;
		BramRepairCol = (TagData & CFRM_BRAM_REPAIR_COL_MASK) >> CFRM_BRAM_REPAIR_COL_SHIFT;
		BramRepairIndex = (TagData & CFRM_BRAM_REPAIR_INDEX_MASK) >> CFRM_BRAM_REPAIR_INDEX_SHIFT;
		BramRepairVal = (TagData & CFRM_BRAM_REPAIR_VAL_MASK) >> CFRM_BRAM_REPAIR_VAL_SHIFT;

		/*
		 * Build address for cfrm registers based on the "row",
		 * i.e. CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		 */
		CframeRowAddr = Pld->Cframe0RegBaseAddr + (0x2000U * BramRepairRow);

		/*
		 * Construct expanded vector:
		 * - [31:0] init to 0
		 * - [4:0] set repair value
		 * - [63:32] init to 0
		 * - [95:64] init to 0
		 * - [70:64] set pair index
		 * - [78:71] set column
		 * - [81:79] set block type to BRAM
		 * - [127:96] init to 0
		 */
		BramExtendedRepair[0U] = 0U;
		BramExtendedRepair[0U] |= (BramRepairVal<<CFRM_BRAM_EXP_REPAIR_VAL_SHIFT);
		BramExtendedRepair[1U] = 0U;
		BramExtendedRepair[2U] = 0U;
		BramExtendedRepair[2U] |= (BramRepairIndex<<CFRM_BRAM_EXP_REPAIR_INDEX_SHIFT);
		BramExtendedRepair[2U] |= (BramRepairCol<<CFRM_BRAM_EXP_REPAIR_COL_SHIFT) ;
		BramExtendedRepair[2U] |= (CFRM_BRAM_EXP_REPAIR_BLK_TYPE_BRAM<<CFRM_BRAM_EXP_REPAIR_BLK_TYPE_SHIFT);
		BramExtendedRepair[3U] = 0U;

		/*
		 * Write to CFRM Reg.
		 * Address to start at = CFRM_REG + word count
		 */
		for (BramRepairWord = 0U; BramRepairWord < 4U; BramRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U)+(BramRepairWord<<2U), BramExtendedRepair[BramRepairWord]);
		}
		/* Trigger repair command */
		XPm_Out32((CframeRowAddr + 0x60U), 0xDU);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x68U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x6CU), 0x0U);
	}

done:
	return TagDataAddr;
}

u32 XPmBisr_RepairUram(u32 EfuseTagAddr, u32 TagSize)
{
	const XPm_PlDomain *Pld;
	u32 TagRow = 0U;
	u32 TagData;
	u32 TagDataAddr;
	u32 CframeRowAddr;
	u32 UramRepairRow;
	u32 UramRepairCol;
	u32 UramRepairIndex;
	u32 UramRepairVal;
	u32 UramExtendedRepair[4];
	u32 UramRepairWord;
	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;

	TagDataAddr = EfuseTagAddr + 4U;

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		/* Return negative address, caller can identify error */
		TagDataAddr = ~0U;
		goto done;
	}

	if (NULL == EfuseCache) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);

	while (TagRow < TagSize) {
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		/* Break down TAG into components */
		UramRepairRow = (TagData & CFRM_URAM_REPAIR_ROW_MASK) >> CFRM_URAM_REPAIR_ROW_SHIFT;
		UramRepairCol = (TagData & CFRM_URAM_REPAIR_COL_MASK) >> CFRM_URAM_REPAIR_COL_SHIFT;
		UramRepairIndex = (TagData & CFRM_URAM_REPAIR_INDEX_MASK) >> CFRM_URAM_REPAIR_INDEX_SHIFT;
		UramRepairVal = (TagData & CFRM_URAM_REPAIR_VAL_MASK) >> CFRM_URAM_REPAIR_VAL_SHIFT;

		/*
		 * Build address for cfrm registers based on the "row",
		 * i.e. CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		 */
		CframeRowAddr = Pld->Cframe0RegBaseAddr + (0x2000U * UramRepairRow);

		/*
		 * Construct expanded vector: BRAM Bottom
		 * - [31:0] init to 0
		 * - [5:0] set repair value
		 * - [63:32] init to 0
		 * - [95:64] init to 0
		 * - [70:64] set pair index
		 * - [76:71] set column
		 * - [81:79] set block type to BRAM
		 * - [127:96] init to 0
		 */
		UramExtendedRepair[0U] = 0U;
		UramExtendedRepair[0U] |= (UramRepairVal<<CFRM_URAM_EXP_REPAIR_VAL_SHIFT);
		UramExtendedRepair[1U] = 0U;
		UramExtendedRepair[2U] = 0U;
		UramExtendedRepair[2U] |= (UramRepairIndex<<CFRM_URAM_EXP_REPAIR_INDEX_SHIFT);
		UramExtendedRepair[2U] |= (UramRepairCol<<CFRM_URAM_EXP_REPAIR_COL_SHIFT) ;
		UramExtendedRepair[2U] |= (CFRM_URAM_EXP_REPAIR_BLK_TYPE_URAM<<CFRM_URAM_EXP_REPAIR_BLK_TYPE_SHIFT);
		UramExtendedRepair[3U] = 0U;

		/*
		 * Write Bottom to CFRM.
		 * Address to start at = CFRM_REG + word count
		 */
		for (UramRepairWord = 0U; UramRepairWord < 4U; UramRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U)+(UramRepairWord<<2U),UramExtendedRepair[UramRepairWord]);
		}
		/* Trigger repair command */
		XPm_Out32((CframeRowAddr + 0x60U), 0xDU);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x68U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x6CU), 0x0U);
	}

done:
	return TagDataAddr;
}

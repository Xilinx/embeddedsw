/******************************************************************************
 * Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xpm_repair.h"


u32 XPmBisr_RepairHardBlock(u32 EfuseTagAddr, u32 TagSize)
{
	const XPm_PlDomain *Pld;
	u32 TagPairCnt;
	u32 TagPair[2] = {0};
	u32 NumPairs;
	u32 TagDataAddr;
	u32 CframeRowAddr;
	u32 HbRepairQTile;
	u32 HbRepairCol;
	u32 HbRepairRow;
	u32 HbRepairVal[2];
	u32 HbExtendedRepair[4];
	u32 HbRepairWord;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;

	TagDataAddr = EfuseTagAddr + 4U;

	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);

	//tag size must be multiple of 2
	if ((TagSize % 2U) != 0U) {
		PmWarn("EFUSE Tag size must be multiple of 2\r\n");
		TagDataAddr += (TagSize << 2U);
		if (((EfuseTagAddr < EfuseTagBitS1Addr) && (TagDataAddr > EfuseTagBitS1Addr)) ||
			((EfuseTagAddr < EfuseTagBitS2Addr) && (TagDataAddr > EfuseTagBitS2Addr))) {
			TagDataAddr += 4U;
		}
		goto done;
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	TagPairCnt = 0U;
	NumPairs = TagSize/2U;
	while (TagPairCnt < NumPairs) {
		//get first half (row,column,qtile,value)
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}
		TagPair[0U] = XPm_In32(TagDataAddr);
		TagDataAddr += 4U;
		//get second half (value)
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}
		TagPair[1U] = XPm_In32(TagDataAddr);
		TagDataAddr += 4U;
		TagPairCnt++;

		//break down the components
		HbRepairRow = (TagPair[0U] & CFRM_HB_REPAIR_ROW_MASK) >> CFRM_HB_REPAIR_ROW_SHIFT;
		HbRepairCol = (TagPair[0U] & CFRM_HB_REPAIR_COL_MASK) >> CFRM_HB_REPAIR_COL_SHIFT;
		HbRepairQTile = (TagPair[0U] & CFRM_HB_REPAIR_QTILE_MASK) >> CFRM_HB_REPAIR_QTILE_SHIFT;

		HbRepairVal[0U] = TagPair[1U];
		HbRepairVal[1U] = TagPair[0U] & CFRM_HB_REPAIR_VAL0_MASK;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr = Pld->Cframe0RegBaseAddr + (0x2000U * HbRepairRow);

		//construct expanded vector
		// REPAIR_VALUE[31:0] = BISR Value[31:0]  (align to LSB)
		// REPAIR_VALUE[63:32] = BISR Value[52:32]
		//[95:64] init to 0
		// REPAIR_TILE[70:64] = Q-tile[4:0]  (Align to LSB)
		// REPAIR_COLUMN[78:71] = Column[1:0]    (Align to LSB)
		// REPAIR_BLK_TYPE[81:79]=3’b101   (FUSE Tag is Hard-ip, i.e. MRMAC, SDFEC etc)
		//[127:96] init to 0
		HbExtendedRepair[0U] = HbRepairVal[0U]; //[31:0 ] (from second row)
		HbExtendedRepair[1U] = HbRepairVal[1U]; //[52:32]       (from first row of efuse pair)
		HbExtendedRepair[2U] = 0U;
		HbExtendedRepair[2U] |= (HbRepairQTile<<CFRM_HB_EXP_REPAIR_QTILE_SHIFT);
		HbExtendedRepair[2U] |= (HbRepairCol<<CFRM_HB_EXP_REPAIR_COL_SHIFT);
		HbExtendedRepair[2U] |= (CFRM_HB_EXP_REPAIR_BLK_TYPE<<CFRM_HB_EXP_REPAIR_BLK_TYPE_SHIFT);
		HbExtendedRepair[3U] = 0U;

		//write to CFRM Reg
		//address to start at = CFRM_REG + word count
		for (HbRepairWord = 0U; HbRepairWord < 4U; HbRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U) + (HbRepairWord << 2U), HbExtendedRepair[HbRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr + 0x60U), 0xDU);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x68U), 0x0U);
		XPm_Out32((CframeRowAddr + 0x6CU), 0x0U);
	}

done:
	return TagDataAddr;
}

/******************************************************************************
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
#include "xpm_bisr.h"
#include "xpm_repair.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_pmc.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_pldomain.h"

/* GT Repair */
#define GTY_CACHE_DATA_REGISTER_OFFSET			(0x64U)
#define CPM5_GTYP_FIXED_BASEADDR			(0xFC000000U)
#define CPM5_GTYP_EFUSE_ENDPOINT_SHIFT			(16U)

/* BRAM Repair */
#define CFRM_BRAM_REPAIR_ROW_MASK			(0x000f0000U)
#define CFRM_BRAM_REPAIR_ROW_SHIFT			(16U)
#define CFRM_BRAM_REPAIR_COL_MASK			(0x0000fc00U)
#define CFRM_BRAM_REPAIR_COL_SHIFT			(10U)
#define CFRM_BRAM_REPAIR_INDEX_MASK			(0x000003E0U)
#define CFRM_BRAM_REPAIR_INDEX_SHIFT			(5U)
#define CFRM_BRAM_REPAIR_VAL_MASK			(0x0000001FU)
#define CFRM_BRAM_REPAIR_VAL_SHIFT			(0U)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_BRAM		(0x3UL)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_BRAM_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_BRAM_EXP_REPAIR_INDEX_SHIFT		(0U)
#define CFRM_BRAM_EXP_REPAIR_VAL_SHIFT			(0U)

/* URAM Repair */
#define CFRM_URAM_REPAIR_ROW_MASK			(0x000f0000U)
#define CFRM_URAM_REPAIR_ROW_SHIFT			(16U)
#define CFRM_URAM_REPAIR_COL_MASK			(0x0000f800U)
#define CFRM_URAM_REPAIR_COL_SHIFT			(11U)
#define CFRM_URAM_REPAIR_INDEX_MASK			(0x000007C0U)
#define CFRM_URAM_REPAIR_INDEX_SHIFT			(6U)
#define CFRM_URAM_REPAIR_VAL_MASK			(0x0000003FU)
#define CFRM_URAM_REPAIR_VAL_SHIFT			(0U)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_URAM		(0x4UL)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_URAM_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_URAM_EXP_REPAIR_INDEX_SHIFT		(0U)
#define CFRM_URAM_EXP_REPAIR_VAL_SHIFT			(0U)

u32 XPmBisr_CopyStandard(u32 EfuseTagAddr, u32 TagSize, u64 BisrDataDestAddr)
{
	u64 TagRow;
	u32 TagData;
	u32 TagDataAddr;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;

	/* EFUSE Tag Data start pos */
	TagDataAddr = EfuseTagAddr + 4U;

	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		/* Return max possible address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);

	/*v Collect Repair data from EFUSE and write to endpoint base + word offset */
	TagRow = 0U;
	while (TagRow < (u64)TagSize) {
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}
		TagData = XPm_In32(TagDataAddr);
		swea(BisrDataDestAddr + (TagRow << 2U), TagData);
		TagRow++;
		TagDataAddr += 4U;
	}

done:
	return TagDataAddr;
}

XStatus XPmBisr_RepairGty(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr, u32 TagId)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue, EfuseEndpointShift;
	u32 BaseAddr, BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Modify Base Address based on the Tag type */
	switch(TagId) {
	case GTY_TAG_ID:
	case GTYP_TAG_ID:
	case GTM_TAG_ID:
		/* GTY, GTYP and GTM lie in NPI Address space */
		BaseAddr = NPI_FIXED_BASEADDR;
		EfuseEndpointShift = NPI_EFUSE_ENDPOINT_SHIFT;
		Status = XST_SUCCESS;
		break;
	case CPM5_GTYP_TAG_ID:
		/* CPM5_GTYP lies in CPM5 Address space */
		BaseAddr = CPM5_GTYP_FIXED_BASEADDR;
		EfuseEndpointShift = CPM5_GTYP_EFUSE_ENDPOINT_SHIFT;
		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_BISR_UNSUPPORTED_ID;
		Status = XST_FAILURE;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	BaseAddr = BaseAddr + (TagOptional<< EfuseEndpointShift);
	BisrDataDestAddr = BaseAddr + GTY_CACHE_DATA_REGISTER_OFFSET;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger Bisr */
	PmOut32(BaseAddr + GTY_PCSR_MASK_OFFSET, GTY_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr + GTY_PCSR_CONTROL_OFFSET, GTY_PCSR_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(BaseAddr + GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto fail;
	}

	PmIn32(BaseAddr + GTY_PCSR_STATUS_OFFSET, RegValue);
	if ((RegValue & GTY_PCSR_STATUS_BISR_PASS_MASK) != GTY_PCSR_STATUS_BISR_PASS_MASK) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto fail;
	}

fail:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

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

XStatus XPmBisr_Repair2(u32 TagId)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u32 EfuseRowTag;
	u32 EfuseCurrAddr;
	u32 EfuseNextAddr;
	u32 ExitCodeSeen;
	u32 EfuseBisrTagId;
	u32 EfuseBisrSize;
	u32 EfuseBisrOptional;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;
	const XPm_Device *EfuseCache;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, EfuseCache, XPm_Device,
						XPmDevice_GetById, PM_DEV_EFUSE_CACHE);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	Status = XPM_STRICT_CHECK_IF_EQUAL_FOR_FUNC(StatusTmp, PLATFORM_VERSION_SILICON, u32,
							XPm_GetPlatform);
	if ((XST_SUCCESS != Status) && (XST_SUCCESS != StatusTmp)) {
		Status = XST_SUCCESS;
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);


	/* check requested ID is a valid ID */
	if (255U < TagId) {
		DbgErr = XPM_INT_ERR_BISR_UNKN_TAG_ID;
		Status = XST_FAILURE;
		goto done;
	}

	/* Scan EFUSE looking for valid tags that match requested tag, exit on 0, skip row on all 1's */
	EfuseNextAddr = EfuseCacheBaseAddr + EFUSE_CACHE_BISR_RSVD_0_OFFSET;
	ExitCodeSeen = 0U;

	while (0U == ExitCodeSeen) {
		/* Read Efuse data */
		EfuseCurrAddr = EfuseNextAddr;
		EfuseRowTag = XPm_In32(EfuseCurrAddr);
		EfuseBisrTagId = (EfuseRowTag & PMC_EFUSE_BISR_TAG_ID_MASK)>>PMC_EFUSE_BISR_TAG_ID_SHIFT;
		EfuseBisrSize = (EfuseRowTag & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
		EfuseBisrOptional = (EfuseRowTag & PMC_EFUSE_BISR_OPTIONAL_MASK)>>PMC_EFUSE_BISR_OPTIONAL_SHIFT;

		if (PMC_EFUSE_BISR_EXIT_CODE == EfuseRowTag) {
			ExitCodeSeen = 1U;
		} else if(PMC_EFUSE_BISR_SKIP_CODE == EfuseRowTag) {
			/* Skip code found, increment address */
			EfuseNextAddr += 4U;
		} else if (TagId != EfuseBisrTagId) {
			/*
			 * TagId does not match found Efuse tag. Increment
			 * to next address to check for matching TagId on next
			 * iteration.
			 */
			EfuseNextAddr = (EfuseCurrAddr + 4U);
			EfuseNextAddr += (EfuseBisrSize << 2U);
			/* Skip reserved tag bits */
			if (((EfuseCurrAddr < EfuseTagBitS1Addr) && (EfuseNextAddr > EfuseTagBitS1Addr)) ||
			    ((EfuseCurrAddr < EfuseTagBitS2Addr) && (EfuseNextAddr > EfuseTagBitS2Addr))) {
				EfuseNextAddr += 4U;
			}
		} else {
			switch(TagId) {
			case VDU_TAG_ID:
				Status = XPmRepair_Vdu(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case BFRB_TAG_ID:
				Status = XPmRepair_Bfrb(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case DDRMC5_CRYPTO_TAG_ID:
				Status = XPmRepair_Ddrmc5_Crypto(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case CPM5N_TAG_ID:
				Status = XPmRepair_Cpm5n(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case LPX_TAG_ID:
				Status = XPmRepair_Lpx(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case FPX_TAG_ID:
				Status = XPmRepair_Fpx(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case HNICX_NTHUB_TAG_ID:
				Status = XPmRepair_Hnicx_Nthub(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case HNICX_LCS_TAG_ID:
				Status = XPmRepair_Hnicx_Lcs(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case HNICX_DPU_TAG_ID:
				Status = XPmRepair_Hnicx_Dpu(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
			case DDRMC5_MAIN_TAG_ID:
				Status = XPmRepair_Ddrmc5_Main(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
				break;
#ifdef XCVP1902
			case VP1902_LAGUNA_TAG_ID:
				EfuseNextAddr = XPmRepair_Laguna_vp1902(EfuseCurrAddr, EfuseBisrSize);
				if (EfuseNextAddr != ~0U) {
					Status = XST_SUCCESS;
				}
				break;
#endif
			default:
				DbgErr = XPM_INT_ERR_BAD_TAG_TYPE;
				Status = XST_FAILURE;
				break;
			}

			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_BISR_REPAIR;
				goto done;
			}
		}

		if ((EfuseNextAddr == EfuseTagBitS1Addr) || (EfuseNextAddr == EfuseTagBitS2Addr)) {
			EfuseNextAddr += 4U;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmBisr_TagSupportCheck2(u32 TagId)
{
	XStatus Status = XST_FAILURE;
	switch(TagId) {
	case VDU_TAG_ID:
	case BFRB_TAG_ID:
	case DDRMC5_CRYPTO_TAG_ID:
	case CPM5N_TAG_ID:
	case LPX_TAG_ID:
	case FPX_TAG_ID:
	case HNICX_NTHUB_TAG_ID:
	case HNICX_LCS_TAG_ID:
	case HNICX_DPU_TAG_ID:
	case DDRMC5_MAIN_TAG_ID:
#ifdef XCVP1902
	case VP1902_LAGUNA_TAG_ID:
#endif
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

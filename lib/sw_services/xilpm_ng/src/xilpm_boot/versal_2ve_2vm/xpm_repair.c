/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
#include "xpm_repair.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_pmc.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_pldomain.h"


/* XPmBisr_CopyStandard() copies efuse data to BisrDataDestAddr for repair
 * Arguments:
 *	 BisrDataAddr	   - Address after TagID words. The first BIST data
 *	      NOTE: It Need to Increase 1 Word before copying in OLD CODE
 *	 TagSize	   - Number of words to be copied
 *	 BisrDataDestAddr  - 64 bits Destination address to be copied
 * Return:
 *	 none
 */
void XPmBisr_CopyStandard(u32 * BisrDataAddr, u32 TagSize, u64 BisrDataDestAddr)
{
	u32 TagData;

	/* Collect Repair data from EFUSE and write to endpoint base + word offset */
	for (u32 TagRow = 0U; TagRow<TagSize; TagRow++)
	{
		if (((u32)BisrDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD) || ((u32)BisrDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD)) {
			BisrDataAddr++;
		}
		TagData = *BisrDataAddr++;
		swea(BisrDataDestAddr, TagData);
		BisrDataDestAddr += 4;
	}
}

XStatus XPmcFw_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;
	/**
	 * Read the Register value
	 */
	l_RegValue = Xil_In32(RegAddr);
	/**
	 * Loop while the MAsk is not set or we timeout
	 */
	while(((l_RegValue & Mask) != Mask) && (TimeOut > 0U)){
		/**
		 * Latch up the Register value again
		 */
		l_RegValue = Xil_In32(RegAddr);
		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);
}

XStatus XPmBisr_Repair(u32 TagId)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u32 EfuseRowTag;
	u32 EfuseCurrAddr;
	u32 EfuseNextAddr;
	u32 ExitCodeSeen;
	u32 EfuseBisrTagId;
	u32 EfuseBisrSize;
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
	if ((u32)MAX_BISR_TAG_NUM <= TagId) {
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
			case LPXC_TAG_ID:
				Status = XPmRepair_Lpd(&EfuseCurrAddr);
				break;
			case FPXC_TAG_ID:
				Status = XPmRepair_Fpd(&EfuseCurrAddr);
				break;
			case VCU2_TAG_ID:
				Status = XPmRepair_Vcu2(&EfuseCurrAddr);
				break;
			case ISP2_TAG_ID:
				Status = XPmRepair_ISP2(&EfuseCurrAddr);
				break;
			case GT_TAG_ID:
				Status = XPmRepair_GtmGtyGtyp(&EfuseCurrAddr);
				break;
			case DDRMC5_CRYPTO_TAG_ID:
				Status = XPmRepair_Ddrmc5_Crypto(&EfuseCurrAddr);
				break;
			case MMI_TAG_ID:
				Status = XPmRepair_Mmi(&EfuseCurrAddr);
				break;
			case DDRMC5_MAIN_TAG_ID:
				Status = XPmRepair_Ddrmc5_Main(&EfuseCurrAddr);
				break;
			case MMI_GTYP_TAG_ID:
				Status = XPmRepair_Mmi_Gtyp(&EfuseCurrAddr);
				break;
			case AIE2PS_TAG_ID:
				Status = XPmRepair_Aie2p_s(&EfuseCurrAddr);
				break;
			case MRMAC_TAG_ID:
				EfuseNextAddr = XPmBisr_RepairHardBlock(EfuseCurrAddr, EfuseBisrSize);
				if (EfuseNextAddr != ~0U) {
					Status = XST_SUCCESS;
				}
				break;
			case BRAM_TAG_ID:
				EfuseNextAddr = XPmBisr_RepairBram(EfuseCurrAddr, EfuseBisrSize);
				if (EfuseNextAddr != ~0U) {
					Status = XST_SUCCESS;
				}
				break;
			case URAM_TAG_ID:
				EfuseNextAddr = XPmBisr_RepairUram(EfuseCurrAddr, EfuseBisrSize);
				if (EfuseNextAddr != ~0U) {
					Status = XST_SUCCESS;
				}
				break;
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

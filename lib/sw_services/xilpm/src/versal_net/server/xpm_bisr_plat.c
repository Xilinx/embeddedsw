/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_err.h"
#include "xpm_debug.h"

/* Defines */
#define TAG_ID_VALID_MASK				(0x80000000U)
#define TAG_ID_TYPE_MASK				(0x7FFFFFFFU)

#define TAG_ID_TYPE_ME					(1U)
#define TAG_ID_TYPE_CFRM_HB				(2U)
#define TAG_ID_TYPE_CFRM_BR				(3U)
#define TAG_ID_TYPE_CFRM_UR				(4U)
#define TAG_ID_TYPE_DDRMC				(5U)
#define TAG_ID_TYPE_CPM					(6U)
#define TAG_ID_TYPE_GTY					(7U)
#define TAG_ID_TYPE_LPD					(8U)
#define TAG_ID_TYPE_FPD					(9U)
#define TAG_ID_TYPE_CPM5				(10U)
#define TAG_ID_TYPE_CPM5_GTYP			(11U)
#define TAG_ID_TYPE_GTYP				(12U)
#define TAG_ID_TYPE_GTM					(13U)
#define TAG_ID_TYPE_XRAM				(14U)
#define TAG_ID_TYPE_LAGUNA             (15U)
#define TAG_ID_ARRAY_SIZE				(256U)

typedef struct XPm_NidbEfuseGrpInfo {
	u8 RdnCntl;
	u16 NpiBase;
	u8 NpiOffset;
} XPm_NidbEfuseGrpInfo;

static void XPmBisr_InitTagIdList(
		u32 (*XPmTagIdWhiteListiPtr)[TAG_ID_ARRAY_SIZE])
{
	u32 *XPmTagIdWhiteList = (u32 *)XPmTagIdWhiteListiPtr;

	XPmTagIdWhiteList[LPD_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_LPD;
	XPmTagIdWhiteList[FPD_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_FPD;
	XPmTagIdWhiteList[CPM_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_CPM;
	XPmTagIdWhiteList[CPM5_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CPM5;
	XPmTagIdWhiteList[MEA_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_ME;
	XPmTagIdWhiteList[MEB_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_ME;
	XPmTagIdWhiteList[MEC_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_ME;
	XPmTagIdWhiteList[DDRMC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_DDRMC;
	XPmTagIdWhiteList[GTY_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_GTY;
	XPmTagIdWhiteList[DCMAC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[HSC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[ILKN_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[MRMAC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[SDFEC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[BRAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_BR;
	XPmTagIdWhiteList[URAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_UR;
	XPmTagIdWhiteList[CPM5_GTYP_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CPM5_GTYP;
	XPmTagIdWhiteList[GTYP_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_GTYP;
	XPmTagIdWhiteList[GTM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_GTM;
	XPmTagIdWhiteList[XRAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_XRAM;
	XPmTagIdWhiteList[LAGUNA_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_LAGUNA;

	return;
}

static XStatus XPmBisr_TagSupportCheck(u32 TagId,
			const u32 (*XPmTagIdWhiteList)[TAG_ID_ARRAY_SIZE])
{
	XStatus Status = XST_FAILURE;
	if (TAG_ID_VALID_MASK ==
		((*XPmTagIdWhiteList)[TagId] & TAG_ID_VALID_MASK)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus XPmBisr_RepairLpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairFpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}


static XStatus XPmBisr_RepairCpm(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairCpm5(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}


static XStatus XPmBisr_RepairDdrMc(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairME(u32 EfuseTagAddr, u32 TagId,u32 TagSize,u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;
	(void)TagId;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairHardBlock(u32 EfuseTagAddr, u32 TagSize)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairXram(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagDataAddr;
	PmErr("Not implemented");
	return Status;
}

static XStatus XPmBisr_RepairLaguna(u32 EfuseTagAddr, u32 TagSize)
{
	XStatus Status = XPM_ERR_BISR;
	(void)EfuseTagAddr;
	(void)TagSize;
	PmErr("Not implemented");
	return Status;
}

XStatus XPmBisr_Repair(u32 TagId)
{
	XStatus Status = XST_FAILURE;
	u32 EfuseRowTag;
	u32 EfuseCurrAddr;
	u32 EfuseNextAddr;
	u32 ExitCodeSeen;
	u32 EfuseBisrTagId;
	u32 EfuseBisrSize;
	u32 EfuseBisrOptional;
	u32 TagType;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;
	u32 XPmTagIdWhiteList[TAG_ID_ARRAY_SIZE] = {0};

	EfuseTagBitS1Addr = (EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET);
	EfuseTagBitS2Addr = (EFUSE_CACHE_BASEADDR + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET);

	//set up the white list
	XPmBisr_InitTagIdList(&XPmTagIdWhiteList);

	//check requested ID is a valid ID
	if (TagId > 255U) {
		XPmBisr_SwError(PMC_EFUSE_BISR_UNKN_TAG_ID);
		DbgErr = XPM_INT_ERR_BISR_UNKN_TAG_ID;
		Status = XST_FAILURE;
		goto done;
	}

	//Compare to the white list to check it is a supported TAG
	if (TAG_ID_VALID_MASK == (XPmTagIdWhiteList[TagId] & TAG_ID_VALID_MASK)) {
		TagType = XPmTagIdWhiteList[TagId] & TAG_ID_TYPE_MASK;
	} else {
		XPmBisr_SwError(PMC_EFUSE_BISR_INVLD_TAG_ID);
		DbgErr = XPM_INT_ERR_BISR_INVALID_ID;
		Status = XST_FAILURE;
		goto done;
	}

	//Scan EFUSE looking for valid tags that match requested tag, exit on 0, skip row on all 1's
	EfuseNextAddr = EFUSE_CACHE_BASEADDR + EFUSE_CACHE_BISR_RSVD_0_OFFSET;
	ExitCodeSeen = 0U;

	while (0U == ExitCodeSeen) {
		//read efuse row
		EfuseCurrAddr = EfuseNextAddr;
		EfuseRowTag = XPm_In32(EfuseCurrAddr);

		if (EfuseRowTag == PMC_EFUSE_BISR_EXIT_CODE) {
			ExitCodeSeen = 1U;
		} else if(EfuseRowTag==PMC_EFUSE_BISR_SKIP_CODE) { 	//SKIP Code Found
			EfuseNextAddr += 4U;//then increment address and try again
		} else {//Within Valid range and not a skip
			//grab fields from the tag
			EfuseBisrTagId = (EfuseRowTag & PMC_EFUSE_BISR_TAG_ID_MASK)>>PMC_EFUSE_BISR_TAG_ID_SHIFT;
			EfuseBisrSize = (EfuseRowTag & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
			EfuseBisrOptional = (EfuseRowTag & PMC_EFUSE_BISR_OPTIONAL_MASK)>>PMC_EFUSE_BISR_OPTIONAL_SHIFT;
			if (XST_SUCCESS ==
			    XPmBisr_TagSupportCheck(EfuseBisrTagId,
			                            &XPmTagIdWhiteList)) {//check supported TAG_ID
				if (EfuseBisrTagId == TagId) {//check if matched TAG_ID
					switch(TagType) {
					case TAG_ID_TYPE_ME:
						Status = XPmBisr_RepairME(EfuseCurrAddr,EfuseBisrTagId,EfuseBisrSize,EfuseBisrOptional, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_LPD:
						Status = XPmBisr_RepairLpd(EfuseCurrAddr, EfuseBisrSize, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_FPD:
						Status = XPmBisr_RepairFpd(EfuseCurrAddr, EfuseBisrSize, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_CPM:
						Status = XPmBisr_RepairCpm(EfuseCurrAddr, EfuseBisrSize, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_GTY:
					case TAG_ID_TYPE_GTYP:
					case TAG_ID_TYPE_GTM:
					case TAG_ID_TYPE_CPM5_GTYP:
						Status = XPmBisr_RepairGty(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr, TagId);
						break;
					case TAG_ID_TYPE_DDRMC:
						Status = XPmBisr_RepairDdrMc(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_CFRM_BR: //BRAM repair function
						EfuseNextAddr = (u32)XPmBisr_RepairBram(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					case TAG_ID_TYPE_CFRM_UR: //URAM Repair function
						EfuseNextAddr = (u32)XPmBisr_RepairUram(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					case TAG_ID_TYPE_CFRM_HB: //HardBlock repair function
						EfuseNextAddr = (u32)XPmBisr_RepairHardBlock(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					case TAG_ID_TYPE_CPM5:
						Status = XPmBisr_RepairCpm5(EfuseCurrAddr, EfuseBisrSize, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_XRAM:
						Status = XPmBisr_RepairXram(EfuseCurrAddr, EfuseBisrSize, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_LAGUNA:
						EfuseNextAddr = (u32)XPmBisr_RepairLaguna(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					default: //block type not recognized, no function to handle it
						XPmBisr_SwError(PMC_EFUSE_BISR_BAD_TAG_TYPE);
						DbgErr = XPM_INT_ERR_BAD_TAG_TYPE;
						Status = XST_FAILURE;
						break;
					}
					if (XST_SUCCESS != Status) {
						DbgErr = XPM_INT_ERR_BISR_REPAIR;
						goto done;
					}
				} else {	//calculate the next efuse address if not matched ID
					EfuseNextAddr = (EfuseCurrAddr + 4U);//move to first data address of this tag
					EfuseNextAddr += (EfuseBisrSize << 2U); //move down number of words from the tag size
					//did we cross over tbit row in the data size space
					if (((EfuseCurrAddr < EfuseTagBitS1Addr) && (EfuseNextAddr > EfuseTagBitS1Addr)) ||
					    ((EfuseCurrAddr < EfuseTagBitS2Addr) && (EfuseNextAddr > EfuseTagBitS2Addr))) {
						EfuseNextAddr += 4U;
					}
				}
			} else { 	//Not supported
				XPmBisr_SwError(PMC_EFUSE_BISR_UNSUPPORTED_ID);
				DbgErr = XPM_INT_ERR_BISR_UNSUPPORTED_ID;
				Status = XST_FAILURE;
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

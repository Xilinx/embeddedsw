/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_npdomain.h"
#include "xpm_powerdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_core.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_device.h"
#include "xpm_psfpdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_pldomain.h"

/* Defines */
#define PMC_EFUSE_BISR_EXIT_CODE			(0U)
#define PMC_EFUSE_BISR_SKIP_CODE			(0xFFFFFFFFU)
#define PMC_EFUSE_BISR_TAG_ID_MASK			(0xFF000000U)
#define PMC_EFUSE_BISR_TAG_ID_SHIFT			(24U)
#define PMC_EFUSE_BISR_SIZE_MASK			(0x00FF0000U)
#define PMC_EFUSE_BISR_SIZE_SHIFT			(16U)
#define PMC_EFUSE_BISR_OPTIONAL_MASK			(0x0000FFFFU)
#define PMC_EFUSE_BISR_OPTIONAL_SHIFT			(0U)

#define TAG_ID_VALID_MASK				(0x80000000U)
#define TAG_ID_VALID_SHIFT				(31U)
#define TAG_ID_TYPE_MASK				(0x7FFFFFFFU)
#define TAG_ID_TYPE_SHIFT				(0U)

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
#define TAG_ID_ARRAY_SIZE				(256U)

#define PMC_EFUSE_BISR_UNKN_TAG_ID			(0x1U)
#define PMC_EFUSE_BISR_INVLD_TAG_ID			(0x2U)
#define PMC_EFUSE_BISR_BAD_TAG_TYPE			(0x3U)
#define PMC_EFUSE_BISR_UNSUPPORTED_ID			(0x4U)
#define PMC_EFUSE_BISR_CFRM_HB_BAD_SIZE			(0x5U)

#ifndef VIVADO_ME_BASEADDR
	#define VIVADO_ME_BASEADDR			(0x0200000000U)
#endif
#define ME_BISR_FIXED_OFFSET				(0x36010U)
#define ME_BISR_FIXED_OFFSET_MASK			(0x00000FFFFFU)
#define ME_BISR_FIXED_OFFSET_SHIFT			(0U)
#define ME_BISR_EFUSE_OFFSET_MASK			(0x0FFFF00000U)
#define ME_BISR_EFUSE_OFFSET_SHIFT			(20U)
#define ME_BISR_CACHE_CTRL_OFFSET			(0x36000U)
#define ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK		(0x00000001U)
#define ME_BISR_CACHE_STATUS_OFFSET			(0x36008U)
#define ME_BISR_CACHE_STATUS_BISR_DONE_MASK		(0x00000001U)
#define ME_BISR_CACHE_STATUS_BISR_PASS_MASK		(0x00000002U)

#define NPI_FIXED_BASEADDR				(0x00F6000000U)
#define NPI_FIXED_MASK					(0x00FE000000U)
#define NPI_EFUSE_ENDPOINT_SHIFT			(16U)
#define NPI_EFUSE_ENDPOINT_ADDR_MASK			(0x0001FF0000U)

#define DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET		(0x268U)
#define DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET		(0x258U)
#define DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET		(0x004U)
#define DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET		(0x000U)
#define DDRMC_NPI_PCSR_LOCK_REGISTER_OFFSET		(0x00CU)
#define DDRMC_NPI_PCSR_BISR_TRIGGER_MASK		(0x02000000U)
#define DDRMC_NPI_CACHE_STATUS_BISR_DONE_MASK		(0x00000001U)
#define DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK		(0x00000002U)
#define DDRMC_NPI_CLK_GATE_REGISTER_OFFSET		(0x24CU)
#define DDRMC_NPI_CLK_GATE_BISREN_MASK			(0x00000040U)

#define GTY_CACHE_DATA_REGISTER_OFFSET		(0x64U)

//BRAM Repair
#define CFRM_BRAM_REPAIR_ROW_MASK			(0x000f0000U)
#define CFRM_BRAM_REPAIR_ROW_SHIFT			(16U)
#define CFRM_BRAM_REPAIR_COL_MASK			(0x0000fc00U)
#define CFRM_BRAM_REPAIR_COL_SHIFT			(10U)
#define CFRM_BRAM_REPAIR_INDEX_MASK			(0x000003E0U)
#define CFRM_BRAM_REPAIR_INDEX_SHIFT			(5U)
#define CFRM_BRAM_REPAIR_VAL_MASK			(0x0000001FU)
#define CFRM_BRAM_REPAIR_VAL_SHIFT			(0U)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_BRAM		(0x3UL)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_MASK		(0x00038000U)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_WIDTH		(3U)
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_BRAM_EXP_REPAIR_COL_MASK			(0x00007F80U)
#define CFRM_BRAM_EXP_REPAIR_COL_WIDTH			(8U)
#define CFRM_BRAM_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_BRAM_EXP_REPAIR_INDEX_MASK			(0x0000007FU)
#define CFRM_BRAM_EXP_REPAIR_INDEX_WIDTH		(7U)
#define CFRM_BRAM_EXP_REPAIR_INDEX_SHIFT		(0U)
#define CFRM_BRAM_EXP_REPAIR_VAL_MASK			(0xFFFFFFFFU)
#define CFRM_BRAM_EXP_REPAIR_VAL_SHIFT			(0U)

//URAM Repair
#define CFRM_URAM_REPAIR_ROW_MASK			(0x000f0000U)
#define CFRM_URAM_REPAIR_ROW_SHIFT			(16U)
#define CFRM_URAM_REPAIR_COL_MASK			(0x0000f800U)
#define CFRM_URAM_REPAIR_COL_SHIFT			(11U)
#define CFRM_URAM_REPAIR_INDEX_MASK			(0x000007C0U)
#define CFRM_URAM_REPAIR_INDEX_SHIFT			(6U)
#define CFRM_URAM_REPAIR_VAL_MASK			(0x0000003FU)
#define CFRM_URAM_REPAIR_VAL_SHIFT			(0U)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_URAM		(0x4UL)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_MASK		(0x00038000U)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_WIDTH		(3U)
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_URAM_EXP_REPAIR_COL_MASK			(0x00007F80U)
#define CFRM_URAM_EXP_REPAIR_COL_WIDTH			(8U)
#define CFRM_URAM_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_URAM_EXP_REPAIR_INDEX_MASK			(0x0000007FU)
#define CFRM_URAM_EXP_REPAIR_INDEX_WIDTH		(7U)
#define CFRM_URAM_EXP_REPAIR_INDEX_SHIFT		(0U)
#define CFRM_URAM_EXP_REPAIR_VAL_MASK			(0xFFFFFFFFU)
#define CFRM_URAM_EXP_REPAIR_VAL_SHIFT			(0U)

//CFRM_HB Repair
#define CFRM_HB_REPAIR_QTILE_MASK			(0x03E00000U)
#define CFRM_HB_REPAIR_QTILE_SHIFT			(21U)
#define CFRM_HB_REPAIR_COL_MASK				(0x0C000000U)
#define CFRM_HB_REPAIR_COL_SHIFT			(26U)
#define CFRM_HB_REPAIR_ROW_MASK				(0xF0000000U)
#define CFRM_HB_REPAIR_ROW_SHIFT			(28U)
#define CFRM_HB_REPAIR_VAL0_MASK			(0x001FFFFFU) //value field in the first row
#define CFRM_HB_REPAIR_VAL0_SHIFT			(0U)
#define CFRM_HB_REPAIR_VAL1_MASK			(0xFFFFFFFFU) //value field in the second row
#define CFRM_HB_REPAIR_VAL1_SHIFT			(0U)
#define CFRM_HB_EXP_REPAIR_BLK_TYPE			(0x5UL)
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_MASK		(0x00038000U)
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_WIDTH		(3U)
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_HB_EXP_REPAIR_COL_MASK			(0x00007F80U)
#define CFRM_HB_EXP_REPAIR_COL_WIDTH			(8U)
#define CFRM_HB_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_HB_EXP_REPAIR_QTILE_MASK			(0x0000007FU)
#define CFRM_HB_EXP_REPAIR_QTILE_WIDTH			(7U)
#define CFRM_HB_EXP_REPAIR_QTILE_SHIFT			(0U)
#define CFRM_HB_EXP_REPAIR_VAL_MASK			(0xFFFFFFFFU)
#define CFRM_HB_EXP_REPAIR_VAL_SHIFT			(0U)
#define CFRM_HB_EXP_REPAIR_VAL1_MASK			(0x001FFFFFU)
#define CFRM_HB_EXP_REPAIR_VAL1_SHIFT			(0U)

//CPM5_GTYP Repair
#define CPM5_GTYP_FIXED_BASEADDR			(0xFC000000U)
#define CPM5_GTYP_EFUSE_ENDPOINT_SHIFT			(16U)

//NIDB Lane Repair
#define MAX_NIDB_EFUSE_GROUPS				(0x5U)
#define NPI_ROOT_BASEADDR				(NPI_BASEADDR + NPI_NIR_0_OFFSET)
#define NIDB_OFFSET_DIFF				(0x00010000U)
#define NIDB_PCSR_LOCK_OFFSET				(0x0000000CU)
#define NIDB_PCSR_MASK_OFFSET				(0x00000000U)
#define NIDB_PCSR_MASK_ODISABLE_MASK			(0x00000004U)
#define NIDB_PCSR_CONTROL_OFFSET			(0x00000004U)
#define NIDB_LANE_REPAIR_UNLOCK_OFFSET			(0x00000038U)
#define NIDB_LANE_REPAIR_UNLOCK_VAL			(0xE6172839U)
#define NIDB_REPAIR_OFFSET				(0x00000010U)

//XRAM Repair
#define XRAM_SLCR_PCSR_BISR_TRIGGER_MASK		(0x08000000U)
#define XRAM_SLCR_PCSR_BISR_CLR_MASK			(0x10000000U)
#define XRAM_SLCR_PCSR_PSR_BISR_DONE_MASK		(0x00004000U)
#define XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK		(0x00008000U)

typedef struct XPm_NidbEfuseGrpInfo {
	u8 RdnCntl;
	u16 NpiBase;
	u8 NpiOffset;
} XPm_NidbEfuseGrpInfo;

static u32 XPmTagIdWhiteList[TAG_ID_ARRAY_SIZE] = {0};

static void XPmBisr_InitTagIdList(void)
{
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
	XPmTagIdWhiteList[ILKN_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[MRMAC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[SDFEC_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_HB;
	XPmTagIdWhiteList[BRAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_BR;
	XPmTagIdWhiteList[URAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CFRM_UR;
	XPmTagIdWhiteList[CPM5_GTYP_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_CPM5_GTYP;
	XPmTagIdWhiteList[GTYP_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_GTYP;
	XPmTagIdWhiteList[GTM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_GTM;
	XPmTagIdWhiteList[XRAM_TAG_ID] = TAG_ID_VALID_MASK | TAG_ID_TYPE_XRAM;

	return;
}

static XStatus XPmBisr_TagSupportCheck(u32 TagId)
{
	if (TAG_ID_VALID_MASK == (XPmTagIdWhiteList[TagId] & TAG_ID_VALID_MASK)) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

static void XPmBisr_SwError(u32 ErrorCode)
{
	XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		goto done;
	}

	XPm_Out32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PMC_GSW_ERR_OFFSET,
		  XPm_In32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PMC_GSW_ERR_OFFSET) |
			   ((u32)1U << ErrorCode) |
			   (1UL << PMC_GLOBAL_PMC_GSW_ERR_CR_FLAG_SHIFT));

done:
	return;
}

static u32 XPmBisr_CopyStandard(u32 EfuseTagAddr, u32 TagSize, u64 BisrDataDestAddr)
{
	u64 TagRow;
	u32 TagData;
	u32 TagDataAddr;

	//EFUSE Tag Data start pos
	TagDataAddr = EfuseTagAddr + 4U;

	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		/* Return max possible address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	//Collect Repair data from EFUSE and write to endpoint base + word offset
	TagRow = 0;
	while (TagRow < (u64)TagSize) {
		if (TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
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

static XStatus XPmBisr_RepairGty(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr, u32 TagType)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue, EfuseEndpointShift;
	u32 BaseAddr, BisrDataDestAddr;

	/* Modify Base Address based on the Tag type */
	switch(TagType) {
	case TAG_ID_TYPE_GTY:
	case TAG_ID_TYPE_GTYP:
	case TAG_ID_TYPE_GTM:
		/* GTY, GTYP and GTM lie in NPI Address space */
		BaseAddr = NPI_FIXED_BASEADDR;
		EfuseEndpointShift = NPI_EFUSE_ENDPOINT_SHIFT;
		Status = XST_SUCCESS;
		break;
	case TAG_ID_TYPE_CPM5_GTYP:
		/* CPM5_GTYP lies in CPM5 Address space */
		BaseAddr = CPM5_GTYP_FIXED_BASEADDR;
		EfuseEndpointShift = CPM5_GTYP_EFUSE_ENDPOINT_SHIFT;
		Status = XST_SUCCESS;
		break;
	default:
		XPmBisr_SwError(PMC_EFUSE_BISR_UNSUPPORTED_ID);
		Status = XST_FAILURE;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	BaseAddr = BaseAddr | (TagOptional<< EfuseEndpointShift);
	BisrDataDestAddr = BaseAddr | GTY_CACHE_DATA_REGISTER_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Unlock PCSR */
	PmOut32(BaseAddr | GTY_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Trigger Bisr */
	PmOut32(BaseAddr | GTY_PCSR_MASK_OFFSET, GTY_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr | GTY_PCSR_CONTROL_OFFSET, GTY_PCSR_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(BaseAddr | GTY_PCSR_STATUS_OFFSET, GTY_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmIn32(BaseAddr | GTY_PCSR_STATUS_OFFSET, RegValue);
	if ((RegValue & GTY_PCSR_STATUS_BISR_PASS_MASK) != GTY_PCSR_STATUS_BISR_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite Trigger Bit */
	PmOut32(BaseAddr | GTY_PCSR_MASK_OFFSET, GTY_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr | GTY_PCSR_CONTROL_OFFSET, 0);

	/* Lock PCSR */
	PmOut32(BaseAddr | GTY_PCSR_LOCK_OFFSET, 1);
done:
	return Status;
}

static XStatus XPmBisr_RepairLpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u64 BisrDataDestAddr;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);

	if (NULL == LpDomain) {
		goto done;
	}

	BisrDataDestAddr = LpDomain->LpdSlcrBaseAddr + (u64)LPD_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	LpDomain->LpdBisrFlags |= LPD_BISR_DATA_COPIED;

	Status = XPmBisr_TriggerLpd();

done:
	return Status;
}

int XPmBisr_TriggerLpd(void)
{
	int Status = XST_FAILURE;
	XPm_PsLpDomain *PsLpd;
	u32 RegValue;

	PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	if (NULL == PsLpd) {
		goto done;
	}

	/* Trigger Bisr */
	PmRmw32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_BISR_CACHE_CTRL_1_OFFSET,
		(LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK | LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK),
		(LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK | LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK));

	PmRmw32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_BISR_CACHE_CTRL_0_OFFSET,
		LPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK,
		LPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_BISR_CACHE_STATUS_OFFSET,
				 (LPD_SLCR_BISR_DONE_GLOBAL_MASK |
				  LPD_SLCR_BISR_DONE_1_MASK |
				  LPD_SLCR_BISR_DONE_0_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr Status */
	PmIn32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & (LPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  LPD_SLCR_BISR_PASS_1_MASK |
				  LPD_SLCR_BISR_PASS_0_MASK)) !=
				  (LPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  LPD_SLCR_BISR_PASS_1_MASK |
				  LPD_SLCR_BISR_PASS_0_MASK)) {
		Status = XST_FAILURE;
	}

	/* Unwrite Trigger Bits */
	PmRmw32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_BISR_CACHE_CTRL_1_OFFSET,
		(LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
		 LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK), 0);
done:
	return Status;
}

static XStatus XPmBisr_RepairFpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	XPm_PsFpDomain *PsFpd;
	u32 RegValue;
	u64 BisrDataDestAddr;

	PsFpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	if (NULL == PsFpd) {
		goto done;
	}

	BisrDataDestAddr = PsFpd->FpdSlcrBaseAddr + (u64)FPD_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger Bisr */
	PmRmw32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_BISR_CACHE_CTRL_1_OFFSET,
		(FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK),
		(FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK));

	PmRmw32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_BISR_CACHE_CTRL_0_OFFSET,
		FPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK,
		FPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_BISR_CACHE_STATUS_OFFSET,
				 (FPD_SLCR_BISR_DONE_GLOBAL_MASK |
				  FPD_SLCR_BISR_DONE_3_MASK |
				  FPD_SLCR_BISR_DONE_2_MASK |
				  FPD_SLCR_BISR_DONE_1_MASK |
				  FPD_SLCR_BISR_DONE_0_MASK),
				  XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr Status */
	PmIn32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & (FPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  FPD_SLCR_BISR_PASS_3_MASK |
				  FPD_SLCR_BISR_PASS_2_MASK |
				  FPD_SLCR_BISR_PASS_1_MASK |
				  FPD_SLCR_BISR_PASS_0_MASK)) !=
				  (FPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  FPD_SLCR_BISR_PASS_3_MASK |
				  FPD_SLCR_BISR_PASS_2_MASK |
				  FPD_SLCR_BISR_PASS_1_MASK |
				  FPD_SLCR_BISR_PASS_0_MASK)) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite Trigger Bits */
        PmRmw32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_BISR_CACHE_CTRL_1_OFFSET,
		(FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
		 FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK), 0);

done:
	return Status;
}


static XStatus XPmBisr_RepairCpm(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	XPm_CpmDomain *Cpm;
	u32 RegValue;
	u64 BisrDataDestAddr;

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM);
	if (NULL == Cpm) {
		goto done;
	}

	BisrDataDestAddr = Cpm->CpmSlcrBaseAddr + (u64)CPM_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Clear BISR data test registers */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM_SLCR_BISR_CACHE_CTRL_CLR_MASK);
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0);

	/* Trigger Bisr */
	PmRmw32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK,
		CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_STATUS_OFFSET,
				 CPM_SLCR_BISR_CACHE_STATUS_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr status */
	PmIn32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) != CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0);

done:
	return Status;
}

static XStatus XPmBisr_RepairCpm5(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	XPm_CpmDomain *Cpm;
	u32 RegValue;
	u64 BisrDataDestAddr;

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PM_POWER_CPM5);
	if (NULL == Cpm) {
		goto done;
	}

	BisrDataDestAddr = Cpm->CpmSlcrBaseAddr + (u64)CPM5_SLCR_BISR_CACHE_DATA_0_OFFSET;
	/* Disable write protection */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_WPROTP_OFFSET, 0x0);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Clear BISR data test registers */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM5_SLCR_BISR_CACHE_CTRL_CLR_MASK);
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0);

	/* Trigger Bisr */
	PmRmw32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM5_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK,
		CPM5_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_STATUS_OFFSET,
				 CPM5_SLCR_BISR_CACHE_STATUS_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr status */
	PmIn32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & CPM5_SLCR_BISR_CACHE_STATUS_PASS_MASK) !=
		CPM5_SLCR_BISR_CACHE_STATUS_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite trigger bit */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0);

	/* Enable write protection */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_WPROTP_OFFSET, 0x1);

done:
	return Status;
}


static XStatus XPmBisr_RepairDdrMc(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 BaseAddr, BisrDataDestAddr;
	XPm_NpDomain *NpDomain = (XPm_NpDomain *)XPmPower_GetById(PM_POWER_NOC);

	if (NULL == NpDomain) {
		goto done;
	}

	BaseAddr = NPI_FIXED_BASEADDR | (TagOptional<<NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr | DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET;

	if (0U == NpDomain->BisrDataCopied) {
		/* Copy repair data */
		*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize,
						    BisrDataDestAddr);
		NpDomain->BisrDataCopied = 1;
	}

	/* Unlock PCSR */
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_LOCK_REGISTER_OFFSET, PCSR_UNLOCK_VAL);

	/* Enable Bisr clock */
	PmRmw32(BaseAddr | DDRMC_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC_NPI_CLK_GATE_BISREN_MASK, DDRMC_NPI_CLK_GATE_BISREN_MASK);

	/*Trigger Bisr */
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET, DDRMC_NPI_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET, DDRMC_NPI_PCSR_BISR_TRIGGER_MASK);

	/* Wait for Bisr to be done and check status */
	Status = XPm_PollForMask(BaseAddr | DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET,
				 DDRMC_NPI_CACHE_STATUS_BISR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmIn32(BaseAddr | DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET, RegValue);
	if ((RegValue & DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK) != DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Disable Bisr Clock */
	PmRmw32(BaseAddr | DDRMC_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC_NPI_CLK_GATE_BISREN_MASK, ~DDRMC_NPI_CLK_GATE_BISREN_MASK);

	/* Unwrite Trigger Bit */
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET, DDRMC_NPI_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET, 0);

	/* Lock PCSR */
	PmOut32(BaseAddr | DDRMC_NPI_PCSR_LOCK_REGISTER_OFFSET, 1);

done:
	return Status;
}

static XStatus XPmBisr_RepairME(u32 EfuseTagAddr, u32 TagId,u32 TagSize,u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 BaseAddr, BisrDataDestAddr;

	/* Compilation warning fix */
	(void)TagId;

	BaseAddr = (u32)VIVADO_ME_BASEADDR | (TagOptional << ME_BISR_EFUSE_OFFSET_SHIFT);
	BisrDataDestAddr = BaseAddr | ME_BISR_FIXED_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger Bisr */
	PmRmw32(BaseAddr | ME_BISR_CACHE_CTRL_OFFSET, ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK, ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(BaseAddr | ME_BISR_CACHE_STATUS_OFFSET,
				 ME_BISR_CACHE_STATUS_BISR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr Status */
	PmIn32(BaseAddr | ME_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & ME_BISR_CACHE_STATUS_BISR_PASS_MASK) !=
				  ME_BISR_CACHE_STATUS_BISR_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unwrite Trigger Bit */
	PmRmw32(BaseAddr | ME_BISR_CACHE_CTRL_OFFSET, ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK, 0);

done:
	return Status;
}

static u32 XPmBisr_RepairBram(u32 EfuseTagAddr, u32 TagSize)
{
	XPm_PlDomain *Pld;
	u32 TagRow = 0;
	u32 TagData;
	u32 TagDataAddr;
	u32 CframeRowAddr;
	u32 BramRepairRow;
	u32 BramRepairCol;
	u32 BramRepairIndex;
	u32 BramRepairVal;
	u32 BramExtendedRepair[4];
	u32 BramRepairWord;
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);

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

	while (TagRow < TagSize) {
		if (TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
			TagDataAddr += 4U;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		//break down TAG into components:
		BramRepairRow = (TagData & CFRM_BRAM_REPAIR_ROW_MASK) >> CFRM_BRAM_REPAIR_ROW_SHIFT;
		BramRepairCol = (TagData & CFRM_BRAM_REPAIR_COL_MASK) >> CFRM_BRAM_REPAIR_COL_SHIFT;
		BramRepairIndex = (TagData & CFRM_BRAM_REPAIR_INDEX_MASK) >> CFRM_BRAM_REPAIR_INDEX_SHIFT;
		BramRepairVal = (TagData & CFRM_BRAM_REPAIR_VAL_MASK) >> CFRM_BRAM_REPAIR_VAL_SHIFT;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr = Pld->Cframe0RegBaseAddr + (0x2000U * BramRepairRow);

		//construct expanded vector
		//[31:0] init to 0
		//[4:0] set repair value
		//[63:32] init to 0
		//[95:64] init to 0
		//[70:64] set pair index
		//[78:71] set column
		//[81:79] set block type to BRAM
		//[127:96] init to 0
		BramExtendedRepair[0] = 0;
		BramExtendedRepair[0] |= (BramRepairVal<<CFRM_BRAM_EXP_REPAIR_VAL_SHIFT);
		BramExtendedRepair[1] = 0;
		BramExtendedRepair[2] = 0;
		BramExtendedRepair[2] |= (BramRepairIndex<<CFRM_BRAM_EXP_REPAIR_INDEX_SHIFT);
		BramExtendedRepair[2] |= (BramRepairCol<<CFRM_BRAM_EXP_REPAIR_COL_SHIFT) ;
		BramExtendedRepair[2] |= (CFRM_BRAM_EXP_REPAIR_BLK_TYPE_BRAM<<CFRM_BRAM_EXP_REPAIR_BLK_TYPE_SHIFT);
		BramExtendedRepair[3] = 0;

		//write to CFRM Reg
		//address to start at = CFRM_REG + word count
		for (BramRepairWord = 0; BramRepairWord < 4U; BramRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U)+(BramRepairWord<<2), BramExtendedRepair[BramRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr + 0x60U), 0xD);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0);
		XPm_Out32((CframeRowAddr + 0x68U), 0x0);
		XPm_Out32((CframeRowAddr + 0x6CU), 0x0);

	}

done:
	return TagDataAddr;
}

static u32 XPmBisr_RepairUram(u32 EfuseTagAddr, u32 TagSize)
{
	XPm_PlDomain *Pld;
	u32 TagRow = 0;
	u32 TagData;
	u32 TagDataAddr;
	u32 CframeRowAddr;
	u32 UramRepairRow;
	u32 UramRepairCol;
	u32 UramRepairIndex;
	u32 UramRepairVal;
	u32 UramExtendedRepair[4];
	u32 UramRepairWord;
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);

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

	while (TagRow < TagSize) {
		if (TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
			TagDataAddr += 4U;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		//break down TAG into components:
		UramRepairRow = (TagData & CFRM_URAM_REPAIR_ROW_MASK) >> CFRM_URAM_REPAIR_ROW_SHIFT;
		UramRepairCol = (TagData & CFRM_URAM_REPAIR_COL_MASK) >> CFRM_URAM_REPAIR_COL_SHIFT;
		UramRepairIndex = (TagData & CFRM_URAM_EXP_REPAIR_INDEX_MASK) >> CFRM_URAM_REPAIR_INDEX_SHIFT;
		UramRepairVal = (TagData & CFRM_URAM_REPAIR_VAL_MASK) >> CFRM_URAM_REPAIR_VAL_SHIFT;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr = Pld->Cframe0RegBaseAddr + (0x2000U * UramRepairRow);

		//construct expanded vector: BRAM Bottom
		//[31:0] init to 0
		//[5:0] set repair value
		//[63:32] init to 0
		//[95:64] init to 0
		//[70:64] set pair index
		//[76:71] set column
		//[81:79] set block type to BRAM
		//[127:96] init to 0
		UramExtendedRepair[0] = 0;
		UramExtendedRepair[0] |= (UramRepairVal<<CFRM_URAM_EXP_REPAIR_VAL_SHIFT);
		UramExtendedRepair[1] = 0;
		UramExtendedRepair[2] = 0;
		UramExtendedRepair[2] |= (UramRepairIndex<<CFRM_URAM_EXP_REPAIR_INDEX_SHIFT);
		UramExtendedRepair[2] |= (UramRepairCol<<CFRM_URAM_EXP_REPAIR_COL_SHIFT) ;
		UramExtendedRepair[2] |= (CFRM_URAM_EXP_REPAIR_BLK_TYPE_URAM<<CFRM_URAM_EXP_REPAIR_BLK_TYPE_SHIFT);
		UramExtendedRepair[3] = 0;

		//write Bottom to CFRM
		//address to start at = CFRM_REG + word count
		for (UramRepairWord = 0; UramRepairWord < 4U; UramRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U)+(UramRepairWord<<2),UramExtendedRepair[UramRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr + 0x60U), 0xD);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0);
		XPm_Out32((CframeRowAddr + 0x68U), 0x0);
		XPm_Out32((CframeRowAddr + 0x6CU), 0x0);
	}

done:
	return TagDataAddr;
}

static u32 XPmBisr_RepairHardBlock(u32 EfuseTagAddr, u32 TagSize)
{
	XPm_PlDomain *Pld;
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

	TagDataAddr = EfuseTagAddr + 4U;

	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	//tag size must be multiple of 2
	if ((TagSize % 2U) != 0U) {
		XPmBisr_SwError(PMC_EFUSE_BISR_CFRM_HB_BAD_SIZE);
		TagDataAddr += (TagSize << 2);
		if ((EfuseTagAddr < (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) &&
		    TagDataAddr > (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET)) ||
		    (EfuseTagAddr < (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET) &&
		    TagDataAddr > (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET))) {
			TagDataAddr += 4U;
		}
		return TagDataAddr;
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		/* Return negative address so error can be identified by caller */
		TagDataAddr = ~0U;
		goto done;
	}

	TagPairCnt = 0;
	NumPairs = TagSize/2U;
	while (TagPairCnt < NumPairs) {
		//get first half (row,column,qtile,value)
		if (TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
			TagDataAddr += 4U;
		}
		TagPair[0] = XPm_In32(TagDataAddr);
		TagDataAddr += 4U;
		//get second half (value)
		if (TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    TagDataAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
			TagDataAddr += 4U;
		}
		TagPair[1] = XPm_In32(TagDataAddr);
		TagDataAddr += 4U;
		TagPairCnt++;

		//break down the components
		HbRepairRow = (TagPair[0] & CFRM_HB_REPAIR_ROW_MASK) >> CFRM_HB_REPAIR_ROW_SHIFT;
		HbRepairCol = (TagPair[0] & CFRM_HB_REPAIR_COL_MASK) >> CFRM_HB_REPAIR_COL_SHIFT;
		HbRepairQTile = (TagPair[0] & CFRM_HB_REPAIR_QTILE_MASK) >> CFRM_HB_REPAIR_QTILE_SHIFT;

		HbRepairVal[0] = TagPair[1];
		HbRepairVal[1] = TagPair[0] & CFRM_HB_REPAIR_VAL0_MASK;

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
		HbExtendedRepair[0] = HbRepairVal[0]; //[31:0 ] (from second row)
		HbExtendedRepair[1] = HbRepairVal[1];	//[52:32]	(from first row of efuse pair)
		HbExtendedRepair[2] = 0;
		HbExtendedRepair[2] |= (HbRepairQTile<<CFRM_HB_EXP_REPAIR_QTILE_SHIFT);
		HbExtendedRepair[2] |= (HbRepairCol<<CFRM_HB_EXP_REPAIR_COL_SHIFT);
		HbExtendedRepair[2] |= (CFRM_HB_EXP_REPAIR_BLK_TYPE<<CFRM_HB_EXP_REPAIR_BLK_TYPE_SHIFT);
		HbExtendedRepair[3] = 0;

		//write to CFRM Reg
		//address to start at = CFRM_REG + word count
		for (HbRepairWord=0; HbRepairWord < 4U; HbRepairWord++) {
			XPm_Out32((CframeRowAddr + 0x250U)+(HbRepairWord<<2),HbExtendedRepair[HbRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr + 0x60U), 0xD);
		XPm_Out32((CframeRowAddr + 0x64U), 0x0);
                XPm_Out32((CframeRowAddr + 0x68U), 0x0);
                XPm_Out32((CframeRowAddr + 0x6CU), 0x0);

	}

done:
	return TagDataAddr;
}

static XStatus XPmBisr_RepairXram(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	XPm_Device *Device = NULL;
	u32 RegValue, BaseAddr;
	u64 BisrDataDestAddr;

	/* Not possible to reach here if Device doesn't exist hence no */
	/* check for existence of Device */
	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);

	/* Calculate Destination address */
	/* Dest. Addr = slcr_address + cache_data_offset */
	BaseAddr = Device->Node.BaseAddress;
	BisrDataDestAddr = BaseAddr + (u64)XRAM_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Write unlock code to PCSR_LOCK register */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

	/* Clear the BISR Test Data */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET,
			XRAM_SLCR_PCSR_BISR_CLR_MASK);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET,
			XRAM_SLCR_PCSR_BISR_CLR_MASK);

	/* Exit the BISR Test Data Clear Mode*/
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET,  0x0);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, 0x0);

	/* Copy Data from EFUSE to BISR Cache of XRAM */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* BISR Trigger */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET,
		XRAM_SLCR_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET,
		XRAM_SLCR_PCSR_BISR_TRIGGER_MASK);

	/* Poll for BISR_DONE */
	Status = XPm_PollForMask(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET,
		XRAM_SLCR_PCSR_PSR_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check BISR Pass/Fail */
	PmIn32(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET, RegValue);
	if (XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK != (RegValue &
		XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK)) {
		Status = XPM_ERR_BISR;
		goto done;
	}

	/*  Exit the memory repair operation */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET, 0x0);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, 0x0);

	/* Write unlock code to PCSR_LOCK register */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, 0x0);

	Status = XST_SUCCESS;

done:
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
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		goto done;
	}

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	//set up the white list
	XPmBisr_InitTagIdList();

	//check requested ID is a valid ID
	if (TagId > 255U) {
		XPmBisr_SwError(PMC_EFUSE_BISR_UNKN_TAG_ID);
		Status = XST_FAILURE;
		goto done;
	}

	//Compare to the white list to check it is a supported TAG
	if (TAG_ID_VALID_MASK == (XPmTagIdWhiteList[TagId] & TAG_ID_VALID_MASK)) {
		TagType = XPmTagIdWhiteList[TagId] & TAG_ID_TYPE_MASK;
	} else {
		XPmBisr_SwError(PMC_EFUSE_BISR_INVLD_TAG_ID);
		Status = XST_FAILURE;
		goto done;
	}

	//Scan EFUSE looking for valid tags that match requested tag, exit on 0, skip row on all 1's
	EfuseNextAddr = EfuseCache->Node.BaseAddress + EFUSE_CACHE_BISR_RSVD_0_OFFSET;
	ExitCodeSeen = 0;

	while (0U == ExitCodeSeen) {
		//read efuse row
		EfuseCurrAddr = EfuseNextAddr;
		EfuseRowTag = XPm_In32(EfuseCurrAddr);

		if (EfuseRowTag == PMC_EFUSE_BISR_EXIT_CODE) {
			ExitCodeSeen = 1;
		} else if(EfuseRowTag==PMC_EFUSE_BISR_SKIP_CODE) { 	//SKIP Code Found
			EfuseNextAddr += 4U;//then increment address and try again
		} else {//Within Valid range and not a skip
			//grab fields from the tag
			EfuseBisrTagId = (EfuseRowTag & PMC_EFUSE_BISR_TAG_ID_MASK)>>PMC_EFUSE_BISR_TAG_ID_SHIFT;
			EfuseBisrSize = (EfuseRowTag & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
			EfuseBisrOptional = (EfuseRowTag & PMC_EFUSE_BISR_OPTIONAL_MASK)>>PMC_EFUSE_BISR_OPTIONAL_SHIFT;
			if (XST_SUCCESS == XPmBisr_TagSupportCheck(EfuseBisrTagId)) {//check supported TAG_ID
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
						Status = XPmBisr_RepairGty(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr, TagType);
						break;
					case TAG_ID_TYPE_DDRMC:
						Status = XPmBisr_RepairDdrMc(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
						break;
					case TAG_ID_TYPE_CFRM_BR: //BRAM repair function
						EfuseNextAddr = XPmBisr_RepairBram(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					case TAG_ID_TYPE_CFRM_UR: //URAM Repair function
						EfuseNextAddr = XPmBisr_RepairUram(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
					case TAG_ID_TYPE_CFRM_HB: //HardBlock repair function
						EfuseNextAddr = XPmBisr_RepairHardBlock(EfuseCurrAddr, EfuseBisrSize);
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
					default: //block type not recognized, no function to handle it
						XPmBisr_SwError(PMC_EFUSE_BISR_BAD_TAG_TYPE);
						Status = XST_FAILURE;
						break;
					}
					if (XST_SUCCESS != Status) {
						goto done;
					}
				} else {	//calculate the next efuse address if not matched ID
					EfuseNextAddr = (EfuseCurrAddr + 4U);//move to first data address of this tag
					EfuseNextAddr += (EfuseBisrSize << 2); //move down number of words from the tag size
					//did we cross over tbit row in the data size space
					if ((EfuseCurrAddr < (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) &&
					     EfuseNextAddr > (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET)) ||
					    (EfuseCurrAddr < (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET) &&
					     EfuseNextAddr > (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET))) {
						EfuseNextAddr += 4U;
					}
				}
			} else { 	//Not supported
				XPmBisr_SwError(PMC_EFUSE_BISR_UNSUPPORTED_ID);
				Status = XST_FAILURE;
				goto done;
			}
		}
		if (EfuseNextAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET) ||
		    EfuseNextAddr == (EfuseCache->Node.BaseAddress + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET)) {
			EfuseNextAddr += 4U;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static void NidbEfuseGrpInit(XPm_NidbEfuseGrpInfo *EfuseGroup)
{
	XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	u32 BaseAddr = EfuseCache->Node.BaseAddress;
	u32 RegVal;

	/* Initialize 1st NIDB Group */
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_0_OFFSET);
	EfuseGroup[0].NpiOffset = (u8)((RegVal & EFUSE_CACHE_NIDB_0_NPI_OFFSET_0_MASK)
	>> EFUSE_CACHE_NIDB_0_NPI_OFFSET_0_SHIFT);
	EfuseGroup[0].NpiBase = (u16)((RegVal & EFUSE_CACHE_NIDB_0_NPI_BASE_0_MASK) >>
	EFUSE_CACHE_NIDB_0_NPI_BASE_0_SHIFT);
	EfuseGroup[0].RdnCntl = (u8)((RegVal & EFUSE_CACHE_NIDB_0_RDN_CNTRL_0_MASK) >>
	EFUSE_CACHE_NIDB_0_RDN_CNTRL_0_SHIFT);

	/* Initialize 2nd NIDB Group */
	EfuseGroup[1].NpiOffset = (u8)((RegVal & EFUSE_CACHE_NIDB_0_NPI_OFFSET_1_MASK)
	>> EFUSE_CACHE_NIDB_0_NPI_OFFSET_1_SHIFT);
	EfuseGroup[1].NpiBase = (u16)((RegVal & EFUSE_CACHE_NIDB_0_NPI_BASE_1_MASK) >>
	EFUSE_CACHE_NIDB_0_NPI_BASE_1_SHIFT);
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_1_OFFSET);
	EfuseGroup[1].RdnCntl = (u8)((RegVal & EFUSE_CACHE_NIDB_1_RDN_CNTRL_1_MASK) >>
	EFUSE_CACHE_NIDB_1_RDN_CNTRL_1_SHIFT);

	/* Initialize 3rd NIDB Group */
	EfuseGroup[2].NpiOffset = (u8)((RegVal & EFUSE_CACHE_NIDB_1_NPI_OFFSET_2_MASK)
	>> EFUSE_CACHE_NIDB_1_NPI_OFFSET_2_SHIFT);
	EfuseGroup[2].NpiBase = (u16)((RegVal & EFUSE_CACHE_NIDB_1_NPI_BASE_2_MASK) >>
	EFUSE_CACHE_NIDB_1_NPI_BASE_2_SHIFT);
	EfuseGroup[2].RdnCntl = (u8)((RegVal & EFUSE_CACHE_NIDB_1_RDN_CNTL_2_MASK) >>
	EFUSE_CACHE_NIDB_1_RDN_CNTL_2_SHIFT);

	/* Initialize 4th NIDB Group */
	/* Portion of NPI_BASE_3 is stored in NIDB_2 [8:3] and NIDB_1 [2:0] */
	EfuseGroup[3].NpiOffset = (u8)((RegVal & EFUSE_CACHE_NIDB_1_NPI_OFFSET_3_MASK
	>> EFUSE_CACHE_NIDB_1_NPI_OFFSET_3_SHIFT));
	EfuseGroup[3].NpiBase = (u16)((RegVal & EFUSE_CACHE_NIDB_1_NPI_BASE_3_MASK) >>
	EFUSE_CACHE_NIDB_1_NPI_BASE_3_SHIFT);
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_2_OFFSET);
	EfuseGroup[3].NpiBase |= (u16)((RegVal & EFUSE_CACHE_NIDB_2_NPI_BASE_3_MASK) <<
	EFUSE_CACHE_NIDB_1_NPI_BASE_3_WIDTH);
	EfuseGroup[3].RdnCntl = (u8)((RegVal & EFUSE_CACHE_NIDB_2_RDN_CNTL_3_MASK) >>
	EFUSE_CACHE_NIDB_2_RDN_CNTL_3_SHIFT);

	/* Initialize 5th Group*/
	EfuseGroup[4].NpiOffset = (u8)((RegVal & EFUSE_CACHE_NIDB_2_NPI_OFFSET_4_MASK)
	>> EFUSE_CACHE_NIDB_2_NPI_OFFSET_4_SHIFT);
	EfuseGroup[4].NpiBase = (u16)((RegVal & EFUSE_CACHE_NIDB_2_NPI_BASE_4_MASK) >>
	EFUSE_CACHE_NIDB_2_NPI_BASE_4_SHIFT);
	EfuseGroup[4].RdnCntl = (u8)((RegVal & EFUSE_CACHE_NIDB_2_RDN_CNTRL_4_MASK) >>
	EFUSE_CACHE_NIDB_2_RDN_CNTRL_4_SHIFT);

}

XStatus XPmBisr_NidbLaneRepair(void)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 RepairAddr  = 0x0;
	u32 NocSwId     = 0x0;
	u32 SlvSkipAddr = 0x0;
	u32 NidbAddr    = 0x0;
	struct XPm_NidbEfuseGrpInfo NidbEfuseGrpInfo[MAX_NIDB_EFUSE_GROUPS];

	/* Check SLR TYPE */
	if ( SlrType == SLR_TYPE_MONOLITHIC_DEV ||
		 SlrType == SLR_TYPE_INVALID) {
		PmInfo("Not an SSIT Device\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	/* Calculates first NIDB Address */
	NocSwId = XPm_In32(PMC_GLOBAL_BASEADDR + PMC_GLOBAL_SSIT_NOC_ID_OFFSET) &
	PMC_GLOBAL_SSIT_NOC_ID_SWITCHID_MASK;
	/* This is partial address of NoC */
	SlvSkipAddr =  ((NocSwId << 10) - NIDB_OFFSET_DIFF) >> 16U;

	/* Initialize NIDB Group Info Array */
	NidbEfuseGrpInit(NidbEfuseGrpInfo);

	/* lane repair logic */
	for(i=0; i<MAX_NIDB_EFUSE_GROUPS; ++i) {
		if (0x0U == NidbEfuseGrpInfo[i].RdnCntl) {
			continue;
		}

		/* Skip Lane Repair for left most NIDB for Slave SSIT */
		if (SlrType != SLR_TYPE_SSIT_DEV_MASTER_SLR &&
			NidbEfuseGrpInfo[i].NpiBase == SlvSkipAddr) {
			continue;
		}

		/* Calculate Absolute Base Address */
		NidbAddr = NidbEfuseGrpInfo[i].NpiBase;
		NidbAddr = (NidbAddr << 16U) + NPI_ROOT_BASEADDR;

		/* Unlock PCSR */
		XPm_Out32(NidbAddr + NIDB_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

		/* Unlock Lane Repair Registers */
		XPm_Out32(NidbAddr + NIDB_LANE_REPAIR_UNLOCK_OFFSET,
		NIDB_LANE_REPAIR_UNLOCK_VAL);

		/* Calculate Repair Address */
		RepairAddr = NidbAddr + NIDB_REPAIR_OFFSET +
		(NidbEfuseGrpInfo[i].NpiOffset << 2);

		/* Write Repair Data */
		XPm_Out32(RepairAddr, NidbEfuseGrpInfo[i].RdnCntl);
	}

	for(i=0; i<MAX_NIDB_EFUSE_GROUPS; ++i) {

		NidbAddr = NidbEfuseGrpInfo[i].NpiBase;
		NidbAddr = (NidbAddr << 16U) + NPI_ROOT_BASEADDR;

		/* Lock Lane Repair Registers */
		XPm_Out32(NidbAddr + NIDB_LANE_REPAIR_UNLOCK_OFFSET, 0x1);

		/* Lock PCSR Register */
		XPm_Out32(NidbAddr + NIDB_PCSR_MASK_OFFSET,
		NIDB_PCSR_MASK_ODISABLE_MASK);
		XPm_Out32(NidbAddr + NIDB_PCSR_CONTROL_OFFSET, 0x0);
		XPm_Out32(NidbAddr + NIDB_PCSR_LOCK_OFFSET, 0x0);
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

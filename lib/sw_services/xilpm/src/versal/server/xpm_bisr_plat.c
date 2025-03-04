/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserve.
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
#include "xpm_debug.h"
#include "xpm_bisr_plat.h"

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
#define TAG_ID_TYPE_LAGUNA				(15U)
#define TAG_ID_ARRAY_SIZE				(256U)

#define ME_BISR_FIXED_OFFSET				(0x36010U)
#define ME_BISR_EFUSE_OFFSET_SHIFT			(20U)
#define ME_BISR_CACHE_CTRL_OFFSET			(0x36000U)
#define ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK		(0x00000001U)
#define ME_BISR_CACHE_STATUS_OFFSET			(0x36008U)
#define ME_BISR_CACHE_STATUS_BISR_DONE_MASK		(0x00000001U)
#define ME_BISR_CACHE_STATUS_BISR_PASS_MASK		(0x00000002U)

#define DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET		(0x268U)
#define DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET		(0x258U)
#define DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET		(0x004U)
#define DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET		(0x000U)
#define DDRMC_NPI_PCSR_BISR_TRIGGER_MASK		(0x02000000U)
#define DDRMC_NPI_CACHE_STATUS_BISR_DONE_MASK		(0x00000001U)
#define DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK		(0x00000002U)
#define DDRMC_NPI_CLK_GATE_REGISTER_OFFSET		(0x24CU)
#define DDRMC_NPI_CLK_GATE_BISREN_MASK			(0x00000040U)

//CFRM_HB Repair
#define CFRM_HB_REPAIR_QTILE_MASK			(0x03E00000U)
#define CFRM_HB_REPAIR_QTILE_SHIFT			(21U)
#define CFRM_HB_REPAIR_COL_MASK				(0x0C000000U)
#define CFRM_HB_REPAIR_COL_SHIFT			(26U)
#define CFRM_HB_REPAIR_ROW_MASK				(0xF0000000U)
#define CFRM_HB_REPAIR_ROW_SHIFT			(28U)
#define CFRM_HB_REPAIR_VAL0_MASK			(0x001FFFFFU) //value field in the first row
#define CFRM_HB_EXP_REPAIR_BLK_TYPE			(0x5UL)
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_SHIFT		(15U)
#define CFRM_HB_EXP_REPAIR_COL_SHIFT			(7U)
#define CFRM_HB_EXP_REPAIR_QTILE_SHIFT			(0U)

//NIDB Lane Repair
#define MAX_NIDB_EFUSE_GROUPS				(0x5U)
#define NPI_ROOT_BASEADDR				(NPI_BASEADDR + NPI_NIR_0_OFFSET)
#define NIDB_OFFSET_DIFF				(0x00010000U)

#define NIDB_LANE_REPAIR_UNLOCK_VAL			(0xE6172839U)

//XRAM Repair
#define XRAM_SLCR_PCSR_BISR_TRIGGER_MASK		(0x08000000U)
#define XRAM_SLCR_PCSR_BISR_CLR_MASK			(0x10000000U)
#define XRAM_SLCR_PCSR_PSR_BISR_DONE_MASK		(0x00004000U)
#define XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK		(0x00008000U)

#ifndef XCVP1902
/* Laguna Repair */
#define LAGUNA_FUSE_Y0_LSB		   (0U)
#define LAGUNA_FUSE_Y0_BITS		   (9U)
#define LAGUNA_FUSE_Y1_LSB		   (LAGUNA_FUSE_Y0_LSB + LAGUNA_FUSE_Y0_BITS)
#define LAGUNA_FUSE_Y1_BITS		   (9U)
#define LAGUNA_FUSE_X1_LSB		   (LAGUNA_FUSE_Y1_LSB + LAGUNA_FUSE_Y1_BITS)
#define LAGUNA_FUSE_X1_BITS		   (8U)
#define LAGUNA_FUSE_REPAIR_LSB		   (LAGUNA_FUSE_X1_LSB + LAGUNA_FUSE_X1_BITS)
#define LAGUNA_FUSE_REPAIR_BITS	   (6U)
#define HALF_TILE_NUM_PER_FRAME	   (48U)
#define RCLK_TILES_NUM_IN_FRAME	   (4U)
#define FDRO_BASEADDR		   (0xF12C2000U)
#define FSR_TILE_END		   (95U)
#define HALF_FSR_START		   (48U)
#endif

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
	XStatus Status = XST_FAILURE;
	u64 BisrDataDestAddr;
	XPm_PsLpDomain *LpDomain = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (NULL == LpDomain) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	BisrDataDestAddr = LpDomain->LpdSlcrBaseAddr + (u64)LPD_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	LpDomain->LpdBisrFlags |= LPD_BISR_DATA_COPIED;

	Status = XPmBisr_TriggerLpd();
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_TRIGGER;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmBisr_TriggerLpd(void)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_PsLpDomain *PsLpd;
	u32 RegValue;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, PsLpd, XPm_PsLpDomain,
				       XPmPower_GetById, PM_POWER_LPD);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
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
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
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
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPmBisr_RepairFpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_PsFpDomain *PsFpd;
	u32 RegValue;
	u64 BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, PsFpd, XPm_PsFpDomain,
				       XPmPower_GetById, PM_POWER_FPD);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
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
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
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
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


static XStatus XPmBisr_RepairCpm(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_CpmDomain *Cpm;
	u32 RegValue;
	u64 BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, Cpm, XPm_CpmDomain,
				       XPmPower_GetById, PM_POWER_CPM);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	BisrDataDestAddr = Cpm->CpmSlcrBaseAddr + (u64)CPM_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Clear BISR data test registers */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM_SLCR_BISR_CACHE_CTRL_CLR_MASK);
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0U);

	/* Trigger Bisr */
	PmRmw32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK,
		CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_STATUS_OFFSET,
				 CPM_SLCR_BISR_CACHE_STATUS_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto done;
	}

	/* Check Bisr status */
	PmIn32(Cpm->CpmSlcrBaseAddr + CPM_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) != CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPmBisr_RepairCpm5(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	volatile XStatus Status = XPM_ERR_BISR;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_CpmDomain *Cpm;
	u32 RegValue;
	u64 BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, Cpm, XPm_CpmDomain,
				       XPmPower_GetById, PM_POWER_CPM5);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_FAILURE;
		goto done;
	}

	BisrDataDestAddr = Cpm->CpmSlcrBaseAddr + (u64)CPM5_SLCR_BISR_CACHE_DATA_0_OFFSET;
	/* Disable write protection */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_WPROTP_OFFSET, 0x0U);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Clear BISR data test registers */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM5_SLCR_BISR_CACHE_CTRL_CLR_MASK);
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET, 0x0U);

	/* Trigger Bisr */
	PmRmw32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_CTRL_OFFSET,
		CPM5_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK,
		CPM5_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_STATUS_OFFSET,
				 CPM5_SLCR_BISR_CACHE_STATUS_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto done;
	}

	/* Check Bisr status */
	PmIn32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & CPM5_SLCR_BISR_CACHE_STATUS_PASS_MASK) !=
		CPM5_SLCR_BISR_CACHE_STATUS_PASS_MASK) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

	/* Enable write protection */
	PmOut32(Cpm->CpmSlcrBaseAddr + CPM5_SLCR_WPROTP_OFFSET, 0x1U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


static XStatus XPmBisr_RepairDdrMc(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u32 BaseAddr, BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	BaseAddr = NPI_FIXED_BASEADDR + (TagOptional<<NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr + DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize,
					    BisrDataDestAddr);

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Enable Bisr clock */
	PmRmw32(BaseAddr + DDRMC_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC_NPI_CLK_GATE_BISREN_MASK, DDRMC_NPI_CLK_GATE_BISREN_MASK);

	/*Trigger Bisr */
	PmOut32(BaseAddr + DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET, DDRMC_NPI_PCSR_BISR_TRIGGER_MASK);
	PmOut32(BaseAddr + DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET, DDRMC_NPI_PCSR_BISR_TRIGGER_MASK);

	/* Wait for Bisr to be done and check status */
	Status = XPm_PollForMask(BaseAddr + DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET,
				 DDRMC_NPI_CACHE_STATUS_BISR_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto fail;
	}

	PmIn32(BaseAddr + DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET, RegValue);
	if ((RegValue & DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK) != DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto fail;
	}

	/* Disable Bisr Clock */
	PmRmw32(BaseAddr + DDRMC_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC_NPI_CLK_GATE_BISREN_MASK, ~DDRMC_NPI_CLK_GATE_BISREN_MASK);

fail:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPmBisr_RepairME(u32 EfuseTagAddr, u32 TagId,u32 TagSize,u32 TagOptional, u32 *TagDataAddr)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	u32 RegValue;
	u64 BaseAddr, BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *AieDev;

	/* Compilation warning fix */
	(void)TagId;

	Status = XPM_STRICT_CHECK_IF_NOT_NULL(StatusTmp, AieDev, XPm_Device,
				       XPmDevice_GetById, PM_DEV_AIE);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XST_FAILURE;
		goto done;
	}

	BaseAddr = (u64)VIVADO_ME_BASEADDR + ((u64)TagOptional << ME_BISR_EFUSE_OFFSET_SHIFT);
	BisrDataDestAddr = BaseAddr + ME_BISR_FIXED_OFFSET;

	/**
	 * Using posted writes in NPI in a configuration sequence that later
	 * performs AXI-MM transfers needs to ensure that NPI writes have
	 * finished before AXI-MM requests are sent. Ensure NPI writes from CDO
	 * have finished by dummy NPI register read.
	 */
	PmIn32(AieDev->Node.BaseAddress + NPI_PCSR_STATUS_OFFSET, RegValue);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Set NPI_PRIVILEGED_CTRL bit */
	PmRmw32(AieDev->Node.BaseAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK);

	/* Trigger Bisr */
	PmRmw64(BaseAddr + ME_BISR_CACHE_CTRL_OFFSET, ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK, ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask64(BaseAddr + ME_BISR_CACHE_STATUS_OFFSET,
				   ME_BISR_CACHE_STATUS_BISR_DONE_MASK,
				   XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto done;
	}

	/* Check Bisr Status */
	PmIn64(BaseAddr + ME_BISR_CACHE_STATUS_OFFSET, RegValue);
	if ((RegValue & ME_BISR_CACHE_STATUS_BISR_PASS_MASK) !=
				  ME_BISR_CACHE_STATUS_BISR_PASS_MASK) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XST_FAILURE;
		goto done;
	}

done:
	/* Clear NPI_PRIVILEGED_CTRL bit */
	PmRmw32(AieDev->Node.BaseAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK, 0);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static u32 XPmBisr_RepairHardBlock(u32 EfuseTagAddr, u32 TagSize)
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
		// REPAIR_COLUMN[78:71] = Column[1:0]	 (Align to LSB)
		// REPAIR_BLK_TYPE[81:79]=3’b101   (FUSE Tag is Hard-ip, i.e. MRMAC, SDFEC etc)
		//[127:96] init to 0
		HbExtendedRepair[0U] = HbRepairVal[0U]; //[31:0 ] (from second row)
		HbExtendedRepair[1U] = HbRepairVal[1U];	//[52:32]	(from first row of efuse pair)
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

static XStatus XPmBisr_RepairXram(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XPM_ERR_BISR;
	const XPm_Device *Device = NULL;
	u32 RegValue, BaseAddr;
	u64 BisrDataDestAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Not possible to reach here if Device doesn't exist hence no */
	/* check for existence of Device */
	Device = XPmDevice_GetByIndex((u32)XPM_NODEIDX_DEV_XRAM_0);

	/* Calculate Destination address */
	/* Dest. Addr = slcr_address + cache_data_offset */
	BaseAddr = Device->Node.BaseAddress;
	BisrDataDestAddr = BaseAddr + (u64)XRAM_SLCR_BISR_CACHE_DATA_0_OFFSET;

	/* Write unlock code to PCSR_LOCK register */
	XPm_UnlockPcsr(BaseAddr);

	/* Clear the BISR Test Data */
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET,
			XRAM_SLCR_PCSR_BISR_CLR_MASK);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET,
			XRAM_SLCR_PCSR_BISR_CLR_MASK);

	/* Exit the BISR Test Data Clear Mode*/
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_PCR_OFFSET,  0x0U);
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_MASK_OFFSET, 0x0U);

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
		DbgErr = XPM_INT_ERR_BISR_DONE_TIMEOUT;
		goto done;
	}

	/* Check BISR Pass/Fail */
	PmIn32(BaseAddr + XRAM_SLCR_PCSR_PSR_OFFSET, RegValue);
	if (XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK != (RegValue &
		XRAM_SLCR_PCSR_PSR_BISR_PASS_MASK)) {
		DbgErr = XPM_INT_ERR_BISR_PASS;
		Status = XPM_ERR_BISR;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	/* Write lock code to PCSR_LOCK register */
	XPm_LockPcsr(BaseAddr);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

#ifndef XCVP1902
static void LagunaRmwOneFrame(const XPm_PlDomain *Pld, u32 RowIndex,
		u32 FrameAddr, u32 LowerTile, u32 UpperTile, u32 RepairWord)
{
	u32 CFrameAddr;
	u32 FrameData[100U];
	u32 FdriAddr;
	u32 current_fdro;
	u32 i;
	u8 Idx = 0U;

	/* Get CFRAME Address */
	CFrameAddr = Pld->Cframe0RegBaseAddr + (XCFRAME_FRAME_OFFSET * (u32)RowIndex);

	/* Enable CFRAME Row */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_ROWON);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(300U);

	/* Enable read configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_RCFG);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(200U);

	/* Set Frame address register */
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x0U, FrameAddr);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0xCU, 0x0U);

	/* Set read count to one frame. One frame is 25 quadwords */
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x0U, 0x19U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0xCU, 0x0U);

	/* Read 25 quadwords of Frame data. FDRO address is 128 bits wide so read
	 * 4 words of data per iteration.
	 */
	for (i = 0U; i < 25U; i++) {
		current_fdro = FDRO_BASEADDR;

		FrameData[Idx] = XPm_In32(current_fdro);
		current_fdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(current_fdro);
		current_fdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(current_fdro);
		current_fdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(current_fdro);
		++Idx;
	}

	/* Modify frame data with repair value */
	for (i = LowerTile; i <= UpperTile; i++) {
		if ((i < HALF_TILE_NUM_PER_FRAME) || (i >= (HALF_TILE_NUM_PER_FRAME + RCLK_TILES_NUM_IN_FRAME))) {
			FrameData[i] |= RepairWord;
		}
	}

	/* Enable write configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_WCFG);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(200U);

	/* Set Frame address register */
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x0U, FrameAddr);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0xCU, 0x0U);

	FdriAddr = CFRAME0_FDRI_BASEADDR + (XCFRAME_FRAME_OFFSET * (u32)RowIndex);

	/* Write 100 words of Frame data */
	for (i = 0U; i < 100U; i++) {
		XPm_Out32(FdriAddr, FrameData[i]);
		FdriAddr += 0x4U;
	}
}

static u32 XPmBisr_RepairLaguna(u32 EfuseTagAddr, u32 TagSize)
{
	const XPm_PlDomain *Pld;
	u32 TagRow = 0U;
	u32 TagData;
	u32 TagDataAddr;
	u32 RepairWord;
	u32 LagunaX;
	u32 Temp;
	u32 HalfFsr;
	u32 NumberOfRows;
	u32 Row0;
	u32 Row1;
	u32 Tile0;
	u32 Tile1;
	u8 LowerTile;
	u8 UpperTile;
	u32 RowIndex;
	u32 FrameAddr;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;
	u8 i;

	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);

	TagDataAddr = EfuseTagAddr + 4U;

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		/* Return negative address so error can be identified by caller*/
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

	/* Broadcast row on command */
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_ROWON);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* Get device row info */
	HalfFsr = (XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET) &
		(u32)CFU_APB_CFU_ROW_RANGE_HALF_FSR_MASK) >> CFU_APB_CFU_ROW_RANGE_HALF_FSR_SHIFT;
	NumberOfRows = XPm_In32(Pld->CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET)
		& (u32)CFU_APB_CFU_ROW_RANGE_NUM_MASK;

	while (TagRow < TagSize) {
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}

		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		/* Laguna pointer coordinate for Y0 */
		Temp = (TagData >> LAGUNA_FUSE_Y0_LSB) & (((u32)1U << LAGUNA_FUSE_Y0_BITS) - 1U);
		/* Row index of Y0 and Tile index within Y0. 96 tiles per FSR row */
		Row0 = Temp / 96U;
		Tile0 = Temp % 96U;

		/* Laguna pointer coordinate for Y1 */
		Temp = (TagData >> LAGUNA_FUSE_Y1_LSB) & (((u32)1U << LAGUNA_FUSE_Y1_BITS) - 1U);
		/* Row index of Y0 and Tile index within Y1. 96 tiles per FSR row */
		Row1 = Temp / 96U;
		Tile1 = Temp % 96U;

		/* Laguna pointer coordinate for X1 */
		LagunaX = (TagData >> LAGUNA_FUSE_X1_LSB) & (((u32)1U << LAGUNA_FUSE_X1_BITS) - 1U);

		/*
		 * Form 32 bit repair word from 6 bits of repair word.
		 * Only 6 bits in a word need to be modified with repair value.
		 * bit	   5	  4	3     2     1	  0
		 * bit	  26	 22    18    14    10	  6
		 */
		RepairWord = 0U;
		for (i = 0U; i < 6U; i++) {
			if ((TagData & ((u32)1U << (LAGUNA_FUSE_REPAIR_LSB + LAGUNA_FUSE_REPAIR_BITS - 1U))) != 0U) {
				RepairWord |= 1U;
			}
			RepairWord <<= 4U;
			TagData <<= 1U;
		}

		RepairWord <<= 2U;

		/* Walk through each FSR row that requires repair */
		for (RowIndex = Row0; RowIndex <= Row1; RowIndex++) {
			LowerTile = 0U;
			UpperTile = FSR_TILE_END;

			/* Bottom row requires repair */
			if (RowIndex == Row0) {
				LowerTile = (u8)Tile0;
			}

			/* Top row requires repair */
			if (RowIndex == Row1) {
				UpperTile = (u8)Tile1;
			}

			/* Top row is Half FSR */
			if ((RowIndex == (NumberOfRows - 1U)) && (HalfFsr == 1U)) {
					LowerTile += HALF_FSR_START;
					UpperTile += HALF_FSR_START;
			}

			/*
			 * Adjust lower and upper tiles to accommodate RCLK tiles.
			 * RCLK occupies words 48-51 so skip these four words.
			 */
			if (LowerTile > (HALF_TILE_NUM_PER_FRAME - 1U)) {
				LowerTile += RCLK_TILES_NUM_IN_FRAME;
			}
			if (UpperTile > (HALF_TILE_NUM_PER_FRAME - 1U)) {
				UpperTile += RCLK_TILES_NUM_IN_FRAME;
			}

			/* Construct block type-6 frame address */
			FrameAddr = LagunaX;
			FrameAddr |= ((u32)FRAME_BLOCK_TYPE_6 << CFRAME0_REG_FAR_BLOCKTYPE_SHIFT);

			LagunaRmwOneFrame(Pld, RowIndex, FrameAddr, LowerTile,
					UpperTile, RepairWord);
		}
	}

done:
	return TagDataAddr;
}
#endif

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
	u32 EfuseBisrOptional;
	u32 TagType;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;
	u32 XPmTagIdWhiteList[TAG_ID_ARRAY_SIZE] = {0};
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

	//set up the white list
	XPmBisr_InitTagIdList(&XPmTagIdWhiteList);

	//check requested ID is a valid ID
	if (TagId > 255U) {
		DbgErr = XPM_INT_ERR_BISR_UNKN_TAG_ID;
		Status = XST_FAILURE;
		goto done;
	}

	//Compare to the white list to check it is a supported TAG
	if (TAG_ID_VALID_MASK == (XPmTagIdWhiteList[TagId] & TAG_ID_VALID_MASK)) {
		TagType = XPmTagIdWhiteList[TagId] & TAG_ID_TYPE_MASK;
	} else {
		DbgErr = XPM_INT_ERR_BISR_INVALID_ID;
		Status = XST_FAILURE;
		goto done;
	}

	//Scan EFUSE looking for valid tags that match requested tag, exit on 0, skip row on all 1's
	EfuseNextAddr = EfuseCacheBaseAddr + EFUSE_CACHE_BISR_RSVD_0_OFFSET;
	ExitCodeSeen = 0U;

	while (0U == ExitCodeSeen) {
		//read efuse row
		EfuseCurrAddr = EfuseNextAddr;
		EfuseRowTag = XPm_In32(EfuseCurrAddr);

		if (EfuseRowTag == PMC_EFUSE_BISR_EXIT_CODE) {
			ExitCodeSeen = 1U;
		} else if(EfuseRowTag==PMC_EFUSE_BISR_SKIP_CODE) {	//SKIP Code Found
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
#ifndef XCVP1902
					case TAG_ID_TYPE_LAGUNA:
						EfuseNextAddr = XPmBisr_RepairLaguna(EfuseCurrAddr, EfuseBisrSize);
						if (EfuseNextAddr != ~0U) {
							Status = XST_SUCCESS;
						}
						break;
#endif
					default: //block type not recognized, no function to handle it
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
			} else {	//check bisr2 supported TAG_ID else return error
				Status = XPmBisr_TagSupportCheck2(EfuseBisrTagId);
				if (XST_SUCCESS == Status) {
					goto done;
				}
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

static void NidbEfuseGrpInit(XPm_NidbEfuseGrpInfo *EfuseGroup)
{
	/* Note: hardcode EFUSE_CACHE_BASEADDR */
	/* Since this function is used in early stage of boot sequence */
	/* prior topology CDOs loaded*/
	u32 BaseAddr = EFUSE_CACHE_BASEADDR;
	u32 NidbRegMask;
	u32 NidbRegShift;
	u32 RegVal;

	/* Initialize 1st NIDB Group */
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_0_OFFSET);
	NidbRegMask = EFUSE_CACHE_NIDB_0_NPI_OFFSET_0_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_0_NPI_OFFSET_0_SHIFT;
	EfuseGroup[0U].NpiOffset = (u8)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_0_NPI_BASE_0_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_0_NPI_BASE_0_SHIFT;
	EfuseGroup[0U].NpiBase = (u16)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_0_RDN_CNTRL_0_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_0_RDN_CNTRL_0_SHIFT;
	EfuseGroup[0U].RdnCntl = (u8)((RegVal & NidbRegMask) >> NidbRegShift);

	/* Initialize 2nd NIDB Group */
	NidbRegMask = EFUSE_CACHE_NIDB_0_NPI_OFFSET_1_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_0_NPI_OFFSET_1_SHIFT;
	EfuseGroup[1U].NpiOffset = (u8)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_0_NPI_BASE_1_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_0_NPI_BASE_1_SHIFT;
	EfuseGroup[1U].NpiBase = (u16)((RegVal & NidbRegMask) >> NidbRegShift);
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_1_OFFSET);
	NidbRegMask = EFUSE_CACHE_NIDB_1_RDN_CNTRL_1_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_RDN_CNTRL_1_SHIFT;
	EfuseGroup[1U].RdnCntl = (u8)((RegVal & NidbRegMask) >> NidbRegShift);

	/* Initialize 3rd NIDB Group */
	NidbRegMask = EFUSE_CACHE_NIDB_1_NPI_OFFSET_2_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_NPI_OFFSET_2_SHIFT;
	EfuseGroup[2U].NpiOffset = (u8)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_1_NPI_BASE_2_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_NPI_BASE_2_SHIFT;
	EfuseGroup[2U].NpiBase = (u16)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_1_RDN_CNTL_2_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_RDN_CNTL_2_SHIFT;
	EfuseGroup[2U].RdnCntl = (u8)((RegVal & NidbRegMask) >> NidbRegShift);

	/* Initialize 4th NIDB Group */
	/* Portion of NPI_BASE_3 is stored in NIDB_2 [8:3] and NIDB_1 [2:0] */
	NidbRegMask = EFUSE_CACHE_NIDB_1_NPI_OFFSET_3_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_NPI_OFFSET_3_SHIFT;
	EfuseGroup[3U].NpiOffset = (u8)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_1_NPI_BASE_3_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_NPI_BASE_3_SHIFT;
	EfuseGroup[3U].NpiBase = (u16)((RegVal & NidbRegMask) >> NidbRegShift);
	RegVal = XPm_In32(BaseAddr + EFUSE_CACHE_NIDB_2_OFFSET);
	NidbRegMask = EFUSE_CACHE_NIDB_2_NPI_BASE_3_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_1_NPI_BASE_3_WIDTH;
	EfuseGroup[3U].NpiBase |= (u16)((RegVal & NidbRegMask) << NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_2_RDN_CNTL_3_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_2_RDN_CNTL_3_SHIFT;
	EfuseGroup[3U].RdnCntl = (u8)((RegVal & NidbRegMask) >> NidbRegShift);

	/* Initialize 5th Group*/
	NidbRegMask = EFUSE_CACHE_NIDB_2_NPI_OFFSET_4_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_2_NPI_OFFSET_4_SHIFT;
	EfuseGroup[4U].NpiOffset = (u8)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_2_NPI_BASE_4_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_2_NPI_BASE_4_SHIFT;
	EfuseGroup[4U].NpiBase = (u16)((RegVal & NidbRegMask) >> NidbRegShift);
	NidbRegMask = EFUSE_CACHE_NIDB_2_RDN_CNTRL_4_MASK;
	NidbRegShift = EFUSE_CACHE_NIDB_2_RDN_CNTRL_4_SHIFT;
	EfuseGroup[4U].RdnCntl = (u8)((RegVal & NidbRegMask) >> NidbRegShift);

}

static XStatus XPmBisr_NidbRepairLane(u32 RepairLeftMostNIDBOnly)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 RepairAddr	= 0x0U;
	u32 NocSwId	= 0x0U;
	u32 RomRepairAddr = 0x0U;
	u32 NidbAddr	= 0x0U;
	u32 SlrType;
	struct XPm_NidbEfuseGrpInfo NidbEfuseGrpInfo[MAX_NIDB_EFUSE_GROUPS];

	/* Check SLR TYPE */
	SlrType = XPm_GetSlrType();
	if ((SlrType == SLR_TYPE_MONOLITHIC_DEV) ||
	    (SlrType == SLR_TYPE_INVALID)) {
		PmInfo("Not an SSIT Device\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	/* Calculates first NIDB Address */
	NocSwId = XPm_In32(PMC_GLOBAL_BASEADDR + PMC_GLOBAL_SSIT_NOC_ID_OFFSET) &
	PMC_GLOBAL_SSIT_NOC_ID_SWITCHID_MASK;
	/* This is partial address of NoC */
	RomRepairAddr =  ((NocSwId << 10U) - NIDB_OFFSET_DIFF) >> 16U;

	/* Initialize NIDB Group Info Array */
	NidbEfuseGrpInit(NidbEfuseGrpInfo);

	/* lane repair logic */
	for(i = 0U; i < MAX_NIDB_EFUSE_GROUPS; ++i) {
		if (0x0U == NidbEfuseGrpInfo[i].RdnCntl) {
			continue;
		}
		if (SLR_TYPE_SSIT_DEV_MASTER_SLR != SlrType) {
			if (NidbEfuseGrpInfo[i].NpiBase == RomRepairAddr) {
				if(0U == RepairLeftMostNIDBOnly) {
					continue;
				}
			}
			else {
				if (0U != RepairLeftMostNIDBOnly) {
					continue;
				}
			}
		}
		/* Calculate Absolute Base Address */
		NidbAddr = NidbEfuseGrpInfo[i].NpiBase;
		NidbAddr = (NidbAddr << 16U) + NPI_ROOT_BASEADDR;

		/* Unlock PCSR */
		XPm_UnlockPcsr(NidbAddr);

		/* Unlock Lane Repair Registers */
		XPm_Out32(NidbAddr + NIDB_REG_REPAIR_LOCK_OFFSET,
		NIDB_LANE_REPAIR_UNLOCK_VAL);

		/* Calculate Repair Address */
		RepairAddr = NidbAddr + NIDB_REG_RX_REPAIR_LN_0_OFFSET +
		(NidbEfuseGrpInfo[i].NpiOffset << 2U);

		/* Write Repair Data */
		XPm_Out32(RepairAddr, NidbEfuseGrpInfo[i].RdnCntl);
	}

	for(i = 0U; i < MAX_NIDB_EFUSE_GROUPS; ++i) {

		/* Not a valid EFUSE if RDN_CNTRL=0.. so skip to next entry */
		if (0x0U == NidbEfuseGrpInfo[i].RdnCntl) {
			continue;
		}

		if (SLR_TYPE_SSIT_DEV_MASTER_SLR != SlrType) {
			if (NidbEfuseGrpInfo[i].NpiBase == RomRepairAddr) {
				if(0U == RepairLeftMostNIDBOnly) {
					continue;
				}
			} else {
				if (0U != RepairLeftMostNIDBOnly) {
					continue;
				}
			}
		}
		NidbAddr = NidbEfuseGrpInfo[i].NpiBase;
		NidbAddr = (NidbAddr << 16U) + NPI_ROOT_BASEADDR;

		/* Lock Lane Repair Registers */
		XPm_Out32(NidbAddr + NIDB_REG_REPAIR_LOCK_OFFSET, 0x1U);

		/* Lock PCSR Register */
		XPm_Out32(NidbAddr + NIDB_REG_PCSR_MASK_OFFSET,
			NIDB_REG_PCSR_MASK_ODISABLE_MASK);
		XPm_Out32(NidbAddr + NIDB_REG_PCSR_CONTROL_OFFSET, 0x0U);
		XPm_LockPcsr(NidbAddr);
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmBisr_NidbLeftMostLaneRepair(void){
	return XPmBisr_NidbRepairLane(1U);
}
XStatus XPmBisr_NidbLaneRepair(void){
	return XPmBisr_NidbRepairLane(0U);
}

/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_powerdomain.h"

/* Defines */
#define PMC_EFUSE_BISR_START_ADDR 	EFUSE_CACHE_BISR_RSVD_0
#define PMC_EFUSE_BISR_EXIT_CODE 	0
#define PMC_EFUSE_BISR_SKIP_CODE 	0xFFFFFFFF
#define PMC_EFUSE_BISR_TAG_ID_MASK 	0xFF000000
#define PMC_EFUSE_BISR_TAG_ID_SHIFT 	24
#define PMC_EFUSE_BISR_SIZE_MASK 	0x00FF0000
#define PMC_EFUSE_BISR_SIZE_SHIFT 	16
#define PMC_EFUSE_BISR_OPTIONAL_MASK 	0x0000FFFF
#define PMC_EFUSE_BISR_OPTIONAL_SHIFT 	0

#define TAG_ID_VALID_MASK 		0x80000000
#define TAG_ID_VALID_SHIFT 		31
#define TAG_ID_TYPE_MASK		0x7FFFFFFF
#define TAG_ID_TYPE_SHIFT		0

#define TAG_ID_TYPE_ME			1
#define TAG_ID_TYPE_CFRM_HB		2
#define TAG_ID_TYPE_CFRM_BR		3
#define TAG_ID_TYPE_CFRM_UR		4
#define TAG_ID_TYPE_DDRMC		5
#define TAG_ID_TYPE_CPM			6
#define TAG_ID_TYPE_GTY			7
#define TAG_ID_TYPE_LPD			8
#define TAG_ID_TYPE_FPD			9
#define TAG_ID_ARRAY_SIZE 		256

#define PMC_EFUSE_BISR_UNKN_TAG_ID 	0x1
#define PMC_EFUSE_BISR_INVLD_TAG_ID	0x2
#define PMC_EFUSE_BISR_BAD_TAG_TYPE	0x3
#define PMC_EFUSE_BISR_UNSUPPORTED_ID 	0x4
#define PMC_EFUSE_BISR_CFRM_HB_BAD_SIZE	0x5

#ifndef VIVADO_ME_BASEADDR
	#define VIVADO_ME_BASEADDR 	0x0200000000
#endif
#define ME_BISR_FIXED_OFFSET 		0x36010
#define ME_BISR_FIXED_OFFSET_MASK 	0x00000FFFFF
#define ME_BISR_FIXED_OFFSET_SHIFT 	0
#define ME_BISR_EFUSE_OFFSET_MASK 	0x0FFFF00000
#define ME_BISR_EFUSE_OFFSET_SHIFT 	20
#define ME_BISR_CACHE_CTRL_OFFSET	0x36000
#define ME_BISR_CACHE_CTRL_BISR_TRIGGER_MASK	0x00000001
#define ME_BISR_CACHE_STATUS_OFFSET	0x36008
#define ME_BISR_CACHE_STATUS_BISR_DONE_MASK	0x00000001
#define ME_BISR_CACHE_STATUS_BISR_PASS_MASK	0x00000002

#define NPI_FIXED_BASEADDR 		0x00F6000000
#define NPI_FIXED_MASK 			0x00FE000000
#define NPI_EFUSE_ENDPOINT_SHIFT 	16
#define NPI_EFUSE_ENDPOINT_ADDR_MASK 	0x0001FF0000

#define DDRMC_NPI_CACHE_STATUS_REGISTER_OFFSET 	0x268
#define DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET 	0x258
#define DDRMC_NPI_PCSR_CONTROL_REGISTER_OFFSET 	0x004
#define DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET 	0x000
#define DDRMC_NPI_PCSR_LOCK_REGISTER_OFFSET 	0x00C
#define DDRMC_NPI_PCSR_BISR_TRIGGER_MASK	0x02000000
#define DDRMC_NPI_CACHE_STATUS_BISR_DONE_MASK	0x00000001
#define DDRMC_NPI_CACHE_STATUS_BISR_PASS_MASK	0x00000002
#define DDRMC_NPI_CLK_GATE_REGISTER_OFFSET	0x24C
#define DDRMC_NPI_CLK_GATE_BISREN_MASK		0x00000040

#define GTY_NPI_CACHE_DATA_REGISTER_OFFSET 	0x64

//BRAM Repair
#define CFRM_BRAM_REPAIR_ROW_MASK 		0x00078000
#define CFRM_BRAM_REPAIR_ROW_SHIFT 		15
#define CFRM_BRAM_REPAIR_COL_MASK 		0x00007E00
#define CFRM_BRAM_REPAIR_COL_SHIFT 		9
#define CFRM_BRAM_REPAIR_INDEX_MASK 		0x000001E0
#define CFRM_BRAM_REPAIR_INDEX_SHIFT 		5
#define CFRM_BRAM_REPAIR_VAL_MASK 		0x0000001F
#define CFRM_BRAM_REPAIR_VAL_SHIFT 		0
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_BRAM 	0x3
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_MASK 	0x00038000
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_WIDTH 	3
#define CFRM_BRAM_EXP_REPAIR_BLK_TYPE_SHIFT 	15
#define CFRM_BRAM_EXP_REPAIR_COL_MASK 		0x00007F80
#define CFRM_BRAM_EXP_REPAIR_COL_WIDTH 		8
#define CFRM_BRAM_EXP_REPAIR_COL_SHIFT 		7
#define CFRM_BRAM_EXP_REPAIR_INDEX_MASK 	0x0000007F
#define CFRM_BRAM_EXP_REPAIR_INDEX_WIDTH 	7
#define CFRM_BRAM_EXP_REPAIR_INDEX_SHIFT	0
#define CFRM_BRAM_EXP_REPAIR_VAL_MASK 		0xFFFFFFFF
#define CFRM_BRAM_EXP_REPAIR_VAL_SHIFT 		0

//URAM Repair
#define CFRM_URAM_REPAIR_ROW_MASK 		0x00078000
#define CFRM_URAM_REPAIR_ROW_SHIFT 		15
#define CFRM_URAM_REPAIR_COL_MASK 		0x00007800
#define CFRM_URAM_REPAIR_COL_SHIFT 		11
#define CFRM_URAM_REPAIR_INDEX_MASK 		0x000007C0
#define CFRM_URAM_REPAIR_INDEX_SHIFT 		6
#define CFRM_URAM_REPAIR_VAL_MASK 		0x0000003F
#define CFRM_URAM_REPAIR_VAL_SHIFT 		0
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_URAM 	0x4
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_MASK 	0x00038000
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_WIDTH 	3
#define CFRM_URAM_EXP_REPAIR_BLK_TYPE_SHIFT 	15
#define CFRM_URAM_EXP_REPAIR_COL_MASK 		0x00007F80
#define CFRM_URAM_EXP_REPAIR_COL_WIDTH 		8
#define CFRM_URAM_EXP_REPAIR_COL_SHIFT 		7
#define CFRM_URAM_EXP_REPAIR_INDEX_MASK 	0x0000007F
#define CFRM_URAM_EXP_REPAIR_INDEX_WIDTH	7
#define CFRM_URAM_EXP_REPAIR_INDEX_SHIFT 	0
#define CFRM_URAM_EXP_REPAIR_VAL_MASK 		0xFFFFFFFF
#define CFRM_URAM_EXP_REPAIR_VAL_SHIFT 		0

//CFRM_HB Repair
#define CFRM_HB_REPAIR_QTILE_MASK 	0x03E00000
#define CFRM_HB_REPAIR_QTILE_SHIFT 	21
#define CFRM_HB_REPAIR_COL_MASK 	0x0C000000
#define CFRM_HB_REPAIR_COL_SHIFT 	26
#define CFRM_HB_REPAIR_ROW_MASK 	0xF0000000
#define CFRM_HB_REPAIR_ROW_SHIFT 	28
#define CFRM_HB_REPAIR_VAL0_MASK 	0x001FFFFF //value field in the first row
#define CFRM_HB_REPAIR_VAL0_SHIFT 	0
#define CFRM_HB_REPAIR_VAL1_MASK 	0xFFFFFFFF //value field in the second row
#define CFRM_HB_REPAIR_VAL1_SHIFT 	0
#define CFRM_HB_EXP_REPAIR_BLK_TYPE 		0x5
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_MASK 	0x00038000
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_WIDTH 	3
#define CFRM_HB_EXP_REPAIR_BLK_TYPE_SHIFT 	15
#define CFRM_HB_EXP_REPAIR_COL_MASK 		0x00007F80
#define CFRM_HB_EXP_REPAIR_COL_WIDTH 		8
#define CFRM_HB_EXP_REPAIR_COL_SHIFT 		7
#define CFRM_HB_EXP_REPAIR_QTILE_MASK 		0x0000007F
#define CFRM_HB_EXP_REPAIR_QTILE_WIDTH		7
#define CFRM_HB_EXP_REPAIR_QTILE_SHIFT 		0
#define CFRM_HB_EXP_REPAIR_VAL_MASK 		0xFFFFFFFF
#define CFRM_HB_EXP_REPAIR_VAL_SHIFT 		0
#define CFRM_HB_EXP_REPAIR_VAL1_MASK 		0x001FFFFF
#define CFRM_HB_EXP_REPAIR_VAL1_SHIFT 		0

static u32 XPmTagIdWhiteList[TAG_ID_ARRAY_SIZE] = {0};
static void XPmBisr_InitTagIdList()
{
	XPmTagIdWhiteList[LPD_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_LPD;
	XPmTagIdWhiteList[FPD_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_FPD;
	XPmTagIdWhiteList[CPM_TAG_ID] =	TAG_ID_VALID_MASK | TAG_ID_TYPE_CPM;
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
	XPm_Out32(PMC_GLOBAL_PMC_GSW_ERR,XPm_In32(PMC_GLOBAL_PMC_GSW_ERR)|(1<<ErrorCode)|(1<<PMC_GLOBAL_PMC_GSW_ERR_CR_FLAG_SHIFT));
	return;
}

static u32 XPmBisr_CopyStandard(u32 EfuseTagAddr, u32 TagSize, u64 BisrDataDestAddr)
{
	u32 TagRow;
	u32 TagData;
	u32 TagDataAddr;

	//EFUSE Tag Data start pos
	TagDataAddr = EfuseTagAddr+4;

	//Collect Repair data from EFUSE and write to endpoint base + word offset
	TagRow = 0;
	while (TagRow<TagSize) {
		if (TagDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || TagDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			TagDataAddr+=4;
		}
		TagData = XPm_In32(TagDataAddr);
		swea(BisrDataDestAddr+(TagRow<<2),TagData);
		TagRow++;
		TagDataAddr += 4;
	}
	return TagDataAddr;
}

static XStatus XPmBisr_RepairGty(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u64 BaseAddr, BisrDataDestAddr;

	BaseAddr = NPI_FIXED_BASEADDR | (TagOptional<<NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr | GTY_NPI_CACHE_DATA_REGISTER_OFFSET;

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
	u64 BisrDataDestAddr;

	BisrDataDestAddr = LPD_SLCR_BISR_CACHE_DATA_0;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	return XPmBisr_TriggerLpd();
}

int XPmBisr_TriggerLpd(void)
{
	int Status = XST_SUCCESS;
	u32 RegValue;

	/* Trigger Bisr */
	PmRmw32(LPD_SLCR_BISR_CACHE_CTRL_1, (LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK | LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK),
		 (LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK | LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK));

	PmRmw32(LPD_SLCR_BISR_CACHE_CTRL_0, LPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK,
		LPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(LPD_SLCR_BISR_CACHE_STATUS,
				 (LPD_SLCR_BISR_DONE_GLOBAL_MASK |
				  LPD_SLCR_BISR_DONE_1_MASK |
				  LPD_SLCR_BISR_DONE_0_MASK),
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr Status */
	PmIn32(LPD_SLCR_BISR_CACHE_STATUS, RegValue);
	if ((RegValue & (LPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  LPD_SLCR_BISR_PASS_1_MASK |
				  LPD_SLCR_BISR_PASS_0_MASK)) !=
				  (LPD_SLCR_BISR_PASS_GLOBAL_MASK |
				  LPD_SLCR_BISR_PASS_1_MASK |
				  LPD_SLCR_BISR_PASS_0_MASK)) {
		Status = XST_FAILURE;
	}

	/* Unwrite Trigger Bits */
	PmRmw32(LPD_SLCR_BISR_CACHE_CTRL_1, (LPD_SLCR_CACHE_CTRL_1_PGEN0_MASK | LPD_SLCR_CACHE_CTRL_1_PGEN1_MASK), 0);
done:
	return Status;
}

static XStatus XPmBisr_RepairFpd(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u64 BisrDataDestAddr;

	BisrDataDestAddr = FPD_SLCR_BISR_CACHE_DATA_0;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger Bisr */
	PmRmw32(FPD_SLCR_BISR_CACHE_CTRL_1, (FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK),
			(FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
			FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK));

	PmRmw32(FPD_SLCR_BISR_CACHE_CTRL_0, FPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK,
		FPD_SLCR_CACHE_CTRL_0_BISR_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(FPD_SLCR_BISR_CACHE_STATUS,
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
	PmIn32(FPD_SLCR_BISR_CACHE_STATUS, RegValue);
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
        PmRmw32(FPD_SLCR_BISR_CACHE_CTRL_1, (FPD_SLCR_CACHE_CTRL_1_PGEN0_MASK |
                        FPD_SLCR_CACHE_CTRL_1_PGEN1_MASK |
                        FPD_SLCR_CACHE_CTRL_1_PGEN2_MASK |
                        FPD_SLCR_CACHE_CTRL_1_PGEN3_MASK), 0);

done:
	return Status;
}


static XStatus XPmBisr_RepairCpm(u32 EfuseTagAddr, u32 TagSize, u32 *TagDataAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u64 BisrDataDestAddr;

	BisrDataDestAddr = CPM_SLCR_BISR_CACHE_DATA_0;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Clear BISR data test registers */
	PmOut32(CPM_SLCR_BISR_CACHE_CTRL, CPM_SLCR_BISR_CACHE_CTRL_CLR_MASK);
	PmOut32(CPM_SLCR_BISR_CACHE_CTRL, 0x0);

	/* Trigger Bisr */
	PmRmw32(CPM_SLCR_BISR_CACHE_CTRL, CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK,
		CPM_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK);

	/* Wait for Bisr to finish */
	Status = XPm_PollForMask(CPM_SLCR_BISR_CACHE_STATUS,
				 CPM_SLCR_BISR_CACHE_STATUS_DONE_MASK,
				 XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check Bisr status */
	PmIn32(CPM_SLCR_BISR_CACHE_STATUS, RegValue);
	if ((RegValue & CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) != CPM_SLCR_BISR_CACHE_STATUS_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	PmOut32(CPM_SLCR_BISR_CACHE_CTRL, 0x0);

done:
	return Status;
}

static XStatus XPmBisr_RepairDdrMc(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u64 BaseAddr, BisrDataDestAddr;

	BaseAddr = NPI_FIXED_BASEADDR | (TagOptional<<NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr | DDRMC_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

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
	XStatus Status = XST_SUCCESS;
	u32 RegValue;
	u64 BaseAddr, BisrDataDestAddr;

	/* Compilation warning fix */
	(void)TagId;

	BaseAddr = VIVADO_ME_BASEADDR | (TagOptional<<ME_BISR_EFUSE_OFFSET_SHIFT);
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

static int XPmBisr_RepairBram(u32 EfuseTagAddr, u32 TagSize)
{
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

	TagDataAddr = EfuseTagAddr + 4;

	while (TagRow < TagSize) {
		if (TagDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || TagDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			TagDataAddr += 4;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4;

		//break down TAG into components:
		BramRepairRow = (TagData & CFRM_BRAM_REPAIR_ROW_MASK) >> CFRM_BRAM_REPAIR_ROW_SHIFT;
		BramRepairCol = (TagData & CFRM_BRAM_REPAIR_COL_MASK) >> CFRM_BRAM_REPAIR_COL_SHIFT;
		BramRepairIndex = (TagData & CFRM_BRAM_REPAIR_INDEX_MASK) >> CFRM_BRAM_REPAIR_INDEX_SHIFT;
		BramRepairVal = (TagData & CFRM_BRAM_REPAIR_VAL_MASK) >> CFRM_BRAM_REPAIR_VAL_SHIFT;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr=CFRAME0_REG_BASEADDR+(0x2000 * BramRepairRow);

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
		for (BramRepairWord=0; BramRepairWord<4; BramRepairWord++) {
			XPm_Out32((CframeRowAddr+0x250)+(BramRepairWord<<2), BramExtendedRepair[BramRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr+0x60), 0xD);
	}
	return TagDataAddr;
}

static int XPmBisr_RepairUram(u32 EfuseTagAddr, u32 TagSize)
{
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

	TagDataAddr = EfuseTagAddr + 4;

	while (TagRow < TagSize) {
		if (TagDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || TagDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			TagDataAddr += 4;
		}
		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4;

		//break down TAG into components:
		UramRepairRow = (TagData & CFRM_URAM_REPAIR_ROW_MASK) >> CFRM_URAM_REPAIR_ROW_SHIFT;
		UramRepairCol = (TagData & CFRM_URAM_REPAIR_COL_MASK) >> CFRM_URAM_REPAIR_COL_SHIFT;
		UramRepairIndex = (TagData & CFRM_URAM_EXP_REPAIR_INDEX_MASK) >> CFRM_URAM_REPAIR_INDEX_SHIFT;
		UramRepairVal = (TagData & CFRM_URAM_REPAIR_VAL_MASK) >> CFRM_URAM_REPAIR_VAL_SHIFT;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr = CFRAME0_REG_BASEADDR + (0x2000 * UramRepairRow);

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
		for (UramRepairWord=0; UramRepairWord<4; UramRepairWord++) {
			XPm_Out32((CframeRowAddr+0x250)+(UramRepairWord<<2),UramExtendedRepair[UramRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr+0x60),0xD);
	}
	return TagDataAddr;
}

static int XPmBisr_RepairHardBlock(u32 EfuseTagAddr, u32 TagSize)
{
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

	//tag size must be multiple of 2
	TagDataAddr = EfuseTagAddr + 4;

	if ((TagSize%2) != 0) {
		XPmBisr_SwError(PMC_EFUSE_BISR_CFRM_HB_BAD_SIZE);
		TagDataAddr += (TagSize << 2);
		if ((EfuseTagAddr < EFUSE_CACHE_TBITS1_BISR_RSVD && TagDataAddr > EFUSE_CACHE_TBITS1_BISR_RSVD)
			||(EfuseTagAddr < EFUSE_CACHE_TBITS2_BISR_RSVD && TagDataAddr > EFUSE_CACHE_TBITS2_BISR_RSVD)) {
			TagDataAddr += 4;
		}
		return TagDataAddr;
	}

	TagPairCnt = 0;
	NumPairs = TagSize/2;
	while (TagPairCnt < NumPairs) {
		//get first half (row,column,qtile,value)
		if (TagDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || TagDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			TagDataAddr += 4;
		}
		TagPair[0] = XPm_In32(TagDataAddr);
		TagDataAddr += 4;
		//get second half (value)
		if (TagDataAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || TagDataAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			TagDataAddr += 4;
		}
		TagPair[1] = XPm_In32(TagDataAddr);
		TagDataAddr += 4;
		TagPairCnt++;

		//break down the components
		HbRepairRow = (TagPair[0] & CFRM_HB_REPAIR_ROW_MASK) >> CFRM_HB_REPAIR_ROW_SHIFT;
		HbRepairCol = (TagPair[0] & CFRM_HB_REPAIR_COL_MASK) >> CFRM_HB_REPAIR_COL_SHIFT;
		HbRepairQTile = (TagPair[0] & CFRM_HB_REPAIR_QTILE_MASK) >> CFRM_HB_REPAIR_QTILE_SHIFT;

		HbRepairVal[0] = TagPair[1];
		HbRepairVal[1] = TagPair[0] & CFRM_HB_REPAIR_VAL0_MASK;

		//Build address for cfrm registers based on the "row"
		//CFRM0_REG=0xF12D0000, CFRM1_REG=0xF12D2000, ...
		CframeRowAddr = CFRAME0_REG_BASEADDR + (0x2000 * HbRepairRow);

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
		for (HbRepairWord=0; HbRepairWord <4; HbRepairWord++) {
			XPm_Out32((CframeRowAddr+0x250)+(HbRepairWord<<2),HbExtendedRepair[HbRepairWord]);
		}
		//Trigger repair command
		XPm_Out32((CframeRowAddr+0x60),0xD);

	}
	return TagDataAddr;
}

XStatus XPmBisr_Repair(u32 TagId)
{
	XStatus Status = XST_SUCCESS;
	u32 EfuseRowTag;
	u32 EfuseCurrAddr;
	u32 EfuseNextAddr;
	u32 ExitCodeSeen;
	u32 EfuseBisrTagId;
	u32 EfuseBisrSize;
	u32 EfuseBisrOptional;
	u32 TagType;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	//set up the white list
	XPmBisr_InitTagIdList();

	//check requested ID is a valid ID
	if (TagId > 255) {
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
	EfuseNextAddr = PMC_EFUSE_BISR_START_ADDR;
	ExitCodeSeen = 0;

	while (ExitCodeSeen == 0) {
		//read efuse row
		EfuseCurrAddr = EfuseNextAddr;
		EfuseRowTag = XPm_In32(EfuseCurrAddr);

		if (EfuseRowTag == PMC_EFUSE_BISR_EXIT_CODE) {
			ExitCodeSeen = 1;
		} else if(EfuseRowTag==PMC_EFUSE_BISR_SKIP_CODE) { 	//SKIP Code Found
			EfuseNextAddr += 4;//then increment address and try again
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
							Status = XPmBisr_RepairGty(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
							break;
						case TAG_ID_TYPE_DDRMC:
							Status = XPmBisr_RepairDdrMc(EfuseCurrAddr, EfuseBisrSize, EfuseBisrOptional, &EfuseNextAddr);
							break;
						case TAG_ID_TYPE_CFRM_BR: //BRAM repair function
							EfuseNextAddr = XPmBisr_RepairBram(EfuseCurrAddr, EfuseBisrSize);
							break;
						case TAG_ID_TYPE_CFRM_UR: //URAM Repair function
							EfuseNextAddr = XPmBisr_RepairUram(EfuseCurrAddr, EfuseBisrSize);
							break;
						case TAG_ID_TYPE_CFRM_HB: //HardBlock repair function
							EfuseNextAddr = XPmBisr_RepairHardBlock(EfuseCurrAddr, EfuseBisrSize);
							break;
						default: //block type not recognized, no function to handle it
							XPmBisr_SwError(PMC_EFUSE_BISR_BAD_TAG_TYPE);
							Status = XST_FAILURE;
							goto done;
					}
				} else {	//calculate the next efuse address if not matched ID
					EfuseNextAddr = (EfuseCurrAddr + 4);//move to first data address of this tag
					EfuseNextAddr += (EfuseBisrSize << 2); //move down number of words from the tag size
					//did we cross over tbit row in the data size space
					if ((EfuseCurrAddr < EFUSE_CACHE_TBITS1_BISR_RSVD && EfuseNextAddr > EFUSE_CACHE_TBITS1_BISR_RSVD)
						|| (EfuseCurrAddr < EFUSE_CACHE_TBITS2_BISR_RSVD && EfuseNextAddr > EFUSE_CACHE_TBITS2_BISR_RSVD)) {
						EfuseNextAddr += 4;
					}
				}
			} else { 	//Not supported
				XPmBisr_SwError(PMC_EFUSE_BISR_UNSUPPORTED_ID);
				Status = XST_FAILURE;
				goto done;
			}
		}
		if (EfuseNextAddr == EFUSE_CACHE_TBITS1_BISR_RSVD || EfuseNextAddr == EFUSE_CACHE_TBITS2_BISR_RSVD) {
			EfuseNextAddr += 4;
		}
	}

done:
	return Status;
}

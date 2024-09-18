/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_BISR_H_
#define XPM_BISR_H_

#include "xpm_common.h"
#include "xpm_bisr_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* BISR Tag IDs */
#define LPD_TAG_ID	0x03
#define FPD_TAG_ID	0x04
#define CPM_TAG_ID	0x05
#define XRAM_TAG_ID	0x06
#define CPM5_TAG_ID	0x07
#define MEA_TAG_ID	0x08
#define MEB_TAG_ID	0x09
#define MEC_TAG_ID	0x0A
#define DDRMC_TAG_ID	0x0B
#define GTY_TAG_ID	0x0C
#define DCMAC_TAG_ID	0x0D
#define ILKN_TAG_ID	0x0E
#define MRMAC_TAG_ID	0x0F
#define SDFEC_TAG_ID	0x10
#define BRAM_TAG_ID	0x11
#define URAM_TAG_ID	0x12
#define LAGUNA_TAG_ID	0x13
#define VDU_TAG_ID  0x15
#define HSC_TAG_ID	0x16U
#define CPM5_GTYP_TAG_ID	0x17
#define GTYP_TAG_ID	0x18
#define GTM_TAG_ID	0x19
#define BFRB_TAG_ID	0x1B
#define DDRMC5_CRYPTO_TAG_ID   0x1C
#define CPM5N_TAG_ID	0x1D
#define VP1902_LAGUNA_TAG_ID	0x1E
#define LPX_TAG_ID	0x20
#define FPX_TAG_ID	0x21
#define HNICX_NTHUB_TAG_ID	0x22
#define HNICX_LCS_TAG_ID       0x23
#define HNICX_DPU_TAG_ID       0x24
#define DDRMC5_MAIN_TAG_ID   0x26


#define PMC_EFUSE_BISR_EXIT_CODE			(0U)
#define PMC_EFUSE_BISR_SKIP_CODE			(0xFFFFFFFFU)
#define PMC_EFUSE_BISR_TAG_ID_MASK			(0xFF000000U)
#define PMC_EFUSE_BISR_TAG_ID_SHIFT			(24U)
#define PMC_EFUSE_BISR_SIZE_MASK			(0x00FF0000U)
#define PMC_EFUSE_BISR_SIZE_SHIFT			(16U)
#define PMC_EFUSE_BISR_OPTIONAL_MASK			(0x0000FFFFU)
#define PMC_EFUSE_BISR_OPTIONAL_SHIFT			(0U)

#define NPI_FIXED_BASEADDR				(0xF6000000U)
#define NPI_EFUSE_ENDPOINT_SHIFT			(16U)

/************************** Function Prototypes ******************************/
XStatus XPmBisr_Repair2(u32 TagId);
u32 XPmBisr_CopyStandard(u32 EfuseTagAddr, u32 TagSize, u64 BisrDataDestAddr);
XStatus XPmBisr_RepairGty(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr, u32 TagId);
u32 XPmBisr_RepairBram(u32 EfuseTagAddr, u32 TagSize);
u32 XPmBisr_RepairUram(u32 EfuseTagAddr, u32 TagSize);
XStatus XPmBisr_TagSupportCheck2(u32 TagId);
#ifdef __cplusplus
}
#endif

#endif /* XPM_BISR_H_ */

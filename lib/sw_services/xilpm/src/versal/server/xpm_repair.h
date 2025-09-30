/******************************************************************************
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_REPAIR_H_
#define XPM_REPAIR_H_

#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

maybe_unused static inline XStatus XPmRepair_Cpm5n(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Lpx(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Fpx(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Hnicx_Nthub(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Hnicx_Dpu(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Hnicx_Lcs(u32 EfuseTagAddr, u32 TagSize,
					u32 TagOptional, const u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

/************************** Function Prototypes ******************************/
XStatus XPmRepair_Vdu(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Bfrb(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Ddrmc5_Crypto(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Ddrmc5_Main(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
#if defined(XCVR1602) || defined(XCVR1652)
XStatus XPmRepair_Cram_CacheType_SDFEC(u32 * EfuseRowTagPtr, u32 *EfuseNextAddr);

// BISR Error Bit used in bit 29:0 in PMC_GLOBAL_PMC_GSW_ERR. set by XPmBisr_SwError()
#define PMC_GSW_ERR_BISR_UNKN_TAG_ID      0x1U        // Tag ID requested to repair  is not in XPmTagIdWhiteList[]
#define PMC_GSW_ERR_BISR_INVLD_TAG_SIZE   0x2U        //
#define PMC_GSW_ERR_BISR_BAD_TAG_TYPE     0x3U        // tag     in XPmTagIdWhiteList[], but not support by current FW
#define PMC_GSW_ERR_BISR_UNSUPPORTED_ID   0x4U        // eFuse has TagID not in XPmTagIdWhiteList[]

typedef enum {
	PMC_EFUSE_BISR_STATUS_PASSED = 0,       // The BISR for TAGID has run and passed
	PMC_EFUSE_BISR_STATUS_NOREPAIR = 1,       // There is no BISR Repair data for TAGID
	PMC_EFUSE_BISR_STATUS_NOT_RUN = 5,      // eFuse Sorted, The TAG hasn't run the Bisr Repair
	ERRSTS_REPAIR_TIMEOUT         = 0x93,   // ERRSTS_PMC2ATE_REPAIR_TIMEOUT
	ERRSTS_REPAIR_FUNCTION_FAILED = 0x94,   // ERRSTS_PMC2ATE_REPAIR_FUNCTION_FAILED, Repair Done but not Pass
	ERRSTS_REPAIR_INVALID_TAGID   = 0x95,   // ERRSTS_PMC2ATE_REPAIR_INVALID_TAGID, TagID defined, but not valid for device
	ERRSTS_REPAIR_INIT_FAIL       = 0x96,   // ERRSTS_PMC2ATE_REPAIR_INIT_FAIL, may due to isolation or other reason
	ERRSTS_REPAIR_UNSUPPORT_TAGID = 0x97,   // ERRSTS_PMC2ATE_REPAIR_UNSUPPORT_TAGID, Valid TagID, but not support by GOQ FW
	ERRSTS_REPAIR_BAD_TAG_SIZE    = 0x98,   // ERRSTS_PMC2ATE_REPAIR_BAD_TAG_SIZE, BISR_SIZE error. for HardBlock repair, the tag size is unexpected (must be 1)
	ERRSTS_REPAIR_TAGID_OUTOFRANGE = 0x99,   // ERRSTS_PMC2ATE_REPAIR_TAGID_OUTOFRANGE. can't be written into st_BisrTagsTable.status
} Bisr_Repair_Status;
#endif

#ifdef XCVP1902
u32 XPmRepair_Laguna_vp1902(u32 EfuseTagAddr, u32 TagSize);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPM_REPAIR_H_ */

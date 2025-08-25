/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*****************************************************************************
 * @file xpm_repair_gtm_gty_gtyp.c
 * This is bisr handler for GT module
 *
 ******************************************************************************/

#include "xpm_repair.h"


#define     GT_PCSR_CTRL_BISR_TRIGGER_MASK          GTYP_NPI_SLAVE_0_REG_PCSR_CONTROL_BISR_TRIGGER_MASK      //  0x20000000
#define     GT_PCSR_STATUS_BISR_DONE_MASK           GTYP_NPI_SLAVE_0_REG_PCSR_STATUS_BISR_DONE_MASK
#define     GT_PCSR_STATUS_BISR_PASS_MASK           GTYP_NPI_SLAVE_0_REG_PCSR_STATUS_BISR_PASS_MASK
#define     GT_NPI_BISR_CACHE_DATA_OFFSET          (GTYP_NPI_SLAVE_0_BISR_CACHE_DATA_0 - GTYP_NPI_SLAVE_0_BASEADDR) //    0x64


XStatus XPmRepair_GtmGtyGtyp(u32 * EfuseTagAddr, u32 * TagDataAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 TagOptional = (TagIdWd & PMC_EFUSE_BISR_OPTIONAL_MASK) >> PMC_EFUSE_BISR_OPTIONAL_SHIFT;

    u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
    u64 BisrDataDestAddr =(u64)BaseAddr + (u64)GT_NPI_BISR_CACHE_DATA_OFFSET;
    if(TagSize > 2U){ /*Sanity check incase user enters incorrect data size*/  /*Dest reg assumed to be GTYP_NPI_SLAVE_0_BISR_CACHE_DATA_0*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    /* Unlock PCSR */
    XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);

    *TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

    /* Trigger Bisr */
    XPm_Out32(BaseAddr + NPI_PCSR_MASK_OFFSET,    GT_PCSR_CTRL_BISR_TRIGGER_MASK);
    XPm_Out32(BaseAddr + NPI_PCSR_CONTROL_OFFSET, GT_PCSR_CTRL_BISR_TRIGGER_MASK);

    /* Wait for Bisr to finish */
    Status = XPmcFw_UtilPollForMask(BaseAddr + NPI_PCSR_STATUS_OFFSET, GT_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        Status = ERRSTS_REPAIR_TIMEOUT;
        goto done;
    }
    u32 RegValue = XPm_In32(BaseAddr + NPI_PCSR_STATUS_OFFSET);
    if ((RegValue & GT_PCSR_STATUS_BISR_PASS_MASK) == 0U) {
        Status = ERRSTS_REPAIR_FUNCTION_FAILED;
        goto done;
    }
    Status = XST_SUCCESS;
done:
    /* Lock PCSR */
    XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, 1U);
    return Status;
}

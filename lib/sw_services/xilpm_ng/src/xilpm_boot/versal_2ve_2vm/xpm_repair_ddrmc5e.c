/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*  File name: xpm_repair_ddrmc5e.c
 *  THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 *  It is used for DDRMC5E module repair
 *  NOTE:
 *    The BISR tag ID for DDRMC5 and DDRMC5E is same.
 */

#include "xpm_repair.h"

XStatus XPmRepair_Ddrmc5_Main(u32 * EfuseTagAddr, u32 * TagDataAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 TagOptional = (TagIdWd & PMC_EFUSE_BISR_OPTIONAL_MASK) >> PMC_EFUSE_BISR_OPTIONAL_SHIFT;

    u32 RegAddrOffset = (DDRMC5E_UB_0_BISR_CACHE_DATA_0 - DDRMC5E_UB_0_BASEADDR); //    0x244U
    u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);     // Module Base Address
    u64 BisrDataDestAddr = (u64)BaseAddr + (u64)RegAddrOffset;
    if(TagSize > 2U){ /*Sanity check added in case user enters invalid data size*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    XPm_UnlockPcsr(BaseAddr);

    /* Copy repair data */
    *TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

    /* Enable BISR clock */
    XPm_RMW32((BaseAddr + (DDRMC5E_UB_0_CLK_GATE - DDRMC5E_UB_0_BASEADDR)),
            DDRMC5E_UB_0_CLK_GATE_BISR_EN_MASK, DDRMC5E_UB_0_CLK_GATE_BISR_EN_MASK);

    /* Trigger BISR. repair main if not specified crypto.  */
    XPm_RMW32((BaseAddr + NPI_PCSR_MASK_OFFSET),
            DDRMC5E_UB_0_DDRMC_PCSR_MASK_BISR_TRIGGER_MASK, DDRMC5E_UB_0_DDRMC_PCSR_MASK_BISR_TRIGGER_MASK);
    XPm_RMW32((BaseAddr + NPI_PCSR_CTRL_OFFSET),
            DDRMC5E_UB_0_DDRMC_PCSR_CONTROL_BISR_TRIGGER_MASK, DDRMC5E_UB_0_DDRMC_PCSR_CONTROL_BISR_TRIGGER_MASK);

    /* Wait for BISR to be done and check status */
    Status = XPm_PollForMask((BaseAddr + (DDRMC5E_UB_0_BISR_CACHE_STATUS - DDRMC5E_UB_0_BASEADDR)),
            DDRMC5E_UB_0_BISR_CACHE_STATUS_DONE_MASK, XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        Status = ERRSTS_REPAIR_TIMEOUT;
        goto done;
    }
    u32 RegValue = XPm_In32(BaseAddr + (DDRMC5E_UB_0_BISR_CACHE_STATUS - DDRMC5E_UB_0_BASEADDR));
    if ((RegValue & DDRMC5E_UB_0_BISR_CACHE_STATUS_PASS_MASK) == 0U) {
        Status = ERRSTS_REPAIR_FUNCTION_FAILED;
        goto done;
    }
    Status = XST_SUCCESS;
done:
    /* Disable BISR clock */
    XPm_RMW32((BaseAddr + (DDRMC5E_UB_0_CLK_GATE - DDRMC5E_UB_0_BASEADDR)),
            DDRMC5E_UB_0_CLK_GATE_BISR_EN_MASK, 0U);
    XPm_LockPcsr(BaseAddr);
    return Status;
}



XStatus XPmRepair_Ddrmc5_Crypto(u32 * EfuseTagAddr, u32 * TagDataAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK) >> PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 TagOptional = (TagIdWd & PMC_EFUSE_BISR_OPTIONAL_MASK) >> PMC_EFUSE_BISR_OPTIONAL_SHIFT;

    u32 RegAddrOffset = (DDRMC5E_UB_0_BISR_CACHE_DATA_CRYPTO_0 - DDRMC5E_UB_0_BASEADDR); //    0x244U
    u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);     // Module Base Address
    u64 BisrDataDestAddr = (u64)BaseAddr + (u64)RegAddrOffset;
    if(TagSize > 2U){  /*Sanity check added incase user enters invalid data size*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    XPm_UnlockPcsr(BaseAddr);
    /* Copy repair data */
    *TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);


    /* Enable BISR clock */
    XPm_RMW32(BaseAddr + (DDRMC5E_UB_0_CLK_GATE - DDRMC5E_UB_0_BASEADDR), DDRMC5E_UB_0_CLK_GATE_BISR_EN_CRYPTO_MASK, DDRMC5E_UB_0_CLK_GATE_BISR_EN_CRYPTO_MASK);

    /* Trigger BISR */
    XPm_RMW32((BaseAddr + NPI_PCSR_MASK_OFFSET),
            DDRMC5E_UB_0_DDRMC_PCSR_MASK_BISR_TRIGGER_CRYPTO_MASK, DDRMC5E_UB_0_DDRMC_PCSR_MASK_BISR_TRIGGER_CRYPTO_MASK);
    XPm_RMW32((BaseAddr + NPI_PCSR_CTRL_OFFSET),
            DDRMC5E_UB_0_DDRMC_PCSR_CONTROL_BISR_TRIGGER_CRYPTO_MASK, DDRMC5E_UB_0_DDRMC_PCSR_CONTROL_BISR_TRIGGER_CRYPTO_MASK);

    /* Wait for BISR to be done and check status */
    Status = XPm_PollForMask((BaseAddr + (DDRMC5E_UB_0_BISR_CACHE_STATUS_CRYPTO - DDRMC5E_UB_0_BASEADDR)),
            DDRMC5E_UB_0_BISR_CACHE_STATUS_CRYPTO_DONE_MASK, XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        Status = ERRSTS_REPAIR_TIMEOUT;
        goto done;
    }
    u32 RegValue = XPm_In32(BaseAddr + (DDRMC5E_UB_0_BISR_CACHE_STATUS_CRYPTO - DDRMC5E_UB_0_BASEADDR));
    if ((RegValue & DDRMC5E_UB_0_BISR_CACHE_STATUS_CRYPTO_PASS_MASK) == 0U) {
        Status = ERRSTS_REPAIR_FUNCTION_FAILED;
        goto done;
    }
    Status = XST_SUCCESS;
done:
    /* Disable BISR clock */
    XPm_RMW32(BaseAddr + (DDRMC5E_UB_0_CLK_GATE - DDRMC5E_UB_0_BASEADDR), DDRMC5E_UB_0_CLK_GATE_BISR_EN_CRYPTO_MASK, 0U);

    XPm_LockPcsr(BaseAddr);
    return Status;
}

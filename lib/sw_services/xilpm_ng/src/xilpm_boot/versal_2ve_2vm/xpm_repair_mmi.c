/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*  File name: xpm_repair_mmi.c
 *  THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 *  It is used for mmi and mmi_gtyp module repair   */

#include "xpm_repair.h"

XStatus XPmRepair_Mmi(u32 * EfuseTagAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 RegValue;
    u32 PassMask;
    u64 BisrDataDestAddr = (u64)MMI_SLCR_BISR_CACHE_DATA_0;
    if(TagSize > 32U){ /*Sanity check incase user enters incorrect data size*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    /* Disable writing protection */
    XPm_Out32(MMI_SLCR_WPROTP, 0U);

    /* Copy repair data */
    XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

    /* Pulse test clr */
    XPm_RMW32(MMI_SLCR_BISR_CACHE_CTRL, MMI_SLCR_BISR_CACHE_CTRL_CLR_MASK, MMI_SLCR_BISR_CACHE_CTRL_CLR_MASK);
    XPm_RMW32(MMI_SLCR_BISR_CACHE_CTRL, MMI_SLCR_BISR_CACHE_CTRL_CLR_MASK, 0U);

    /* Trigger BISR */
    XPm_RMW32(MMI_SLCR_BISR_CACHE_CTRL, MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GLOBAL_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_SC3_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_SC2_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_SC1_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_SC0_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_CG1_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_GPU_CG0_MASK
            | MMI_SLCR_BISR_CACHE_CTRL_TRIGGER_PCIE_UDH_INTWRAP_MASK
            , SET_ALLBITS_TO_ONE);

    /* Wait for BISR to finish */
    Status = XPm_PollForMask(MMI_SLCR_BISR_CACHE_STATUS, MMI_SLCR_BISR_CACHE_STATUS_DONE_GLOBAL_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_SC3_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_SC2_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_SC1_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_SC0_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_CG1_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_GPU_CG0_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_DONE_PCIE_UDH_INTWRAP_MASK
            , XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        goto done;
    }

    /* Check for BISR pass */
    PassMask = (MMI_SLCR_BISR_CACHE_STATUS_PASS_GLOBAL_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_SC3_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_SC2_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_SC1_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_SC0_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_CG1_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_GPU_CG0_MASK
            | MMI_SLCR_BISR_CACHE_STATUS_PASS_PCIE_UDH_INTWRAP_MASK);
    RegValue = XPm_In32(MMI_SLCR_BISR_CACHE_STATUS);
    if ((RegValue & PassMask) != PassMask) {
        Status = XST_FAILURE;
    }

done:
    /* Enable write protect */
    XPm_Out32(MMI_SLCR_WPROTP, 1U);
    return Status;
}


XStatus XPmRepair_Mmi_Gtyp(u32 * EfuseTagAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 RegValue;
    u32 PassMask;
    u64 BisrDataDestAddr = (u64)MMI_GTYP_CFG_BISR_CACHE_DATA_0;
    if(TagSize > 2U){ /*Sanity check incase user enters incorrect data size*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    /* Disable writing protection */
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_LOCK, 0xF9E8D7C6U);

    /* Pulse test clr */
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_MASK, MMI_GTYP_CFG_REG_PCSR_MASK_BISR_TEST_CLR_MASK);
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_CONTROL, SET_ALLBITS_TO_ONE);
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_CONTROL, 0U);

    /* Copy repair data */
    XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

    /* Trigger BISR */
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_MASK, MMI_GTYP_CFG_REG_PCSR_MASK_BISR_TRIGGER_MASK);
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_CONTROL, SET_ALLBITS_TO_ONE);

    /* Wait for BISR to finish */
    Status = XPm_PollForMask(MMI_GTYP_CFG_REG_PCSR_STATUS, MMI_GTYP_CFG_REG_PCSR_STATUS_BISR_DONE_MASK
            , XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        goto done;
    }

    /* Check for BISR pass */
    PassMask = (MMI_GTYP_CFG_REG_PCSR_STATUS_BISR_PASS_MASK);
    RegValue = XPm_In32(MMI_GTYP_CFG_REG_PCSR_STATUS);
    if ((RegValue & PassMask) != PassMask) {
        Status = XST_FAILURE;
    }

done:
    /* Enable write protect */
    XPm_Out32(MMI_GTYP_CFG_REG_PCSR_LOCK, 0U);
    return Status;
}

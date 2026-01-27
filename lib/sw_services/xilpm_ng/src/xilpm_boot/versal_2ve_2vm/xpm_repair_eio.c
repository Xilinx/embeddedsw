/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*  File name: xpm_repair_seio.c
 *  THIS FILE IS DESIGNED BY GOQ FIRMWARE TEAM FOR PLM SPECIALLY
 *  It is used for EIO module repair   */

#include "xpm_repair.h"

XStatus XPmRepair_Eio(u32 * EfuseTagAddr, u32 * TagDataAddr)
{
    XStatus Status = XST_FAILURE;
    u32 TagIdWd = *EfuseTagAddr++;
    u32 TagSize = (TagIdWd & PMC_EFUSE_BISR_SIZE_MASK)>>PMC_EFUSE_BISR_SIZE_SHIFT;
    u32 RegValue;
    u32 PassMask;
    u64 BisrDataDestAddr = (u64)EIO_SLCR_BISR_CACHE_DATA_0;

    if(TagSize > 4U){/*Sanity check in case user enters incorrect data size*/
        Status = ERRSTS_REPAIR_BAD_TAG_SIZE;
        goto done;
    }
    /* Disable writing protection */
    Xil_Out32(EIO_SLCR_WPROT0, 0U);

    /* Copy repair data */
    *TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

    /* Pulse test clr */
    XPm_RMW32(EIO_SLCR_BISR_CACHE_CTRL, EIO_SLCR_BISR_CACHE_CTRL_CLR_MASK, EIO_SLCR_BISR_CACHE_CTRL_CLR_MASK);
    XPm_RMW32(EIO_SLCR_BISR_CACHE_CTRL, EIO_SLCR_BISR_CACHE_CTRL_CLR_MASK, 0U);

    /* Trigger BISR */
    XPm_RMW32(EIO_SLCR_BISR_CACHE_CTRL, EIO_SLCR_BISR_CACHE_CTRL_TRIGGER_MASK, SET_ALLBITS_TO_ONE);

    /* Wait for BISR to finish */
    Status = XPm_PollForMask(EIO_SLCR_BISR_CACHE_STATUS, EIO_SLCR_BISR_CACHE_STATUS_DONE_MASK, XPM_POLL_TIMEOUT);
    if (XST_SUCCESS != Status) {
        Status = ERRSTS_REPAIR_TIMEOUT;
        goto done;
    }

    /* Check for BISR pass */
    PassMask = EIO_SLCR_BISR_CACHE_STATUS_PASS_MASK;
    RegValue = XPm_In32(EIO_SLCR_BISR_CACHE_STATUS);
    if ((RegValue & PassMask) != PassMask) {
        Status = ERRSTS_REPAIR_FUNCTION_FAILED;
    }

done:
    /* Enable write protect */
    Xil_Out32(EIO_SLCR_WPROT0, 1U);
    return Status;
}

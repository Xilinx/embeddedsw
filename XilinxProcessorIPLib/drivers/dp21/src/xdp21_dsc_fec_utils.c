/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdp21_dsc_fec_utils.c
 * @brief Additional utility functions for DP 2.1 DSC and FEC functionality
 *
 * This file contains utility functions for DSC and FEC operations, including
 * bandwidth calculations, compatibility checks, and integration helpers.
 */

/***************************** Include Files *********************************/
#include "xdp21_dsc.h"

#ifdef XDP_USE_DSC

#include "xdebug.h"
#include "xstatus.h"

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Calculate chunk size for DSC configuration
 *
 * @param PpsConfigPtr Pointer to PPS configuration
 * @param ChunkSize Pointer to store calculated chunk size
 *
 * @return
 *   - XST_SUCCESS if calculation is successful
 *   - XST_INVALID_PARAM if any pointer is NULL
 *
 ******************************************************************************/
int XDp21_DscCalculateChunkSize(const XDp21_DscPps *PpsConfigPtr, u32 *ChunkSize)
{
    u32 SliceWidth, BitsPerPixel, BitsPerComponent;

    /* Verify arguments */
    if (PpsConfigPtr == NULL || ChunkSize == NULL) {
        return XST_INVALID_PARAM;
    }

    SliceWidth = PpsConfigPtr->slice_width;
    BitsPerPixel = (PpsConfigPtr->bits_per_pixel_high << 8) | PpsConfigPtr->bits_per_pixel_low;
    BitsPerComponent = PpsConfigPtr->bits_per_component;
    (void)BitsPerComponent; /* Reserved for future validation */

    /* Calculate chunk size based on slice width and compression ratio */
    *ChunkSize = (SliceWidth * BitsPerPixel) / (8 * 16);  /* 16 = DSC bits per pixel fraction */

    /* Ensure minimum chunk size */
    if (*ChunkSize < 32) {
        *ChunkSize = 32;
    }

    /* Align to 8-byte boundary */
    *ChunkSize = (*ChunkSize + 7) & ~7;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Validate DSC configuration parameters
 * @note This function is deprecated - use XDp21_DscValidatePps() instead
 */
int XDp21_DscValidateConfig(const XDp21_DscPps *PpsConfigPtr)
{
    return XDp21_DscValidatePps(PpsConfigPtr);
}

/*****************************************************************************/
/**
 * @brief Get FEC error count (placeholder - FEC config not defined in core driver)
 */
int XDp21_FecGetErrorCount(const XDp21_FecConfig *FecConfigPtr, u16 *ErrorCount)
{
    if (FecConfigPtr == NULL || ErrorCount == NULL) {
        return XST_INVALID_PARAM;
    }

    *ErrorCount = (u16)FecConfigPtr->fec_error_count_corrected;
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Clear FEC error count
 *
 * @param FecConfigPtr Pointer to FEC configuration
 *
 * @return
 *   - XST_SUCCESS if error count is cleared
 *   - XST_INVALID_PARAM if FecConfigPtr is NULL
 *
 ******************************************************************************/
int XDp21_FecClearErrorCount(XDp21_FecConfig *FecConfigPtr)
{
    /* Verify arguments */
    if (FecConfigPtr == NULL) {
        return XST_INVALID_PARAM;
    }

    /* Clear all error counters defined in XDp21_FecConfig */
    FecConfigPtr->fec_error_count_uncorrected = 0;
    FecConfigPtr->fec_error_count_corrected = 0;
    FecConfigPtr->fec_bit_error_count = 0;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Calculate FEC overhead in bandwidth
 *
 * @param LinkRate Link rate in Mbps
 * @param LaneCount Number of lanes
 * @param OverheadBps Pointer to store overhead in bits per second
 *
 * @return
 *   - XST_SUCCESS if calculation is successful
 *   - XST_INVALID_PARAM if OverheadBps is NULL
 *
 ******************************************************************************/
int XDp21_FecCalculateOverhead(u32 LinkRate, u32 LaneCount, u32 *OverheadBps)
{
    u32 TotalBandwidth;

    /* Verify arguments */
    if (OverheadBps == NULL) {
        return XST_INVALID_PARAM;
    }

    /* Calculate total link bandwidth */
    TotalBandwidth = LinkRate * LaneCount * 1000000; /* Convert to bps */

    /* Calculate FEC overhead - approximately 3% */
    *OverheadBps = (TotalBandwidth * 3) / 100;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Check DSC and FEC compatibility for given link configuration
 *
 * @param DscConfigPtr Pointer to DSC configuration
 * @param FecConfigPtr Pointer to FEC configuration
 * @param LinkRate Link rate in 270 Mbps units
 * @param LaneCount Number of lanes
 *
 * @return
 *   - XST_SUCCESS if configuration is compatible
 *   - XST_INVALID_PARAM if any pointer is NULL
 *   - XST_FAILURE if configuration is not compatible
 *
 ******************************************************************************/
int XDp21_DscFecCheckCompatibility(const XDp21_DscContext *DscConfigPtr,
                                   const XDp21_FecConfig *FecConfigPtr,
                                   u32 LinkRate, u32 LaneCount)
{
    u32 AvailableBandwidth, RequiredBandwidth, FecOverhead;
    u32 ActualLinkRate;

    /* Verify arguments */
    if (DscConfigPtr == NULL || FecConfigPtr == NULL) {
        return XST_INVALID_PARAM;
    }

    /* Convert link rate to actual Mbps */
    ActualLinkRate = LinkRate * 270;

    /* Calculate available bandwidth */
    AvailableBandwidth = ActualLinkRate * LaneCount;

    /* Subtract FEC overhead if enabled */
    if (FecConfigPtr->fec_ready) {
        XDp21_FecCalculateOverhead(ActualLinkRate, LaneCount, &FecOverhead);
        AvailableBandwidth -= FecOverhead / 1000000; /* Convert to Mbps */
    }

    /* Calculate required bandwidth for DSC stream */
    if (DscConfigPtr->dsc_passthrough_enable) {
        const XDp21_DscPps *PpsPtr = &DscConfigPtr->pps_data;
        u32 BitsPerPixel = (PpsPtr->bits_per_pixel_high << 8) | PpsPtr->bits_per_pixel_low;
        RequiredBandwidth = (PpsPtr->pic_width * PpsPtr->pic_height * 60 * BitsPerPixel) /
                           (1000000 * 16); /* Assuming 60 Hz, 16 = bits per pixel fraction */
    } else {
        /* Uncompressed bandwidth calculation */
        RequiredBandwidth = (u32)(1920ULL * 1080 * 60 * 24 / 1000000); /* Example: 1080p60 RGB */
    }

    /* Check compatibility */
    if (RequiredBandwidth > AvailableBandwidth) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "Bandwidth insufficient: Required=%u Mbps, Available=%u Mbps\n",
                    (unsigned int)RequiredBandwidth,
                    (unsigned int)AvailableBandwidth);
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Get FEC status
 *
 * @param FecConfigPtr Pointer to FEC configuration
 *
 * @return
 *   - XST_SUCCESS if status is retrieved
 *   - XST_INVALID_PARAM if FecConfigPtr is NULL
 *
 ******************************************************************************/
int XDp21_FecGetStatus(XDp21_FecConfig *FecConfigPtr)
{
    /* Verify arguments */
    if (FecConfigPtr == NULL) {
        return XST_INVALID_PARAM;
    }
//DR
#if 0
    /* Update status based on enable state and error count */
    if (!FecConfigPtr->FecEnable) {
        FecConfigPtr->FecStatus = XDP21_FEC_STATUS_DISABLED;
    } else if (FecConfigPtr->FecErrorCount > 0) {
        FecConfigPtr->FecStatus = XDP21_FEC_STATUS_CORRECTING;
    } else {
        FecConfigPtr->FecStatus = XDP21_FEC_STATUS_ENABLED;
    }
#endif
    return XST_SUCCESS;
}

#endif /* XDP_USE_DSC */
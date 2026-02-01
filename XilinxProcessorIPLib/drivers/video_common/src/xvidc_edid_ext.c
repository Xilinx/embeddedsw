/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvidc_edid_ext.c
*
* <b>Software Initialization & Configuration</b>
*
* <b>Interrupts </b>
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* 1.0   mmo  24-01-2017 EDID Parser capability
* </pre>
*
******************************************************************************/
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xvidc_edid_ext.h"

static XV_VidC_PicAspectRatio xv_vidc_getPicAspectRatio(u16 hres, u16 vres);

/*****************************************************************************/
/**
*
* This function calculates the picture aspect ratio from horizontal and
* vertical resolution values.
*
* @param    hres is the horizontal resolution in pixels.
* @param    vres is the vertical resolution in pixels.
*
* @return   XV_VidC_PicAspectRatio structure containing the width and height
*           aspect ratio values (16:10, 4:3, 5:4, 16:9, or 0:0 if no match).
*
******************************************************************************/
static XV_VidC_PicAspectRatio xv_vidc_getPicAspectRatio(u16 hres, u16 vres) {
    XV_VidC_PicAspectRatio ar;
/** Macro to check if horizontal/vertical resolution matches specified aspect ratio */
#define HAS_RATIO_OF(x, y)  (hres == (vres*(x)/(y))&&!((vres*(x))%(y)))
    if (HAS_RATIO_OF(16, 10)) {
        ar.width = 16;
        ar.height = 10;
        return ar;
    }
    if (HAS_RATIO_OF(4, 3)) {
        ar.width = 4;
        ar.height = 3;
        return ar;
    }
    if (HAS_RATIO_OF(5, 4)) {
        ar.width = 5;
        ar.height = 4;
        return ar;
    }
    if (HAS_RATIO_OF(16, 9)) {
        ar.width = 16;
        ar.height = 9;
        return ar;
#undef HAS_RATIO
    } else {
        ar.width = 0;
        ar.height = 0;
        return ar;
    }
}


void XV_VidC_EdidCtrlParamInit (XV_VidC_EdidCntrlParam *EdidCtrlParam) {

	/* Verify arguments. */
	Xil_AssertVoid(EdidCtrlParam != NULL);

	(void)memset((void *)EdidCtrlParam,  0,
			sizeof(XV_VidC_EdidCntrlParam));

	EdidCtrlParam->IsHdmi                = XVIDC_ISDVI; /**< HDMI or DVI mode flag */
    EdidCtrlParam->IsYCbCr444Supp        = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:4:4 color space support */
    EdidCtrlParam->IsYCbCr420Supp        = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:2:0 color space support */
    EdidCtrlParam->IsYCbCr422Supp        = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:2:2 color space support */
    EdidCtrlParam->IsYCbCr444DeepColSupp = XVIDC_NOT_SUPPORTED; /**< Deep color support for YCbCr 4:4:4 */
    EdidCtrlParam->Is30bppSupp           = XVIDC_NOT_SUPPORTED; /**< 30 bits per pixel support */
    EdidCtrlParam->Is36bppSupp           = XVIDC_NOT_SUPPORTED; /**< 36 bits per pixel support */
    EdidCtrlParam->Is48bppSupp           = XVIDC_NOT_SUPPORTED; /**< 48 bits per pixel support */
    EdidCtrlParam->IsYCbCr420dc30bppSupp = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:2:0 30bpp deep color support */
    EdidCtrlParam->IsYCbCr420dc36bppSupp = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:2:0 36bpp deep color support */
    EdidCtrlParam->IsYCbCr420dc48bppSupp = XVIDC_NOT_SUPPORTED; /**< YCbCr 4:2:0 48bpp deep color support */
    EdidCtrlParam->MaxFrlLineRateSupp    = XVIDC_NOT_SUPPORTED; /**< Maximum Fixed Rate Link line rate */
    EdidCtrlParam->MaxFrlLanesSupp       = XVIDC_NOT_SUPPORTED; /**< Maximum FRL lanes supported */
    EdidCtrlParam->IsSCDCReadRequestReady= XVIDC_NOT_SUPPORTED; /**< SCDC read request ready flag */
    EdidCtrlParam->IsSCDCPresent         = XVIDC_NOT_SUPPORTED; /**< SCDC presence flag */
    EdidCtrlParam->MaxFrameRateSupp      = 0; /**< Maximum supported frame rate */
    EdidCtrlParam->MaxTmdsMhz            = 0; /**< Maximum TMDS clock in MHz */
    /* DisplayID/EDID 2.0 initialization */
    EdidCtrlParam->IsDispIdPresent       = XVIDC_NOT_SUPPORTED; /**< DisplayID presence flag */
    EdidCtrlParam->DispIdVersion         = 0; /**< DisplayID version number */
    EdidCtrlParam->DispIdProductType     = 0; /**< DisplayID product type */
    EdidCtrlParam->DispIdExtensionCount  = 0; /**< DisplayID extension count */
    /* Product Identification */
    (void)memset(EdidCtrlParam->DispIdManufacturer, 0, sizeof(EdidCtrlParam->DispIdManufacturer)); /**< DisplayID manufacturer ID */
    EdidCtrlParam->DispIdProductCode     = 0; /**< DisplayID product code */
    EdidCtrlParam->DispIdSerialNumber    = 0; /**< DisplayID serial number */
    EdidCtrlParam->DispIdModelYear       = 0; /**< DisplayID model year */
    EdidCtrlParam->DispIdModelWeek       = 0; /**< DisplayID model week */
    (void)memset(EdidCtrlParam->DispIdProductString, 0, sizeof(EdidCtrlParam->DispIdProductString)); /**< DisplayID product name string */
    EdidCtrlParam->DispIdProductStringLen = 0; /**< Product string length */
    /* Display Parameters */
    EdidCtrlParam->DispIdImageWidthMm    = 0; /**< Display image width in mm */
    EdidCtrlParam->DispIdImageHeightMm   = 0; /**< Display image height in mm */
    EdidCtrlParam->DispIdNativeWidth     = 0; /**< Native horizontal resolution */
    EdidCtrlParam->DispIdNativeHeight    = 0; /**< Native vertical resolution */
    EdidCtrlParam->DispIdFeatureSupportFlags = 0; /**< Feature support flags */
    EdidCtrlParam->DispIdGamma           = 0xFF;  /**< Gamma value (0xFF = not defined) */
    EdidCtrlParam->DispIdAspectRatio     = 0; /**< Display aspect ratio */
    EdidCtrlParam->DispIdBitDepthNative  = 0; /**< Native bit depth */
    EdidCtrlParam->DispIdBitDepthOverall = 0; /**< Overall bit depth */
    EdidCtrlParam->DispIdScanOrientation = 0; /**< Scan orientation */
    EdidCtrlParam->DispIdTechnology      = 0; /**< Display technology type */
    EdidCtrlParam->DispIdAudioSupport    = XVIDC_NOT_SUPPORTED; /**< Audio support flag */
    EdidCtrlParam->DispIdSeparateAudio   = XVIDC_NOT_SUPPORTED; /**< Separate audio inputs flag */
    EdidCtrlParam->DispIdAudioOverride   = XVIDC_NOT_SUPPORTED; /**< Audio input override flag */
    EdidCtrlParam->DispIdPowerSequenceReq = XVIDC_NOT_SUPPORTED; /**< Power management sequence required */
    EdidCtrlParam->DispIdFixedPixelFormat = XVIDC_NOT_SUPPORTED; /**< Fixed pixel format flag */
    EdidCtrlParam->DispIdDeinterlacing   = XVIDC_NOT_SUPPORTED; /**< Deinterlacing support */
    /* Color Characteristics */
    EdidCtrlParam->DispIdColorDepth      = 0; /**< Color depth */
    EdidCtrlParam->DispIdColorEncoding   = 0; /**< Color encoding format */
    /* DisplayID Interface Color Depth Support */
    EdidCtrlParam->DispIdRgbColorDepth     = 8; /**< RGB interface color depth (default 8 bpc) */
    EdidCtrlParam->DispIdYCbCr444ColorDepth = 8; /**< YCbCr 4:4:4 interface color depth (default 8 bpc) */
    EdidCtrlParam->DispIdYCbCr422ColorDepth = 8; /**< YCbCr 4:2:2 interface color depth (default 8 bpc) */
    EdidCtrlParam->DispIdYCbCr420ColorDepth = 8; /**< YCbCr 4:2:0 interface color depth (default 8 bpc) */
    EdidCtrlParam->DispIdPrimaryRedX     = 0; /**< Red primary chromaticity X */
    EdidCtrlParam->DispIdPrimaryRedY     = 0; /**< Red primary chromaticity Y */
    EdidCtrlParam->DispIdPrimaryGreenX   = 0; /**< Green primary chromaticity X */
    EdidCtrlParam->DispIdPrimaryGreenY   = 0; /**< Green primary chromaticity Y */
    EdidCtrlParam->DispIdPrimaryBlueX    = 0; /**< Blue primary chromaticity X */
    EdidCtrlParam->DispIdPrimaryBlueY    = 0; /**< Blue primary chromaticity Y */
    EdidCtrlParam->DispIdWhitePointX     = 0; /**< White point chromaticity X */
    EdidCtrlParam->DispIdWhitePointY     = 0; /**< White point chromaticity Y */
    EdidCtrlParam->DispIdMinLuminance    = 0; /**< Minimum luminance */
    EdidCtrlParam->DispIdMaxLuminance    = 0; /**< Maximum luminance */
    EdidCtrlParam->DispIdMaxFullFrameLum = 0; /**< Maximum full frame luminance */
    /* Timing Range Data (0x09) */
    EdidCtrlParam->DispIdTimingRangeMinHfreq = 0; /**< Timing range minimum horizontal frequency */
    EdidCtrlParam->DispIdTimingRangeMaxHfreq = 0; /**< Timing range maximum horizontal frequency */
    EdidCtrlParam->DispIdTimingRangeMinVfreq = 0; /**< Timing range minimum vertical frequency */
    EdidCtrlParam->DispIdTimingRangeMaxVfreq = 0; /**< Timing range maximum vertical frequency */
    EdidCtrlParam->DispIdTimingRangeMaxPixclk = 0; /**< Timing range maximum pixel clock */
    EdidCtrlParam->DispIdTimingRangeFlags = 0; /**< Timing range flags */
    /* Serial Number String (0x0A) */
    (void)memset(EdidCtrlParam->DispIdSerialString, 0, sizeof(EdidCtrlParam->DispIdSerialString)); /**< DisplayID serial number string */
    EdidCtrlParam->DispIdSerialStringLen = 0; /**< Serial string length */
    /* ASCII String (0x0B) */
    (void)memset(EdidCtrlParam->DispIdAsciiString, 0, sizeof(EdidCtrlParam->DispIdAsciiString)); /**< DisplayID ASCII string */
    EdidCtrlParam->DispIdAsciiStringLen = 0; /**< ASCII string length */
    /* Device Data (0x0C) */
    EdidCtrlParam->DispIdDeviceType = 0; /**< Device type */
    EdidCtrlParam->DispIdDeviceColorSpace = 0; /**< Device color space */
    EdidCtrlParam->DispIdDeviceHorSize = 0; /**< Device horizontal size */
    EdidCtrlParam->DispIdDeviceVerSize = 0; /**< Device vertical size */
    /* Power Sequencing (0x0D) */
    EdidCtrlParam->DispIdPowerSeqT1 = 0; /**< Power sequence timing T1 */
    EdidCtrlParam->DispIdPowerSeqT2 = 0; /**< Power sequence timing T2 */
    EdidCtrlParam->DispIdPowerSeqT3 = 0; /**< Power sequence timing T3 */
    EdidCtrlParam->DispIdPowerSeqT4 = 0; /**< Power sequence timing T4 */
    /* Transfer Characteristics (0x0E) */
    EdidCtrlParam->DispIdTransferType = 0; /**< Transfer characteristics type */
    EdidCtrlParam->DispIdTransferFlags = 0; /**< Transfer characteristics flags */
    EdidCtrlParam->DispIdOecfEotfData[0] = 0; /**< OECF/EOTF data byte 0 */
    EdidCtrlParam->DispIdOecfEotfData[1] = 0; /**< OECF/EOTF data byte 1 */
    /* Short Timings */
    (void)memset(EdidCtrlParam->DispIdShortTiming, 0, sizeof(EdidCtrlParam->DispIdShortTiming)); /**< DisplayID short timing descriptors */
    EdidCtrlParam->DispIdNumShortTimings = 0; /**< Number of short timing descriptors */
    /* DMT Timings */
    (void)memset(EdidCtrlParam->DispIdDmtTiming, 0, sizeof(EdidCtrlParam->DispIdDmtTiming)); /**< DisplayID DMT timing codes */
    EdidCtrlParam->DispIdNumDmtTimings = 0; /**< Number of DMT timing codes */
    /* CVT Timings */
    (void)memset(EdidCtrlParam->DispIdCvtTiming, 0, sizeof(EdidCtrlParam->DispIdCvtTiming)); /**< DisplayID CVT timing codes */
    EdidCtrlParam->DispIdNumCvtTimings = 0; /**< Number of CVT timing codes */
    /* Video Timing Modes */
    EdidCtrlParam->DispIdNumTimings      = 0; /**< Total number of video timing modes */
    /* Supported Timing Modes */
    (void)memset(EdidCtrlParam->DispIdType6Timing, 0, sizeof(EdidCtrlParam->DispIdType6Timing)); /**< DisplayID Type VI timing descriptors */
    EdidCtrlParam->DispIdNumType6Timings = 0; /**< Number of Type VI timing descriptors */
    (void)memset(EdidCtrlParam->DispIdType7Timing, 0, sizeof(EdidCtrlParam->DispIdType7Timing)); /**< DisplayID Type VII timing descriptors */
    EdidCtrlParam->DispIdNumType7Timings = 0; /**< Number of Type VII timing descriptors */
    (void)memset(EdidCtrlParam->DispIdType8Timing, 0, sizeof(EdidCtrlParam->DispIdType8Timing)); /**< DisplayID Type VIII timing descriptors */
    EdidCtrlParam->DispIdNumType8Timings = 0; /**< Number of Type VIII timing descriptors */
    (void)memset(EdidCtrlParam->DispIdType9Timing, 0, sizeof(EdidCtrlParam->DispIdType9Timing)); /**< DisplayID Type IX timing descriptors */
    EdidCtrlParam->DispIdNumType9Timings = 0; /**< Number of Type IX timing descriptors */
    (void)memset(EdidCtrlParam->DispIdType10Timing, 0, sizeof(EdidCtrlParam->DispIdType10Timing)); /**< DisplayID Type X timing descriptors */
    EdidCtrlParam->DispIdNumType10Timings = 0; /**< Number of Type X timing descriptors */
    /* Display Interface Features */
    EdidCtrlParam->DispIdInterfaceType   = 0; /**< Display interface type */
    EdidCtrlParam->DispIdNumLanes        = 0; /**< Number of interface lanes */
    EdidCtrlParam->DispIdInterfaceVersion = 0; /**< Interface version */
    EdidCtrlParam->DispIdContentProtection = 0; /**< Content protection support */
    EdidCtrlParam->DispIdSpreadSpectrum  = 0; /**< Spread spectrum clocking support */
    EdidCtrlParam->DispIdColorFormats    = 0; /**< Supported color formats */
    EdidCtrlParam->DispIdMinPixelClkMhz  = 0; /**< Minimum pixel clock in MHz */
    EdidCtrlParam->DispIdMaxPixelClkMhz  = 0; /**< Maximum pixel clock in MHz */
    /* Stereo Display Interface */
    EdidCtrlParam->DispIdStereoInterface = 0; /**< Stereo display interface type */
    EdidCtrlParam->DispIdStereoPolarity  = 0; /**< Stereo polarity */
    /* Tiled Display Topology */
    EdidCtrlParam->DispIdIsTiled         = XVIDC_NOT_SUPPORTED; /**< Tiled display flag */
    EdidCtrlParam->DispIdTileRows        = 0; /**< Number of tile rows */
    EdidCtrlParam->DispIdTileCols        = 0; /**< Number of tile columns */
    EdidCtrlParam->DispIdTileLocH        = 0; /**< Tile horizontal location */
    EdidCtrlParam->DispIdTileLocV        = 0; /**< Tile vertical location */
    EdidCtrlParam->DispIdTileWidth       = 0; /**< Tile width in pixels */
    EdidCtrlParam->DispIdTileHeight      = 0; /**< Tile height in pixels */
    EdidCtrlParam->DispIdTileBezelTop    = 0; /**< Tile bezel width - top */
    EdidCtrlParam->DispIdTileBezelBottom = 0; /**< Tile bezel width - bottom */
    EdidCtrlParam->DispIdTileBezelLeft   = 0; /**< Tile bezel width - left */
    EdidCtrlParam->DispIdTileBezelRight  = 0; /**< Tile bezel width - right */
    EdidCtrlParam->DispIdTileCapabilities = 0; /**< Tile capabilities */
    (void)memset(EdidCtrlParam->DispIdTileSerialNum, 0, sizeof(EdidCtrlParam->DispIdTileSerialNum)); /**< Tile serial number */
    /* Container ID */
    (void)memset(EdidCtrlParam->DispIdContainerId, 0, sizeof(EdidCtrlParam->DispIdContainerId)); /**< Container ID UUID */
    /* Vendor Specific */
    (void)memset(EdidCtrlParam->DispIdVendorTag, 0, sizeof(EdidCtrlParam->DispIdVendorTag)); /**< Vendor-specific data block tag */
    (void)memset(EdidCtrlParam->DispIdVendorData, 0, sizeof(EdidCtrlParam->DispIdVendorData)); /**< Vendor-specific data */
    EdidCtrlParam->DispIdVendorDataLen   = 0; /**< Vendor data length */
    /* CTA DisplayID */
    EdidCtrlParam->DispIdCtaPresent      = XVIDC_NOT_SUPPORTED; /**< CTA DisplayID presence flag */
    (void)memset(EdidCtrlParam->DispIdCtaVic, 0, sizeof(EdidCtrlParam->DispIdCtaVic)); /**< CTA Video Identification Codes */
    EdidCtrlParam->DispIdNumCtaVic       = 0; /**< Number of CTA VICs */
    EdidCtrlParam->DispIdCtaBlockTag     = 0; /**< CTA data block tag */
    EdidCtrlParam->DispIdCtaBlockPayloadSize = 0; /**< CTA data block payload size */
    /* Adaptive Refresh Rate */
    EdidCtrlParam->DispIdAdaptiveRefreshFlags = 0; /**< Adaptive refresh rate flags */
    /* Unicode String */
    (void)memset(EdidCtrlParam->DispIdUnicodeString, 0, sizeof(EdidCtrlParam->DispIdUnicodeString)); /**< Unicode string */
    EdidCtrlParam->DispIdUnicodeStringLen = 0; /**< Unicode string length */
    /* Detailed Timing Type 7 */
    (void)memset(EdidCtrlParam->DispIdDetailedTiming7Data, 0, sizeof(EdidCtrlParam->DispIdDetailedTiming7Data)); /**< Detailed timing type 7 data */
    EdidCtrlParam->DispIdDetailedTiming7Len = 0; /**< Detailed timing type 7 data length */
}

XV_VidC_TimingParam
XV_VidC_timing
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    XV_VidC_TimingParam timing;

    timing.hres   = xvidc_edid_detailed_timing_horizontal_active(dtb);
    timing.vres   = xvidc_edid_detailed_timing_vertical_active(dtb);
    timing.htotal = timing.hres +
                        xvidc_edid_detailed_timing_horizontal_blanking(dtb);
    timing.vtotal = timing.vres +
                          xvidc_edid_detailed_timing_vertical_blanking(dtb);
    timing.hfp    = xvidc_edid_detailed_timing_horizontal_sync_offset(dtb);
    timing.vfp    = xvidc_edid_detailed_timing_vertical_sync_offset(dtb);
    timing.hsync_width  =
                xvidc_edid_detailed_timing_horizontal_sync_pulse_width(dtb);
    timing.vsync_width  =
                  xvidc_edid_detailed_timing_vertical_sync_pulse_width(dtb);
    timing.pixclk       = xvidc_edid_detailed_timing_pixel_clock(dtb);
    timing.vfreq        = (timing.pixclk / (timing.vtotal * timing.htotal));
    timing.vidfrmt      = (XVidC_VideoFormat) dtb->interlaced;
    timing.aspect_ratio =
                         xv_vidc_getPicAspectRatio (timing.hres, timing.vres);
    timing.hsync_polarity = dtb->signal_pulse_polarity;
    timing.vsync_polarity = dtb->signal_serration_polarity;

    return timing;
}

#if XVIDC_EDID_VERBOSITY > 1
/*****************************************************************************/
/**
*
* This function converts a double-precision floating point value into integer
* and decimal components for display purposes (available when verbosity > 1).
*
* @param    in_val is the double-precision floating point value to convert.
*
* @return   XV_VidC_DoubleRep structure containing the integer part and
*           decimal part (scaled by 10000).
*
******************************************************************************/
XV_VidC_DoubleRep Double2Int (double in_val) {
	XV_VidC_DoubleRep DR;

	DR.Integer = in_val;
	DR.Decimal = (in_val - DR.Integer) * 10000;

	return (DR);
}
#endif

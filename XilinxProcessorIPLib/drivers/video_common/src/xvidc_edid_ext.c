/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_edid.h
*
* <b>Software Initalization & Configuration</b>
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

static XV_VidC_PicAspectRatio xv_vidc_getPicAspectRatio(u16 hres, u16 vres) {
    XV_VidC_PicAspectRatio ar;
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

	EdidCtrlParam->IsHdmi                = XVIDC_ISDVI;
    EdidCtrlParam->IsYCbCr444Supp        = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr420Supp        = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr422Supp        = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr444DeepColSupp = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->Is30bppSupp           = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->Is36bppSupp           = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->Is48bppSupp           = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr420dc30bppSupp = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr420dc36bppSupp = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsYCbCr420dc48bppSupp = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->MaxFrlLineRateSupp    = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->MaxFrlLanesSupp       = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsSCDCReadRequestReady= XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->IsSCDCPresent         = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->MaxFrameRateSupp      = 0;
    EdidCtrlParam->MaxTmdsMhz            = 0;
    /* DisplayID/EDID 2.0 initialization */
    EdidCtrlParam->IsDispIdPresent       = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdVersion         = 0;
    EdidCtrlParam->DispIdProductType     = 0;
    EdidCtrlParam->DispIdExtensionCount  = 0;
    /* Product Identification */
    (void)memset(EdidCtrlParam->DispIdManufacturer, 0, sizeof(EdidCtrlParam->DispIdManufacturer));
    EdidCtrlParam->DispIdProductCode     = 0;
    EdidCtrlParam->DispIdSerialNumber    = 0;
    EdidCtrlParam->DispIdModelYear       = 0;
    EdidCtrlParam->DispIdModelWeek       = 0;
    (void)memset(EdidCtrlParam->DispIdProductString, 0, sizeof(EdidCtrlParam->DispIdProductString));
    EdidCtrlParam->DispIdProductStringLen = 0;
    /* Display Parameters */
    EdidCtrlParam->DispIdImageWidthMm    = 0;
    EdidCtrlParam->DispIdImageHeightMm   = 0;
    EdidCtrlParam->DispIdNativeWidth     = 0;
    EdidCtrlParam->DispIdNativeHeight    = 0;
    EdidCtrlParam->DispIdFeatureSupportFlags = 0;
    EdidCtrlParam->DispIdGamma           = 0xFF;  /* Not defined */
    EdidCtrlParam->DispIdAspectRatio     = 0;
    EdidCtrlParam->DispIdBitDepthNative  = 0;
    EdidCtrlParam->DispIdBitDepthOverall = 0;
    EdidCtrlParam->DispIdScanOrientation = 0;
    EdidCtrlParam->DispIdTechnology      = 0;
    EdidCtrlParam->DispIdAudioSupport    = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdSeparateAudio   = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdAudioOverride   = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdPowerSequenceReq = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdFixedPixelFormat = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdDeinterlacing   = XVIDC_NOT_SUPPORTED;
    /* Color Characteristics */
    EdidCtrlParam->DispIdColorDepth      = 0;
    EdidCtrlParam->DispIdColorEncoding   = 0;
    /* DisplayID Interface Color Depth Support */
    EdidCtrlParam->DispIdRgbColorDepth     = 8; /* Default to 8 bpc */
    EdidCtrlParam->DispIdYCbCr444ColorDepth = 8; /* Default to 8 bpc */
    EdidCtrlParam->DispIdYCbCr422ColorDepth = 8; /* Default to 8 bpc */
    EdidCtrlParam->DispIdYCbCr420ColorDepth = 8; /* Default to 8 bpc */
    EdidCtrlParam->DispIdPrimaryRedX     = 0;
    EdidCtrlParam->DispIdPrimaryRedY     = 0;
    EdidCtrlParam->DispIdPrimaryGreenX   = 0;
    EdidCtrlParam->DispIdPrimaryGreenY   = 0;
    EdidCtrlParam->DispIdPrimaryBlueX    = 0;
    EdidCtrlParam->DispIdPrimaryBlueY    = 0;
    EdidCtrlParam->DispIdWhitePointX     = 0;
    EdidCtrlParam->DispIdWhitePointY     = 0;
    EdidCtrlParam->DispIdMinLuminance    = 0;
    EdidCtrlParam->DispIdMaxLuminance    = 0;
    EdidCtrlParam->DispIdMaxFullFrameLum = 0;
    /* Timing Range Data (0x09) */
    EdidCtrlParam->DispIdTimingRangeMinHfreq = 0;
    EdidCtrlParam->DispIdTimingRangeMaxHfreq = 0;
    EdidCtrlParam->DispIdTimingRangeMinVfreq = 0;
    EdidCtrlParam->DispIdTimingRangeMaxVfreq = 0;
    EdidCtrlParam->DispIdTimingRangeMaxPixclk = 0;
    EdidCtrlParam->DispIdTimingRangeFlags = 0;
    /* Serial Number String (0x0A) */
    (void)memset(EdidCtrlParam->DispIdSerialString, 0, sizeof(EdidCtrlParam->DispIdSerialString));
    EdidCtrlParam->DispIdSerialStringLen = 0;
    /* ASCII String (0x0B) */
    (void)memset(EdidCtrlParam->DispIdAsciiString, 0, sizeof(EdidCtrlParam->DispIdAsciiString));
    EdidCtrlParam->DispIdAsciiStringLen = 0;
    /* Device Data (0x0C) */
    EdidCtrlParam->DispIdDeviceType = 0;
    EdidCtrlParam->DispIdDeviceColorSpace = 0;
    EdidCtrlParam->DispIdDeviceHorSize = 0;
    EdidCtrlParam->DispIdDeviceVerSize = 0;
    /* Power Sequencing (0x0D) */
    EdidCtrlParam->DispIdPowerSeqT1 = 0;
    EdidCtrlParam->DispIdPowerSeqT2 = 0;
    EdidCtrlParam->DispIdPowerSeqT3 = 0;
    EdidCtrlParam->DispIdPowerSeqT4 = 0;
    /* Transfer Characteristics (0x0E) */
    EdidCtrlParam->DispIdTransferType = 0;
    EdidCtrlParam->DispIdTransferFlags = 0;
    EdidCtrlParam->DispIdOecfEotfData[0] = 0;
    EdidCtrlParam->DispIdOecfEotfData[1] = 0;
    /* Short Timings */
    (void)memset(EdidCtrlParam->DispIdShortTiming, 0, sizeof(EdidCtrlParam->DispIdShortTiming));
    EdidCtrlParam->DispIdNumShortTimings = 0;
    /* DMT Timings */
    (void)memset(EdidCtrlParam->DispIdDmtTiming, 0, sizeof(EdidCtrlParam->DispIdDmtTiming));
    EdidCtrlParam->DispIdNumDmtTimings = 0;
    /* CVT Timings */
    (void)memset(EdidCtrlParam->DispIdCvtTiming, 0, sizeof(EdidCtrlParam->DispIdCvtTiming));
    EdidCtrlParam->DispIdNumCvtTimings = 0;
    /* Video Timing Modes */
    EdidCtrlParam->DispIdNumTimings      = 0;
    /* Supported Timing Modes */
    (void)memset(EdidCtrlParam->DispIdType6Timing, 0, sizeof(EdidCtrlParam->DispIdType6Timing));
    EdidCtrlParam->DispIdNumType6Timings = 0;
    (void)memset(EdidCtrlParam->DispIdType7Timing, 0, sizeof(EdidCtrlParam->DispIdType7Timing));
    EdidCtrlParam->DispIdNumType7Timings = 0;
    (void)memset(EdidCtrlParam->DispIdType8Timing, 0, sizeof(EdidCtrlParam->DispIdType8Timing));
    EdidCtrlParam->DispIdNumType8Timings = 0;
    (void)memset(EdidCtrlParam->DispIdType9Timing, 0, sizeof(EdidCtrlParam->DispIdType9Timing));
    EdidCtrlParam->DispIdNumType9Timings = 0;
    (void)memset(EdidCtrlParam->DispIdType10Timing, 0, sizeof(EdidCtrlParam->DispIdType10Timing));
    EdidCtrlParam->DispIdNumType10Timings = 0;
    /* Display Interface Features */
    EdidCtrlParam->DispIdInterfaceType   = 0;
    EdidCtrlParam->DispIdNumLanes        = 0;
    EdidCtrlParam->DispIdInterfaceVersion = 0;
    EdidCtrlParam->DispIdContentProtection = 0;
    EdidCtrlParam->DispIdSpreadSpectrum  = 0;
    EdidCtrlParam->DispIdColorFormats    = 0;
    EdidCtrlParam->DispIdMinPixelClkMhz  = 0;
    EdidCtrlParam->DispIdMaxPixelClkMhz  = 0;
    /* Stereo Display Interface */
    EdidCtrlParam->DispIdStereoInterface = 0;
    EdidCtrlParam->DispIdStereoPolarity  = 0;
    /* Tiled Display Topology */
    EdidCtrlParam->DispIdIsTiled         = XVIDC_NOT_SUPPORTED;
    EdidCtrlParam->DispIdTileRows        = 0;
    EdidCtrlParam->DispIdTileCols        = 0;
    EdidCtrlParam->DispIdTileLocH        = 0;
    EdidCtrlParam->DispIdTileLocV        = 0;
    EdidCtrlParam->DispIdTileWidth       = 0;
    EdidCtrlParam->DispIdTileHeight      = 0;
    EdidCtrlParam->DispIdTileBezelTop    = 0;
    EdidCtrlParam->DispIdTileBezelBottom = 0;
    EdidCtrlParam->DispIdTileBezelLeft   = 0;
    EdidCtrlParam->DispIdTileBezelRight  = 0;
    EdidCtrlParam->DispIdTileCapabilities = 0;
    (void)memset(EdidCtrlParam->DispIdTileSerialNum, 0, sizeof(EdidCtrlParam->DispIdTileSerialNum));
    /* Container ID */
    (void)memset(EdidCtrlParam->DispIdContainerId, 0, sizeof(EdidCtrlParam->DispIdContainerId));
    /* Vendor Specific */
    (void)memset(EdidCtrlParam->DispIdVendorTag, 0, sizeof(EdidCtrlParam->DispIdVendorTag));
    (void)memset(EdidCtrlParam->DispIdVendorData, 0, sizeof(EdidCtrlParam->DispIdVendorData));
    EdidCtrlParam->DispIdVendorDataLen   = 0;
    /* CTA DisplayID */
    EdidCtrlParam->DispIdCtaPresent      = XVIDC_NOT_SUPPORTED;
    (void)memset(EdidCtrlParam->DispIdCtaVic, 0, sizeof(EdidCtrlParam->DispIdCtaVic));
    EdidCtrlParam->DispIdNumCtaVic       = 0;
    EdidCtrlParam->DispIdCtaBlockTag     = 0;
    EdidCtrlParam->DispIdCtaBlockPayloadSize = 0;
    /* Adaptive Refresh Rate */
    EdidCtrlParam->DispIdAdaptiveRefreshFlags = 0;
    /* Unicode String */
    (void)memset(EdidCtrlParam->DispIdUnicodeString, 0, sizeof(EdidCtrlParam->DispIdUnicodeString));
    EdidCtrlParam->DispIdUnicodeStringLen = 0;
    /* Detailed Timing Type 7 */
    (void)memset(EdidCtrlParam->DispIdDetailedTiming7Data, 0, sizeof(EdidCtrlParam->DispIdDetailedTiming7Data));
    EdidCtrlParam->DispIdDetailedTiming7Len = 0;
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
XV_VidC_DoubleRep Double2Int (double in_val) {
	XV_VidC_DoubleRep DR;

	DR.Integer = in_val;
	DR.Decimal = (in_val - DR.Integer) * 10000;

	return (DR);
}
#endif

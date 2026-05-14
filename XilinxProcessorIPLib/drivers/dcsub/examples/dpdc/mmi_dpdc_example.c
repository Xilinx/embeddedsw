/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_example.c
*
* This file contains the main application entry point
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 Initial version
*
* </pre>
*
******************************************************************************/

#include <stdio.h>
#include <sleep.h>
#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "xdcsub.h"
#include "mmi_dpdc_menus.h"
#include "mmi_dc_nonlive_test.h"
#include "mmi_dc_live_test.h"
#include "mmi_dp_init.h"

/* External declarations from mmi_dc_nonlive_test.c */
extern void outbyte(char c);
extern char inbyte(void);

RunConfig RunCfg;
XDcSub DcSub;
XDcDma DcDma;
XDc Dc;
XMmiDp DpPsuPtr;

/* SDTV CSC Coefficients */
u32 CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000 };
u32 CSCOffset_RGB[] = { 0x0000, 0x0000, 0x0000 };

/* YUV-to-RGB input conversion (SDTV BT.601) */
u32 In_CSCCoeff_YUV[] = { 0x1000, 0x0000, 0x166F, 0x1000, 0x7A7F, 0x7493, 0x1000, 0x1C5A, 0x0000 };
u32 In_CSCOffset_YUV[] = { 0x0000, 0x1800, 0x1800 };

/* RGB-to-YUV output conversion (SDTV BT.601) */
u32 Out_CSCCoeff_YUV[] = { 0x04C8, 0x0964, 0x01D3, 0x7D4C, 0x7AB4, 0x0800, 0x0800, 0x7945, 0x7EB5 };
u32 Out_CSCOffset_YUV[] = { 0x0000, 0x0800, 0x0800 };


/*****************************************************************************/
/**
*
* This function converts XDc_VideoFormat enum to string name
*
* @param    format - Format enum value
*
* @return   String name of the format
*
* @note     Returns "UNKNOWN" for unrecognized formats
*
******************************************************************************/
static const char* format_to_string(XDc_VideoFormat format)
{
    switch(format) {
        /* YCbCr 4:2:2 Interleaved 8-bit */
        case CbY0CrY1: return "CbY0CrY1";
        case CrY0CbY1: return "CrY0CbY1";
        case Y0CrY1Cb: return "Y0CrY1Cb";
        case Y0CbY1Cr: return "Y0CbY1Cr";

        /* YCbCr 4:2:2 Semi-Planar 8-bit */
        case YV16: return "YV16";
        case YV16Ci: return "YV16Ci";
        case YV16Ci2: return "YV16Ci2";

        /* YCbCr 4:4:4 */
        case YV24: return "YV24";
        case YUV444: return "YUV444";
        case MONOCHROME: return "MONOCHROME";

        /* RGB 8-bit */
        case RGB888: return "RGB888";
        case RGB565: return "RGB565";
        case RGB888_GFX: return "RGB888_GFX";
        case RGB888dc: return "RGB888dc";

        /* RGBA 8-bit */
        case RGBA8888: return "RGBA8888";
        case RGBA8880: return "RGBA8880";
        case ABGR8888: return "ABGR8888";
        case BGR888: return "BGR888";
        case RGBA5551: return "RGBA5551";
        case RGBA4444: return "RGBA4444";

        /* 10-bit Formats */
        case RGB888_10BPC: return "RGB888_10BPC";
        case YV16_10BPC: return "YV16_10BPC";
        case YUV444_10BPC: return "YUV444_10BPC";
        case YV16Ci2_10BPC: return "YV16Ci2_10BPC";
        case YV16Ci_10BPC: return "YV16Ci_10BPC";
        case YV24_10BPC: return "YV24_10BPC";
        case MONOCHROME_10BPC: return "MONOCHROME_10BPC";
        case RGB888dcm_10BPC: return "RGB888dcm_10BPC";
        case RGB888dcl_10BPC: return "RGB888dcl_10BPC";
        case RGB10A2g: return "RGB10A2g";
        case RGBA8888dcm_10BPC: return "RGBA8888dcm_10BPC";
        case RGBA8888dcl_10BPC: return "RGBA8888dcl_10BPC";
        case YUV444g_10BPC: return "YUV444g_10BPC";
        case YUV444dcm_10BPC: return "YUV444dcm_10BPC";
        case YUV444dcl_10BPC: return "YUV444dcl_10BPC";

        /* 12-bit Formats */
        case RGB888dcm_12BPC: return "RGB888dcm_12BPC";
        case RGB888dcl_12BPC: return "RGB888dcl_12BPC";
        case RGBA8888dcm_12BPC: return "RGBA8888dcm_12BPC";
        case RGBA8888dcl_12BPC: return "RGBA8888dcl_12BPC";
        case YUV444dcm_12BPC: return "YUV444dcm_12BPC";
        case YUV444dcl_12BPC: return "YUV444dcl_12BPC";

        /* GPU/DC YUV Variants */
        case YUV444g: return "YUV444g";
        case YUV444dc: return "YUV444dc";
        case YUVA444g: return "YUVA444g";

        /* YV24 Planar 10/12-bit */
        case YV24dcm_10BPC: return "YV24dcm_10BPC";
        case YV24dcl_10BPC: return "YV24dcl_10BPC";
        case YV24dcm_12BPC: return "YV24dcm_12BPC";
        case YV24dcl_12BPC: return "YV24dcl_12BPC";

        /* YV16CI Semi-Planar 10/12-bit */
        case YV16CIdcm_10BPC: return "YV16CIdcm_10BPC";
        case YV16CIdcl_10BPC: return "YV16CIdcl_10BPC";
        case YV16CI2dcm_10BPC: return "YV16CI2dcm_10BPC";
        case YV16CI2dcl_10BPC: return "YV16CI2dcl_10BPC";
        case YV16CIdcm_12BPC: return "YV16CIdcm_12BPC";
        case YV16CIdcl_12BPC: return "YV16CIdcl_12BPC";
        case YV16CI2dcm_12BPC: return "YV16CI2dcm_12BPC";
        case YV16CI2dcl_12BPC: return "YV16CI2dcl_12BPC";

        /* DC YV16CI Semi-Planar */
        case YV16CIdc: return "YV16CIdc";
        case YV16CI2dc: return "YV16CI2dc";
        case YV16CIdc_420: return "YV16CIdc_420";
        case YV16CI2dc_420: return "YV16CI2dc_420";

        /* DC Planar */
        case YV24dc: return "YV24dc";

        /* 4:2:0 Formats */
        case YV16_420: return "YV16_420";
        case YV16Ci_420: return "YV16Ci_420";
        case YV16Ci2_420: return "YV16Ci2_420";
        case YV16_420_10BPC: return "YV16_420_10BPC";
        case YV16Ci_420_10BPC: return "YV16Ci_420_10BPC";
        case YV16Ci2_420_10BPC: return "YV16Ci2_420_10BPC";
        case YV16CIdcm_420_10BPC: return "YV16CIdcm_420_10BPC";
        case YV16CIdcl_420_10BPC: return "YV16CIdcl_420_10BPC";
        case YV16CI2dcm_420_10BPC: return "YV16CI2dcm_420_10BPC";
        case YV16CI2dcl_420_10BPC: return "YV16CI2dcl_420_10BPC";
        case YV16CIdcm_420_12BPC: return "YV16CIdcm_420_12BPC";
        case YV16CIdcl_420_12BPC: return "YV16CIdcl_420_12BPC";
        case YV16CI2dcm_420_12BPC: return "YV16CI2dcm_420_12BPC";
        case YV16CI2dcl_420_12BPC: return "YV16CI2dcl_420_12BPC";

        /* Y-only */
        case Ydc_ONLY: return "Ydc_ONLY";
        case Ydcm_ONLY_10BPC: return "Ydcm_ONLY_10BPC";
        case Ydcl_ONLY_10BPC: return "Ydcl_ONLY_10BPC";
        case Ydcm_ONLY_12BPC: return "Ydcm_ONLY_12BPC";
        case Ydcl_ONLY_12BPC: return "Ydcl_ONLY_12BPC";

        /* Live Formats */
        case RGB_8BPC: return "RGB_8BPC";
        case RGB_10BPC: return "RGB_10BPC";
        case RGB_12BPC: return "RGB_12BPC";
        case RGB_6BPC: return "RGB_6BPC";
        case YCbCr444_8BPC: return "YCbCr444_8BPC";
        case YCbCr444_10BPC: return "YCbCr444_10BPC";
        case YCbCr444_12BPC: return "YCbCr444_12BPC";
        case YCbCr422_8BPC: return "YCbCr422_8BPC";
        case YCbCr422_10BPC: return "YCbCr422_10BPC";
        case YCbCr422_12BPC: return "YCbCr422_12BPC";

        default: return "UNKNOWN";
    }
}

/*****************************************************************************/
/**
*
* This function maps InitRunConfig format values to XDc_VideoFormat
*
* @param    format - Format value from InitRunConfig
*
* @return   XDc_VideoFormat enum value
*
* @note     Maps directly to XDc_VideoFormat enum indices
*
******************************************************************************/
static XDc_VideoFormat MapFormat(u32 format)
{
    /* The format values from menus directly map to XDc_VideoFormat enum values */
    /* Non-Live Formats: 0-69 (up to Ydcl_ONLY_12BPC) */
    /* Live Formats: 70-79 (Y_ONLY_8BPC through YCbCr422_12BPC) */

    /* Valid format range: 0-79 */
    if (format <= YCbCr422_12BPC) {
        /* Direct mapping - format values match XDc_VideoFormat enum */
        return (XDc_VideoFormat)format;
    }

    /* If format is out of range, default to RGB_8BPC for live output */
    xil_printf("Warning: Invalid format %d, defaulting to RGB_8BPC\r\n", format);
    return RGB_8BPC;
}

/*****************************************************************************/
/**
*
* This function applies the user configuration to RunConfig structure
*
* @param    userConfig - User configuration from menu
* @param    runConfig - Platform RunConfig structure
*
* @return   None
*
* @note     None
*
******************************************************************************/
static void XDpDc_ApplyUserConfig(InitRunConfig *userConfig, RunConfig *runConfig)
{
    u32 idx;

    /* Set resolution */
    runConfig->Width = userConfig->width;
    runConfig->Height = userConfig->height;

    runConfig->operatingmode = userConfig->operatingmode;
    runConfig->presentationmode = userConfig->presentationmode;
    runConfig->livevidselect = userConfig->livevidselect;
    runConfig->livevideo01mode = userConfig->livevideo01mode;
    runConfig->livevideo02mode = userConfig->livevideo02mode;
    runConfig->livevideoalphaen = userConfig->livevideoalphaen;
    runConfig->livevideosdpen = userConfig->livevideosdpen;
    runConfig->byp_streams = userConfig->byp_streams;

    for (idx = 0; idx < 4; idx++) {
        runConfig->avpg[idx].bpc = userConfig->avpg[idx].bpc;
        runConfig->avpg[idx].ppc = userConfig->avpg[idx].ppc;
        runConfig->avpg[idx].pix_fmt = userConfig->avpg[idx].pix_fmt;
        runConfig->avpg[idx].pattern = userConfig->avpg[idx].pattern;
        runConfig->avpg[idx].colorimetry = userConfig->avpg[idx].colorimetry;
    }

    /* Resolve standard video mode and pixel clock */
    runConfig->VideoMode = XVidC_GetVideoModeId(
        userConfig->width, userConfig->height,
        userConfig->frame_rate, FALSE);
    if (runConfig->VideoMode == XVIDC_VM_NUM_SUPPORTED) {
        xil_printf("WARNING: No standard mode for %dx%d@%dHz, "
                   "defaulting to 640x480@60\r\n",
                   userConfig->width, userConfig->height,
                   userConfig->frame_rate);
        runConfig->VideoMode = XVIDC_VM_640x480_60_P;
    }
    runConfig->PixelClkHz = XVidC_GetPixelClockHzByVmId(runConfig->VideoMode);
    xil_printf("VideoMode:              %s\r\n",
               XVidC_GetVideoModeStr(runConfig->VideoMode));
    xil_printf("PixelClkHz:             %llu Hz (%llu.%03llu MHz)\r\n",
               runConfig->PixelClkHz,
               runConfig->PixelClkHz / 1000000,
               (runConfig->PixelClkHz % 1000000) / 1000);

    /* Set video formats */
    if (runConfig->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
        runConfig->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
        runConfig->Stream1Format = MapFormat(userConfig->stream1_format);
        runConfig->Stream2Format = MapFormat(userConfig->stream2_format);

        /* Set cursor enable */
        runConfig->CursorEnable =
            userConfig->cursor_enable ? CB_ENABLE : CB_DISABLE;

        /* Set cursor coordinates and size */
        runConfig->CursorCoordX = userConfig->cursor_coord_x;
        runConfig->CursorCoordY = userConfig->cursor_coord_y;
        runConfig->CursorSizeX = userConfig->cursor_size_x;
        runConfig->CursorSizeY = userConfig->cursor_size_y;
    }

    runConfig->OutStreamFormat = MapFormat(userConfig->output_format);

    /* Set audio enable */
    runConfig->AudioEnable = userConfig->audio_enable ? XDC_AUD_ENABLE : XDC_AUD_DISABLE;
    runConfig->AudioChannels = userConfig->audio_enable ?
                               userConfig->audio_channels : 0;

    /* Set SDP enable */
    runConfig->SdpEnable = userConfig->sdp_enable ? 1U : 0U;

    if ((runConfig->CursorEnable == CB_ENABLE) && (runConfig->SdpEnable != 0U))
        xil_printf("INFO: Cursor+SDP mode selected (cursor phase then SDP phase)\r\n");
    if ((runConfig->AudioEnable == XDC_AUD_ENABLE) && (runConfig->SdpEnable != 0U)) {
        xil_printf("WARNING: Audio and SDP test mode both enabled. Disabling audio.\r\n");
        runConfig->AudioEnable = XDC_AUD_DISABLE;
    }

    /* Set partial plane blend enable and parameters */
    if (userConfig->partial_plane_blend_enable) {
        if (userConfig->ppb_stream_select == 1) {
            runConfig->Stream1PbEnable = PB_ENABLE;
            runConfig->Stream2PbEnable = PB_DISABLE;
        } else {
            runConfig->Stream1PbEnable = PB_DISABLE;
            runConfig->Stream2PbEnable = PB_ENABLE;
        }
        /* Copy partial plane blend parameters */
        runConfig->PpbCoordX = userConfig->ppb_coord_x;
        runConfig->PpbCoordY = userConfig->ppb_coord_y;
        runConfig->PpbSizeX = userConfig->ppb_size_x;
        runConfig->PpbSizeY = userConfig->ppb_size_y;
        runConfig->PpbOffsetX = userConfig->ppb_offset_x;
        runConfig->PpbOffsetY = userConfig->ppb_offset_y;
    } else {
        runConfig->Stream1PbEnable = PB_DISABLE;
        runConfig->Stream2PbEnable = PB_DISABLE;
    }

    runConfig->MaxLaneCount = userConfig->lane_count;
    runConfig->MaxLinkRate = userConfig->link_rate;

    xil_printf("\r\n=== Applied Configuration to RunConfig ===\r\n");
    xil_printf("Width:                  %d\r\n", runConfig->Width);
    xil_printf("Height:                 %d\r\n", runConfig->Height);

    if (runConfig->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
        runConfig->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
        xil_printf("Stream1Format:          %s (%d)\r\n",
                   format_to_string(runConfig->Stream1Format),
                   runConfig->Stream1Format);
        xil_printf("Stream2Format:          %s (%d)\r\n",
                   format_to_string(runConfig->Stream2Format),
                   runConfig->Stream2Format);

        xil_printf("CursorEnable:           %s\r\n",
                   runConfig->CursorEnable ? "ENABLED" : "DISABLED");
        if (runConfig->CursorEnable) {
            xil_printf("  CursorPosition:       (%d, %d)\r\n",
                       runConfig->CursorCoordX, runConfig->CursorCoordY);
            xil_printf("  CursorSize:           %d x %d\r\n",
                       runConfig->CursorSizeX, runConfig->CursorSizeY);
        }
    }

    if (runConfig->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
        runConfig->presentationmode == XDCSUB_PPTMODE_LIVE) {
            /* Print AVPG configuration */
        u8 idx;

        for (idx = 0; idx < XDC_LIVE_EX_MAX_AVPATGEN_COUNT; idx++)
            xil_printf(
                "AVPAT #%d:\t\t%d bpc, %s ppc, pattern %d, %s, %s \r\n",
                idx,
                runConfig->avpg[idx].bpc,
                runConfig->avpg[idx].ppc ? "Quad" : "Dual",
                runConfig->avpg[idx].pattern,
                runConfig->avpg[idx].pix_fmt ? "YUV 422" : "RGB",
                runConfig->avpg[idx].colorimetry ? "BT.709" : "BT.601");
    }

    xil_printf("OutStreamFormat:        %s (%d)\r\n", format_to_string(runConfig->OutStreamFormat), runConfig->OutStreamFormat);
    xil_printf("AudioEnable:            %s\r\n", runConfig->AudioEnable ? "ENABLED" : "DISABLED");
    if (runConfig->AudioEnable) {
        xil_printf("AudioChannels:          %d\r\n", runConfig->AudioChannels);
    }
    xil_printf("SdpEnable:              %s\r\n", runConfig->SdpEnable ? "ENABLED" : "DISABLED");
    xil_printf("Stream1 PartialBlend:   %s\r\n", runConfig->Stream1PbEnable == PB_ENABLE ? "ENABLED" : "DISABLED");
    xil_printf("Stream2 PartialBlend:   %s\r\n", runConfig->Stream2PbEnable == PB_ENABLE ? "ENABLED" : "DISABLED");
    xil_printf("=========================================\r\n\r\n");
}

/*****************************************************************************/
/**
*
* This is the main function for the application
*
* @param    None
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note     None
*
******************************************************************************/
int main(void)
{
    InitRunConfig userConfig;
    u32 Status;

    /* Disable caches */
    Xil_DCacheDisable();
    Xil_ICacheDisable();

    xil_printf("\r\n");
    xil_printf("==================================================\r\n");
    xil_printf("  DisplayPort Configuration Application\r\n");
    xil_printf("  Build timestamp - %s %s\r\n", __DATE__, __TIME__);
    xil_printf("==================================================\r\n");
    xil_printf("\r\n");

restart:
    /* Initialize configuration with defaults */
    XDpDc_InitConfigDefaults(&userConfig);

    /* Run the interactive menu loop */
    XDpDc_MenuLoop(&userConfig);

    xil_printf("\r\n=== Starting Platform Initialization ===\r\n");

    /* Apply user configuration to platform RunConfig */
    XDpDc_ApplyUserConfig(&userConfig, &RunCfg);

    /* Bandwidth check when link params are explicitly set (non-auto) */
    if (userConfig.lane_count && userConfig.link_rate) {
        const XDc_VideoAttribute *OutVidAttr;
        u32 bpp;
        u64 pixel_bw_kbps, link_bw_kbps;
        u64 utilization;
        const char *link_rate_str;

        OutVidAttr = XDc_GetLiveVideoAttribute(RunCfg.OutStreamFormat);
        bpp = OutVidAttr ? OutVidAttr->BPP : 24;

        switch (userConfig.link_rate) {
        case 0x06: link_rate_str = "RBR (1.62 Gbps)"; break;
        case 0x0A: link_rate_str = "HBR (2.70 Gbps)"; break;
        case 0x14: link_rate_str = "HBR2 (5.40 Gbps)"; break;
        case 0x1E: link_rate_str = "HBR3 (8.10 Gbps)"; break;
        default:   link_rate_str = "Unknown"; break;
        }

        pixel_bw_kbps = (RunCfg.PixelClkHz / 1000) * bpp;
        link_bw_kbps = (u64)userConfig.lane_count *
                       (u64)userConfig.link_rate * 216000;
        utilization = (pixel_bw_kbps * 100) / link_bw_kbps;

        xil_printf("\r\n");
        xil_printf("==================================================\r\n");
        xil_printf("  Link Bandwidth Check\r\n");
        xil_printf("==================================================\r\n");
        xil_printf("  Resolution:       %dx%d\r\n",
                   RunCfg.Width, RunCfg.Height);
        xil_printf("  Pixel Clock:      %llu Hz (%llu.%03llu MHz)\r\n",
                   RunCfg.PixelClkHz,
                   RunCfg.PixelClkHz / 1000000,
                   (RunCfg.PixelClkHz % 1000000) / 1000);
        xil_printf("  Output Format:    %s (BPP=%d)\r\n",
                   format_to_string(RunCfg.OutStreamFormat), bpp);
        xil_printf("  DC Output BW:     %llu.%02llu Gbps\r\n",
                   pixel_bw_kbps / 1000000,
                   (pixel_bw_kbps % 1000000) / 10000);
        xil_printf("  Link Config:      %d lane(s) x %s\r\n",
                   userConfig.lane_count, link_rate_str);
        xil_printf("  Link BW (eff):    %llu.%02llu Gbps (80%% of raw)\r\n",
                   link_bw_kbps / 1000000,
                   (link_bw_kbps % 1000000) / 10000);
        xil_printf("  BW Utilization:   %llu%%\r\n", utilization);
        xil_printf("  Threshold:        75%%\r\n");

        if (pixel_bw_kbps * 4 > link_bw_kbps * 3) {
            xil_printf("--------------------------------------------------\r\n");
            xil_printf("  RESULT: FAIL - DC output exceeds 75%% of link BW\r\n");
            xil_printf("  Returning to configuration menu...\r\n");
            xil_printf("==================================================\r\n\r\n");
            goto restart;
        }

        xil_printf("--------------------------------------------------\r\n");
        xil_printf("  RESULT: PASS\r\n");
        xil_printf("==================================================\r\n");
    }

    if (RunCfg.operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
        if (RunCfg.presentationmode == XDCSUB_PPTMODE_NONLIVE)
            Status = XDpDc_MmiDcNonliveTest(&RunCfg);

        if (RunCfg.presentationmode == XDCSUB_PPTMODE_LIVE)
            Status = XDpDc_MmiDcLiveTest(&RunCfg);
    }

    if (Status != XST_SUCCESS) {
        xil_printf(
            "\r\n**************************************************\r\n");
        xil_printf("  Pipeline initialization FAILED\r\n");
        xil_printf("  Returning to main menu...\r\n");
        xil_printf(
            "**************************************************\r\n\r\n");
        goto restart;
    }

    xil_printf("Successfully ran ");
    if (RunCfg.operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
        if (RunCfg.presentationmode == XDCSUB_PPTMODE_NONLIVE)
            xil_printf(" MMI_DC_NONLIVE_TEST\r\n");

        if (RunCfg.presentationmode == XDCSUB_PPTMODE_LIVE)
            xil_printf("MMI_DC_LIVE_TEST\r\n");
    }

    xil_printf("\r\n");
    xil_printf("==================================================\r\n");
    xil_printf("  Runtime Menu (display active)                   \r\n");
    xil_printf("==================================================\r\n");
    xil_printf("  u - AUX Read/Write\r\n");
    xil_printf("  l - Print DPCD Link/Sink Status (0x200-0x2FF)\r\n");
    xil_printf("  m - Power Cycle Monitor\r\n");
    xil_printf("  q - Quit\r\n");
    xil_printf("  h - Show this menu\r\n");
    xil_printf("==================================================\r\n");

    while (1) {
        char rtcmd;
        xil_printf("\r\nRuntime> ");
        rtcmd = inbyte();
        outbyte(rtcmd);
        outbyte('\r');
        outbyte('\n');

        switch (rtcmd) {
        case 'u':
        case 'U':
            XDpDc_AuxMenu(RunCfg.DpPsuPtr);
            break;
        case 'l':
        case 'L':
        {
            u32 addr;
            u8 val;
            u32 status;

            xil_printf("\r\n=== DPCD Link/Sink Status (0x00200 - 0x002FF) ===\r\n");
            xil_printf("Addr     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\r\n");
            xil_printf("-------- -----------------------------------------------\r\n");

            for (addr = 0x200; addr <= 0x2FF; addr++) {
                if ((addr & 0xF) == 0)
                    xil_printf("0x%05x: ", addr);

                val = 0;
                status = XMmiDp_AuxRead(RunCfg.DpPsuPtr, addr, 1, &val);
                if (status == XST_SUCCESS)
                    xil_printf("%02x ", val);
                else
                    xil_printf("-- ");

                if ((addr & 0xF) == 0xF)
                    xil_printf("\r\n");
            }
            xil_printf("=================================================\r\n");
            break;
        }
        case 'm':
        case 'M':
            xil_printf("Power cycling monitor...\r\n");
            XMmiDp_SinkPowerDown(RunCfg.DpPsuPtr);
            xil_printf("  Monitor powered down, waiting...\r\n");
            sleep(10);
            XMmiDp_SinkPowerUp(RunCfg.DpPsuPtr);
            xil_printf("  Monitor powered up, waiting...\r\n");
            sleep(5);
            xil_printf("Power cycle complete.\r\n");
            break;
        case 'q':
        case 'Q':
            xil_printf("Exiting runtime menu.\r\n");
            return XST_SUCCESS;
        case 'h':
        case 'H':
            xil_printf("  u - AUX Read/Write\r\n");
            xil_printf("  l - Print DPCD Link/Sink Status (0x200-0x2FF)\r\n");
            xil_printf("  m - Power Cycle Monitor\r\n");
            xil_printf("  q - Quit\r\n");
            xil_printf("  h - Show this menu\r\n");
            break;
        default:
            xil_printf("Unknown command. Press 'h' for help.\r\n");
        }
    }

    return XST_SUCCESS;
}

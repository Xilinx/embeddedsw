/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_menus.c
*
* This file contains menu implementation for configuration
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 Initial version with InitRunConfig support
*
* </pre>
*
******************************************************************************/

#include "mmi_dpdc_example.h"
#include "mmi_dpdc_platform.h"
#include "mmi_dpdc_menus.h"
#include "xil_printf.h"
#include "xdcsub.h"
#include "xdc.h"
#include "xmmidp.h"
#include <string.h>
#include "xavpg.h"


extern void outbyte(char c);
extern char inbyte(void);

static char read_char(void)
{
    char c = inbyte();
    outbyte(c);
    return c;
}

static u32 read_uint(void)
{
    u32 val = 0;
    char c;

    while (1) {
        c = inbyte();
        if (c >= '0' && c <= '9') {
            outbyte(c);
            val = val * 10 + (c - '0');
        } else if (c == '\r' || c == '\n') {
            outbyte('\r');
            outbyte('\n');
            break;
        } else if (c == 0x08 || c == 0x7F) {
            /* Backspace */
            val = val / 10;
            outbyte(0x08);
            outbyte(' ');
            outbyte(0x08);
        }
    }

    return val;
}

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
static const char* format_to_string(u32 format)
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
* This function displays the resolution help menu
*
* @param    None
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_ResolutionHelpMenu(void)
{
    xil_printf("\r\n");
    xil_printf("============================================\r\n");
    xil_printf("          Resolution Help Menu              \r\n");
    xil_printf("============================================\r\n");
    xil_printf("Common Resolutions:\r\n");
    xil_printf("  640x480   (VGA)\r\n");
    xil_printf("  1280x720  (720p/HD)\r\n");
    xil_printf("  1920x1080 (1080p/Full HD)\r\n");
    xil_printf("  3840x2160 (4K/UHD)\r\n");
    xil_printf("  7680x4320 (8K)\r\n");
    xil_printf("\r\n");
    xil_printf("Width range:  1-7680 pixels\r\n");
    xil_printf("Height range: 1-4320 pixels\r\n");
    xil_printf("============================================\r\n");
}

/*****************************************************************************/
/**
*
* This function displays the format help menu
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_FormatHelpMenu(void)
{
    xil_printf("\r\n");
    xil_printf("============================================\r\n");
    xil_printf("            Format Help Menu                 \r\n");
    xil_printf("============================================\r\n");
    xil_printf("Non-Live Formats (0-31):\r\n");
    xil_printf("  0  - RGBA8888\r\n");
    xil_printf("  1  - ABGR8888\r\n");
    xil_printf("  2  - RGB888\r\n");
    xil_printf("  3  - BGR888\r\n");
    xil_printf("  4  - RGBA5551\r\n");
    xil_printf("  5  - RGBA4444\r\n");
    xil_printf("  6  - RGB565\r\n");
    xil_printf("  7  - YV16\r\n");
    xil_printf("  8  - YV24\r\n");
    xil_printf("  9  - YV16CI\r\n");
    xil_printf("  10 - YUYV\r\n");
    xil_printf("  11 - NV16\r\n");
    xil_printf("  12 - NV12\r\n");
    xil_printf("  13 - MONO\r\n");
    xil_printf("  ... (more formats available, 0-31)\r\n");
    xil_printf("============================================\r\n");
}

/*****************************************************************************/
/**
*
* This function displays all supported non-live input formats
*
* @param    None
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_ListNonliveFormats(void)
{
    xil_printf("\r\n");
    xil_printf("========================================================\r\n");
    xil_printf("       Non-Live Input Formats (Frame Buffer)            \r\n");
    xil_printf("========================================================\r\n");
    xil_printf("Format#  Format Name              Description\r\n");
    xil_printf("--------------------------------------------------------\r\n");
    xil_printf("  %2d     %-20s  YUV 4:2:2\r\n", CbY0CrY1, "CbY0CrY1");
    xil_printf("  %2d     %-20s  YUV 4:2:2\r\n", CrY0CbY1, "CrY0CbY1");
    xil_printf("  %2d     %-20s  YUV 4:2:2\r\n", Y0CrY1Cb, "Y0CrY1Cb");
    xil_printf("  %2d     %-20s  YUV 4:2:2\r\n", Y0CbY1Cr, "Y0CbY1Cr");
    xil_printf("  %2d     %-20s  YUV 4:2:2\r\n", YV16, "YV16");
    xil_printf("  %2d     %-20s  YUV 4:4:4\r\n", YV24, "YV24");
    xil_printf("  %2d     %-20s  YUV 4:2:2 Interleaved\r\n", YV16Ci, "YV16Ci");
    xil_printf("  %2d     %-20s  Monochrome\r\n", MONOCHROME, "MONOCHROME");
    xil_printf("  %2d     %-20s  YUV 4:2:2 Interleaved 2\r\n", YV16Ci2, "YV16Ci2");
    xil_printf("  %2d     %-20s  YUV 4:4:4\r\n", YUV444, "YUV444");
    xil_printf("  %2d     %-20s  RGB 24-bit\r\n", RGB888, "RGB888");
    xil_printf("  %2d     %-20s  RGBA 32-bit (alt)\r\n", RGBA8880, "RGBA8880");
    xil_printf("  %2d     %-20s  RGB 10-bit per channel\r\n", RGB888_10BPC, "RGB888_10BPC");
    xil_printf("  %2d     %-20s  YUV 4:4:4 10-bit\r\n", YUV444_10BPC, "YUV444_10BPC");
    xil_printf("  %2d     %-20s  YV16 4:2:0\r\n", YV16_420, "YV16_420");
    xil_printf("  %2d     %-20s  RGBA 32-bit\r\n", RGBA8888, "RGBA8888");
    xil_printf("  %2d     %-20s  ABGR 32-bit\r\n", ABGR8888, "ABGR8888");
    xil_printf("  %2d     %-20s  RGB 24-bit (GFX)\r\n", RGB888_GFX, "RGB888_GFX");
    xil_printf("  %2d     %-20s  BGR 24-bit\r\n", BGR888, "BGR888");
    xil_printf("  %2d     %-20s  RGBA 16-bit (5551)\r\n", RGBA5551, "RGBA5551");
    xil_printf("  %2d     %-20s  RGBA 16-bit (4444)\r\n", RGBA4444, "RGBA4444");
    xil_printf("  %2d     %-20s  RGB 16-bit (565)\r\n", RGB565, "RGB565");
    xil_printf("========================================================\r\n");
    xil_printf("Note: Non-live formats read from frame buffers in DDR\r\n");
    xil_printf("      Use format number to configure in custom option\r\n");
    xil_printf("      Total formats: 0-%d (70+ formats available)\r\n", RGB565);
    xil_printf("========================================================\r\n");
}

/*****************************************************************************/
/**
*
* This function displays all supported live output formats
*
* @param    None
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_ListLiveFormats(void)
{
    xil_printf("\r\n");
    xil_printf("========================================================\r\n");
    xil_printf("         Live Output Formats (Stream Interface)         \r\n");
    xil_printf("========================================================\r\n");
    xil_printf("Format#  Format Name              Description\r\n");
    xil_printf("--------------------------------------------------------\r\n");
    xil_printf("  %2d     %-20s  Y Only 8-bit\r\n", Y_ONLY_8BPC, "Y_ONLY_8BPC");
    xil_printf("  %2d     %-20s  Y Only 10-bit\r\n", Y_ONLY_10BPC, "Y_ONLY_10BPC");
    xil_printf("  %2d     %-20s  Y Only 12-bit\r\n", Y_ONLY_12BPC, "Y_ONLY_12BPC");
    xil_printf("  %2d     %-20s  RGB 6-bit per channel\r\n", RGB_6BPC, "RGB_6BPC");
    xil_printf("  %2d     %-20s  RGB 8-bit per channel\r\n", RGB_8BPC, "RGB_8BPC");
    xil_printf("  %2d     %-20s  RGB 10-bit per channel\r\n", RGB_10BPC, "RGB_10BPC");
    xil_printf("  %2d     %-20s  RGB 12-bit per channel\r\n", RGB_12BPC, "RGB_12BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:4:4 6-bit\r\n", YCbCr444_6BPC, "YCbCr444_6BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:4:4 8-bit\r\n", YCbCr444_8BPC, "YCbCr444_8BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:4:4 10-bit\r\n", YCbCr444_10BPC, "YCbCr444_10BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:4:4 12-bit\r\n", YCbCr444_12BPC, "YCbCr444_12BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:2:2 8-bit\r\n", YCbCr422_8BPC, "YCbCr422_8BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:2:2 10-bit\r\n", YCbCr422_10BPC, "YCbCr422_10BPC");
    xil_printf("  %2d     %-20s  YCbCr 4:2:2 12-bit\r\n", YCbCr422_12BPC, "YCbCr422_12BPC");
    xil_printf("========================================================\r\n");
    xil_printf("Note: Live formats stream directly to DisplayPort output\r\n");
    xil_printf("      Use format number to configure in custom option\r\n");
    xil_printf("      These are used for OutStreamFormat configuration\r\n");
    xil_printf("========================================================\r\n");
}

/*****************************************************************************/
/**
*
* This function displays the main help menu
*
* @param    None
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_MainHelpMenu(InitRunConfig *config)
{
    xil_printf("\r\n");
    xil_printf("====================================================\r\n");
    if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
        if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE)
            xil_printf("--           Non Live InitRunConfig Menu             --\r\n");
        if (config->presentationmode == XDCSUB_PPTMODE_LIVE)
            xil_printf("--               Live InitRunConfig Menu             --\r\n");
        if (config->presentationmode == XDCSUB_PPTMODE_MIXED)
            xil_printf("--            Mixed InitRunConfig Menu               --\r\n");
    }

    xil_printf("====================================================\r\n");
    xil_printf("Available Commands:\r\n");
    xil_printf("  r - Configure Resolution\r\n");
    if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
        if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
            xil_printf("  f - Configure Stream1 Format\r\n");
            xil_printf("  g - Configure Stream2 Format\r\n");
            xil_printf("  c - Toggle Cursor Enable (with Position & Size config)\r\n");
            xil_printf("  x - Configure Cursor Position & Size\r\n");
            xil_printf("  a - Toggle Audio Enable (and select channels 1-8)\r\n");
            xil_printf("  n - Toggle SDP Enable\r\n");
        }

        if (config->presentationmode == XDCSUB_PPTMODE_LIVE) {
            /* TODO - Add SDP and audio */
            xil_printf("  f - config AVPG 1\r\n");
            xil_printf("  g - config AVPG 2\r\n");
        }

        if (config->presentationmode == XDCSUB_PPTMODE_MIXED) {
                xil_printf("  f - config AVPG 1 (Live) \r\n");
                xil_printf("  g - Configure Stream2 Format (Non live)\r\n");
        }

        xil_printf("  p - Toggle Partial Plane Blend Enable\r\n");
        xil_printf("  b - Configure Partial Plane Blend Parameters\r\n");
        xil_printf("  o - Configure Output Format\r\n");
    }
    xil_printf("  t - Select Presentation Mode (Non live/Live/Mixed)\r\n");
    xil_printf("  l - Configure DP Link (Lane Count & Link Rate)\r\n");
    xil_printf("  m - Toggle Power Cycle Monitor on Start\r\n");
    xil_printf("  d - Display Current Configuration\r\n");
    xil_printf("  h - Display This Help Menu\r\n");
    xil_printf("  s - Start Application\r\n");
    xil_printf("  q - Quit\r\n");
    xil_printf("====================================================\r\n");
    xil_printf("Note: Format lists available in format config menus\r\n");
    xil_printf("====================================================\r\n");
}

/*****************************************************************************/
/**
*
* This function displays the current configuration
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_DisplayConfig(InitRunConfig *config)
{
    xil_printf("\r\n");
    xil_printf("=== Current Configuration ===\r\n");
    xil_printf("Resolution:             %d x %d @%dHz\r\n", config->width, config->height, config->frame_rate);

    xil_printf("Stream1 Format:         %s (%d)\r\n",
               format_to_string(config->stream1_format),
               config->stream1_format);
    xil_printf("Stream2 Format:         %s (%d)\r\n",
               format_to_string(config->stream2_format),
               config->stream2_format);
    xil_printf("Output Format:          %s (%d)\r\n",
               format_to_string(config->output_format), config->output_format);

    if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
        config->presentationmode == XDCSUB_PPTMODE_NONLIVE) {

        xil_printf("Cursor Enable:          %s\r\n",
                   config->cursor_enable ? "Yes" : "No");
        if (config->cursor_enable) {
            xil_printf("  Cursor Position:      (%d, %d)\r\n",
                       config->cursor_coord_x, config->cursor_coord_y);
            xil_printf("  Cursor Size:          %d x %d\r\n",
                       config->cursor_size_x, config->cursor_size_y);
        }
    }

    if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
        (config->presentationmode == XDCSUB_PPTMODE_LIVE ||
         config->presentationmode == XDCSUB_PPTMODE_MIXED)) {
        u8 idx, max_idx;

        max_idx = (config->presentationmode == XDCSUB_PPTMODE_LIVE) ? 2 : 1;

        for (idx = 0; idx < max_idx; idx++)
            xil_printf(
                "AVPAT #%d:\t\t%d bpc, %s ppc, pattern %d, %s, %s \r\n",
                idx,
                config->avpg[idx].bpc,
                config->avpg[idx].ppc ? "Quad" : "Dual",
                config->avpg[idx].pattern,
                config->avpg[idx].pix_fmt ? "YUV 422" : "RGB",
                config->avpg[idx].colorimetry ? "BT.709" : "BT.601");
    }

    xil_printf("Audio Enable:           %s\r\n", config->audio_enable ? "Yes" : "No");
    if (config->audio_enable) {
        xil_printf("Audio Channels:         %d\r\n", config->audio_channels);
    }
    xil_printf("SDP Enable:             %s\r\n", config->sdp_enable ? "Yes" : "No");
    xil_printf("Partial Plane Blend:    %s\r\n", config->partial_plane_blend_enable ? "Yes" : "No");
    if (config->partial_plane_blend_enable) {
        xil_printf("  Source Stream:        %s\r\n", config->ppb_stream_select == 1 ? "Stream1" : "Stream2");
        xil_printf("  Blend Stream:         %s\r\n", config->ppb_stream_select == 1 ? "Stream2" : "Stream1");
        xil_printf("  Position (xp, yp):    (%d, %d)\r\n", config->ppb_coord_x, config->ppb_coord_y);
        xil_printf("  Size (xp_size, yp_size): %d x %d\r\n", config->ppb_size_x, config->ppb_size_y);
        xil_printf("  Offset (xs, ys):      (%d, %d)\r\n", config->ppb_offset_x, config->ppb_offset_y);
    }
    xil_printf("DP Lane Count:          %s",
               config->lane_count == 0 ? "Auto" :
               config->lane_count == 1 ? "1" :
               config->lane_count == 2 ? "2" : "4");
    xil_printf("\r\n");
    xil_printf("DP Link Rate:           %s",
               config->link_rate == 0    ? "Auto" :
               config->link_rate == 0x06 ? "RBR (1.62 Gbps)" :
               config->link_rate == 0x0A ? "HBR (2.70 Gbps)" :
               config->link_rate == 0x14 ? "HBR2 (5.40 Gbps)" :
               config->link_rate == 0x1E ? "HBR3 (8.10 Gbps)" : "Unknown");
    xil_printf("\r\n");
    xil_printf("=============================\r\n");
}

/*****************************************************************************/
/**
*
* This function gets width from user
*
* @param    None
*
* @return   Width value
*
* @note     None
*
******************************************************************************/
u32 XDpDc_GetWidth(void)
{
    u32 width;
    xil_printf("\r\nEnter width (1-7680): ");
    width = read_uint();

    if (width < 1 || width > 7680) {
        xil_printf("Invalid width! Using default 1920\r\n");
        width = 1920;
    }

    return width;
}

/*****************************************************************************/
/**
*
* This function gets height from user
*
* @param    None
*
* @return   Height value
*
* @note     None
*
******************************************************************************/
u32 XDpDc_GetHeight(void)
{
    u32 height;
    xil_printf("Enter height (1-4320): ");
    height = read_uint();

    if (height < 1 || height > 4320) {
        xil_printf("Invalid height! Using default 1080\r\n");
        height = 1080;
    }

    return height;
}

/*****************************************************************************/
/**
*
* This function configures resolution
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   XST_SUCCESS
*
* @note     None
*
******************************************************************************/
u32 XDpDc_ConfigureResolution(InitRunConfig *config)
{
    char choice;

    xil_printf("\r\n=== Resolution Configuration ===\r\n");
    xil_printf("1. 640x480 (VGA)\r\n");
    xil_printf("2. 1280x720 (720p)\r\n");
    xil_printf("3. 1920x1080 (1080p)\r\n");
    xil_printf("4. 3840x2160 (4K)\r\n");
    xil_printf("5. Custom\r\n");
    xil_printf("Enter choice (1-5): ");

    choice = read_char();
    xil_printf("\r\n");

    switch(choice) {
        case '1':
            config->width = 640;
            config->height = 480;
            break;
        case '2':
            config->width = 1280;
            config->height = 720;
            break;
        case '3':
            config->width = 1920;
            config->height = 1080;
            break;
        case '4':
            config->width = 3840;
            config->height = 2160;
            break;
        case '5':
            config->width = XDpDc_GetWidth();
            config->height = XDpDc_GetHeight();
            break;
        default:
            xil_printf("Invalid choice! Using default 1920x1080\r\n");
            config->width = 1920;
            config->height = 1080;
    }

    xil_printf("\r\nSelect Frame Rate:\r\n");
    xil_printf("  1 - 24 Hz\r\n");
    xil_printf("  2 - 30 Hz\r\n");
    xil_printf("  3 - 50 Hz\r\n");
    xil_printf("  4 - 60 Hz (default)\r\n");
    xil_printf("  5 - Custom\r\n");
    xil_printf("Enter choice (1-5): ");

    choice = read_char();
    xil_printf("\r\n");

    switch(choice) {
        case '1': config->frame_rate = 24; break;
        case '2': config->frame_rate = 30; break;
        case '3': config->frame_rate = 50; break;
        case '4': config->frame_rate = 60; break;
        case '5':
            xil_printf("Enter frame rate (Hz): ");
            config->frame_rate = read_uint();
            break;
        default:
            config->frame_rate = 60;
    }

    xil_printf("Resolution set to: %dx%d @%dHz\r\n", config->width, config->height, config->frame_rate);
    return XST_SUCCESS;
}

void XAvpg_Config_printcfg(InitRunConfig *config, u8 idx)
{
    xil_printf("\r\n ==== AVPG #%d configuration ==== \r\n", idx);

    xil_printf("  bits per component = %d\r\n", config->avpg[idx].bpc);
    xil_printf("  pixels per clock   = %d\r\n", config->avpg[idx].ppc ? XAVPATGEN_PPC_QUAD : XAVPATGEN_PPC_DUAL);
    xil_printf("  pixel format       = %s\r\n", config->avpg[idx].pix_fmt ? "YUV422" : "RGB");
    xil_printf("  colorimetry        = BT.%d\r\n", config->avpg[idx].colorimetry ? 709 : 601);
    xil_printf("  pattern            = ");

    switch(config->avpg[idx].pattern) {
        case 1:
            xil_printf("Color Ramp");
            break;
        case 2:
            xil_printf("Black and white vertical lines");
            break;
        case 3:
            xil_printf("Color Square Pattern");
            break;
        case 4:
            xil_printf("Flat Red");
            break;
        case 5:
            xil_printf("Flat Green");
            break;
        case 6:
            xil_printf("Flat Blue");
            break;
        case 7:
            xil_printf("Flat Yellow");
            break;
        default:
            xil_printf("None");
    }

    xil_printf("\r\n");
    xil_printf("\r\n ================================ \r\n");
}

void XAvpg_Config_bpc(InitRunConfig *config, u8 idx)
{
    char choice;

avpg_cfg_bpc_menu:
    /* Print the options */
    xil_printf("\r\nAV PAT GEN bits per component options\r\n");
    xil_printf("  8 - Set  8 bpc\r\n");
    xil_printf("  A - Set 10 bpc\r\n");
    xil_printf("  C - Set 12 bpc\r\n");
    xil_printf("Enter choice: ");

    choice = read_char();
    /* Read the options */
    switch (choice) {
        case '8':
            config->avpg[idx].bpc = 8;
            break;
        case 'A':
            /* fall through */
        case 'a':
            config->avpg[idx].bpc = 10;
            break;
        case 'C':
            /* fall through */
        case 'c':
            config->avpg[idx].bpc = 12;
            break;
        default:
            xil_printf("Invalid input \r\n");
            goto avpg_cfg_bpc_menu;
    }

    XAvpg_Config_printcfg(config, idx);
}

void XAvpg_Config_ppc(InitRunConfig *config, u8 idx)
{
    char choice;

avpg_cfg_ppc_menu:
    /* Default value */
    config->avpg[idx].ppc = 0;

    /* Print the options */
    xil_printf("\r\nAV PAT GEN pixels per clock options\r\n");
    xil_printf("  2 - Dual pixel per clock\r\n");
    if (config->operatingmode == XDCSUB_OPMODE_BYPASS) {
        /* Only valid for DC Bypass mode */
        xil_printf("  4 - Quad pixel per clock\r\n");
        xil_printf("Enter choice: ");

        choice = read_char();
        /* Read the options */
        switch (choice) {
            case '2':
                config->avpg[idx].ppc = 0;
                break;
            case '4':
                config->avpg[idx].ppc = 1;
                break;
            default:
                xil_printf("Invalid input\r\n");
                goto avpg_cfg_ppc_menu;
        }
    }

    XAvpg_Config_printcfg(config, idx);
}

void XAvpg_Config_pat(InitRunConfig *config, u8 idx)
{
    char choice;

avpg_cfg_pat_menu:
    /* Print the options */
    xil_printf("\r\nAV PAT GEN pattern options\r\n");
    xil_printf("  1 - Color ramp\r\n");
    xil_printf("  2 - Black and white vertical lines\r\n");
    xil_printf("  3 - Color square\r\n");
    xil_printf("  4 - Flat Red\r\n");
    xil_printf("  5 - Flat Green\r\n");
    xil_printf("  6 - Flat Blue\r\n");
    xil_printf("  7 - Flat Yellow\r\n");
    xil_printf("Enter choice: ");

    choice = read_char();
    if (choice < '1' || choice > '7') {
        xil_printf("Invalid input. \r\n");
        goto avpg_cfg_pat_menu;
    }

    config->avpg[idx].pattern = choice - '0';

    XAvpg_Config_printcfg(config, idx);
}

void XAvpg_Config_col(InitRunConfig *config, u8 idx)
{
    char choice;

avpg_cfg_col_menu:
    /* Print the options */
    xil_printf("\r\nAV PAT GEN colorimetry options\r\n");
    xil_printf("  0 - BT. 601\r\n");
    xil_printf("  1 - BT. 709\r\n");
    xil_printf("Enter choice: ");

    choice = read_char();
    if (choice < '0' || choice > '1') {
        xil_printf("Invalid input. \r\n");
        goto avpg_cfg_col_menu;
    }

    config->avpg[idx].colorimetry = choice - '0';

    XAvpg_Config_printcfg(config, idx);
}

void XAvpg_Config_fmt(InitRunConfig *config, u8 idx)
{
    char choice;

avpg_cfg_fmt_menu:
    /* Print the options */
    xil_printf("\r\nAV PAT GEN pixel format options\r\n");
    xil_printf("  0 - RGB\r\n");
    xil_printf("  1 - YUV422\r\n");
    xil_printf("Enter choice: ");

    choice = read_char();
    if (choice < '0' || choice > '1') {
        xil_printf("Invalid input. \r\n");
        goto avpg_cfg_fmt_menu;
    }

    config->avpg[idx].pix_fmt = choice - '0';

    XAvpg_Config_printcfg(config, idx);
}

void XAvpg_Config(InitRunConfig *config, u8 idx)
{
    char choice;

    if (config->avpg[idx].bpc == 0)
        config->avpg[idx].bpc = 8;

    /* Disable audio */
    config->audio_enable = 0;
    config->audio_channels = 0;

avpg_cfg_menu:

    /* For the first AVPATGEN */
    if(idx == XAVPATGEN_INST_0) {
        /* For RGB color space */
        if (config->avpg[idx].pix_fmt == XAVPATGEN_CS_RGB) {
            switch(config->avpg[idx].bpc) {
                case 8:
                config->stream1_format = RGB_8BPC;
                break;
                case 10:
                config->stream1_format = RGB_10BPC;
                break;
                case 12:
                config->stream1_format = RGB_12BPC;
                break;
            }
        } else {
            switch(config->avpg[idx].bpc) {
                case 8:
                config->stream1_format = YCbCr422_8BPC;
                break;
                case 10:
                config->stream1_format = YCbCr422_10BPC;
                break;
                case 12:
                config->stream1_format = YCbCr422_12BPC;
                break;
            }
        }
    }

    if(idx == XAVPATGEN_INST_1) {
        if (config->avpg[idx].pix_fmt == XAVPATGEN_CS_RGB) {
            switch(config->avpg[idx].bpc) {
                case 8:
                config->stream2_format = RGB_8BPC;
                break;
                case 10:
                config->stream2_format = RGB_10BPC;
                break;
                case 12:
                config->stream2_format = RGB_12BPC;
                break;
            }
        } else {
            switch(config->avpg[idx].bpc) {
                case 8:
                config->stream2_format = YCbCr422_8BPC;
                break;
                case 10:
                config->stream2_format = YCbCr422_10BPC;
                break;
                case 12:
                config->stream2_format = YCbCr422_12BPC;
                break;
            }
        }
    }

    /* Print the options */
    xil_printf("\r\nAV PAT GEN %d config options\r\n", idx);
    xil_printf("  1 - Set bits per component\r\n");
    xil_printf("  2 - Set pixels per clock\r\n");
    xil_printf("  3 - Set pattern\r\n");
    xil_printf("  4 - Set colorimetry\r\n");
    xil_printf("  5 - Set pixel format\r\n");
    xil_printf("  6 - Print AVPG config\r\n");
    xil_printf("  9 - Exit\r\n");
    xil_printf("Enter choice (1-9): ");

    choice = read_char();

    switch (choice) {
        case '1':
            XAvpg_Config_bpc(config, idx);
            goto avpg_cfg_menu;
            break;
        case '2':
            XAvpg_Config_ppc(config, idx);
            goto avpg_cfg_menu;
            break;
        case '3':
            XAvpg_Config_pat(config, idx);
            goto avpg_cfg_menu;
            break;
        case '4':
            XAvpg_Config_col(config, idx);
            goto avpg_cfg_menu;
            break;
        case '5':
            XAvpg_Config_fmt(config, idx);
            goto avpg_cfg_menu;
            break;
        case '6':
            XAvpg_Config_printcfg(config, idx);
            goto avpg_cfg_menu;
            break;
        case '9':
            /* Exit case */
            break;
        default:
            xil_printf("Invalid input \r\n");
            goto avpg_cfg_menu;
    }
}

/*****************************************************************************/
/**
*
* This function configures format
*
* @param    format - Pointer to format variable
* @param    format_name - Name of the format being configured
*
* @return   XST_SUCCESS
*
* @note     None
*
******************************************************************************/
u32 XDpDc_ConfigureFormat(u32 *format, const char *format_name)
{
    char choice;

    xil_printf("\r\n=== %s Configuration ===\r\n", format_name);
    xil_printf("Common Formats:\r\n");
    xil_printf("1. RGBA8888 (%d)\r\n", RGBA8888);
    xil_printf("2. RGB888 (%d)\r\n", RGB888);
    xil_printf("3. RGB565 (%d)\r\n", RGB565);
    xil_printf("4. YV16 (%d)\r\n", YV16);
    xil_printf("5. YV16_420 (%d)\r\n", YV16_420);
    xil_printf("6. RGB888_GFX (%d)\r\n", RGB888_GFX);
    xil_printf("7. Custom (enter format number)\r\n");
    xil_printf("n. List All Non-Live Input Formats\r\n");
    xil_printf("l. List All Live Output Formats\r\n");
    xil_printf("Enter choice (1-7, n, l): ");

    choice = read_char();
    xil_printf("\r\n");

    switch(choice) {
        case '1':
            *format = RGBA8888;
            break;
        case '2':
            *format = RGB888;
            break;
        case '3':
            *format = RGB565;
            break;
        case '4':
            *format = YV16;
            break;
        case '5':
            *format = YV16_420;
            break;
        case '6':
            *format = RGB888_GFX;
            break;
        case '7':
            xil_printf("Enter format number: ");
            *format = read_uint();
            xil_printf("%s set to: %s (%d)\r\n", format_name, format_to_string(*format), *format);
            return XST_SUCCESS;
        case 'n':
        case 'N':
            XDpDc_ListNonliveFormats();
            xil_printf("\r\nPress any key to return to format menu...\r\n");
            read_char();
            return XDpDc_ConfigureFormat(format, format_name);
        case 'l':
        case 'L':
            XDpDc_ListLiveFormats();
            xil_printf("\r\nPress any key to return to format menu...\r\n");
            read_char();
            return XDpDc_ConfigureFormat(format, format_name);
        default:
            xil_printf("Invalid choice! Using RGBA8888\r\n");
            *format = RGBA8888;
    }

    xil_printf("%s set to: %s (%d)\r\n", format_name, format_to_string(*format), *format);
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures output format (live stream)
*
* @param    format - Pointer to format variable
* @param    format_name - Name of the format being configured
*
* @return   XST_SUCCESS
*
* @note     This is specialized for live output formats
*
******************************************************************************/
u32 XDpDc_ConfigureOutputFormat(u32 *format, const char *format_name)
{
    char choice;

    xil_printf("\r\n=== %s Configuration ===\r\n", format_name);
    xil_printf("Live Output Formats:\r\n");
    xil_printf("1. RGB_8BPC  (RGB 8-bit per channel) - %d\r\n", RGB_8BPC);
    xil_printf("2. RGB_10BPC (RGB 10-bit per channel) - %d\r\n", RGB_10BPC);
    xil_printf("3. RGB_12BPC (RGB 12-bit per channel) - %d\r\n", RGB_12BPC);
    xil_printf("4. Custom (enter format number)\r\n");
    xil_printf("l. List All Live Output Formats\r\n");
    xil_printf("Enter choice (1-4, l): ");

    choice = read_char();
    xil_printf("\r\n");

    switch(choice) {
        case '1':
            *format = RGB_8BPC;
            break;
        case '2':
            *format = RGB_10BPC;
            break;
        case '3':
            *format = RGB_12BPC;
            break;
        case '4':
            xil_printf("Enter format number: ");
            *format = read_uint();
            xil_printf("%s set to: %s (%d)\r\n", format_name, format_to_string(*format), *format);
            return XST_SUCCESS;
        case 'l':
        case 'L':
            XDpDc_ListLiveFormats();
            xil_printf("\r\nPress any key to return to format menu...\r\n");
            read_char();
            return XDpDc_ConfigureOutputFormat(format, format_name);
        default:
            xil_printf("Invalid choice! Using RGB_8BPC\r\n");
            *format = RGB_8BPC;
    }

    xil_printf("%s set to: %s (%d)\r\n", format_name, format_to_string(*format), *format);
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures cursor parameters (coordinates and size)
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   XST_SUCCESS
*
* @note     None
*
******************************************************************************/
u32 XDpDc_ConfigureCursor(InitRunConfig *config)
{
    xil_printf("\r\n=== Cursor Configuration ===\r\n");
    xil_printf("Current Cursor Settings:\r\n");
    xil_printf("  Position: (%d, %d)\r\n", config->cursor_coord_x, config->cursor_coord_y);
    xil_printf("  Size: %d x %d\r\n", config->cursor_size_x, config->cursor_size_y);
    xil_printf("\r\n");

    xil_printf("Enter Cursor X Coordinate (0-640): ");
    config->cursor_coord_x = read_uint();

    xil_printf("Enter Cursor Y Coordinate (0-480): ");
    config->cursor_coord_y = read_uint();

    xil_printf("Enter Cursor Width (1-256): ");
    config->cursor_size_x = read_uint();

    xil_printf("Enter Cursor Height (1-256): ");
    config->cursor_size_y = read_uint();

    xil_printf("\r\nCursor configured:\r\n");
    xil_printf("  Position: (%d, %d)\r\n", config->cursor_coord_x, config->cursor_coord_y);
    xil_printf("  Size: %d x %d\r\n", config->cursor_size_x, config->cursor_size_y);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures partial plane blend parameters
*
* @param	config - Pointer to InitRunConfig structure
*
* @return	XST_SUCCESS
*
* @note		None
*
******************************************************************************/
u32 XDpDc_ConfigurePartialPlane(InitRunConfig *config)
{
    u32 stream_choice;

    xil_printf("\r\n=== Partial Plane Blend Configuration ===\r\n");
    xil_printf("Current Partial Plane Settings:\r\n");
    xil_printf("  Stream: %s\r\n", config->ppb_stream_select == 1 ? "Stream1" : "Stream2");
    xil_printf("  Position: (%d, %d)\r\n", config->ppb_coord_x, config->ppb_coord_y);
    xil_printf("  Size: %d x %d\r\n", config->ppb_size_x, config->ppb_size_y);
    xil_printf("  Offset: (%d, %d)\r\n", config->ppb_offset_x, config->ppb_offset_y);
    xil_printf("\r\n");

    /* Select stream */
    xil_printf("Select Partial Plane Source:\r\n");
    xil_printf("  1 - Stream1 (as source, Stream2 as blend)\r\n");
    xil_printf("  2 - Stream2 (as source, Stream1 as blend)\r\n");
    xil_printf("Enter choice (1 or 2): ");
    stream_choice = read_uint();

    if (stream_choice == 1 || stream_choice == 2) {
        config->ppb_stream_select = stream_choice;
    } else {
        xil_printf("Invalid choice, defaulting to Stream1\r\n");
        config->ppb_stream_select = 1;
    }

    /* Configure coordinates */
    xil_printf("Enter Partial Plane X Coordinate (xp): ");
    config->ppb_coord_x = read_uint();

    xil_printf("Enter Partial Plane Y Coordinate (yp): ");
    config->ppb_coord_y = read_uint();

    /* Configure size */
    xil_printf("Enter Partial Plane Width (xp_size): ");
    config->ppb_size_x = read_uint();

    xil_printf("Enter Partial Plane Height (yp_size): ");
    config->ppb_size_y = read_uint();

    /* Configure offset */
    xil_printf("Enter Source Plane X Offset (xs): ");
    config->ppb_offset_x = read_uint();

    xil_printf("Enter Source Plane Y Offset (ys): ");
    config->ppb_offset_y = read_uint();

    /* Display summary */
    xil_printf("\r\nPartial Plane Blend configured:\r\n");
    xil_printf("  Source Stream: %s\r\n", config->ppb_stream_select == 1 ? "Stream1" : "Stream2");
    xil_printf("  Blend Stream: %s\r\n", config->ppb_stream_select == 1 ? "Stream2" : "Stream1");
    xil_printf("  Partial Plane Position (xp, yp): (%d, %d)\r\n",
               config->ppb_coord_x, config->ppb_coord_y);
    xil_printf("  Partial Plane Size (xp_size, yp_size): %d x %d\r\n",
               config->ppb_size_x, config->ppb_size_y);
    xil_printf("  Source Offset (xs, ys): (%d, %d)\r\n",
               config->ppb_offset_x, config->ppb_offset_y);
    xil_printf("  Final Position in Source: (%d, %d)\r\n",
               config->ppb_offset_x + config->ppb_coord_x,
               config->ppb_offset_y + config->ppb_coord_y);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DP link parameters (lane count and link rate)
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   None
*
******************************************************************************/
static void XDpDc_ConfigureLink(InitRunConfig *config)
{
    char cmd;

    xil_printf("\r\n=== Link Configuration ===\r\n");
    xil_printf("Select Lane Count:\r\n");
    xil_printf("  0 - Auto (use sink max)\r\n");
    xil_printf("  1 - 1 Lane\r\n");
    xil_printf("  2 - 2 Lanes\r\n");
    xil_printf("  4 - 4 Lanes\r\n");
    xil_printf("Enter choice: ");
    cmd = inbyte();
    outbyte(cmd);
    outbyte('\r');
    outbyte('\n');

    switch (cmd) {
    case '0': config->lane_count = 0; break;
    case '1': config->lane_count = 1; break;
    case '2': config->lane_count = 2; break;
    case '4': config->lane_count = 4; break;
    default:
        xil_printf("Invalid choice, keeping current: %d\r\n",
                   config->lane_count);
        goto rate_select;
    }
    xil_printf("Lane Count set to: %s\r\n",
               config->lane_count == 0 ? "Auto" :
               config->lane_count == 1 ? "1" :
               config->lane_count == 2 ? "2" : "4");

rate_select:
    xil_printf("\r\nSelect Link Rate:\r\n");
    xil_printf("  0 - Auto (use sink max)\r\n");
    xil_printf("  1 - RBR  (1.62 Gbps, DPCD 0x06)\r\n");
    xil_printf("  2 - HBR  (2.70 Gbps, DPCD 0x0A)\r\n");
    xil_printf("  3 - HBR2 (5.40 Gbps, DPCD 0x14)\r\n");
    xil_printf("  4 - HBR3 (8.10 Gbps, DPCD 0x1E)\r\n");
    xil_printf("Enter choice: ");
    cmd = inbyte();
    outbyte(cmd);
    outbyte('\r');
    outbyte('\n');

    switch (cmd) {
    case '0': config->link_rate = 0;    break;
    case '1': config->link_rate = 0x06; break;
    case '2': config->link_rate = 0x0A; break;
    case '3': config->link_rate = 0x14; break;
    case '4': config->link_rate = 0x1E; break;
    default:
        xil_printf("Invalid choice, keeping current: 0x%02X\r\n",
                   config->link_rate);
        return;
    }

    xil_printf("Link Rate set to: %s\r\n",
               config->link_rate == 0    ? "Auto" :
               config->link_rate == 0x06 ? "RBR (1.62 Gbps)" :
               config->link_rate == 0x0A ? "HBR (2.70 Gbps)" :
               config->link_rate == 0x14 ? "HBR2 (5.40 Gbps)" :
               config->link_rate == 0x1E ? "HBR3 (8.10 Gbps)" : "Unknown");
}

static void XDpDc_ValidateModeSpecificConfig(InitRunConfig *config)
{
    if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
        if (config->livevidselect != XDCSUB_LIVVID_SEL_NONE) {
            xil_printf("Adjusting livevidselect to Disabled for non-live mode\r\n");
            config->livevidselect = XDCSUB_LIVVID_SEL_NONE;
        }
        if (config->livevideo02mode != XDCSUB_VIDMODE_NONE) {
            xil_printf("Adjusting livevideo02mode to Disabled for non-live mode\r\n");
            config->livevideo02mode = XDCSUB_VIDMODE_NONE;
        }
    } else if (config->presentationmode == XDCSUB_PPTMODE_LIVE) {
        if (config->livevidselect != XDCSUB_LIVVID_SEL_BOTH) {
            xil_printf("Adjusting livevidselect to Both for live mode\r\n");
            config->livevidselect = XDCSUB_LIVVID_SEL_BOTH;
        }
        if (config->livevideo02mode == XDCSUB_VIDMODE_NONE) {
            xil_printf("Adjusting livevideo02mode to Video_only for live mode\r\n");
            config->livevideo02mode = XDCSUB_VIDMODE_VONLY;
        }
    } else if (config->presentationmode == XDCSUB_PPTMODE_MIXED) {
        if (config->livevidselect != XDCSUB_LIVVID_SEL_V01) {
            xil_printf("Adjusting livevidselect to V01 for mixed mode\r\n");
            config->livevidselect = XDCSUB_LIVVID_SEL_V01;
        }
        if (config->livevideo01mode != XDCSUB_VIDMODE_AV) {
            xil_printf("Adjusting livevideo01mode to Audio_&_Video for mixed "
                       "mode\r\n");
            config->livevideo01mode = XDCSUB_VIDMODE_AV;
        }
        if (config->livevideo02mode != XDCSUB_VIDMODE_NONE) {
            xil_printf("Adjusting livevideo02mode to Disabled for mixed mode\r\n");
            config->livevideo02mode = XDCSUB_VIDMODE_NONE;
        }
    }
}

static void XDpDc_SelectPresentationMode(InitRunConfig *config)
{
    char choice;

    xil_printf("\r\n=== Presentation Mode ===\r\n");
    xil_printf("0. Non-Live (both streams from memory)\r\n");
    xil_printf("1. Live (both streams from live source)\r\n");
    xil_printf("2. Mixed (Stream1 live + Stream2 non-live)\r\n");
    xil_printf("Enter choice (0-2): ");
    choice = read_char();
    xil_printf("\r\n");

    switch (choice) {
    case '0':
        config->presentationmode = XDCSUB_PPTMODE_NONLIVE;
        config->livevidselect = XDCSUB_LIVVID_SEL_NONE;
        config->livevideo02mode = XDCSUB_VIDMODE_NONE;
        config->stream1_format = RGBA8888;
        config->stream2_format = RGBA8888;
        config->cursor_enable = 0;
        config->audio_enable = 0;
        config->audio_channels = 0;
        config->sdp_enable = 0;
        break;
    case '1':
        config->presentationmode = XDCSUB_PPTMODE_LIVE;
        config->livevidselect = XDCSUB_LIVVID_SEL_BOTH;
        config->livevideo01mode = XDCSUB_VIDMODE_AV;
        config->livevideo02mode = XDCSUB_VIDMODE_VONLY;
        config->stream1_format = RGB_8BPC;
        config->stream2_format = RGB_8BPC;
        config->avpg[0].bpc = 8;
        config->avpg[0].pix_fmt = 0;
        config->avpg[0].pattern = 1;
        config->avpg[0].colorimetry = 0;
        config->avpg[0].ppc = 0;
        config->avpg[1] = config->avpg[0];
        config->cursor_enable = 0;
        config->audio_enable = 0;
        config->audio_channels = 0;
        config->sdp_enable = 0;
        break;
    case '2':
        config->presentationmode = XDCSUB_PPTMODE_MIXED;
        config->livevidselect = XDCSUB_LIVVID_SEL_V01;
        config->livevideo01mode = XDCSUB_VIDMODE_AV;
        config->livevideo02mode = XDCSUB_VIDMODE_NONE;
        config->stream1_format = RGB_8BPC;
        config->stream2_format = RGBA8888;
        config->avpg[0].bpc = 8;
        config->avpg[0].pix_fmt = 0;
        config->avpg[0].pattern = 1;
        config->avpg[0].colorimetry = 0;
        config->avpg[0].ppc = 0;
        config->cursor_enable = 0;
        config->audio_enable = 0;
        config->audio_channels = 0;
        config->sdp_enable = 0;
        break;
    default:
        xil_printf("Invalid choice, keeping mode %d\r\n",
                   config->presentationmode);
        return;
    }

}

/*****************************************************************************/
/**
*
* This function runs the interactive menu loop
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_MenuLoop(InitRunConfig *config)
{
    char cmd;
    u8 done = 0;

    XDpDc_MainHelpMenu(config);

    while (!done) {
        xil_printf("\r\nEnter command (h for help): ");
        cmd = inbyte();
        outbyte(cmd);
        outbyte('\r');
        outbyte('\n');

        switch(cmd) {
            case 'r':
            case 'R':
                XDpDc_ConfigureResolution(config);
                break;

            case 'f':
            case 'F':
                if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
                    if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE)
                        XDpDc_ConfigureFormat(&config->stream1_format, "Stream1 Format");
                    if (config->presentationmode == XDCSUB_PPTMODE_LIVE ||
                        config->presentationmode == XDCSUB_PPTMODE_MIXED)
                        XAvpg_Config(config, XAVPATGEN_INST_0);
                }
                break;

            case 'g':
            case 'G':
                if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {
                    if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE ||
                        config->presentationmode == XDCSUB_PPTMODE_MIXED)
                        XDpDc_ConfigureFormat(&config->stream2_format, "Stream2 Format");
                    if (config->presentationmode == XDCSUB_PPTMODE_LIVE)
                        XAvpg_Config(config, XAVPATGEN_INST_1);
                }
                break;

            case 'o':
            case 'O':
                XDpDc_ConfigureOutputFormat(&config->output_format, "Output Format");
                break;

            case 'c':
            case 'C':
                config->cursor_enable = !config->cursor_enable;
                xil_printf("Cursor Enable: %s\r\n", config->cursor_enable ? "Yes" : "No");
                if (config->cursor_enable && config->sdp_enable)
                    xil_printf("Cursor+SDP mode enabled (cursor transmits first, then SDP)\r\n");

                /* If enabling cursor, prompt for position & size configuration */
                if (config->cursor_enable) {
                    char subchoice;
                    xil_printf("\r\nConfigure cursor position & size? (y/n): ");
                    subchoice = read_char();
                    xil_printf("\r\n");
                    if (subchoice == 'y' || subchoice == 'Y') {
                        XDpDc_ConfigureCursor(config);
                    } else {
                        xil_printf("Using default cursor settings: Position(%d,%d) Size(%dx%d)\r\n",
                                   config->cursor_coord_x, config->cursor_coord_y,
                                   config->cursor_size_x, config->cursor_size_y);
                    }
                }
                break;

            case 'x':
            case 'X':
                XDpDc_ConfigureCursor(config);
                break;

            case 'a':
            case 'A':
                config->audio_enable = !config->audio_enable;
                xil_printf("Audio Enable: %s\r\n", config->audio_enable ? "Yes" : "No");
                if (config->audio_enable && config->sdp_enable) {
                    config->sdp_enable = 0;
                    xil_printf("SDP disabled (audio path and SDP test mode are mutually exclusive)\r\n");
                }
                if (config->audio_enable) {
                    u32 channels;

                    xil_printf("Enter Number of audio channels (1-8): ");
                    channels = read_uint();
                    if (channels < XDCSUB_AUDIO_CHANNELS_MIN ||
                        channels > XDCSUB_AUDIO_CHANNELS_MAX) {
                        xil_printf("ERROR: Invalid channel count %d. Audio disabled.\r\n",
                                   channels);
                        config->audio_enable = 0;
                        config->audio_channels = 0;
                    } else {
                        config->audio_channels = (u8)channels;
                        xil_printf("Audio Channels: %d\r\n", config->audio_channels);
                    }
                } else {
                    config->audio_channels = 0;
                    xil_printf("Audio Channels: %d\r\n", config->audio_channels);
                }
                break;

            case 'n':
            case 'N':
                config->sdp_enable = !config->sdp_enable;
                xil_printf("SDP Enable: %s\r\n", config->sdp_enable ? "Yes" : "No");
                if (config->sdp_enable && config->cursor_enable)
                    xil_printf("Cursor+SDP mode enabled (cursor transmits first, then SDP)\r\n");
                if (config->sdp_enable && config->audio_enable) {
                    config->audio_enable = 0;
                    xil_printf("Audio disabled (audio path and SDP test mode are mutually exclusive)\r\n");
                }
                break;

            case 'p':
            case 'P':
                config->partial_plane_blend_enable = !config->partial_plane_blend_enable;
                xil_printf("Partial Plane Blend Enable: %s\r\n", config->partial_plane_blend_enable ? "Yes" : "No");

                /* If enabling partial plane, prompt for configuration */
                if (config->partial_plane_blend_enable) {
                    char subchoice;
                    xil_printf("\r\nConfigure partial plane blend parameters? (y/n): ");
                    subchoice = read_char();
                    xil_printf("\r\n");
                    if (subchoice == 'y' || subchoice == 'Y') {
                        XDpDc_ConfigurePartialPlane(config);
                    } else {
                        xil_printf("Using default partial plane settings\r\n");
                    }
                }
                break;

            case 'b':
            case 'B':
                XDpDc_ConfigurePartialPlane(config);
                break;

            case 'l':
            case 'L':
                XDpDc_ConfigureLink(config);
                break;

            case 't':
            case 'T':
                XDpDc_SelectPresentationMode(config);
                XDpDc_MainHelpMenu(config);
                break;

            case 'd':
            case 'D':
                XDpDc_DisplayConfig(config);
                break;

            case 'h':
            case 'H':
                XDpDc_MainHelpMenu(config);
                break;

            case 's':
            case 'S':
                xil_printf("\r\nStarting application with current configuration...\r\n");
                XDpDc_DisplayConfig(config);
                done = 1;
                break;

            case 'q':
            case 'Q':
                xil_printf("\r\nQuitting...\r\n");
                done = 1;
                break;

            default:
                xil_printf("Unknown command. Press 'h' for help.\r\n");
        }
    }
}

/*****************************************************************************/
/**
*
* Reads a hex value from UART input, terminated by Enter.
* Accepts 0-9, a-f, A-F. Supports backspace. Optional "0x" prefix is skipped.
*
******************************************************************************/
static u32 read_hex(void)
{
    u32 val = 0;
    char c;

    while (1) {
        c = inbyte();
        if (c >= '0' && c <= '9') {
            outbyte(c);
            val = (val << 4) | (u32)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            outbyte(c);
            val = (val << 4) | (u32)(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            outbyte(c);
            val = (val << 4) | (u32)(c - 'A' + 10);
        } else if (c == 'x' || c == 'X') {
            outbyte(c);
            val = 0;
        } else if (c == '\r' || c == '\n') {
            outbyte('\r');
            outbyte('\n');
            break;
        } else if (c == 0x08 || c == 0x7F) {
            val = val >> 4;
            outbyte(0x08);
            outbyte(' ');
            outbyte(0x08);
        }
    }

    return val;
}

/*****************************************************************************/
/**
*
* AUX Read/Write submenu. Allows the user to perform DPCD AUX read and
* write transactions with configurable address, data, and iteration count.
*
* @param    DpPtr - Pointer to the initialized XMmiDp instance
*
******************************************************************************/
void XDpDc_AuxMenu(XMmiDp *DpPtr)
{
    char cmd;
    u32 addr, iterations, i, status;
    u8 write_data;
    u8 read_buf[16];

    xil_printf("\r\n");
    xil_printf("============================================\r\n");
    xil_printf("          AUX Read/Write Menu               \r\n");
    xil_printf("============================================\r\n");
    xil_printf("  r - AUX Read\r\n");
    xil_printf("  w - AUX Write\r\n");
    xil_printf("  q - Return\r\n");
    xil_printf("============================================\r\n");
    xil_printf("Enter choice: ");

    cmd = read_char();
    xil_printf("\r\n");

    switch (cmd) {
    case 'r':
    case 'R':
        xil_printf("\r\n=== AUX Read ===\r\n");
        xil_printf("Enter DPCD address (hex): ");
        addr = read_hex();
        xil_printf("Enter number of iterations: ");
        iterations = read_uint();

        xil_printf("\r\nReading DPCD 0x%05x, %d iteration(s):\r\n",
                   addr, iterations);

        for (i = 0; i < iterations; i++) {
            read_buf[0] = 0;
            status = XMmiDp_AuxRead(DpPtr, addr, 1, read_buf);
            if (status == XST_SUCCESS) {
                xil_printf("  [%d] DPCD[0x%05x] = 0x%02x\r\n",
                           i + 1, addr, read_buf[0]);
            } else {
                xil_printf("  [%d] AUX Read FAILED (status=0x%x)\r\n",
                           i + 1, status);
            }
        }
        xil_printf("AUX Read complete.\r\n");
        break;

    case 'w':
    case 'W':
        xil_printf("\r\n=== AUX Write ===\r\n");
        xil_printf("Enter DPCD address (hex): ");
        addr = read_hex();
        xil_printf("Enter data byte (hex): ");
        write_data = (u8)read_hex();
        xil_printf("Enter number of iterations: ");
        iterations = read_uint();

        xil_printf("\r\nWriting 0x%02x to DPCD 0x%05x, %d iteration(s):\r\n",
                   write_data, addr, iterations);

        for (i = 0; i < iterations; i++) {
            status = XMmiDp_AuxWrite(DpPtr, addr, 1, &write_data);
            if (status == XST_SUCCESS) {
                xil_printf("  [%d] Write 0x%02x -> DPCD[0x%05x] OK\r\n",
                           i + 1, write_data, addr);
            } else {
                xil_printf("  [%d] AUX Write FAILED (status=0x%x)\r\n",
                           i + 1, status);
            }
        }
        xil_printf("AUX Write complete.\r\n");
        break;

    case 'q':
    case 'Q':
        break;

    default:
        xil_printf("Invalid choice.\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function initializes the InitRunConfig with default values
*
* @param    config - Pointer to InitRunConfig structure
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDpDc_InitConfigDefaults(InitRunConfig *config)
{
    XDcSub_Config *DcSubCfgPtr;
    const char *opmodefunc = "DC_Functional";
    const char *opmodebypass = "DC_Bypass";
    const char *nlmode = "Non_Live";
    const char *livemode = "Live";
    const char *mixmode = "Mixed";
    const char *dclivmodeboth = "Both";
    const char *dcmodev01 = "V01";
    const char *dcmodev02 = "V02";
    const char *avmode = "Audio_&_Video";
    const char *vidonlymode = "Video_only";
    u32 idx;

    DcSubCfgPtr = XDcSub_LookupConfig(DC_BASEADDR);

    if (!strncmp(DcSubCfgPtr->DcConfig.operatingmode, opmodefunc, strlen(opmodefunc)))
        config->operatingmode = XDCSUB_OPMODE_FUNCTIONAL;

    if (!strncmp(DcSubCfgPtr->DcConfig.operatingmode, opmodebypass, strlen(opmodebypass)))
        config->operatingmode = XDCSUB_OPMODE_BYPASS;

    if (!strncmp(DcSubCfgPtr->DcConfig.presentationmode, nlmode, strlen(nlmode)))
        config->presentationmode = XDCSUB_PPTMODE_NONLIVE;
    else if (!strncmp(DcSubCfgPtr->DcConfig.presentationmode, livemode, strlen(livemode)))
        config->presentationmode = XDCSUB_PPTMODE_LIVE;
    else if (!strncmp(DcSubCfgPtr->DcConfig.presentationmode, mixmode, strlen(mixmode)))
        config->presentationmode = XDCSUB_PPTMODE_MIXED;
    else
        config->presentationmode = XDCSUB_PPTMODE_NONLIVE;

    if (!strncmp(DcSubCfgPtr->DcConfig.livevideoselect, dclivmodeboth, strlen(dclivmodeboth)))
        config->livevidselect = XDCSUB_LIVVID_SEL_BOTH;
    else if (!strncmp(DcSubCfgPtr->DcConfig.livevideoselect, dcmodev01, strlen(dcmodev01)))
        config->livevidselect = XDCSUB_LIVVID_SEL_V01;
    else if (!strncmp(DcSubCfgPtr->DcConfig.livevideoselect, dcmodev02, strlen(dcmodev02)))
        config->livevidselect = XDCSUB_LIVVID_SEL_V02;
    else
        config->livevidselect = XDCSUB_LIVVID_SEL_NONE;

    if (!strncmp(DcSubCfgPtr->DcConfig.livevideo01mode, avmode, strlen(avmode)))
        config->livevideo01mode = XDCSUB_VIDMODE_AV;
    else if (!strncmp(DcSubCfgPtr->DcConfig.livevideo01mode, avmode, strlen(vidonlymode)))
        config->livevideo01mode = XDCSUB_VIDMODE_VONLY;
    else
        config->livevideo01mode = XDCSUB_VIDMODE_NONE;

    if (!strncmp(DcSubCfgPtr->DcConfig.livevideo02mode, vidonlymode, strlen(vidonlymode)))
        config->livevideo02mode = XDCSUB_VIDMODE_VONLY;
    else
        config->livevideo02mode = XDCSUB_VIDMODE_NONE;

    config->livevideosdpen = DcSubCfgPtr->DcConfig.livevideosdpen;
    config->livevideoalphaen = DcSubCfgPtr->DcConfig.livevideoalphaen;

    config->byp_streams = DcSubCfgPtr->DcConfig.streams;

    if (config->byp_streams > 0) {
        u32 i;

        for (i = 0; i < XAVPATGEN_INST_MAX_COUNT; i++) {
            if (!strncmp(DcSubCfgPtr->DcConfig.StreamConfig[i].mode, avmode, strlen(avmode)))
                config->byp_stream_vid_mode[i] = XDCSUB_VIDMODE_AV;
            else
                config->byp_stream_vid_mode[i] = XDCSUB_VIDMODE_VONLY;

            config->byp_stream_pix_mode[i] = DcSubCfgPtr->DcConfig.StreamConfig[i].pixel_mode;
            config->byp_stream_sdp_en[i] = DcSubCfgPtr->DcConfig.StreamConfig[i].sdpen;
        }
    }

    if (config->operatingmode == XDCSUB_OPMODE_FUNCTIONAL) {

        config->width = APP_DEFAULT_WIDTH;
        config->height = APP_DEFAULT_HEIGHT;
        config->frame_rate = APP_DEFAULT_FPS;
        config->output_format = APP_DEFAULT_DC_OUT_FMT;

        config->cursor_enable = 0;
        config->cursor_coord_x = 50;
        config->cursor_coord_y = 50;

        /* Default partial plane blend parameters (from reference) */
        config->partial_plane_blend_enable = 0;
        config->ppb_stream_select = 1;
        config->ppb_coord_x = 0;
        config->ppb_coord_y = 0;
        config->ppb_size_x = 256;
        config->ppb_size_y = 256;
        config->ppb_offset_x = 80;
        config->ppb_offset_y = 80;

        config->audio_enable = 0;
        config->audio_channels = 0;
        config->sdp_enable = 0;

        if (config->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
            config->stream1_format = APP_DEFAULT_NL_PIXFMT;
            config->stream2_format = APP_DEFAULT_NL_PIXFMT;

            config->cursor_enable = 0;
            /* Hardware fixed size - use alpha=0 for smaller cursor */
            config->cursor_size_x = 128;
            /* Hardware fixed size - use alpha=0 for smaller cursor */
            config->cursor_size_y = 128;
        }

        /* Functional Live mode */
        if (config->presentationmode == XDCSUB_PPTMODE_LIVE) {

            config->stream1_format = APP_DEFAULT_LIVE_PIXFMT;
            config->stream2_format = APP_DEFAULT_LIVE_PIXFMT;

            for (idx = 0; idx < 4; idx++) {
                /* Set up AVTPG as 8 bpc RGB, Color ramp, BT.601, dual pixel */
                config->avpg[idx].bpc = 8;
                config->avpg[idx].pix_fmt = 0;
                config->avpg[idx].pattern = 1;
                config->avpg[idx].colorimetry = 0;
                config->avpg[idx].ppc = 0;
            }
        }

        if (config->presentationmode == XDCSUB_PPTMODE_MIXED) {
                config->stream1_format = APP_DEFAULT_LIVE_PIXFMT;
                config->stream2_format = APP_DEFAULT_NL_PIXFMT;
                config->avpg[0].bpc = 8;
                config->avpg[0].pix_fmt = 0;
                config->avpg[0].pattern = 1;
                config->avpg[0].colorimetry = 0;
                config->avpg[0].ppc = 0;
        }

        XDpDc_ValidateModeSpecificConfig(config);

        xil_printf("InitRunConfig initialized with default values\r\n");
        xil_printf("  Presentation Mode: %s\r\n", DcSubCfgPtr->DcConfig.presentationmode);
        xil_printf("  Resolution: %dx%d @%dHz\r\n", config->width, config->height, config->frame_rate);
        xil_printf("  Stream 1 Format: %s\r\n", format_to_string(config->stream1_format));
        xil_printf("  Stream 2 Format: %s\r\n", format_to_string(config->stream2_format));
        xil_printf("  Output Format: %s\r\n", format_to_string(config->output_format));

        xil_printf("  Cursor: Disabled, Position (50,50), Size (128x128)\r\n");
        xil_printf("  Audio: Disabled, Channels: %d\r\n",
                   config->audio_channels);
        xil_printf("  SDP: Disabled\r\n");
    }

    config->lane_count = 0;
    config->link_rate = 0;

    xil_printf("  DP Link: Auto (sink max)\r\n");
}

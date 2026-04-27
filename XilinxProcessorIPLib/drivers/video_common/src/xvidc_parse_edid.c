/* vim: set et fde fdm=syntax ft=c.doxygen ts=4 sts=4 sw=4 : */
/*
 * Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>.
 * Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*****************************************************************************/
/**
 *
 * @file xvidc_parse_edid.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- --- ----------   -----------------------------------------------
 * 1.00  mmo 24-01-2017   Included into video_common
 * 1.01  eb  13-04-2018   Fixed XV_VidC_parse_edid API and
 *                            xvidc_edid_extension_handler struct to enhance
 *                            system stability
 * 1.10  eb  03-08-2018   Updated XV_VidC_parse_edid
 * </pre>
 *
 ******************************************************************************/

#include "string.h"
#include "stdlib.h"
#include "stddef.h"

#include "xil_types.h"
#include "xstatus.h"
#include "xil_exception.h"

#include "xvidc_edid_ext.h"

#if XVIDC_EDID_VERBOSITY > 1
#define CM_2_MM(cm)                             ((cm) * 10)
#define CM_2_IN(cm)                             ((cm) * 0.3937)
#endif
#if XVIDC_EDID_VERBOSITY > 0
#define HZ_2_MHZ(hz)                            ((hz) / 1000000)
#endif


/* DisplayID Data Block Length Macros */
/** DisplayID parameters block length in bytes */
#define XVIDC_DISPLAYID_PARAMETERS_BLOCK_LEN        (12)
/** DisplayID timing range minimum length in bytes */
#define XVIDC_DISPLAYID_TIMING_RANGE_MIN_LEN        (8)
/** DisplayID device data minimum length in bytes */
#define XVIDC_DISPLAYID_DEVICE_DATA_MIN_LEN         (5)
/** DisplayID power sequence minimum length in bytes */
#define XVIDC_DISPLAYID_POWER_SEQ_MIN_LEN           (4)
/** DisplayID transfer characteristics minimum length in bytes */
#define XVIDC_DISPLAYID_TRANSFER_CHAR_MIN_LEN       (1)
/** DisplayID container ID minimum length in bytes */
#define XVIDC_DISPLAYID_CONTAINER_ID_MIN_LEN        (16)
/** DisplayID string maximum length */
#define XVIDC_DISPLAYID_STRING_MAX_LEN              (255)

/* DisplayID Timing Mode Macros */
/** Maximum number of DisplayID timing modes */
#define XVIDC_DISPLAYID_MAX_TIMING_MODES            (8)
/** Maximum number of DisplayID short timings */
#define XVIDC_DISPLAYID_MAX_SHORT_TIMINGS           (16)
/** DisplayID timing descriptor length in bytes */
#define XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN       (20)
/** DisplayID short timing length in bytes */
#define XVIDC_DISPLAYID_SHORT_TIMING_LEN            (3)
/** Maximum DisplayID detailed timing buffer size */
#define XVIDC_DISPLAYID_DETAILED_TIMING_BUFFER_MAX  (32)
/** Maximum DisplayID block data size */
#define XVIDC_DISPLAYID_BLOCK_DATA_MAX              (128)
/** DisplayID refresh rate base value in Hz */
#define XVIDC_DISPLAYID_REFRESH_RATE_BASE           (60)
/** DisplayID pixel clock multiplier */
#define XVIDC_DISPLAYID_PIXEL_CLOCK_MULTIPLIER      (10)
/** DisplayID detailed timing minimum length in bytes */
#define XVIDC_DISPLAYID_DETAILED_TIMING_MIN_LEN     (7)

/* DisplayID Color Characteristics Macros */
/** DisplayID color characteristics minimum length in bytes */
#define XVIDC_DISPLAYID_COLOR_CHAR_MIN_LEN          (4)
/** DisplayID color characteristics legacy length in bytes */
#define XVIDC_DISPLAYID_COLOR_CHAR_LEGACY_LEN       (13)
/** DisplayID color characteristics extended length in bytes */
#define XVIDC_DISPLAYID_COLOR_CHAR_EXTENDED_LEN     (19)
/** DisplayID color primary size in bytes */
#define XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE          (3)
/** DisplayID color white point size in bytes */
#define XVIDC_DISPLAYID_COLOR_WHITEPOINT_SIZE       (3)
/** DisplayID color coordinate divisor for precision */
#define XVIDC_DISPLAYID_COLOR_COORDINATE_DIVISOR    (4096.0)
/** DisplayID color depth offset */
#define XVIDC_DISPLAYID_COLOR_DEPTH_OFFSET          (8)
/** DisplayID parameters bits per color length */
#define XVIDC_DISPLAYID_PARAMETERS_BPC_LEN          (12)
/** CIE 1931 color space year identifier */
#define XVIDC_DISPLAYID_CIE_YEAR_1931               (1931)
/** CIE 1976 color space year identifier */
#define XVIDC_DISPLAYID_CIE_YEAR_1976               (1976)
/** Maximum standard colorspace index */
#define XVIDC_DISPLAYID_STD_COLORSPACE_MAX          (9)
/** DisplayID vendor specific minimum length in bytes */
#define XVIDC_DISPLAYID_VENDOR_SPEC_MIN_LEN         (3)
/** DisplayID Unicode language code length in bytes */
#define XVIDC_DISPLAYID_UNICODE_LANG_LEN            (3)
/** DisplayID maximum block size in bytes */
#define XVIDC_DISPLAYID_BLOCK_SIZE_MAX              (128)

/* DisplayID Data Block Header */
/** DisplayID data block header length in bytes */
#define XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN       (3)

/* DisplayID Bit Masks */
/** DisplayID CIE year bit mask */
#define XVIDC_DISPLAYID_CIE_YEAR_MASK               (0x80)
/** DisplayID transfer function ID bit mask */
#define XVIDC_DISPLAYID_XFER_ID_MASK                (0x0F)
/** DisplayID transfer function ID bit shift */
#define XVIDC_DISPLAYID_XFER_ID_SHIFT               (3)
/** DisplayID temporal color bit mask */
#define XVIDC_DISPLAYID_TEMPORAL_COLOR_MASK         (0x80)
/** DisplayID number of primaries bit mask */
#define XVIDC_DISPLAYID_NUM_PRIMARIES_MASK          (0x07)
/** DisplayID number of primaries bit shift */
#define XVIDC_DISPLAYID_NUM_PRIMARIES_SHIFT         (4)
/** DisplayID number of white points bit mask */
#define XVIDC_DISPLAYID_NUM_WHITEPOINTS_MASK        (0x0F)
/** DisplayID color lower nibble bit mask */
#define XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK     (0x0F)
/** DisplayID color upper nibble bit mask */
#define XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK     (0xF0)
/** DisplayID color upper nibble bit shift */
#define XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT    (4)
/** DisplayID bits per color bit mask */
#define XVIDC_DISPLAYID_BPC_MASK                    (0x0F)
/** DisplayID bits per color bit shift */
#define XVIDC_DISPLAYID_BPC_SHIFT                   (4)

/* DisplayID Interface Features Macros */
/** DisplayID interface minimum length in bytes */
#define XVIDC_DISPLAYID_INTERFACE_MIN_LEN           (3)
/** DisplayID interface extended length in bytes */
#define XVIDC_DISPLAYID_INTERFACE_EXTENDED_LEN      (10)
/** DisplayID interface color depth length in bytes */
#define XVIDC_DISPLAYID_INTERFACE_COLOR_DEPTH_LEN   (8)
/** DisplayID interface pixel clock length in bytes */
#define XVIDC_DISPLAYID_INTERFACE_PIXEL_CLK_LEN     (4)
/** DisplayID interface maximum clock length in bytes */
#define XVIDC_DISPLAYID_INTERFACE_MAX_CLK_LEN       (6)
/** DisplayID maximum interface count */
#define XVIDC_DISPLAYID_INTERFACE_MAX_COUNT         (4)
/** DisplayID default minimum pixel clock in MHz */
#define XVIDC_DISPLAYID_DEFAULT_MIN_PIXEL_CLK       (25)
/** DisplayID interface type for HDMI */
#define XVIDC_DISPLAYID_INTERFACE_TYPE_HDMI         (116)
/** DisplayID maximum pixel clock divisor */
#define XVIDC_DISPLAYID_MAX_PIXEL_CLK_DIVISOR       (10)

/* DisplayID Interface Color Depth Bit Masks */
/** DisplayID color depth 6 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_6BPC_MASK       (0x01)
/** DisplayID color depth 8 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_8BPC_MASK       (0x02)
/** DisplayID color depth 10 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_10BPC_MASK      (0x04)
/** DisplayID color depth 12 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_12BPC_MASK      (0x08)
/** DisplayID color depth 14 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_14BPC_MASK      (0x10)
/** DisplayID color depth 16 bits per color bit mask */
#define XVIDC_DISPLAYID_COLOR_DEPTH_16BPC_MASK      (0x20)
/** DisplayID timing descriptor length in bytes (duplicate definition) */
#define XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN       (20)
/** DisplayID detailed timing divisor */
#define XVIDC_DISPLAYID_DETAILED_TIMING_DIVISOR     (20)

/** EDID block size in bytes */
#define XVIDC_EDID_SIZE                             (128)
/** Maximum number of DisplayID CVT timings */
#define XVIDC_DISPLAYID_MAX_CVT_TIMINGS             (8)
/** DisplayID interface count bit mask */
#define XVIDC_DISPLAYID_INTERFACE_COUNT_MASK        (0x07)
/** DisplayID interface version bit mask */
#define XVIDC_DISPLAYID_INTERFACE_VERSION_MASK      (0x0F)

#if XVIDC_EDID_VERBOSITY > 1
static void
xvidc_disp_cea861_audio_data(
                  const struct xvidc_cea861_audio_data_block * const adb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861_speaker_allocation_data(
          const struct xvidc_cea861_speaker_allocation_data_block * const sadb,
             XV_VidC_EdidCntrlParam *EdidCtrlParam,
             XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861_video_data(
                  const struct xvidc_cea861_video_data_block * const vdb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);
#endif

static void
xvidc_disp_cea861_extended_data(
                  const struct xvidc_cea861_extended_data_block * const edb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861_hf_edid_ext_override_data(
		const struct xvidc_cea861_extended_data_block * const edb,
		XV_VidC_EdidCntrlParam *EdidCtrlParam,
		XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861_hf_sink_capability_data(
		const struct xvidc_cea861_extended_data_block * const edb,
		XV_VidC_EdidCntrlParam *EdidCtrlParam,
		XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861_vendor_data(
                  const struct xvidc_cea861_vendor_specific_data_block * vsdb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_cea861(const struct xvidc_edid_extension * const ext,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

static void
xvidc_dispid_block(const struct xvidc_edid_extension * const ext,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

static void
xvidc_disp_edid1(const struct edid * const edid,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

/* Forward declarations for DisplayID parsing functions - ordered by DisplayID block tag (0x00-0x20) */
static void xvidc_parse_displayid_product_id(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_parameters(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_color_characteristics(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_timing_mode_1(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_timing_mode_2(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_short_timings_3(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_dmt_timings_4(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_dmt_timings(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_cta_timings(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_timing_range(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_serial_number(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_ascii_string(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_device_data(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_power_sequencing(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_transfer_characteristics(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_interface_features(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_stereo_display(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_short_timings_5(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_tiled_display(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_detailed_timings_6(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_detailed_timing(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_detailed_timings_7(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_adaptive_refresh(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_unicode_string(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_cta_block(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_container_id(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
static void xvidc_parse_displayid_vendor_specific(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);

/*****************************************************************************/
/**
*
* This function parse EDID on General Data & VESA Data
*
* @param    edid is a pointer to the EDID structure.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   API Define below here are CEA861 routines
*
******************************************************************************/
static void
xvidc_disp_edid1(const struct edid * const edid,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn)
{
    const struct xvidc_edid_monitor_range_limits *monitor_range_limits = NULL;
    xvidc_edid_monitor_descriptor_string monitor_serial_number = {0};
    xvidc_edid_monitor_descriptor_string monitor_model_name = {0};
    bool has_ascii_string = false;
    char manufacturer[4] = {0};
#if XVIDC_EDID_VERBOSITY > 1
    XV_VidC_DoubleRep min_doubleval;
    XV_VidC_DoubleRep max_doubleval;
#endif

    u8 i;
#if XVIDC_EDID_VERBOSITY > 0
    XV_VidC_TimingParam timing_params;
#endif

#if XVIDC_EDID_VERBOSITY > 1
    struct xvidc_edid_color_characteristics_data characteristics;
    const u8 vlen = edid->maximum_vertical_image_size;
    const u8 hlen = edid->maximum_horizontal_image_size;


    static const char * const display_type[] = {
        [XVIDC_EDID_DISPLAY_TYPE_MONOCHROME] = "Monochrome or greyscale",
        [XVIDC_EDID_DISPLAY_TYPE_RGB]        = "sRGB colour",
        [XVIDC_EDID_DISPLAY_TYPE_NON_RGB]    = "Non-sRGB colour",
        [XVIDC_EDID_DISPLAY_TYPE_UNDEFINED]  = "Undefined",
    };
#endif
    xvidc_edid_manufacturer(edid, manufacturer);

    for (i = 0; i < ARRAY_SIZE(edid->detailed_timings); i++) {
        const struct xvidc_edid_monitor_descriptor * const mon =
            &edid->detailed_timings[i].monitor;

        if (!xvidc_edid_detailed_timing_is_monitor_descriptor(edid, i))
            continue;

        switch (mon->tag) {
        case XVIDC_EDID_MONTIOR_DESCRIPTOR_MANUFACTURER_DEFINED:
            /* This is arbitrary data, just silently ignore it. */
            break;
        case XVIDC_EDID_MONITOR_DESCRIPTOR_ASCII_STRING:
            has_ascii_string = true;
            break;
        case XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_NAME:
            strncpy(monitor_model_name, (char *) mon->data,
                    sizeof(monitor_model_name) - 1);
            break;
        case XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_RANGE_LIMITS:
            monitor_range_limits =
                         (struct xvidc_edid_monitor_range_limits *) &mon->data;
            break;
        case XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_SERIAL_NUMBER:
            strncpy(monitor_serial_number, (char *) mon->data,
                    sizeof(monitor_serial_number) - 1);
            break;
        default:
            if (VerboseEn) {
                xil_printf("unknown monitor descriptor type 0x%02x\n",
                        mon->tag);
            }
            break;
        }
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("Sink Information\r\n");

        xil_printf("  Model name............... %s\r\n",
               *monitor_model_name ? monitor_model_name : "n/a");
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("  Manufacturer............. %s\r\n",
               manufacturer);

        xil_printf("  Product code............. %u\r\n",
               (u16) edid->product_u16);

        if (edid->serial_number_u32)
            xil_printf("  Module serial number..... %u\r\n",
                   (u32) edid->serial_number_u32);
#endif
#if defined(DISPLAY_UNKNOWN)
            xil_printf("  Plug and Play ID......... %s\r\n", NULL);
#endif
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("  Serial number............ %s\r\n",
               *monitor_serial_number ? monitor_serial_number : "n/a");

        xil_printf("  Manufacture date......... %u",
                                                edid->manufacture_year + 1990);
        if (edid->manufacture_week <= 52)
            xil_printf(", ISO week %u", edid->manufacture_week);
        xil_printf("\r\n");
#endif
        xil_printf("  EDID revision............ %u.%u\r\n",
               edid->version, edid->revision);
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("  Input signal type........ %s\r\n",
          edid->video_input_definition.digital.digital ? "Digital" : "Analog");

        if (edid->video_input_definition.digital.digital) {
            xil_printf("  VESA DFP 1.x supported... %s\r\n",
                   edid->video_input_definition.digital.dfp_1x ? "Yes" : "No");
        } else {
            /* Missing Piece: To print analog flags */
        }
#endif
#if defined(DISPLAY_UNKNOWN)
        xil_printf("  Color bit depth.......... %s\r\n", NULL);
#endif
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("  Display type............. %s\r\n",
               display_type[edid->feature_support.display_type]);

        xil_printf("  Screen size.............. %u mm x %u mm\r\n",
               CM_2_MM(hlen), CM_2_MM(vlen));

        xil_printf("  Power management......... %s%s%s%s\r\n",
               edid->feature_support.active_off ? "Active off, " : "",
               edid->feature_support.suspend ? "Suspend, " : "",
               edid->feature_support.standby ? "Standby, " : "",

               (edid->feature_support.active_off ||
                edid->feature_support.suspend    ||
                edid->feature_support.standby) ? "\b\b  " : "n/a");
#endif
        xil_printf("  Extension blocks......... %u\r\n",
               edid->extensions);

#if defined(DISPLAY_UNKNOWN)
        xil_printf("  DDC/CI................... %s\r\n", NULL);
#endif

        xil_printf("\r\n");
	}
#endif
	EdidCtrlParam->Extensions = edid->extensions;
        if (has_ascii_string) {
			if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
				xil_printf("General purpose ASCII string\r\n");
#endif
			}

            for (i = 0; i < ARRAY_SIZE(edid->detailed_timings); i++) {
                if (!xvidc_edid_detailed_timing_is_monitor_descriptor(edid, i))
                    continue;
            }

			if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
				xil_printf("\r\n");
#endif
			}
        }
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("Color characteristics\r\n");

        xil_printf("  Default color space...... %ssRGB\r\n",
               edid->feature_support.standard_default_color_space ? "":"Non-");
#if XVIDC_EDID_VERBOSITY > 1
        min_doubleval =
                Double2Int(xvidc_edid_gamma(edid));
        xil_printf("  Display gamma............ %d.%03d\r\n",
        		min_doubleval.Integer, min_doubleval.Decimal);

        characteristics = xvidc_edid_color_characteristics(edid);

        min_doubleval =
			Double2Int(xvidc_edid_decode_fixed_point(characteristics.red.x));
        max_doubleval =
			Double2Int(xvidc_edid_decode_fixed_point(characteristics.red.y));

        xil_printf("  Red chromaticity......... Rx %d.%03d - Ry %d.%03d\r\n",
        	min_doubleval.Integer, min_doubleval.Decimal,
			max_doubleval.Integer, max_doubleval.Decimal);

        min_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.green.x));
        max_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.green.y));

        xil_printf("  Green chromaticity....... Gx %d.%03d - Gy %d.%03d\r\n",
        	min_doubleval.Integer, min_doubleval.Decimal,
			max_doubleval.Integer, max_doubleval.Decimal);

        min_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.blue.x));
        max_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.blue.y));

        xil_printf("  Blue chromaticity........ Bx %d.%03d - By %d.%03d\r\n",
        	min_doubleval.Integer, min_doubleval.Decimal,
			max_doubleval.Integer, max_doubleval.Decimal);

        min_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.white.x));
        max_doubleval =
        	Double2Int(xvidc_edid_decode_fixed_point(characteristics.white.y));

        xil_printf("  White point (default).... Wx %d.%03d - Wy %d.%03d\r\n",
        	min_doubleval.Integer, min_doubleval.Decimal,
			max_doubleval.Integer, max_doubleval.Decimal);
#endif
#if defined(DISPLAY_UNKNOWN)
        xil_printf("  Additional descriptors... %s\r\n", NULL);
#endif
        xil_printf("\r\n");

        xil_printf("VESA Timing characteristics\r\n");
    }
#endif
    if (monitor_range_limits) {
#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("  Horizontal scan range.... %u - %u kHz\r\n",
                   monitor_range_limits->minimum_horizontal_rate,
                   monitor_range_limits->maximum_horizontal_rate);

            xil_printf("  Vertical scan range...... %u - %u Hz\r\n",
                   monitor_range_limits->minimum_vertical_rate,
                   monitor_range_limits->maximum_vertical_rate);

            xil_printf("  Video bandwidth.......... %u MHz\r\n",
                   monitor_range_limits->maximum_supported_pixel_clock * 10);
        }
#endif
        EdidCtrlParam->MaxFrameRateSupp =
                                   monitor_range_limits->maximum_vertical_rate;
        EdidCtrlParam->MaxTmdsMhz =
                    (monitor_range_limits->maximum_supported_pixel_clock * 10);
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
#if defined(DISPLAY_UNKNOWN)
        xil_printf("  CVT standard............. %s\r\n", NULL);
#endif
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("  GTF standard............. %sSupported\r\n",
               edid->feature_support.default_gtf ? "" : "Not ");
#endif
#if defined(DISPLAY_UNKNOWN)
        xil_printf("  Additional descriptors... %s\r\n", NULL);
#endif
#if XVIDC_EDID_VERBOSITY > 0
        xil_printf("  Preferred timing......... %s\r\n",
               edid->feature_support.preferred_timing_mode ? "Yes" : "No");
#endif
        for (i = 0; i < ARRAY_SIZE(edid->detailed_timings); i++) {
            if (xvidc_edid_detailed_timing_is_monitor_descriptor(edid, i))
                continue;

            timing_params = XV_VidC_timing(&edid->detailed_timings[i].timing);
            EdidCtrlParam->PreferedTiming[i] =
            		XV_VidC_timing(&edid->detailed_timings[i].timing);
#if XVIDC_EDID_VERBOSITY > 0
			if (edid->feature_support.preferred_timing_mode) {
				xil_printf("  Native/preferred timing.. %ux%u%c at %uHz"
											" (%u:%u)\r\n",
											timing_params.hres,
											timing_params.vres,
											timing_params.vidfrmt ? 'i' : 'p',
											timing_params.vfreq,
											timing_params.aspect_ratio.width,
											timing_params.aspect_ratio.height);
				xil_printf("    Modeline............... \"%ux%u\" %u %u %u %u"
								" %u %u %u %u %u %chsync %cvsync\r\n",
							   timing_params.hres,
							   timing_params.vres,
							   HZ_2_MHZ (timing_params.pixclk),
							   (timing_params.hres),
							   (timing_params.hres + timing_params.hfp),
							   (timing_params.hres + timing_params.hfp +
													timing_params.hsync_width),
							   (timing_params.htotal),
							   (timing_params.vres),
							   (timing_params.vres + timing_params.vfp),
							   (timing_params.vres + timing_params.vfp +
													timing_params.vsync_width),
							   (timing_params.vtotal),
							   timing_params.hsync_polarity ? '+' : '-',
							   timing_params.vsync_polarity ? '+' : '-');
			} else {
				xil_printf("  Native/preferred timing.. n/a\r\n");
			}
#endif
        }
#if XVIDC_EDID_VERBOSITY > 0
        xil_printf("\r\n");
#endif
#if XVIDC_EDID_VERBOSITY > 1
        xil_printf("Established Timings supported\r\n");
        if (edid->established_timings.timing_720x400_70)
            xil_printf("   720 x  400p @ 70Hz - IBM VGA\r\n");
        if (edid->established_timings.timing_720x400_88)
            xil_printf("   720 x  400p @ 88Hz - IBM XGA2\r\n");
        if (edid->established_timings.timing_640x480_60)
            xil_printf("   640 x  480p @ 60Hz - IBM VGA\r\n");
        if (edid->established_timings.timing_640x480_67)
            xil_printf("   640 x  480p @ 67Hz - Apple Mac II\r\n");
        if (edid->established_timings.timing_640x480_72)
            xil_printf("   640 x  480p @ 72Hz - VESA\r\n");
        if (edid->established_timings.timing_640x480_75)
            xil_printf("   640 x  480p @ 75Hz - VESA\r\n");
        if (edid->established_timings.timing_800x600_56)
            xil_printf("   800 x  600p @ 56Hz - VESA\r\n");
        if (edid->established_timings.timing_800x600_60)
            xil_printf("   800 x  600p @ 60Hz - VESA\r\n");

        if (edid->established_timings.timing_800x600_72)
            xil_printf("   800 x  600p @ 72Hz - VESA\r\n");
        if (edid->established_timings.timing_800x600_75)
            xil_printf("   800 x  600p @ 75Hz - VESA\r\n");
        if (edid->established_timings.timing_832x624_75)
            xil_printf("   832 x  624p @ 75Hz - Apple Mac II\r\n");
        if (edid->established_timings.timing_1024x768_87)
            xil_printf("  1024 x  768i @ 87Hz - VESA\r\n");
        if (edid->established_timings.timing_1024x768_60)
            xil_printf("  1024 x  768p @ 60Hz - VESA\r\n");
        if (edid->established_timings.timing_1024x768_70)
            xil_printf("  1024 x  768p @ 70Hz - VESA\r\n");
        if (edid->established_timings.timing_1024x768_75)
            xil_printf("  1024 x  768p @ 75Hz - VESA\r\n");
        if (edid->established_timings.timing_1280x1024_75)
            xil_printf("  1280 x 1024p @ 75Hz - VESA\r\n");
#endif
    }
#endif

#if XVIDC_EDID_VERBOSITY > 1
    if (VerboseEn) {
    	xil_printf("Standard Timings supported\r\n");
		for (i = 0; i < ARRAY_SIZE(edid->standard_timing_id); i++) {
			const struct xvidc_edid_standard_timing_descriptor * const desc =
				&edid->standard_timing_id[i];

			if (!memcmp(desc, XVIDC_EDID_STANDARD_TIMING_DESCRIPTOR_INVALID,
																sizeof(*desc)))
			{
				continue;
			} else {
				if (((desc->horizontal_active_pixels + 31)* 8) >= 1000) {
				xil_printf("  %u x",(desc->horizontal_active_pixels + 31)* 8);
				} else {
				xil_printf("   %u x",(desc->horizontal_active_pixels + 31)* 8);
				}
				switch (desc->image_aspect_ratio) {
					case 0: //Aspect Ratio = 16:10
						xil_printf(" %up ",
					(((desc->horizontal_active_pixels + 31)* 8) * 10) / 16);
						break;
					case 1: //Aspect Ratio = 4:3
						xil_printf(" %up ",
					(((desc->horizontal_active_pixels + 31)* 8) * 3) / 4);
						break;
					case 2: //Aspect Ratio = 5:4
						xil_printf(" %up ",
					(((desc->horizontal_active_pixels + 31)* 8) * 4) / 5);
						break;
					case 3: //Aspect Ratio = 16:9
						xil_printf(" %up ",
					(((desc->horizontal_active_pixels + 31)* 8) * 9) / 16);
						break;
					default: //Aspect Ratio = 16:10
						xil_printf(" %up ",
					(((desc->horizontal_active_pixels + 31)* 8) * 10) / 16);
						break;
				}
				xil_printf("@ %uHz\r\n",(desc->refresh_rate + 60));
			}
		}
    }
#endif
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("\r\n");
    }
#endif
}



#if XVIDC_EDID_VERBOSITY > 1
/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 Audio Data
*
* @param    adb is a pointer to the CEA 861 audio data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   API Define below here are CEA861 routines
*
******************************************************************************/
static void
xvidc_disp_cea861_audio_data(
                  const struct xvidc_cea861_audio_data_block * const adb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {
    /* For Future Usage */
    EdidCtrlParam = EdidCtrlParam;

    const u8 descriptors = adb->header.length / sizeof(*adb->sad);
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("CE audio data (formats supported)\r\n");
    }
#endif
    for (u8 i = 0; i < descriptors; i++) {
        const struct xvidc_cea861_short_audio_descriptor * const sad =
            (struct xvidc_cea861_short_audio_descriptor *) &adb->sad[i];

        switch (sad->audio_format) {
            case XVIDC_CEA861_AUDIO_FORMAT_LPCM:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  LPCM        %u-channel, %s%s%s\b%s",
                           sad->channels + 1,
                           sad->flags.lpcm.bitrate_16_bit ? "16/" : "",
                           sad->flags.lpcm.bitrate_20_bit ? "20/" : "",
                           sad->flags.lpcm.bitrate_24_bit ? "24/" : "",

                           ((sad->flags.lpcm.bitrate_16_bit +
                             sad->flags.lpcm.bitrate_20_bit +
                             sad->flags.lpcm.bitrate_24_bit) > 1) ?
                                                      " bit depths" : "-bit");
                }
#endif
                break;
            case XVIDC_CEA861_AUDIO_FORMAT_AC_3:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  AC-3        %u-channel, %4ukHz max. bit rate",
                           sad->channels + 1,
                           (sad->flags.maximum_bit_rate << 3));
                }
#endif
                break;
            case XVIDC_CEA861_AUDIO_FORMAT_DTS:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  DTS         %u-channel, %4ukHz max. bit rate",
                           sad->channels + 1,
                           (sad->flags.maximum_bit_rate << 3));
                }
#endif
                break;

            case XVIDC_CEA861_AUDIO_FORMAT_E_AC_3:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  DD+(E-AC3)  %u-channel,"
                           " %u-Format Dependent Value",
                           sad->channels + 1,
                           (sad->flags.format_dependent));
                }
#endif
                break;

            case XVIDC_CEA861_AUDIO_FORMAT_DTS_HD:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  DTS-HD      %u-channel,"
                           " %u-Format Dependent Value",
                           sad->channels + 1,
                           (sad->flags.format_dependent));
                }
#endif
                break;

            case XVIDC_CEA861_AUDIO_FORMAT_MLP:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  MAL (MLP)   %u-channel,"
                           " %u-Format Dependent Value",
                           sad->channels + 1,
                           (sad->flags.format_dependent));
                }
#endif
                break;
            default:
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("Unknown audio format 0x%02x\r\n",
                            sad->audio_format);
                }
#endif
                continue;
        }
#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf(" at %s%s%s%s%s%s%s\b kHz\r\n",
                   sad->sample_rate_32_kHz ? "32/" : "",
                   sad->sample_rate_44_1_kHz ? "44.1/" : "",
                   sad->sample_rate_48_kHz ? "48/" : "",
                   sad->sample_rate_88_2_kHz ? "88.2/" : "",
                   sad->sample_rate_96_kHz ? "96/" : "",
                   sad->sample_rate_176_4_kHz ? "176.4/" : "",
                   sad->sample_rate_192_kHz ? "192/" : "");
        }
#endif
    }
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("\r\n");
    }
#endif
}
#endif

/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 Extended Data
*
* @param    edb is a pointer to the CEA 861 extended data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861_extended_data(
                  const struct xvidc_cea861_extended_data_block * const edb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("CEA Extended Tags\r\n");
    }
#endif
    switch(edb->xvidc_cea861_extended_tag_codes) {
#if XVIDC_EDID_VERBOSITY > 1
        case XVIDC_CEA861_EXT_TAG_TYPE_VIDEO_CAPABILITY:
            if (VerboseEn) {
                xil_printf("  Video capability data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_VENDOR_SPECIFIC:
            if (VerboseEn) {
                xil_printf("  Vendor-specific video data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_VESA_DISPLAY_DEVICE:
            if (VerboseEn) {
                xil_printf("  VESA video display device data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_VESA_VIDEO_TIMING_BLOCK_EXT:
            if (VerboseEn) {
                xil_printf("VESA video timing block extension\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_RESERVED_FOR_HDMI_VIDEO_DATA_BLOCK:
            if (VerboseEn) {
                xil_printf("Reserved for HDMI video data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_COLORIMETRY:
            if (VerboseEn) {
                xil_printf("  Colorimetry data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_HDR_STATIC_METADATA:
            if (VerboseEn) {
                xil_printf("HDR static metadata data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_HDR_DYNAMIC_METADATA:
            if (VerboseEn) {
                xil_printf("  HDR dynamic metadata data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_VIDEO_FRMT_PREFERENCE:
            if (VerboseEn) {
                xil_printf("  Video format preference data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_CEA_MISC_AUDIO_FIELDS:
            if (VerboseEn) {
                xil_printf("Reserved for CEA miscellaneous audio fields\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_VENDOR_SPECIFC_AUDIO:
            if (VerboseEn) {
                xil_printf("  Vendor-specific audio data block\r\n\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_HDMI_AUDIO:
            if (VerboseEn) {
                xil_printf("  HDMI audio data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_ROOM_CONFIGURATION:
            if (VerboseEn) {
                xil_printf("  Room configuration data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_SPEAKER_LOCATION:
            if (VerboseEn) {
                xil_printf("  Speaker location data block\r\n");
            }
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_INFOFRAME:
            if (VerboseEn) {
                xil_printf("  Video capability data block\r\n");
            }
        break;
#endif
	case XVIDC_CEA861_EXT_TAG_TYPE_HDMI_FORUM_EDID_EXT_OVERRIDE_DATA:
		if (VerboseEn) {
			xil_printf("	HDMI Forum EDID extension override Data Block\r\n");
		}
		xvidc_disp_cea861_hf_edid_ext_override_data(edb, EdidCtrlParam, VerboseEn);
		break;
        case XVIDC_CEA861_EXT_TAG_TYPE_HDMI_FORUM_SINK_CAPABILITY:
            if (VerboseEn) {
                xil_printf("  HDMI Forum Sink Capability Data Block\r\n");
            }
            xvidc_disp_cea861_hf_sink_capability_data(edb, EdidCtrlParam, VerboseEn);
            break;
        case XVIDC_CEA861_EXT_TAG_TYPE_YCBCR420_VIDEO:
#if XVIDC_EDID_VERBOSITY > 1
            if (VerboseEn) {
                xil_printf("  YCbCr 4:2:0 video data block\r\n");
                xil_printf("    YCbCr 4:2:0.............. Supported\r\n");
            }
#endif
            EdidCtrlParam->IsYCbCr420Supp = XVIDC_SUPPORTED;

#if XVIDC_EDID_VERBOSITY > 1
            if (VerboseEn) {
                xil_printf("  CE video identifiers (VICs) - "
                                              " timing/formats supported\r\n");
            }
            /* For VIC 1 to VIC 64, where, the first 7 bit is the VIC number,
             * and the most significant bit is to define the Native Video Format
             * or not.
             * svd/short video descriptor = [native, 7 bits of VIC]
             *       --> Covers for SVD [1-64] and SVD [129-192]
             * VIC 65 to VIC 127, all the 8 bits are the VIC Number
             * VIC 193 to VIC 253, all the 8 bits are the VIC Number
             * svd/short video descriptor = [8 bits of VIC]
             *       --> SVD= 0, 128, 254 and 255 are reserved.
             */
            for (u8 i = 0; i < edb->header.length - 1; i++) {
               u8 vic;
               u8 native=0;
               if ((edb->data[i] & 0x7F) == 0) {
                   continue;
               } else if  (((edb->data[i]) >= 1) && ((edb->data[i]) <= 64)){
                   vic    = (edb->data[i]) & 0x7F;
               } else if  (((edb->data[i]) >= 65) && ((edb->data[i]) <= 127)){
                   vic    = (edb->data[i]);
               } else if  (((edb->data[i]) >= 129) && ((edb->data[i]) <= 192)){
                   vic    = (edb->data[i]) & 0x7F;
                   native = 1;
               } else if  (((edb->data[i]) >= 193) && ((edb->data[i]) <= 253)){
                   vic    = (edb->data[i]);
               } else {
                   continue;
               }

               const struct xvidc_cea861_timing * const timing =
                   &xvidc_cea861_timings[vic];

               if (VerboseEn) {
                   xil_printf("   %s CEA Mode %02u: %4u x %4u%c @ %dHz\r\n",
                   native ? "*" : " ",
                   vic,
                   timing->hactive, timing->vactive,
                   (timing->mode == INTERLACED) ? 'i' : 'p',
                   (u32)(timing->vfreq));
               }
            }
            if (VerboseEn) {
                xil_printf("\r\n");
            }
#endif
        break;

        case XVIDC_CEA861_EXT_TAG_TYPE_YCBCR420_CAPABILITY_MAP:
#if XVIDC_EDID_VERBOSITY > 1
            if (VerboseEn) {
                xil_printf("  YCbCr 4:2:0 capability map data block\r\n");
                xil_printf("    YCbCr 4:2:0.............. Supported\r\n");
            }
#endif
            EdidCtrlParam->IsYCbCr420Supp = XVIDC_SUPPORTED;
#if XVIDC_EDID_VERBOSITY > 1
            for (u8 i = 0; i < edb->header.length - 1; i++) {
                u8 v = edb->data[i];

                for (u8 j = 0; j < 8; j++) {
                    if (v & (1 << j)) {
                        if (VerboseEn) {
                            const struct xvidc_cea861_timing * const timing =
                 &xvidc_cea861_timings[EdidCtrlParam->SuppCeaVIC[(i * 8) + j]];
                            xil_printf("      CEA Mode %02u: %4u x %4u%c"
                                              "@ %dHz\r\n",
                                      EdidCtrlParam->SuppCeaVIC[(i * 8) + j],
                                      timing->hactive, timing->vactive,
                                      (timing->mode == INTERLACED) ? 'i' : 'p',
                                      (u32)(timing->vfreq));
                        }
                    }
                }
            }
#endif
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("\r\n");
            }
#endif
        break;

        default :
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
                xil_printf("  Not Supported: Ext Tag: %03x\r\n",
                                         edb->xvidc_cea861_extended_tag_codes);
#endif
                xil_printf("\r\n");
            }
#endif
        break;
    }
}
#if XVIDC_EDID_VERBOSITY > 1
static void
xvidc_disp_cea861_video_data(
                  const struct xvidc_cea861_video_data_block * const vdb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {
    /* For Future Usage */
    EdidCtrlParam = EdidCtrlParam;

    /* Variable */
    u8 Vic;
    u8 Native;

    if (VerboseEn) {
       xil_printf("CE video identifiers (VICs) - timing/formats"
                                                             " supported\r\n");
    }

    for (u8 i = 0; i < vdb->header.length; i++) {
        /* For VIC 1 to VIC 64, where, the first 7 bit is the VIC number,
         * and the most significant bit is to define the Native Video Format
         * or not.
         * svd/short video descriptor = [native, 7 bits of VIC]
         *       --> Covers for SVD [1-64] and SVD [129-192]
         * VIC 65 to VIC 127, all the 8 bits are the VIC Number
         * VIC 193 to VIC 253, all the 8 bits are the VIC Number
         * svd/short video descriptor = [8 bits of VIC]
         *       --> SVD= 0, 128, 254 and 255 are reserved.
         */
        if (vdb->svd[i].video_identification_code >= 1 &&
                        vdb->svd[i].video_identification_code <= 127) {
                Native = FALSE;
                Vic    = vdb->svd[i].video_identification_code;
        } else if (vdb->svd[i].video_identification_code >= 129 &&
                        vdb->svd[i].video_identification_code <= 192) {
                Native = TRUE;
                Vic    = vdb->svd[i].video_identification_code & 0x7F;
        } else if (vdb->svd[i].video_identification_code >= 193 &&
                        vdb->svd[i].video_identification_code <= 253) {
                Native = FALSE;
                Vic    = vdb->svd[i].video_identification_code;
        } else {
                /* Reserved or Not Valid */
                Native = FALSE;
                Vic    = vdb->svd[i].video_identification_code;
        }

        const struct xvidc_cea861_timing * const timing =
            &xvidc_cea861_timings[Vic];

        EdidCtrlParam->SuppCeaVIC[i] = Vic;
        if (VerboseEn) {
            xil_printf(" %s CEA Mode %02u: %4u x %4u%c @ %dHz\r\n",
                   Native ? "*" : " ",
                   Vic,
                   timing->hactive, timing->vactive,
                   (timing->mode == INTERLACED) ? 'i' : 'p',
                   (u32)(timing->vfreq));
        }
    }
    if (VerboseEn) {
        xil_printf("\r\n");
    }
}
#endif

/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 HDMI Forum Sink Capability Data Block
*
* @param    edb is a pointer to the CEA 861 extended data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861_hf_sink_capability_data(
					const struct xvidc_cea861_extended_data_block * const edb,
					XV_VidC_EdidCntrlParam *EdidCtrlParam,
					XV_VidC_Verbose VerboseEn) {

	const struct xvidc_cea861_hdmi_hf_sink_capability_data_block * const hdmi =
			(struct xvidc_cea861_hdmi_hf_sink_capability_data_block *) edb;

#if XVIDC_EDID_VERBOSITY == 0
    xil_printf("VERBOSITY is disabled : %d\r\n",VerboseEn);
#endif

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("HF - Sink Capability Data block (SCDB)\r\n");
        xil_printf("Cea Extended Tag: %d\r\n",edb->Extended_tag_code);
    }
#endif

#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("  Version.................. %d\r\n",hdmi->version);
            }
#endif
            if (hdmi->max_tmds_char_rate) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  Maximum TMDS clock....... %uMHz\r\n",
                           hdmi->max_tmds_char_rate * 5);
                }
#endif
                    EdidCtrlParam->MaxTmdsMhz = (hdmi->max_tmds_char_rate * 5);
                } else {
#if XVIDC_EDID_VERBOSITY > 0
                    if (VerboseEn) {
                        xil_printf("  Max. Supp. TMDS clock (<=340MHz)\r\n");
                    }
#endif
                }

            EdidCtrlParam->Is3dOsdDisparitySupp = hdmi->osd_disparity_3d;
            EdidCtrlParam->IsDualViewSupp = hdmi->dual_view_3d;
            EdidCtrlParam->IsIndependentViewSupp = hdmi->independent_view_3d;
            EdidCtrlParam->IsLte340McscScamble = hdmi->lte_340mcsc_scramble;
            if(!hdmi->scdc_present) {
		EdidCtrlParam->IsLte340McscScamble = 0;
            }

            EdidCtrlParam->IsCCBPCISupp = hdmi->ccbpci;
            EdidCtrlParam->IsCableAssemblyStatusSupp = hdmi->cable_status;

            EdidCtrlParam->IsSCDCReadRequestReady = hdmi->rr_capable;
            EdidCtrlParam->IsSCDCPresent = hdmi->scdc_present;
            EdidCtrlParam->IsYCbCr420dc30bppSupp = hdmi->dc_30bit_yuv420;
            EdidCtrlParam->IsYCbCr420dc36bppSupp = hdmi->dc_36bit_yuv420;
            EdidCtrlParam->IsYCbCr420dc48bppSupp = hdmi->dc_48bit_yuv420;

            switch (hdmi->max_frl_rate) {
            case XVIDC_MAXFRLRATE_NOT_SUPPORTED:
                EdidCtrlParam->MaxFrlLanesSupp = 0;
                EdidCtrlParam->MaxFrlLineRateSupp = 0;
                break;

            case XVIDC_MAXFRLRATE_3X3GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 3;
                EdidCtrlParam->MaxFrlLineRateSupp = 3;
                break;

            case XVIDC_MAXFRLRATE_3X6GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 3;
                EdidCtrlParam->MaxFrlLineRateSupp = 6;
                break;

            case XVIDC_MAXFRLRATE_4X6GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 6;
                break;

            case XVIDC_MAXFRLRATE_4X8GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 8;
                break;

            case XVIDC_MAXFRLRATE_4X10GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 10;
                break;

            case XVIDC_MAXFRLRATE_4X12GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 12;
                break;

            default:
                EdidCtrlParam->MaxFrlLanesSupp = 0;
                EdidCtrlParam->MaxFrlLineRateSupp = 0;
                break;
            }

            EdidCtrlParam->MaxFrlRateSupp = hdmi->max_frl_rate;

            if (hdmi->header.length >= HDMI_VSDB_EXTENSION_FLAGS_OFFSET) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
                    xil_printf("  RRC Capable Support...... %s\r\n",
                            hdmi->rr_capable ? "Yes" : "No");
                    xil_printf("  SCDC Present............. %s\r\n",
                            hdmi->scdc_present ? "Yes" : "No");
                    xil_printf("  HDMI1.4 Scramble Support. %s\r\n",
                            hdmi->lte_340mcsc_scramble ? "Yes" : "No");
#endif
                    xil_printf("  YUV 420 Deep.C. Support..\r\n");
                    xil_printf("    Supports 48bpp......... %s\r\n",
                            hdmi->dc_48bit_yuv420 ? "Yes" : "No");
                    xil_printf("    Supports 36bpp......... %s\r\n",
                            hdmi->dc_36bit_yuv420 ? "Yes" : "No");
                    xil_printf("    Supports 30bpp......... %s\r\n",
                            hdmi->dc_30bit_yuv420 ? "Yes" : "No");

                    xil_printf("    Supports 3D_OSD_Disparity......... %s\r\n",
                            hdmi->osd_disparity_3d ? "Yes" : "No");
                    xil_printf("    Supports Dual_Video......... %s\r\n",
                            hdmi->dual_view_3d ? "Yes" : "No");
                    xil_printf("    Supports Independent_view......... %s\r\n",
                            hdmi->independent_view_3d ? "Yes" : "No");
                    xil_printf("    Supports Independent_view......... %s\r\n",
                            hdmi->independent_view_3d ? "Yes" : "No");
                    xil_printf("    Supports CCBPCI......... %s\r\n",
                            hdmi->ccbpci ? "Yes" : "No");
                    xil_printf("    Supports cable_status......... %s\r\n",
                            hdmi->cable_status ? "Yes" : "No");

                    xil_printf("    Supports Auto Low-Latency Mode......... %s\r\n",
                            hdmi->allm ? "Yes" : "No");
                    xil_printf("    Supports Fast VActive........ %s\r\n",
                            hdmi->fva ? "Yes" : "No");
                    xil_printf("    Supports QMS-VRR........ %s\r\n",
                            hdmi->qms ? "Yes" : "No");

                    xil_printf("  Compressed Video Transport Support..\r\n");
                    xil_printf("    Supports DSC 10bpc........ %s\r\n",
                            hdmi->dsc_10bpc ? "Yes" : "No");
                    xil_printf("    Supports DSC 12bpc........ %s\r\n",
                            hdmi->dsc_12bpc ? "Yes" : "No");


                    xil_printf("    Max FRL Rate Support... %u\r\n",
                            EdidCtrlParam->MaxFrlRateSupp);
                    xil_printf("    FRL Lanes Support...... %u\r\n",
                            EdidCtrlParam->MaxFrlLanesSupp);
                    xil_printf("    Max FRL Line Rate Support. %u\r\n",
                            EdidCtrlParam->MaxFrlLineRateSupp);
                }
#endif
                EdidCtrlParam->IsFapaStartLocationSupp = hdmi->fapa_start_location;
                EdidCtrlParam->IsAllmSupp = hdmi->allm;
                EdidCtrlParam->IsfavSupp = hdmi->fva;
                EdidCtrlParam->IsQmsVrrSupp = hdmi->qms;

                EdidCtrlParam->IsDsc10bpcSupp = hdmi->dsc_10bpc;
                EdidCtrlParam->IsDsc12bpcSupp = hdmi->dsc_12bpc;
                EdidCtrlParam->IsDsc16bpcSupp = hdmi->dsc_16bpc;
                EdidCtrlParam->IsFapaEndExtended = hdmi->fapa_end_extended;
                EdidCtrlParam->IsDscNativeYCbCr420Supp = hdmi->dsc_native_420;
                EdidCtrlParam->IsVesaDsc12aSupp = hdmi->dsc_1p2;
                EdidCtrlParam->DscTotalChunkBytes = (1024 * (hdmi->dsc_total_chunk_bytes +1));
            }
            #if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
		xil_printf("\r\n");
            }
            #endif
}

/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 HDMI Forum Extension Override Data Block(HF-EEODB)
*
* @param    edb is a pointer to the CEA 861 extended data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861_hf_edid_ext_override_data(
					const struct xvidc_cea861_extended_data_block * const edb,
					XV_VidC_EdidCntrlParam *EdidCtrlParam,
					XV_VidC_Verbose VerboseEn) {

	const struct xvidc_cea861_hdmi_hf_edid_ext_override_data_block * const hdmi =
			(struct xvidc_cea861_hdmi_hf_edid_ext_override_data_block *)edb;

#if XVIDC_EDID_VERBOSITY == 0
    xil_printf("VERBOSITY is disabled : %d\r\n",VerboseEn);
#endif

#if XVIDC_EDID_VERBOSITY > 0
	if (VerboseEn) {
		xil_printf("HF - EDID Extension Override Data block (EEODB)\r\n");
		xil_printf("Cea Extended Tag: %d\r\n", edb->xvidc_cea861_extended_tag_codes);
		xil_printf("Extension block count %d\r\n", hdmi->Edid_extention_block_count);
	}
#endif
	EdidCtrlParam->Extensions = hdmi->Edid_extention_block_count;
}

/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 Vendor Specific Data
*
* @param    vsdb is a pointer to the vendor specific data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861_vendor_data(
                  const struct xvidc_cea861_vendor_specific_data_block * vsdb,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {

    const u8 oui[] = { vsdb->ieee_registration[2],
                            vsdb->ieee_registration[1],
                            vsdb->ieee_registration[0] };

#if XVIDC_EDID_VERBOSITY == 0
    xil_printf("VERBOSITY is disabled : %d\r\n",VerboseEn);
#endif

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("CEA vendor specific data (VSDB)\r\n");
        xil_printf("  IEEE registration number. 0x");
        for (u8 i = 0; i < ARRAY_SIZE(oui); i++)
            xil_printf("%02X", oui[i]);
        xil_printf("\r\n");
    }
#endif
    if (!memcmp(oui, HDMI_OUI, sizeof(oui))) {
       /* HDMI Sink should have the HDMI Vendor Specific Block */
       EdidCtrlParam->IsHdmi = XVIDC_ISHDMI;
       const struct xvidc_cea861_hdmi_vendor_specific_data_block * const hdmi =
            (struct xvidc_cea861_hdmi_vendor_specific_data_block *) vsdb;
#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("  CEC physical address..... %u.%u.%u.%u\r\n",
                   hdmi->port_configuration_a,
                   hdmi->port_configuration_b,
                   hdmi->port_configuration_c,
                   hdmi->port_configuration_d);
        }
#endif

        if (hdmi->header.length >= HDMI_VSDB_EXTENSION_FLAGS_OFFSET) {
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
                xil_printf("  Supports AI (ACP, ISRC).. %s\r\n",
                       hdmi->audio_info_frame ? "Yes" : "No");
#endif
                xil_printf("  Supports 48bpp........... %s\r\n",
                       hdmi->colour_depth_48_bit ? "Yes" : "No");
                xil_printf("  Supports 36bpp........... %s\r\n",
                       hdmi->colour_depth_36_bit ? "Yes" : "No");
                xil_printf("  Supports 30bpp........... %s\r\n",
                       hdmi->colour_depth_30_bit ? "Yes" : "No");
                xil_printf("  Supp. YUV444 Deep Color.. %s\r\n",
                       hdmi->yuv_444_supported ? "Yes" : "No");
#if XVIDC_EDID_VERBOSITY > 1
                xil_printf("  Supports dual-link DVI... %s\r\n",
                       hdmi->dvi_dual_link ? "Yes" : "No");
#endif
            }
#endif
            EdidCtrlParam->Is30bppSupp = hdmi->colour_depth_30_bit;
            EdidCtrlParam->Is36bppSupp = hdmi->colour_depth_36_bit;
            EdidCtrlParam->Is48bppSupp = hdmi->colour_depth_48_bit;
            EdidCtrlParam->IsYCbCr444DeepColSupp = hdmi->yuv_444_supported;
        }

        if (hdmi->header.length >= HDMI_VSDB_MAX_TMDS_OFFSET) {
                if (hdmi->max_tmds_clock) {
#if XVIDC_EDID_VERBOSITY > 0
                    if (VerboseEn) {
                        xil_printf("  Maximum TMDS clock....... %uMHz\r\n",
                               hdmi->max_tmds_clock * 5);
                    }
#endif
                    EdidCtrlParam->MaxTmdsMhz = (hdmi->max_tmds_clock * 5);
                } else {
#if XVIDC_EDID_VERBOSITY > 0
                    if (VerboseEn) {
                        xil_printf("  Maximum TMDS clock....... n/a\r\n");
                    }
#endif
                }
        }

        if (hdmi->header.length >= HDMI_VSDB_LATENCY_FIELDS_OFFSET) {
            if (hdmi->latency_fields) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  Video latency %s........ %ums\r\n",
                           hdmi->interlaced_latency_fields ? "(p)" : "...",
                           (hdmi->video_latency - 1) << 1);
                    xil_printf("  Audio latency %s........ %ums\r\n",
                           hdmi->interlaced_latency_fields ? "(p)" : "...",
                           (hdmi->audio_latency - 1) << 1);
                }
#endif
            }

            if (hdmi->interlaced_latency_fields) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  Video latency (i)........ %ums\r\n",
                           hdmi->interlaced_video_latency);
                    xil_printf("  Audio latency (i)........ %ums\r\n",
                           hdmi->interlaced_audio_latency);
                }
#endif
            }
        }
    }   else if (!memcmp(oui, HDMI_OUI_HF, sizeof(oui))) {
    const struct xvidc_cea861_hdmi_hf_vendor_specific_data_block * const hdmi =
            (struct xvidc_cea861_hdmi_hf_vendor_specific_data_block *) vsdb;

#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("  Version.................. %d\r\n",hdmi->version);
            }
#endif

            if (hdmi->max_tmds_char_rate) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
                    xil_printf("  Maximum TMDS clock....... %uMHz\r\n",
                           hdmi->max_tmds_char_rate * 5);
                }
#endif
                    EdidCtrlParam->MaxTmdsMhz = (hdmi->max_tmds_char_rate * 5);
                } else {
#if XVIDC_EDID_VERBOSITY > 0
                    if (VerboseEn) {
                        xil_printf("  Max. Supp. TMDS clock (<=340MHz)\r\n");
                    }
#endif
                }

            switch (hdmi->max_frl_rate) {
            case XVIDC_MAXFRLRATE_NOT_SUPPORTED:
                EdidCtrlParam->MaxFrlLanesSupp = 0;
                EdidCtrlParam->MaxFrlLineRateSupp = 0;
                break;

            case XVIDC_MAXFRLRATE_3X3GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 3;
                EdidCtrlParam->MaxFrlLineRateSupp = 3;
                break;

            case XVIDC_MAXFRLRATE_3X6GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 3;
                EdidCtrlParam->MaxFrlLineRateSupp = 6;
                break;

            case XVIDC_MAXFRLRATE_4X6GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 6;
                break;

            case XVIDC_MAXFRLRATE_4X8GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 8;
                break;

            case XVIDC_MAXFRLRATE_4X10GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 10;
                break;

            case XVIDC_MAXFRLRATE_4X12GBITSPS:
                EdidCtrlParam->MaxFrlLanesSupp = 4;
                EdidCtrlParam->MaxFrlLineRateSupp = 12;
                break;

            default:
                EdidCtrlParam->MaxFrlLanesSupp = 0;
                EdidCtrlParam->MaxFrlLineRateSupp = 0;
                break;
            }

            EdidCtrlParam->MaxFrlRateSupp = hdmi->max_frl_rate;

            if (hdmi->header.length >= HDMI_VSDB_EXTENSION_FLAGS_OFFSET) {
#if XVIDC_EDID_VERBOSITY > 0
                if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
                    xil_printf("  RRC Capable Support...... %s\r\n",
                            hdmi->rr_capable ? "Yes" : "No");
                    xil_printf("  SCDC Present............. %s\r\n",
                            hdmi->scdc_present ? "Yes" : "No");
                    xil_printf("  HDMI1.4 Scramble Support. %s\r\n",
                            hdmi->lte_340mcsc_scramble ? "Yes" : "No");
#endif
                    xil_printf("  YUV 420 Deep.C. Support..\r\n");
                    xil_printf("    Supports 48bpp......... %s\r\n",
                            hdmi->dc_48bit_yuv420 ? "Yes" : "No");
                    xil_printf("    Supports 36bpp......... %s\r\n",
                            hdmi->dc_36bit_yuv420 ? "Yes" : "No");
                    xil_printf("    Supports 30bpp......... %s\r\n",
                            hdmi->dc_30bit_yuv420 ? "Yes" : "No");
                    xil_printf("    Max FRL Rate Support... %u\r\n",
                            EdidCtrlParam->MaxFrlRateSupp);
                    xil_printf("    FRL Lanes Support...... %u\r\n",
                            EdidCtrlParam->MaxFrlLanesSupp);
                    xil_printf("    Max FRL Line Rate Support. %u\r\n",
                            EdidCtrlParam->MaxFrlLineRateSupp);
                }
#endif
                EdidCtrlParam->IsYCbCr420dc30bppSupp = hdmi->dc_30bit_yuv420;
                EdidCtrlParam->IsYCbCr420dc36bppSupp = hdmi->dc_36bit_yuv420;
                EdidCtrlParam->IsYCbCr420dc48bppSupp = hdmi->dc_48bit_yuv420;
                EdidCtrlParam->IsSCDCReadRequestReady = hdmi->rr_capable;
                EdidCtrlParam->IsSCDCPresent = hdmi->scdc_present;
            }
    }
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("\r\n");
    }
#endif
}

#if XVIDC_EDID_VERBOSITY > 1
/*****************************************************************************/
/**
*
* This function parse EDID on CEA 861 Speaker Allocation
*
* @param    sadb is a pointer to the CEA 861 speaker allocation data block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861_speaker_allocation_data(
          const struct xvidc_cea861_speaker_allocation_data_block * const sadb,
             XV_VidC_EdidCntrlParam *EdidCtrlParam,
             XV_VidC_Verbose VerboseEn) {
    /* For Future Usage */
    EdidCtrlParam = EdidCtrlParam;

    const struct xvidc_cea861_speaker_allocation * const sa = &sadb->payload;
    const u8 * const channel_configuration = (u8 *) sa;

    if (VerboseEn) {
        xil_printf("CEA speaker allocation data\r\n");
        xil_printf("  Channel configuration.... %u.%u\r\n",
               (__builtin_popcountll(channel_configuration[0] & 0xe9) << 1) +
               (__builtin_popcountll(channel_configuration[0] & 0x14) << 0) +
               (__builtin_popcountll(channel_configuration[1] & 0x01) << 1) +
               (__builtin_popcountll(channel_configuration[1] & 0x06) << 0),
               (channel_configuration[0] & 0x02));
        xil_printf("  Front left/right......... %s\r\n",
               sa->front_left_right ? "Yes" : "No");
        xil_printf("  Front LFE................ %s\r\n",
               sa->front_lfe ? "Yes" : "No");
        xil_printf("  Front center............. %s\r\n",
               sa->front_center ? "Yes" : "No");
        xil_printf("  Rear left/right.......... %s\r\n",
               sa->rear_left_right ? "Yes" : "No");
        xil_printf("  Rear center.............. %s\r\n",
               sa->rear_center ? "Yes" : "No");
        xil_printf("  Front left/right center.. %s\r\n",
               sa->front_left_right_center ? "Yes" : "No");
        xil_printf("  Rear left/right center... %s\r\n",
               sa->rear_left_right_center ? "Yes" : "No");
        xil_printf("  Front left/right wide.... %s\r\n",
               sa->front_left_right_wide ? "Yes" : "No");
        xil_printf("  Front left/right high.... %s\r\n",
               sa->front_left_right_high ? "Yes" : "No");
        xil_printf("  Top center............... %s\r\n",
               sa->top_center ? "Yes" : "No");
        xil_printf("  Front center high........ %s\r\n",
               sa->front_center_high ? "Yes" : "No");

        xil_printf("\r\n");
    }

}
#endif

/*****************************************************************************/
/**
*
* This function Parse and Display the CEA-861
*
* @param    ext is a pointer to the EDID extension block.
* @param    EdidCtrlParam is a pointer the EDID Control parameter
* @param    VerboseEn is a pointer to the XV_HdmiTxSs core instance.
*
* @note   None.
*
******************************************************************************/
static void
xvidc_disp_cea861(const struct xvidc_edid_extension * const ext,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {

    const struct xvidc_cea861_timing_block * const ctb =
        (struct xvidc_cea861_timing_block *) ext;
    const u8 offset = offsetof(struct xvidc_cea861_timing_block, data);
    u8 index = 0;

#if XVIDC_EDID_VERBOSITY > 1
    const struct xvidc_edid_detailed_timing_descriptor *dtd = NULL;
    u8 i;

    XV_VidC_TimingParam timing_params;
#endif

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {

        xil_printf("CEA-861 Information\r\n");
        xil_printf("  Revision number.......... %u\r\n",
               ctb->revision);
    }
#endif

    if (ctb->revision >= 2) {
#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
#if XVIDC_EDID_VERBOSITY > 1
            xil_printf("  IT underscan............. %supported\r\n",
                   ctb->underscan_supported ? "S" : "Not s");
#endif
            xil_printf("  Basic audio.............. %supported\r\n",
                   ctb->basic_audio_supported ? "S" : "Not s");
            xil_printf("  YCbCr 4:4:4.............. %supported\r\n",
                   ctb->yuv_444_supported ? "S" : "Not s");
            xil_printf("  YCbCr 4:2:2.............. %supported\r\n",
                   ctb->yuv_422_supported ? "S" : "Not s");
#if XVIDC_EDID_VERBOSITY > 1
            xil_printf("  Native formats........... %u\r\n",
                   ctb->native_dtds);
#endif
        }
#endif
        EdidCtrlParam->IsYCbCr444Supp = ctb->yuv_444_supported;
        EdidCtrlParam->IsYCbCr422Supp = ctb->yuv_422_supported;
    }

#if XVIDC_EDID_VERBOSITY > 1
    dtd = (struct xvidc_edid_detailed_timing_descriptor *)
                                            ((u8 *) ctb + ctb->dtd_offset);
    u32 dtd_offset_bytes = ctb->dtd_offset;
    u32 dtd_size = sizeof(struct xvidc_edid_detailed_timing_descriptor);

    if (dtd_offset_bytes < 128 && (dtd_offset_bytes + dtd_size) <= 128) {
        u32 max_dtds = (128 - dtd_offset_bytes) / dtd_size;

        for (i = 0; i < max_dtds && dtd && dtd->pixel_clock; i++, dtd++) {
            u32 current_offset = dtd_offset_bytes + (i * dtd_size);
            if (current_offset + dtd_size > 128) {
                break;
            }

        timing_params = XV_VidC_timing(dtd);
        if (VerboseEn) {
            xil_printf("  Detailed timing #%u....... %ux%u%c at %uHz "
                                                                "(%u:%u)\r\n",
                                        i + 1,
                                        timing_params.hres,
                                        timing_params.vres,
                                        timing_params.vidfrmt ? 'i' : 'p',
                                        timing_params.vfreq,
                                        timing_params.aspect_ratio.width,
                                        timing_params.aspect_ratio.height);

            xil_printf(
            "    Modeline............... \"%ux%u\" %u %u %u %u %u %u %u %u %u"
                                                        " %chsync %cvsync\r\n",
                                   timing_params.hres,
                                   timing_params.vres,
                                   HZ_2_MHZ (timing_params.pixclk),
                                   (timing_params.hres),
                                   (timing_params.hres + timing_params.hfp),
                                   (timing_params.hres + timing_params.hfp +
                                                    timing_params.hsync_width),
                                   (timing_params.htotal),
                                   (timing_params.vres),
                                   (timing_params.vres + timing_params.vfp),
                                   (timing_params.vres + timing_params.vfp +
                                                    timing_params.vsync_width),
                                   (timing_params.vtotal),
                                   timing_params.hsync_polarity ? '+' : '-',
                                   timing_params.vsync_polarity ? '+' : '-');
        }
    }
#endif
#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("\r\n");
        }
#endif

    if (ctb->revision >= 3) {
        do {
            const struct xvidc_cea861_data_block_header * const header =
                (struct xvidc_cea861_data_block_header *) &ctb->data[index];

            switch (header->tag) {

            case XVIDC_CEA861_DATA_BLOCK_TYPE_AUDIO:
                {
#if XVIDC_EDID_VERBOSITY > 1
                    const struct xvidc_cea861_audio_data_block * const db =
                        (struct xvidc_cea861_audio_data_block *) header;

                    xvidc_disp_cea861_audio_data(db,EdidCtrlParam,VerboseEn);
#endif
                }
                break;

            case XVIDC_CEA861_DATA_BLOCK_TYPE_VIDEO:
                {
#if XVIDC_EDID_VERBOSITY > 1
                    const struct xvidc_cea861_video_data_block * const db =
                        (struct xvidc_cea861_video_data_block *) header;

                    xvidc_disp_cea861_video_data(db,EdidCtrlParam,VerboseEn);
#endif
                }
                break;

            case XVIDC_CEA861_DATA_BLOCK_TYPE_VENDOR_SPECIFIC:
                {
                    const struct
                    xvidc_cea861_vendor_specific_data_block * const db =
                     (struct xvidc_cea861_vendor_specific_data_block *) header;

                    xvidc_disp_cea861_vendor_data(db,EdidCtrlParam,VerboseEn);
                }
                break;

            case XVIDC_CEA861_DATA_BLOCK_TYPE_SPEAKER_ALLOCATION:
                {
#if XVIDC_EDID_VERBOSITY > 1
                    const struct
                    xvidc_cea861_speaker_allocation_data_block * const db =
                  (struct xvidc_cea861_speaker_allocation_data_block *) header;

                    xvidc_disp_cea861_speaker_allocation_data(db,
                                                      EdidCtrlParam,VerboseEn);
#endif
                }
                break;

            case XVIDC_CEA861_DATA_BLOCK_TYPE_EXTENDED:
                {
                    const struct xvidc_cea861_extended_data_block * const db =
                        (struct xvidc_cea861_extended_data_block *) header;

                   xvidc_disp_cea861_extended_data(db,EdidCtrlParam,VerboseEn);
                }
                break;

            default:
#if XVIDC_EDID_VERBOSITY > 1
                if (VerboseEn) {
                    xil_printf("Unknown CEA-861 data block type 0x%02x\r\n",
                            header->tag);
                }
#endif
                break;
            }

            index = index + header->length + sizeof(*header);
        } while (index < ctb->dtd_offset - offset);
    }
#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("\r\n");
    }
#endif
}


/*****************************************************************************/
/**
*
* This structure parse parse EDID routines
*
* @note   API XV_VidC_parse_edid's checking needs to be updated to allow
*         extra parsers.
*
******************************************************************************/
static const struct xvidc_edid_extension_handler {
    /** Pointer to EDID extension information display function
     *  @param arg1 Pointer to EDID extension structure
     *  @param EdidCtrlParam Pointer to EDID control parameters
     *  @param VerboseEn Verbosity enable flag */
    void (* const inf_disp)(const struct xvidc_edid_extension * const,
           XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn);
} xvidc_edid_extension_handlers[] = {
	[XVIDC_EDID_EXTENSION_TIMING]         = { NULL },
	[XVIDC_EDID_EXTENSION_CEA]            = { xvidc_disp_cea861 },
	[XVIDC_EDID_EXTENSION_VTB]            = { NULL },
	[XVIDC_EDID_EXTENSION_XVIDC_EDID_2_0] = { NULL },
	[XVIDC_EDID_EXTENSION_DI]             = { NULL },
	[XVIDC_EDID_EXTENSION_LS]             = { NULL },
	[XVIDC_EDID_EXTENSION_MI]             = { NULL },
	[XVIDC_EDID_EXTENSION_DID]            = { xvidc_dispid_block },
	[XVIDC_EDID_EXTENSION_DTCDB_1]        = { NULL },
	[XVIDC_EDID_EXTENSION_DTCDB_2]        = { NULL },
	[XVIDC_EDID_EXTENSION_DTCDB_3]        = { NULL },
	[XVIDC_EDID_EXTENSION_BLOCK_MAP]      = { NULL },
	[XVIDC_EDID_EXTENSION_DDDB]           = { NULL },
};

void
XV_VidC_parse_edid(const u8 * const data,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn) {
    const struct edid * const edid = (struct edid *) data;
    const struct xvidc_edid_extension * const extensions =
        (struct xvidc_edid_extension *) (data + sizeof(*edid));

    XV_VidC_EdidCtrlParamInit(EdidCtrlParam);

    xvidc_disp_edid1(edid,EdidCtrlParam,VerboseEn);

    for (u8 i = 0; i < edid->extensions; i++) {
        const struct xvidc_edid_extension * const extension = &extensions[i];
        const struct xvidc_edid_extension_handler * const handler =
            &xvidc_edid_extension_handlers[extension->tag];

        if (!handler->inf_disp) {
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("WARNING: block %u contains unknown extension "
                                         " (%#04x)\r\n", i, extensions[i].tag);
            }
#endif
            continue;
        } else {
		(*handler->inf_disp)(extension,EdidCtrlParam,VerboseEn);
        }
    }
}

/*****************************************************************************/
/**
*
* This function handles a single EDID extension block by calling the
* appropriate handler based on the extension tag.
*
* @param    extension is a pointer to the EDID extension block.
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output for debugging.
*
* @note   None.
*
******************************************************************************/
void XV_VidC_handle_extension(const struct xvidc_edid_extension *extension,
		XV_VidC_EdidCntrlParam *EdidCtrlParam,
		XV_VidC_Verbose VerboseEn) {
	const struct xvidc_edid_extension_handler *handler =
			&xvidc_edid_extension_handlers[extension->tag];

	if (!handler->inf_disp) {
#if XVIDC_EDID_VERBOSITY > 0
		if (VerboseEn) {
			u32 block_num = 0;  /* Extension block counter */
			xil_printf("WARNING: block %u contains unknown extension (%#04x)\r\n",
					block_num, extension->tag);
		}
#endif
	} else {
		(*handler->inf_disp)(extension, EdidCtrlParam, VerboseEn);
	}
}

void
XV_VidC_parse_edid_extension(const u8 * const data,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn, u8 SegmentNum) {

    if (SegmentNum == 0) {
	const struct edid *edid = (const struct edid *)data;
	const struct xvidc_edid_extension *extension =
		(const struct xvidc_edid_extension *)(data + sizeof(*edid));

        /* Initialize EDID Control Parameters */
        XV_VidC_EdidCtrlParamInit(EdidCtrlParam);

        xvidc_disp_edid1(edid, EdidCtrlParam, VerboseEn);

        /* Handle the base segment */
        XV_VidC_handle_extension(extension, EdidCtrlParam, VerboseEn);
        return;
    }

     /* Handle segment 1 onwards */
    for (u8 i = 0; i < 2; i++) {
	const struct xvidc_edid_extension *extensions =
		(const struct xvidc_edid_extension *)data;
	const struct xvidc_edid_extension *extension =
		&extensions[i * XVIDC_EDID_BLOCK_SIZE];

	XV_VidC_handle_extension(extension, EdidCtrlParam, VerboseEn);
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Product Identification data block
*
* @param    x is a pointer to the product ID block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_product_id(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    EdidCtrlParam->DispIdProductCode = x[6] | (x[7] << 8);
    EdidCtrlParam->DispIdSerialNumber = x[8] | (x[9] << 8) | (x[10] << 16) | (x[11] << 24);
    EdidCtrlParam->DispIdModelWeek = x[12];
    EdidCtrlParam->DispIdModelYear = 2000 + x[13];

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Product Code: %u\r\n", EdidCtrlParam->DispIdProductCode);
        if (EdidCtrlParam->DispIdSerialNumber)
            xil_printf("    Serial Number: %u\r\n", EdidCtrlParam->DispIdSerialNumber);
        xil_printf("    %s: %u",
            EdidCtrlParam->DispIdModelWeek == 0xff ? "Model Year" : "Year of Manufacture",
            EdidCtrlParam->DispIdModelYear);
        if (EdidCtrlParam->DispIdModelWeek && EdidCtrlParam->DispIdModelWeek <= 0x36)
            xil_printf(", Week %u", EdidCtrlParam->DispIdModelWeek);
        xil_printf("\r\n");
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Display Parameters data block
*
* @param    x is a pointer to the parameters block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_parameters(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];


    if (length < XVIDC_DISPLAYID_PARAMETERS_BLOCK_LEN)
        return;

    EdidCtrlParam->DispIdImageWidthMm = x[3] | (x[4] << 8);
    EdidCtrlParam->DispIdImageHeightMm = x[5] | (x[6] << 8);
    EdidCtrlParam->DispIdNativeWidth = x[7] | (x[8] << 8);
    EdidCtrlParam->DispIdNativeHeight = x[9] | (x[10] << 8);
    EdidCtrlParam->DispIdFeatureSupportFlags = x[11];
    EdidCtrlParam->DispIdGamma = x[12];
    EdidCtrlParam->DispIdAspectRatio = x[13];

    if (length >= XVIDC_DISPLAYID_PARAMETERS_BPC_LEN) {
        u8 bpc_byte = x[14];
        EdidCtrlParam->DispIdDynamicBpcNative = bpc_byte & XVIDC_DISPLAYID_BPC_MASK;
        EdidCtrlParam->DispIdDynamicBpcOverall = (bpc_byte >> XVIDC_DISPLAYID_BPC_SHIFT) & XVIDC_DISPLAYID_BPC_MASK;
    } else {
        EdidCtrlParam->DispIdDynamicBpcNative = 0;
        EdidCtrlParam->DispIdDynamicBpcOverall = 0;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        float gamma_val = (EdidCtrlParam->DispIdGamma + 100) / 100.0f;
        float aspect_val = (EdidCtrlParam->DispIdAspectRatio + 100) / 100.0f;
        u16 width_mm = EdidCtrlParam->DispIdImageWidthMm;
        u16 height_mm = EdidCtrlParam->DispIdImageHeightMm;

        xil_printf("  Display Parameters Data Block\r\n");
        xil_printf("    Image size............... %u.%u mm x %u.%u mm\r\n",
                   width_mm/10, width_mm%10, height_mm/10, height_mm%10);
        xil_printf("    Native resolution........ %ux%u\r\n",
                   EdidCtrlParam->DispIdNativeWidth, EdidCtrlParam->DispIdNativeHeight);
        xil_printf("    Feature support flags.... 0x%02x\r\n", EdidCtrlParam->DispIdFeatureSupportFlags);
        xil_printf("    Gamma.................... %u.%02u\r\n", (u16)gamma_val,
                   (u16)((gamma_val - (u16)gamma_val) * 100));
        xil_printf("    Aspect ratio............. %u.%02u\r\n", (u16)aspect_val,
                   (u16)((aspect_val - (u16)aspect_val) * 100));
        xil_printf("    Dynamic bpc native....... %u\r\n", EdidCtrlParam->DispIdDynamicBpcNative);
        xil_printf("    Dynamic bpc overall...... %u\r\n", EdidCtrlParam->DispIdDynamicBpcOverall);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Color Characteristics data block
*
* @param    x is a pointer to the color block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_color_characteristics(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    unsigned cie_year = (x[1] & XVIDC_DISPLAYID_CIE_YEAR_MASK) ? XVIDC_DISPLAYID_CIE_YEAR_1976 : XVIDC_DISPLAYID_CIE_YEAR_1931;
    unsigned xfer_id = (x[1] >> XVIDC_DISPLAYID_XFER_ID_SHIFT) & XVIDC_DISPLAYID_XFER_ID_MASK;
    unsigned num_whitepoints = x[3] & XVIDC_DISPLAYID_NUM_WHITEPOINTS_MASK;
    unsigned num_primaries = (x[3] >> XVIDC_DISPLAYID_NUM_PRIMARIES_SHIFT) & XVIDC_DISPLAYID_NUM_PRIMARIES_MASK;
    bool temporal_color = x[3] & XVIDC_DISPLAYID_TEMPORAL_COLOR_MASK;
    unsigned offset = XVIDC_DISPLAYID_COLOR_CHAR_MIN_LEN;

    EdidCtrlParam->DispIdColorDepth = x[3] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK;
    EdidCtrlParam->DispIdColorEncoding = (x[3] >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK;

    if (!num_primaries && length > XVIDC_DISPLAYID_COLOR_CHAR_MIN_LEN) {
        offset++;
    }

    if (num_primaries >= 3 && length >= (offset + (3 * XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE))) {
        unsigned idx = offset;
        EdidCtrlParam->DispIdPrimaryRedX = x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8);
        EdidCtrlParam->DispIdPrimaryRedY = ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4);

        idx += XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE;
        EdidCtrlParam->DispIdPrimaryGreenX = x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8);
        EdidCtrlParam->DispIdPrimaryGreenY = ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4);

        idx += XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE;
        EdidCtrlParam->DispIdPrimaryBlueX = x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8);
        EdidCtrlParam->DispIdPrimaryBlueY = ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4);
    } else if (length >= XVIDC_DISPLAYID_COLOR_CHAR_LEGACY_LEN) {
        EdidCtrlParam->DispIdPrimaryRedX = x[4] | (x[5] << 8);
        EdidCtrlParam->DispIdPrimaryRedY = x[6] | (x[7] << 8);
        EdidCtrlParam->DispIdPrimaryGreenX = x[8] | (x[9] << 8);
        EdidCtrlParam->DispIdPrimaryGreenY = x[10] | (x[11] << 8);
        EdidCtrlParam->DispIdPrimaryBlueX = x[12] | (x[13] << 8);
        EdidCtrlParam->DispIdPrimaryBlueY = x[14] | (x[15] << 8);
    }

    offset += XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE * num_primaries;
    if (num_whitepoints >= 1 && length >= (offset + XVIDC_DISPLAYID_COLOR_WHITEPOINT_SIZE)) {
        unsigned idx = offset;
        EdidCtrlParam->DispIdWhitePointX = x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8);
        EdidCtrlParam->DispIdWhitePointY = ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4);
    } else if (length >= XVIDC_DISPLAYID_COLOR_CHAR_EXTENDED_LEN) {
        EdidCtrlParam->DispIdWhitePointX = x[16] | (x[17] << 8);
        EdidCtrlParam->DispIdWhitePointY = x[18] | (x[19] << 8);
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Color Characteristics Data Block\r\n");
        xil_printf("    Uses %s color\r\n", temporal_color ? "temporal" : "spatial");
        xil_printf("    Uses %u CIE (x, y) coordinates\r\n", cie_year);
        if (xfer_id)
            xil_printf("    Associated with Transfer Characteristics Data Block with Identifier %u\r\n", xfer_id);

        offset = XVIDC_DISPLAYID_COLOR_CHAR_MIN_LEN;
        if (!num_primaries) {
            static const char *std_colorspace_ids[] = {
                "sRGB", "BT.601", "BT.709", "Adobe RGB", "DCI-P3", "NTSC", "EBU", "Adobe Wide Gamut RGB", "DICOM"
            };
            xil_printf("    Uses color space %s\r\n",
                x[4] >= XVIDC_DISPLAYID_STD_COLORSPACE_MAX ? "Reserved" : std_colorspace_ids[x[4]]);
            offset++;
        }

        for (unsigned i = 0; i < num_primaries; i++) {
            unsigned idx = offset + XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE * i;
            xil_printf("    Primary #%u: (%.4f, %.4f)\r\n", i,
                (double)(x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8)) / XVIDC_DISPLAYID_COLOR_COORDINATE_DIVISOR,
                (double)(((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4)) / XVIDC_DISPLAYID_COLOR_COORDINATE_DIVISOR);
        }
        offset += XVIDC_DISPLAYID_COLOR_PRIMARY_SIZE * num_primaries;
        for (unsigned i = 0; i < num_whitepoints; i++) {
            unsigned idx = offset + XVIDC_DISPLAYID_COLOR_WHITEPOINT_SIZE * i;
            xil_printf("    White point #%u: (%.4f, %.4f)\r\n", i,
                (double)(x[idx] | ((x[idx + 1] & XVIDC_DISPLAYID_COLOR_LOWER_NIBBLE_MASK) << 8)) / XVIDC_DISPLAYID_COLOR_COORDINATE_DIVISOR,
                (double)(((x[idx + 1] & XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_MASK) >> XVIDC_DISPLAYID_COLOR_UPPER_NIBBLE_SHIFT) | (x[idx + 2] << 4)) / XVIDC_DISPLAYID_COLOR_COORDINATE_DIVISOR);
        }

        xil_printf("    Stored Color Depth: %u-bit\r\n", EdidCtrlParam->DispIdColorDepth + XVIDC_DISPLAYID_COLOR_DEPTH_OFFSET);
        xil_printf("    Stored Color Encoding: 0x%x\r\n", EdidCtrlParam->DispIdColorEncoding);
        if (EdidCtrlParam->DispIdPrimaryRedX || EdidCtrlParam->DispIdPrimaryRedY) {
            xil_printf("    Stored Red Primary: (0x%04x, 0x%04x)\r\n", EdidCtrlParam->DispIdPrimaryRedX, EdidCtrlParam->DispIdPrimaryRedY);
            xil_printf("    Stored Green Primary: (0x%04x, 0x%04x)\r\n", EdidCtrlParam->DispIdPrimaryGreenX, EdidCtrlParam->DispIdPrimaryGreenY);
            xil_printf("    Stored Blue Primary: (0x%04x, 0x%04x)\r\n", EdidCtrlParam->DispIdPrimaryBlueX, EdidCtrlParam->DispIdPrimaryBlueY);
        }
        if (EdidCtrlParam->DispIdWhitePointX || EdidCtrlParam->DispIdWhitePointY) {
            xil_printf("    Stored White Point: (0x%04x, 0x%04x)\r\n", EdidCtrlParam->DispIdWhitePointX, EdidCtrlParam->DispIdWhitePointY);
        }
    }
#else
    (void)VerboseEn;
    (void)cie_year;
    (void)xfer_id;
    (void)temporal_color;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Video Timing Mode 1 data block
*
* @param    x is a pointer to the timing mode 1 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_timing_mode_1(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN) : 0;
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i;

    EdidCtrlParam->DispIdNumTimings = (num_timings > XVIDC_DISPLAYID_MAX_TIMING_MODES) ? XVIDC_DISPLAYID_MAX_TIMING_MODES : num_timings;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("  Video Timing Modes Type 1 - Detailed Timings Data Block\r\n");
        xil_printf("    Number of timings........ %u\r\n\r\n", num_timings);
    }
#endif

    for (i = 0; i < num_timings && offset + (XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN - 1) < XVIDC_DISPLAYID_BLOCK_DATA_MAX && i < XVIDC_DISPLAYID_MAX_TIMING_MODES; i++) {
        u32 pixel_clock_10khz = x[offset] | (x[offset+1] << 8) | (x[offset+2] << 16);
        u16 h_active_encoded = x[offset+4] | (x[offset+5] << 8);
        u16 h_active = h_active_encoded + 1;
        u16 v_active_encoded = x[offset+12] | (x[offset+13] << 8);
        u16 v_active = v_active_encoded + 1;
        u16 h_blank = x[offset+6] | (x[offset+7] << 8);
        u16 v_blank = x[offset+10] | (x[offset+11] << 8);

        if (h_active < 1024 || h_active > 7680 || v_active < 576 || v_active > 4320) {
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("      WARNING: Parsed resolution out of valid range: %u x %u\r\n", h_active, v_active);
            }
#endif
        }

        if (pixel_clock_10khz == 0 || h_active == 0 || v_active == 0) {
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("      Invalid timing data\r\n");
            }
#endif
            offset += XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN;
            continue;
        }

        EdidCtrlParam->DispIdTiming[i].hres = h_active;
        EdidCtrlParam->DispIdTiming[i].vres = v_active;
        EdidCtrlParam->DispIdTiming[i].pixclk = pixel_clock_10khz;

        u32 h_total = (u32)h_active + h_blank;
        u32 v_total = (u32)v_active + v_blank;
        u32 pixel_clock_hz = pixel_clock_10khz * 10000UL;
        u32 refresh_rate = 0;

        if (h_total > 0 && v_total > 0) {
            u64 total_pixels = (u64)h_total * v_total;
            if (total_pixels > 0) {
                refresh_rate = (u32)(pixel_clock_hz / total_pixels);
            }
        }

        u32 pixel_clock_mhz = (pixel_clock_10khz + 50) / 100;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      Pixel Clock.............. %u MHz\r\n", pixel_clock_mhz);
            xil_printf("      Resolution............... %u x %u\r\n", h_active, v_active);
            if (refresh_rate > 0) {
                xil_printf("      Refresh Rate............. %u Hz\r\n", refresh_rate);
            }
            xil_printf("      Horizontal: Active=%u, Blank=%u, Total=%u\r\n",
                       h_active, h_blank, h_total);
            xil_printf("      Vertical: Active=%u, Blank=%u, Total=%u\r\n",
                       v_active, v_blank, v_total);
            xil_printf("\r\n");
        }
#else
        (void)pixel_clock_mhz;
        (void)refresh_rate;
        (void)VerboseEn;
#endif
        offset += XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Video Timing Mode 2 data block
*
* @param    x is a pointer to the timing mode 2 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_timing_mode_2(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN) : 0;
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i;
    u8 start_idx = EdidCtrlParam->DispIdNumTimings;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Video Timing Mode 2 Data Block\r\n");
        xil_printf("    Number of Timings: %u\r\n", num_timings);
    }
#endif

    for (i = 0; i < num_timings && offset + (XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN - 1) < XVIDC_DISPLAYID_BLOCK_DATA_MAX && (start_idx + i) < XVIDC_DISPLAYID_MAX_TIMING_MODES; i++) {
        u32 pixel_clock_10khz = x[offset] | (x[offset+1] << 8) | (x[offset+2] << 16);
        u16 h_active = (x[offset+4] << 8) | x[offset+3];
        u16 h_blank = (x[offset+6] << 8) | x[offset+5];
        u16 v_active = (x[offset+8] << 8) | x[offset+7];
        u16 v_blank = (x[offset+10] << 8) | x[offset+9];
        u16 h_total = h_active + h_blank;
        u16 v_total = v_active + v_blank;

        /* Convert pixel clock to Hz for calculations */
        u32 pixel_clock_hz = pixel_clock_10khz * 10000UL;

        /* Calculate refresh rate if timing values are valid */
        u16 refresh_rate = 0;
        if (h_total > 0 && v_total > 0) {
            refresh_rate = (u16)((pixel_clock_hz + (h_total * v_total / 2)) / (h_total * v_total));
        }

        /* Convert pixel clock to MHz for display purposes */
        u32 pixel_clock_mhz = (pixel_clock_10khz + 50) / 100;

        /* Store timing in DispIdTiming array */
        EdidCtrlParam->DispIdTiming[start_idx + i].hres = h_active;
        EdidCtrlParam->DispIdTiming[start_idx + i].vres = v_active;
        EdidCtrlParam->DispIdTiming[start_idx + i].pixclk = pixel_clock_10khz;
        EdidCtrlParam->DispIdNumTimings = start_idx + i + 1;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("    Timing #%u:\r\n", i+1);
            xil_printf("      Pixel Clock.............. %u MHz\r\n", pixel_clock_mhz);
            xil_printf("      Resolution............... %u x %u\r\n", h_active, v_active);
            xil_printf("      Refresh Rate............. %u Hz\r\n", refresh_rate);
            xil_printf("      Horizontal: Active=%u, Blank=%u, Total=%u\r\n",
                       h_active, h_blank, h_total);
            xil_printf("      Vertical: Active=%u, Blank=%u, Total=%u\r\n",
                       v_active, v_blank, v_total);
            xil_printf("\r\n");
        }
#else
        (void)pixel_clock_mhz;
        (void)refresh_rate;
        (void)VerboseEn;
#endif
        offset += XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Short Timings Type 3 data block
*
* @param    x is a pointer to the short timings 3 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_short_timings_3(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 i, offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_SHORT_TIMING_LEN) : 0;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Short Timings Type 3 Data Block\r\n");
        xil_printf("    Number of Timings: %u\r\n", num_timings);
    }
#endif

    /* Parse short timing descriptors - each is 3 bytes */
    for (i = 0; i < num_timings && offset + (XVIDC_DISPLAYID_SHORT_TIMING_LEN - 1) < XVIDC_DISPLAYID_BLOCK_DATA_MAX && i < XVIDC_DISPLAYID_MAX_SHORT_TIMINGS; i++) {
        u8 timing_idx = x[offset];
        u8 asp_ratio = (x[offset+1] >> 4) & 0x0f;
        u8 refresh = (x[offset+1] & 0x0f) + XVIDC_DISPLAYID_REFRESH_RATE_BASE;

        EdidCtrlParam->DispIdShortTiming[i].AspectRatio = asp_ratio;
        EdidCtrlParam->DispIdShortTiming[i].RefreshRate = refresh;
        EdidCtrlParam->DispIdNumShortTimings = i + 1;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      Timing %u: Index %u, Aspect %u, Refresh %u Hz\r\n", i+1, timing_idx, asp_ratio, refresh);
        }
#else
        (void)timing_idx;
        (void)VerboseEn;
#endif
        offset += XVIDC_DISPLAYID_SHORT_TIMING_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID DMT Timings Type 4 data block
*
* @param    x is a pointer to the DMT timings 4 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_dmt_timings_4(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];
    u32 i, offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u8 num_dmt_ids = length;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    DMT Timings Type 4 Data Block\r\n");
        xil_printf("    Number of DMT IDs: %u\r\n", num_dmt_ids);
    }
#endif

    EdidCtrlParam->DispIdNumDmtTimings = 0;
    for (i = 0; i < num_dmt_ids && offset < 128 && i < 16; i++) {
        u8 dmt_id = x[offset];

        EdidCtrlParam->DispIdDmtTiming[i] = dmt_id;
        EdidCtrlParam->DispIdNumDmtTimings++;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      DMT ID %u: 0x%02x\r\n", i+1, dmt_id);
        }
#endif
        offset++;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID DMT Timings data block
*
* @param    x is a pointer to the DMT timings block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_dmt_timings(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];
    u32 i, offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u8 num_dmt_ids = length;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    DMT Timings Data Block (Type 7)\r\n");
        xil_printf("    Number of DMT IDs: %u\r\n", num_dmt_ids);
    }
#endif

    EdidCtrlParam->DispIdNumDmtTimings = 0;
    for (i = 0; i < num_dmt_ids && offset < 128 && i < 16; i++) {
        u8 dmt_id = x[offset];

        EdidCtrlParam->DispIdDmtTiming[i] = dmt_id;
        EdidCtrlParam->DispIdNumDmtTimings++;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      DMT ID %u: 0x%02x\r\n", i+1, dmt_id);
        }
#endif
        offset++;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID CTA Timings data block
*
* @param    x is a pointer to the CTA timings block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_cta_timings(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 i, offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u8 num_vics = length;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    CTA Timings Data Block\r\n");
        xil_printf("    Number of VICs: %u\r\n", num_vics);
    }
#endif

    /* Parse CTA VIC codes - each is 1 byte */
    EdidCtrlParam->DispIdNumCtaVic = 0;
    for (i = 0; i < num_vics && offset < 128 && i < 16; i++) {
        u8 vic = x[offset] & 0x7f;
        u8 native = (x[offset] >> 7) & 0x01;

        /* Bounds check: VIC must be within valid array range (1-124) */
        if (vic > 124 || vic == 0) {
            if (VerboseEn) {
                xil_printf("      Reserved VIC %u (out of supported range)\r\n", vic);
            }
            offset++;
            continue;
        }

        /* Store CTA VIC */
        EdidCtrlParam->DispIdCtaVic[i] = vic;
        EdidCtrlParam->DispIdNumCtaVic++;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      VIC %u%s\r\n", vic, native ? " (Native)" : "");
        }
#else
        (void)native;
#endif
        offset++;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Video Timing Range data block
*
* @param    x is a pointer to the timing range block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_timing_range(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];

    if (length < XVIDC_DISPLAYID_TIMING_RANGE_MIN_LEN) return;

    EdidCtrlParam->DispIdMinPixelClkMhz = x[3] | (x[4] << 8) | (x[5] << 16);
    EdidCtrlParam->DispIdMaxPixelClkMhz = x[6] | (x[7] << 8) | (x[8] << 16);
    EdidCtrlParam->DispIdTimingRangeMinHfreq = x[9] | (x[10] << 8);
    EdidCtrlParam->DispIdTimingRangeMaxHfreq = x[11] | (x[12] << 8);
    EdidCtrlParam->DispIdTimingRangeMinVfreq = x[13];
    EdidCtrlParam->DispIdTimingRangeMaxVfreq = x[14];
    EdidCtrlParam->DispIdTimingRangeFlags = 1;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Video Timing Range Data Block\r\n");
        xil_printf("    Pixel Clock Range........ %u - %u MHz\r\n", EdidCtrlParam->DispIdMinPixelClkMhz, EdidCtrlParam->DispIdMaxPixelClkMhz);
        xil_printf("    Horizontal Frequency Range %u - %u Hz\r\n", EdidCtrlParam->DispIdTimingRangeMinHfreq, EdidCtrlParam->DispIdTimingRangeMaxHfreq);
        xil_printf("    Vertical Frequency Range. %u - %u Hz\r\n", EdidCtrlParam->DispIdTimingRangeMinVfreq, EdidCtrlParam->DispIdTimingRangeMaxVfreq);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Product Serial Number data block
*
* @param    x is a pointer to the serial number block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_serial_number(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i = 0;

    while (i < XVIDC_DISPLAYID_STRING_MAX_LEN && offset < (u32)(length + 3) && offset < 128 && x[offset] != 0x0A && x[offset] != 0x00) {
        EdidCtrlParam->DispIdProductString[i] = x[offset];
        i++;
        offset++;
    }
    EdidCtrlParam->DispIdProductString[i] = 0x00;
    EdidCtrlParam->DispIdProductStringLen = i;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Serial Number Data Block\r\n");
        xil_printf("    Value: %s\r\n", EdidCtrlParam->DispIdProductString);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID GP ASCII String data block
*
* @param    x is a pointer to the ASCII string block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_ascii_string(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i = 0;

    while (i < XVIDC_DISPLAYID_STRING_MAX_LEN && offset < (u32)(length + 3) && offset < 128 && x[offset] != 0x0A && x[offset] != 0x00) {
        EdidCtrlParam->DispIdAsciiString[i] = x[offset];
        i++;
        offset++;
    }
    EdidCtrlParam->DispIdAsciiString[i] = 0x00;
    EdidCtrlParam->DispIdAsciiStringLen = i;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    ASCII String Data Block\r\n");
        xil_printf("    Value: %s\r\n", (char*)EdidCtrlParam->DispIdAsciiString);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Display Device Data data block
*
* @param    x is a pointer to the device data block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_device_data(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 device_type, orientation, rotation, bezel_type;
    u16 width_mm, height_mm, depth_mm;

    if (length < XVIDC_DISPLAYID_DEVICE_DATA_MIN_LEN) return;

    device_type = x[3];
    orientation = (x[4] >> 5) & 0x07;
    rotation = (x[4] >> 2) & 0x07;
    bezel_type = x[4] & 0x03;

    width_mm = x[5] | (x[6] << 8);
    height_mm = x[7] | (x[8] << 8);
    depth_mm = x[9] | (x[10] << 8);

    EdidCtrlParam->DispIdDeviceType = device_type;
    EdidCtrlParam->DispIdScanOrientation = orientation;
    EdidCtrlParam->DispIdDeviceHorSize = width_mm;
    EdidCtrlParam->DispIdDeviceVerSize = height_mm;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Display Device Data Block\r\n");
        xil_printf("    Device Type: %u\r\n", EdidCtrlParam->DispIdDeviceType);
        xil_printf("    Orientation: %u, Rotation: %u\r\n", EdidCtrlParam->DispIdScanOrientation, rotation);
        xil_printf("    Bezel Type: %u\r\n", bezel_type);
        xil_printf("    Device Size: %u x %u x %u mm\r\n", EdidCtrlParam->DispIdDeviceHorSize, EdidCtrlParam->DispIdDeviceVerSize, depth_mm);
    }
#else
    (void)VerboseEn;
    (void)rotation;
    (void)bezel_type;
    (void)depth_mm;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Interface Power Sequencing data block
*
* @param    x is a pointer to the power sequencing block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_power_sequencing(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 power_on_delay, power_off_delay;

    if (length < XVIDC_DISPLAYID_POWER_SEQ_MIN_LEN) return;

    power_on_delay = x[3];
    power_off_delay = x[4];

    EdidCtrlParam->DispIdPowerSeqT1 = power_on_delay * 50;
    EdidCtrlParam->DispIdPowerSeqT2 = power_off_delay * 50;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Power Sequencing Data Block\r\n");
        xil_printf("    Power On Delay: %u ms\r\n", EdidCtrlParam->DispIdPowerSeqT1);
        xil_printf("    Power Off Delay: %u ms\r\n", EdidCtrlParam->DispIdPowerSeqT2);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Transfer Characteristics data block
*
* @param    x is a pointer to the transfer characteristics block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_transfer_characteristics(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;
    u8 length = x[2];
    u8 transfer_type;

    if (length < XVIDC_DISPLAYID_TRANSFER_CHAR_MIN_LEN) return;

    transfer_type = x[3];
    EdidCtrlParam->DispIdTransferType = transfer_type;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Transfer Characteristics Data Block\r\n");
        xil_printf("    Transfer Type: %u\r\n", EdidCtrlParam->DispIdTransferType);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Display Interface Features data block
*
* @param    x is a pointer to the interface features block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_interface_features(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;

    u8 length = x[2];

    if (length >= XVIDC_DISPLAYID_INTERFACE_MIN_LEN) {
        EdidCtrlParam->DispIdInterfaceType = x[3];

        if (length >= XVIDC_DISPLAYID_INTERFACE_EXTENDED_LEN) {
            u8 count_byte = x[6];
            u8 version_byte = x[7];

            EdidCtrlParam->DispIdNumInterfaces = (count_byte & XVIDC_DISPLAYID_INTERFACE_COUNT_MASK) + 1;
            if (EdidCtrlParam->DispIdNumInterfaces > XVIDC_DISPLAYID_INTERFACE_MAX_COUNT)
                EdidCtrlParam->DispIdNumInterfaces = XVIDC_DISPLAYID_INTERFACE_MAX_COUNT;

            EdidCtrlParam->DispIdInterfaceVersion = version_byte & XVIDC_DISPLAYID_INTERFACE_VERSION_MASK;

            if (length >= XVIDC_DISPLAYID_INTERFACE_COLOR_DEPTH_LEN) {
                u8 rgb_support = x[5];
                u8 ycbcr444_support = x[6];
                u8 ycbcr422_support = x[7];

                if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_16BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 16;
                else if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_14BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 14;
                else if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_12BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 12;
                else if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_10BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 10;
                else if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_8BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 8;
                else if (rgb_support & XVIDC_DISPLAYID_COLOR_DEPTH_6BPC_MASK) EdidCtrlParam->DispIdRgbColorDepth = 6;
                else EdidCtrlParam->DispIdRgbColorDepth = 8;

                if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_16BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 16;
                else if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_14BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 14;
                else if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_12BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 12;
                else if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_10BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 10;
                else if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_8BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 8;
                else if (ycbcr444_support & XVIDC_DISPLAYID_COLOR_DEPTH_6BPC_MASK) EdidCtrlParam->DispIdYCbCr444ColorDepth = 6;
                else EdidCtrlParam->DispIdYCbCr444ColorDepth = 8;

                if (ycbcr422_support & XVIDC_DISPLAYID_COLOR_DEPTH_16BPC_MASK) EdidCtrlParam->DispIdYCbCr422ColorDepth = 16;
                else if (ycbcr422_support & XVIDC_DISPLAYID_COLOR_DEPTH_14BPC_MASK) EdidCtrlParam->DispIdYCbCr422ColorDepth = 14;
                else if (ycbcr422_support & XVIDC_DISPLAYID_COLOR_DEPTH_12BPC_MASK) EdidCtrlParam->DispIdYCbCr422ColorDepth = 12;
                else if (ycbcr422_support & XVIDC_DISPLAYID_COLOR_DEPTH_10BPC_MASK) EdidCtrlParam->DispIdYCbCr422ColorDepth = 10;
                else if (ycbcr422_support & XVIDC_DISPLAYID_COLOR_DEPTH_8BPC_MASK) EdidCtrlParam->DispIdYCbCr422ColorDepth = 8;
                else EdidCtrlParam->DispIdYCbCr422ColorDepth = 8;

                EdidCtrlParam->DispIdYCbCr420ColorDepth = EdidCtrlParam->DispIdYCbCr444ColorDepth;
            } else {
                EdidCtrlParam->DispIdRgbColorDepth = 8;
                EdidCtrlParam->DispIdYCbCr444ColorDepth = 8;
                EdidCtrlParam->DispIdYCbCr422ColorDepth = 8;
                EdidCtrlParam->DispIdYCbCr420ColorDepth = 8;
            }
        } else {
            EdidCtrlParam->DispIdNumInterfaces = 1;
            EdidCtrlParam->DispIdInterfaceVersion = 1;
            EdidCtrlParam->DispIdRgbColorDepth = 8;
            EdidCtrlParam->DispIdYCbCr444ColorDepth = 8;
            EdidCtrlParam->DispIdYCbCr422ColorDepth = 8;
            EdidCtrlParam->DispIdYCbCr420ColorDepth = 8;
        }
    }
    if (length >= XVIDC_DISPLAYID_INTERFACE_PIXEL_CLK_LEN) {
        EdidCtrlParam->DispIdPixelClockRatio = x[4];
    }
    if (length >= XVIDC_DISPLAYID_INTERFACE_MAX_CLK_LEN) {
        u16 raw_max_clock = x[5] | (x[6] << 8);

        EdidCtrlParam->DispIdMaxPixelClock = raw_max_clock;

        EdidCtrlParam->DispIdMinPixelClkMhz = XVIDC_DISPLAYID_DEFAULT_MIN_PIXEL_CLK;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("  Display Interface Data Block\r\n");
        xil_printf("    Number of Interfaces..... %u\r\n", EdidCtrlParam->DispIdNumInterfaces);
        xil_printf("    Interface Version........ %u\r\n", EdidCtrlParam->DispIdInterfaceVersion);

        const char* interface_name = "Unknown";
        u8 interface_code = 0x00;
        if (EdidCtrlParam->DispIdInterfaceType == XVIDC_DISPLAYID_INTERFACE_TYPE_HDMI) {
            interface_name = "HDMI";
            interface_code = 0x21;
        }

        xil_printf("    Interface Type........... %s (0x%02X)\r\n", interface_name, interface_code);
        xil_printf("    Color Depth Support:\r\n");
        xil_printf("      RGB..................... %u bpc\r\n", EdidCtrlParam->DispIdRgbColorDepth);
        xil_printf("      YCbCr 4:4:4............. %u bpc\r\n", EdidCtrlParam->DispIdYCbCr444ColorDepth);
        xil_printf("      YCbCr 4:2:2............. %u bpc\r\n", EdidCtrlParam->DispIdYCbCr422ColorDepth);
        xil_printf("      YCbCr 4:2:0............. %u bpc\r\n", EdidCtrlParam->DispIdYCbCr420ColorDepth);

        u16 realistic_max_clock = EdidCtrlParam->DispIdMaxPixelClock / XVIDC_DISPLAYID_MAX_PIXEL_CLK_DIVISOR;
        xil_printf("    Pixel Clock Range......... %u - %u MHz\r\n\r\n",
                   EdidCtrlParam->DispIdMinPixelClkMhz, realistic_max_clock);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Stereo Display Interface data block
*
* @param    x is a pointer to the stereo display block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_stereo_display(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    VerboseEn = VerboseEn;

    u8 length = x[2];
    u8 interface, polarity = 0;

    if (length >= XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN) {

        interface = (x[XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN] >> 4) & 0x0F;
        polarity = x[XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN] & 0x0F;

        EdidCtrlParam->DispIdStereoInterface = interface;
        EdidCtrlParam->DispIdStereoPolarity = polarity;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Stereo Display Data Block\r\n");
        xil_printf("    Stereo Interface: 0x%02x, Polarity: 0x%02x\r\n", interface, polarity);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Short Timings Type 5 data block
*
* @param    x is a pointer to the short timings 5 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_short_timings_5(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 i, offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    (void)VerboseEn;

    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN) : 0;

    EdidCtrlParam->DispIdNumCvtTimings = 0;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Short Timings Type 5 (CVT-R2) Data Block\r\n");
        xil_printf("    Number of Timings: %u\r\n", num_timings);
    }
#endif

    for (i = 0; i < num_timings && offset + 2 < XVIDC_EDID_SIZE && i < XVIDC_DISPLAYID_MAX_CVT_TIMINGS; i++) {
        u16 h_active = ((x[offset+1] & 0x0F) << 8) | (x[offset+2]);
        u8 aspect_refresh = x[offset+1] >> 4;

        EdidCtrlParam->DispIdCvtTiming[i].HActive = h_active;
        EdidCtrlParam->DispIdCvtTiming[i].RefreshRate = aspect_refresh;
        EdidCtrlParam->DispIdNumCvtTimings++;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      Timing %u: HActive %u, Aspect/Refresh 0x%02x\r\n", i+1, h_active, aspect_refresh);
        }
#endif
        offset += XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Tiled Display Topology data block
*
* @param    x is a pointer to the tiled display block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_tiled_display(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 flags, h_tiles, v_tiles, h_position, v_position;

    u16 pixel_width, pixel_height;

    if (length < 10) return;

    EdidCtrlParam->DispIdIsTiled = XVIDC_SUPPORTED;

    flags = x[3];
    h_tiles = x[4] & 0x1f;
    v_tiles = x[5] & 0x1f;
    h_position = (x[5] >> 5) & 0x07;
    v_position = (x[6] >> 5) & 0x07;

    pixel_width = x[7] | (x[8] << 8);
    pixel_height = x[9] | ((x[10] & 0x0f) << 8);

    EdidCtrlParam->DispIdTileCols = h_tiles + 1;
    EdidCtrlParam->DispIdTileRows = v_tiles + 1;
    EdidCtrlParam->DispIdTileWidth = pixel_width;
    EdidCtrlParam->DispIdTileHeight = pixel_height;
    EdidCtrlParam->DispIdTileLocH = h_position;
    EdidCtrlParam->DispIdTileLocV = v_position;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Tiled Display Topology Data Block\r\n");
        xil_printf("    Tile Layout: %u x %u tiles\r\n", EdidCtrlParam->DispIdTileCols, EdidCtrlParam->DispIdTileRows);
        xil_printf("    Tile Position: (%u, %u)\r\n", EdidCtrlParam->DispIdTileLocH, EdidCtrlParam->DispIdTileLocV);
        xil_printf("    Tile Size: %u x %u pixels\r\n", EdidCtrlParam->DispIdTileWidth, EdidCtrlParam->DispIdTileHeight);
        xil_printf("    Flags: 0x%02x\r\n", flags);
    }
#else
    (void)VerboseEn;
    (void)flags;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Detailed Timings Type 6 data block
*
* @param    x is a pointer to the detailed timings 6 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_detailed_timings_6(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_DETAILED_TIMING_DIVISOR) : 0;

    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i;

    EdidCtrlParam->DispIdNumType6Timings = 0;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Detailed Timings Type 6 Data Block\r\n");
        xil_printf("    Number of Timings: %u\r\n", num_timings);
    }
#else
    (void)VerboseEn;
#endif

    /* Parse detailed timing descriptors - each is 20 bytes */
    for (i = 0; i < num_timings && offset + (XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN - 1) < XVIDC_DISPLAYID_BLOCK_DATA_MAX && i < XVIDC_DISPLAYID_MAX_TIMING_MODES; i++) {
        u32 pixel_clock = x[offset] | (x[offset+1] << 8) | (x[offset+2] << 16);
        u16 h_active = x[offset+3] | (x[offset+4] << 8);
        u16 v_active = x[offset+7] | (x[offset+8] << 8);

        /* Store timing index/marker for Type 6 */
        EdidCtrlParam->DispIdType6Timing[i] = i;
        EdidCtrlParam->DispIdNumType6Timings++;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      Timing %u: %u x %u @ %u kHz\r\n", i+1, h_active, v_active, pixel_clock * XVIDC_DISPLAYID_PIXEL_CLOCK_MULTIPLIER);
        }
#else
        (void)pixel_clock;
        (void)h_active;
        (void)v_active;
        (void)VerboseEn;
#endif
        offset += XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Detailed Timing data block
*
* @param    x is a pointer to the detailed timing block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_detailed_timing(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 i;

    if (length < XVIDC_DISPLAYID_DETAILED_TIMING_MIN_LEN) return;

    /* Store detailed timing in DispIdDetailedTiming7Data */
    if (length <= XVIDC_DISPLAYID_DETAILED_TIMING_BUFFER_MAX) {
        for (i = 0; i < length; i++) {
            EdidCtrlParam->DispIdDetailedTiming7Data[i] = x[i];
        }
        EdidCtrlParam->DispIdDetailedTiming7Len = length;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        u32 pixel_clock = EdidCtrlParam->DispIdDetailedTiming7Data[3] |
                          (EdidCtrlParam->DispIdDetailedTiming7Data[4] << 8) |
                          (EdidCtrlParam->DispIdDetailedTiming7Data[5] << 16);
        u16 h_active = EdidCtrlParam->DispIdDetailedTiming7Data[6] |
                       ((EdidCtrlParam->DispIdDetailedTiming7Data[7] & 0x0f) << 8);
        u16 v_active = ((EdidCtrlParam->DispIdDetailedTiming7Data[7] >> 4) & 0x0f) |
                       (EdidCtrlParam->DispIdDetailedTiming7Data[8] << 4);

        xil_printf("    Detailed Timing Data Block (Type 1)\r\n");
        xil_printf("    Resolution: %u x %u\r\n", h_active, v_active);
        xil_printf("    Pixel Clock: %u kHz\r\n", pixel_clock * XVIDC_DISPLAYID_PIXEL_CLOCK_MULTIPLIER);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Detailed Timings Type 7 data block
*
* @param    x is a pointer to the detailed timings 7 block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_detailed_timings_7(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 num_timings = (length > 0) ? (length / XVIDC_DISPLAYID_DETAILED_TIMING_DIVISOR) : 0;
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u32 i, data_idx = 0;

    /* Store detailed timing data (up to 32 bytes) */
    EdidCtrlParam->DispIdDetailedTiming7Len = 0;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Detailed Timings Type 7 Data Block\r\n");
        xil_printf("    Number of Timings: %u\r\n", num_timings);
    }
#endif

    /* Copy detailed timing descriptors to buffer - each is 20 bytes */
    for (i = 0; i < num_timings && offset + (XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN - 1) < XVIDC_DISPLAYID_BLOCK_DATA_MAX && data_idx < XVIDC_DISPLAYID_DETAILED_TIMING_BUFFER_MAX; i++) {
        u32 pixel_clock = x[offset] | (x[offset+1] << 8) | (x[offset+2] << 16);
        u16 h_active = x[offset+3] | (x[offset+4] << 8);
        u16 v_active = x[offset+7] | (x[offset+8] << 8);
        u32 j;

        /* Store up to 20 bytes per timing, respecting 32-byte buffer limit */
        for (j = 0; j < XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN && data_idx < XVIDC_DISPLAYID_DETAILED_TIMING_BUFFER_MAX; j++) {
            EdidCtrlParam->DispIdDetailedTiming7Data[data_idx] = x[offset + j];
            data_idx++;
        }
        EdidCtrlParam->DispIdDetailedTiming7Len = data_idx;
        EdidCtrlParam->DispIdNumType7Timings = i + 1;

#if XVIDC_EDID_VERBOSITY > 0
        if (VerboseEn) {
            xil_printf("      Timing %u: %u x %u @ %u kHz\r\n", i+1, h_active, v_active, pixel_clock * XVIDC_DISPLAYID_PIXEL_CLOCK_MULTIPLIER);
        }
#else
        (void)pixel_clock;
        (void)h_active;
        (void)v_active;
        (void)VerboseEn;
#endif
        offset += XVIDC_DISPLAYID_TIMING_DESCRIPTOR_LEN;
    }
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Adaptive Refresh Rate data block
*
* @param    x is a pointer to the adaptive refresh block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_adaptive_refresh(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 flags = 0;

    if (length >= XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN) {
        flags = x[XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN];
        EdidCtrlParam->DispIdAdaptiveRefreshFlags = flags;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Adaptive Refresh Rate Data Block\r\n");
        xil_printf("    Flags: 0x%02x\r\n", flags);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Bilingual Unicode String data block
*
* @param    x is a pointer to the unicode string block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_unicode_string(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN;
    u8 lang_code = 0;
    u32 i = 0;

    if (length >= XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN) {
        lang_code = x[XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN];
        EdidCtrlParam->DispIdUnicodeStringLen = (length - XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN > XVIDC_DISPLAYID_STRING_MAX_LEN) ? XVIDC_DISPLAYID_STRING_MAX_LEN : (length - XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN);

        /* Store unicode string bytes (up to XVIDC_DISPLAYID_STRING_MAX_LEN bytes) */
        offset = XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN + 1;
        while (i < EdidCtrlParam->DispIdUnicodeStringLen && offset < (u32)(length + XVIDC_DISPLAYID_DATA_BLOCK_HEADER_LEN) && offset < XVIDC_EDID_SIZE) {
            EdidCtrlParam->DispIdUnicodeString[i] = x[offset];
            i++;
            offset++;
        }
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Bilingual Unicode String Data Block\r\n");
        xil_printf("    Language Code: 0x%02x\r\n", lang_code);
        xil_printf("    Length: %u bytes\r\n", EdidCtrlParam->DispIdUnicodeStringLen);
        xil_printf("    Value: ");

        /* Print unicode string (simplified - just print ASCII range) */
        for (i = 0; i < EdidCtrlParam->DispIdUnicodeStringLen; i++) {
            if (EdidCtrlParam->DispIdUnicodeString[i] >= 32 && EdidCtrlParam->DispIdUnicodeString[i] <= 126) {
                xil_printf("%c", EdidCtrlParam->DispIdUnicodeString[i]);
            }
        }
        xil_printf("\r\n");
    }
#else
    (void)VerboseEn;
    (void)lang_code;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID CTA DisplayID 2.0 Data Block
*
* @param    x is a pointer to the CTA block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_cta_block(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u8 cta_block_header = (length > 0) ? x[3] : 0;

    EdidCtrlParam->DispIdCtaBlockTag = (cta_block_header >> 5) & 0x07;
    EdidCtrlParam->DispIdCtaBlockPayloadSize = cta_block_header & 0x1f;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    CTA DisplayID 2.0 Data Block\r\n");
        xil_printf("    CTA Version: %u\r\n", EdidCtrlParam->DispIdCtaBlockTag);
        xil_printf("    CTA Block Payload Size: %u\r\n", EdidCtrlParam->DispIdCtaBlockPayloadSize);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Container ID data block
*
* @param    x is a pointer to the container ID block data
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_container_id(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];
    u32 container_id_1, container_id_2, container_id_3, container_id_4;

    if (length < XVIDC_DISPLAYID_CONTAINER_ID_MIN_LEN) return;

    container_id_1 = x[3] | (x[4] << 8) | (x[5] << 16) | (x[6] << 24);
    container_id_2 = x[7] | (x[8] << 8) | (x[9] << 16) | (x[10] << 24);
    container_id_3 = x[11] | (x[12] << 8) | (x[13] << 16) | (x[14] << 24);
    container_id_4 = x[15] | (x[16] << 8) | (x[17] << 16) | (x[18] << 24);

    /* Store container ID bytes in array */
    EdidCtrlParam->DispIdContainerId[0] = container_id_1 & 0xFF;
    EdidCtrlParam->DispIdContainerId[1] = (container_id_1 >> 8) & 0xFF;
    EdidCtrlParam->DispIdContainerId[2] = (container_id_1 >> 16) & 0xFF;
    EdidCtrlParam->DispIdContainerId[3] = (container_id_1 >> 24) & 0xFF;
    EdidCtrlParam->DispIdContainerId[4] = container_id_2 & 0xFF;
    EdidCtrlParam->DispIdContainerId[5] = (container_id_2 >> 8) & 0xFF;
    EdidCtrlParam->DispIdContainerId[6] = (container_id_2 >> 16) & 0xFF;
    EdidCtrlParam->DispIdContainerId[7] = (container_id_2 >> 24) & 0xFF;
    EdidCtrlParam->DispIdContainerId[8] = container_id_3 & 0xFF;
    EdidCtrlParam->DispIdContainerId[9] = (container_id_3 >> 8) & 0xFF;
    EdidCtrlParam->DispIdContainerId[10] = (container_id_3 >> 16) & 0xFF;
    EdidCtrlParam->DispIdContainerId[11] = (container_id_3 >> 24) & 0xFF;
    EdidCtrlParam->DispIdContainerId[12] = container_id_4 & 0xFF;
    EdidCtrlParam->DispIdContainerId[13] = (container_id_4 >> 8) & 0xFF;
    EdidCtrlParam->DispIdContainerId[14] = (container_id_4 >> 16) & 0xFF;
    EdidCtrlParam->DispIdContainerId[15] = (container_id_4 >> 24) & 0xFF;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("    Container ID Data Block\r\n");
        xil_printf("    Container ID: 0x%08x-%08x-%08x-%08x\r\n",
                   container_id_1, container_id_2, container_id_3, container_id_4);
    }
#else
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Vendor-Specific Data Block
*
* @param    x is a pointer to the vendor-specific data block
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_parse_displayid_vendor_specific(const u8 *x, XV_VidC_EdidCntrlParam *EdidCtrlParam, XV_VidC_Verbose VerboseEn)
{
    u8 length = x[2];

    if (length < 3) return;

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        u8 vendor_id[3];

        /* Extract vendor ID (IEEE OUI) */
        vendor_id[0] = x[3];
        vendor_id[1] = x[4];
        vendor_id[2] = x[5];

        xil_printf("    Vendor-Specific Data Block\r\n");
        xil_printf("    Vendor ID (IEEE OUI): 0x%02x%02x%02x\r\n",
                   vendor_id[2], vendor_id[1], vendor_id[0]);
        xil_printf("    Data length: %d bytes\r\n", length - 3);
    }
#else
    (void)EdidCtrlParam;
    (void)VerboseEn;
#endif
}

/*****************************************************************************/
/**
*
* This function parses DisplayID Extension Block
*
* @param    ext is a pointer to the EDID extension
* @param    EdidCtrlParam is a pointer to the EDID Control parameter
* @param    VerboseEn enables verbose output
*
******************************************************************************/
static void
xvidc_dispid_block(const struct xvidc_edid_extension * const ext,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn)
{
    const u8 *data = (const u8 *)ext;
    u8 index = 5;
    u8 block_length = data[2];
    u8 end_index = 5 + block_length;
    u8 max_iterations = 64;

    /* Validate DisplayID extension tag */
    if (data[0] != XVIDC_EDID_EXTENSION_DID) {
        return;
    }

    /* Extract header information */
    u8 version_major = (data[1] >> 4) & 0x0f;
    u8 version_minor = data[1] & 0x0f;
    EdidCtrlParam->DispIdProductType = data[3];
    EdidCtrlParam->DispIdExtensionCount = data[4];

    /* Set DisplayID presence and version */
    EdidCtrlParam->IsDispIdPresent = XVIDC_SUPPORTED;
    EdidCtrlParam->DispIdVersion = (version_major << 4) | version_minor;

    /* Determine DisplayID version for proper block tag mapping */
    u8 is_displayid_2_0 = (version_major == 2 && version_minor == 0);

    /* Extract product information from base EDID (first 128 bytes) */
    const u8 *base_edid = (const u8 *)ext - 128;
    if (base_edid[0] == 0x00 && base_edid[1] == 0xFF) {
        u16 mfg_id = (base_edid[8] << 8) | base_edid[9];
        u16 product_code = base_edid[10] | (base_edid[11] << 8);
        u32 serial_number = (base_edid[15] << 24) | (base_edid[14] << 16) |
                           (base_edid[13] << 8) | base_edid[12];
        u8 mfg_week = base_edid[16];
        u8 mfg_year = base_edid[17];

        /* Store in EdidCtrlParam for later use */
        EdidCtrlParam->ManufacturerId = mfg_id;
        EdidCtrlParam->DispIdProductCode = product_code;
        EdidCtrlParam->DispIdSerialNumber = serial_number;
        EdidCtrlParam->ManufactureWeek = mfg_week;
        EdidCtrlParam->ManufactureYear = mfg_year + 1990;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("DisplayID Extension Block\r\n");
        xil_printf("  Version byte (raw)....... 0x%02x\r\n", data[1]);
        xil_printf("  Version.................. DisplayID %u.%u\r\n", version_major, version_minor);
        xil_printf("  Section Length........... %u\r\n", block_length);
        xil_printf("  Product Type............. 0x%02x\r\n", EdidCtrlParam->DispIdProductType);
        xil_printf("  Extension Count.......... %u\r\n\r\n", EdidCtrlParam->DispIdExtensionCount);

        /* Print Product Identification from stored EdidCtrlParam data */
        if (EdidCtrlParam->ManufacturerId) {
            char mfg_str[4];
            mfg_str[0] = ((EdidCtrlParam->ManufacturerId >> 10) & 0x1F) + '@';
            mfg_str[1] = ((EdidCtrlParam->ManufacturerId >> 5) & 0x1F) + '@';
            mfg_str[2] = (EdidCtrlParam->ManufacturerId & 0x1F) + '@';
            mfg_str[3] = '\0';

            xil_printf("  Product Identification\r\n");
            xil_printf("    Manufacturer............. %s\r\n", mfg_str);
            xil_printf("    Product Code............. 0x%04X\r\n", EdidCtrlParam->DispIdProductCode);
            xil_printf("    Serial Number............ 0x%08X\r\n", EdidCtrlParam->DispIdSerialNumber);
            xil_printf("    Year of Manufacture...... %u", EdidCtrlParam->ManufactureYear);
            if (EdidCtrlParam->ManufactureWeek > 0 && EdidCtrlParam->ManufactureWeek <= 53) {
                xil_printf(", Week %u", EdidCtrlParam->ManufactureWeek);
            }
            xil_printf("\r\n\r\n");
        }
    }
#endif

    while ((index < end_index) && (max_iterations > 0)) {
        max_iterations--;

        if (index + 3 > end_index) {
            break;
        }

        u8 tag = data[index];
        u8 len = data[index + 2];

        if (len == 0) {
            index += 3;
            continue;
        }

        if (index + 3 + len > end_index) {
#if XVIDC_EDID_VERBOSITY > 0
            if (VerboseEn) {
                xil_printf("  WARNING: Data block extends beyond section\r\n");
            }
#endif
            break;
        }

        /* Use different block tag mappings based on DisplayID version */
        if (is_displayid_2_0) {
            /* DisplayID 2.0 block tag mapping */
            switch (tag) {
                case 0x20:
                    xvidc_parse_displayid_product_id(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x21:
                    xvidc_parse_displayid_parameters(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x22:
                    xvidc_parse_displayid_timing_mode_1(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x23:
                    xvidc_parse_displayid_dmt_timings_4(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x24:
                    xvidc_parse_displayid_short_timings_5(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x25:
                    xvidc_parse_displayid_timing_range(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x26:
                    xvidc_parse_displayid_interface_features(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x27:
                    xvidc_parse_displayid_stereo_display(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x28:
                    xvidc_parse_displayid_tiled_display(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x29:
                    xvidc_parse_displayid_container_id(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x7E:
                    xvidc_parse_displayid_vendor_specific(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x81:
                    xvidc_parse_displayid_cta_block(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                default:
#if XVIDC_EDID_VERBOSITY > 0
                    if (VerboseEn) {
                        xil_printf("  Unknown DisplayID 2.0 block tag: 0x%02x\r\n", tag);
                    }
#endif
                    break;
            }
        } else {
            /* DisplayID 1.x block tag mapping (backward compatibility) */
            switch (tag) {
                case 0x00:
                    xvidc_parse_displayid_product_id(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x01:
                    xvidc_parse_displayid_parameters(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x02:
                    xvidc_parse_displayid_color_characteristics(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x03:
                    xvidc_parse_displayid_timing_mode_1(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x04:
                    xvidc_parse_displayid_timing_mode_2(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x05:
                    xvidc_parse_displayid_short_timings_3(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x06:
                    xvidc_parse_displayid_dmt_timings_4(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x07:
                    xvidc_parse_displayid_dmt_timings(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x08:
                    xvidc_parse_displayid_detailed_timings_6(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x09:
                    xvidc_parse_displayid_cta_timings(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0A:
                    xvidc_parse_displayid_serial_number(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0B:
                    xvidc_parse_displayid_ascii_string(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0C:
                    xvidc_parse_displayid_device_data(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0D:
                    xvidc_parse_displayid_power_sequencing(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0E:
                    xvidc_parse_displayid_transfer_characteristics(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x0F:
                    xvidc_parse_displayid_interface_features(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x10:
                    xvidc_parse_displayid_stereo_display(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x11:
                    xvidc_parse_displayid_short_timings_5(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x12:
                    xvidc_parse_displayid_tiled_display(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x13:
                    xvidc_parse_displayid_detailed_timing(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x14:
                    xvidc_parse_displayid_detailed_timings_7(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x15:
                    xvidc_parse_displayid_adaptive_refresh(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x16:
                    xvidc_parse_displayid_unicode_string(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x17:
                    xvidc_parse_displayid_cta_block(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                /* Cases 0x18-0x1F are reserved in DisplayID specification - not implemented */
                case 0x20:
                    xvidc_parse_displayid_container_id(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x21:
                    xvidc_parse_displayid_timing_range(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                case 0x7F:
                    xvidc_parse_displayid_vendor_specific(&data[index], EdidCtrlParam, VerboseEn);
                    break;
                default:
#if XVIDC_EDID_VERBOSITY > 1
                    if (VerboseEn) {
                        xil_printf("    Unknown DisplayID 1.x block tag: 0x%02X (length: %u)\r\n",
                                   tag, len);
                    }
#endif
                    break;
            }
        }

        index += 3 + len;
    }

#if XVIDC_EDID_VERBOSITY > 0
    if (VerboseEn) {
        xil_printf("\r\n");
    }
#endif
}

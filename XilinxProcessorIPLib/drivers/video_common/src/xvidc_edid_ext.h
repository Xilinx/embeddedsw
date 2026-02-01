/* vim: set et fde fdm=syntax ft=c.doxygen ts=4 sts=4 sw=4 : */
/*
 * Copyright © 2010-2021 Saleem Abdulrasool <compnerd@compnerd.org>.
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

#ifndef xvidc_edid_h
#define xvidc_edid_h

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "xvidc.h"
#include "xil_assert.h"
#include "xvidc_cea861.h"

/** EDID block size in bytes (128 bytes) */
#define XVIDC_EDID_BLOCK_SIZE                         (0x80)
/** Maximum number of EDID extension blocks (254) */
#define XVIDC_EDID_MAX_EXTENSIONS                     (0xFE)

/** EDID extension header signature (8 bytes) */
static const u8 XVIDC_EDID_EXT_HEADER[] =
                            { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
/** Invalid standard timing descriptor marker */
static const u8 XVIDC_EDID_STANDARD_TIMING_DESCRIPTOR_INVALID[] =
                                                                { 0x01, 0x01 };

/**
 * EDID extension block types.
 */
enum xvidc_edid_extension_type {
    XVIDC_EDID_EXTENSION_TIMING           = 0x01, /**< Timing Extension */
    XVIDC_EDID_EXTENSION_CEA              = 0x02, /**< Additional Timing Block
                                             Data (CEA EDID Timing Extension) */
    XVIDC_EDID_EXTENSION_VTB              = 0x10, /**< Video Timing Block
                                                          Extension (VTB-EXT) */
    XVIDC_EDID_EXTENSION_XVIDC_EDID_2_0= 0x20, /**< EDID 2.0 Extension */
    XVIDC_EDID_EXTENSION_DI               = 0x40, /**< Display Information
                                                          Extension (DI-EXT) */
    XVIDC_EDID_EXTENSION_LS               = 0x50, /**< Localised String
                                                          Extension (LS-EXT) */
    XVIDC_EDID_EXTENSION_MI               = 0x60, /**< Microdisplay Interface
                                                          Extension (MI-EXT) */
    XVIDC_EDID_EXTENSION_DID              = 0x70, /**< Display ID Extension */
    XVIDC_EDID_EXTENSION_DTCDB_1          = 0xA7, /**< Display Transfer
                                          Characteristics Data Block (DTCDB) */
    XVIDC_EDID_EXTENSION_DTCDB_2          = 0xAF,  /**< Display Transfer Characteristics Data Block (DTCDB) */
    XVIDC_EDID_EXTENSION_DTCDB_3          = 0xBF,  /**< Display Transfer Characteristics Data Block (DTCDB) */
    XVIDC_EDID_EXTENSION_BLOCK_MAP        = 0xF0, /**< Block Map */
    XVIDC_EDID_EXTENSION_DDDB             = 0xFF  /**< Display Device Data
                                                                 Block (DDDB) */
};

/**
 * Display type enumeration.
 */
enum xvidc_edid_display_type {
    XVIDC_EDID_DISPLAY_TYPE_MONOCHROME,  /**< Monochrome display */
    XVIDC_EDID_DISPLAY_TYPE_RGB,         /**< RGB color display */
    XVIDC_EDID_DISPLAY_TYPE_NON_RGB,     /**< Non-RGB color display */
    XVIDC_EDID_DISPLAY_TYPE_UNDEFINED,   /**< Undefined display type */
};

/**
 * Display aspect ratio enumeration.
 */
enum xvidc_edid_aspect_ratio {
    XVIDC_EDID_ASPECT_RATIO_16_10,  /**< 16:10 aspect ratio */
    XVIDC_EDID_ASPECT_RATIO_4_3,    /**< 4:3 aspect ratio */
    XVIDC_EDID_ASPECT_RATIO_5_4,    /**< 5:4 aspect ratio */
    XVIDC_EDID_ASPECT_RATIO_16_9,   /**< 16:9 aspect ratio */
};

/**
 * Signal sync type enumeration.
 */
enum xvidc_edid_signal_sync {
    XVIDC_EDID_SIGNAL_SYNC_ANALOG_COMPOSITE,          /**< Analog composite sync */
    XVIDC_EDID_SIGNAL_SYNC_BIPOLAR_ANALOG_COMPOSITE,  /**< Bipolar analog composite sync */
    XVIDC_EDID_SIGNAL_SYNC_DIGITAL_COMPOSITE,         /**< Digital composite sync */
    XVIDC_EDID_SIGNAL_SYNC_DIGITAL_SEPARATE,          /**< Digital separate sync */
};

/**
 * Stereo display mode enumeration.
 */
enum xvidc_edid_stereo_mode {
    XVIDC_EDID_STEREO_MODE_NONE,                       /**< No stereo */
    XVIDC_EDID_STEREO_MODE_RESERVED,                   /**< Reserved stereo mode */
    XVIDC_EDID_STEREO_MODE_FIELD_SEQUENTIAL_RIGHT,     /**< Field sequential stereo, right eye first */
    XVIDC_EDID_STEREO_MODE_2_WAY_INTERLEAVED_RIGHT,    /**< 2-way interleaved stereo, right eye first */
    XVIDC_EDID_STEREO_MODE_FIELD_SEQUENTIAL_LEFT,      /**< Field sequential stereo, left eye first */
    XVIDC_EDID_STEREO_MODE_2_WAY_INTERLEAVED_LEFT,     /**< 2-way interleaved stereo, left eye first */
    XVIDC_EDID_STEREO_MODE_4_WAY_INTERLEAVED,          /**< 4-way interleaved stereo */
    XVIDC_EDID_STEREO_MODE_SIDE_BY_SIDE_INTERLEAVED,   /**< Side-by-side interleaved stereo */
};

/**
 * EDID monitor descriptor types.
 */
enum xvidc_edid_monitor_descriptor_type {
    XVIDC_EDID_MONTIOR_DESCRIPTOR_MANUFACTURER_DEFINED        = 0x0F,  /**< Manufacturer defined data */
    XVIDC_EDID_MONITOR_DESCRIPTOR_STANDARD_TIMING_IDENTIFIERS = 0xFA,  /**< Standard timing identifiers */
    XVIDC_EDID_MONITOR_DESCRIPTOR_COLOR_POINT                 = 0xFB,  /**< Color point data */
    XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_NAME                = 0xFC,  /**< Monitor name string */
    XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_RANGE_LIMITS        = 0xFD,  /**< Monitor range limits */
    XVIDC_EDID_MONITOR_DESCRIPTOR_ASCII_STRING                = 0xFE,  /**< Unspecified ASCII string */
    XVIDC_EDID_MONITOR_DESCRIPTOR_MONITOR_SERIAL_NUMBER       = 0xFF,  /**< Monitor serial number */
};

/**
 * Secondary timing support enumeration.
 */
enum xvidc_edid_secondary_timing_support {
    XVIDC_EDID_SECONDARY_TIMING_NOT_SUPPORTED,  /**< Secondary timing not supported */
    XVIDC_EDID_SECONDARY_TIMING_GFT           = 0x02,  /**< GTF (Generalized Timing Formula) supported */
};

/**
 * DisplayID 2.0 data block tag codes (VESA DisplayID 2.0 Standard).
 */
enum xvidc_displayid_2_0_block_tag {
    XVIDC_DISPLAYID_2_0_PRODUCT_ID            = 0x20, /**< Product Identification (mandatory) */
    XVIDC_DISPLAYID_2_0_DISPLAY_PARAMS        = 0x21, /**< Display Parameters (mandatory) */
    XVIDC_DISPLAYID_2_0_TYPE_VII_TIMING       = 0x22, /**< Type VII Detailed Timings */
    XVIDC_DISPLAYID_2_0_TYPE_VIII_TIMING      = 0x23, /**< Type VIII Enumerated Timing Code */
    XVIDC_DISPLAYID_2_0_TYPE_IX_TIMING        = 0x24, /**< Type IX Formula-based Timings */
    XVIDC_DISPLAYID_2_0_DYNAMIC_TIMING_RANGE  = 0x25, /**< Dynamic Video Timing Range */
    XVIDC_DISPLAYID_2_0_INTERFACE_FEATURES    = 0x26, /**< Display Interface Features (mandatory) */
    XVIDC_DISPLAYID_2_0_STEREO_INTERFACE      = 0x27, /**< Stereo Display Interface */
    XVIDC_DISPLAYID_2_0_TILED_TOPOLOGY        = 0x28, /**< Tiled Display Topology */
    XVIDC_DISPLAYID_2_0_CONTAINER_ID          = 0x29, /**< Container ID */
    XVIDC_DISPLAYID_2_0_VENDOR_SPECIFIC       = 0x7E, /**< Vendor-specific Data */
    XVIDC_DISPLAYID_2_0_CTA_DISPLAYID         = 0x81  /**< CTA DisplayID */
};

/**
 * DisplayID 1.x data block tag codes (legacy support for backward compatibility).
 */
enum xvidc_displayid_1_x_block_tag {
    XVIDC_DISPLAYID_1_X_PRODUCT_ID            = 0x00, /**< Product Identification */
    XVIDC_DISPLAYID_1_X_DISPLAY_PARAMS        = 0x01, /**< Display Parameters */
    XVIDC_DISPLAYID_1_X_COLOR_CHARS           = 0x02, /**< Color Characteristics */
    XVIDC_DISPLAYID_1_X_TYPE_I_TIMING         = 0x03, /**< Type I Detailed Timings */
    XVIDC_DISPLAYID_1_X_TYPE_II_TIMING        = 0x04, /**< Type II Detailed Timings */
    XVIDC_DISPLAYID_1_X_TYPE_III_TIMING       = 0x05, /**< Type III Short Timings */
    XVIDC_DISPLAYID_1_X_TYPE_IV_TIMING        = 0x06, /**< Type IV Short Timings (DMT ID) */
    XVIDC_DISPLAYID_1_X_VESA_TIMING           = 0x07, /**< VESA Timing Standard */
    XVIDC_DISPLAYID_1_X_CEA_TIMING            = 0x08, /**< CEA Timing Standard */
    XVIDC_DISPLAYID_1_X_TIMING_RANGE          = 0x09, /**< Video Timing Range Limits */
    XVIDC_DISPLAYID_1_X_SERIAL_NUMBER         = 0x0A, /**< Product Serial Number */
    XVIDC_DISPLAYID_1_X_ASCII_STRING          = 0x0B, /**< General Purpose ASCII String */
    XVIDC_DISPLAYID_1_X_DEVICE_DATA           = 0x0C, /**< Display Device Data */
    XVIDC_DISPLAYID_1_X_POWER_SEQUENCING      = 0x0D, /**< Interface Power Sequencing */
    XVIDC_DISPLAYID_1_X_TRANSFER_CHARS        = 0x0E, /**< Transfer Characteristics */
    XVIDC_DISPLAYID_1_X_INTERFACE_DATA        = 0x0F, /**< Display Interface Data */
    XVIDC_DISPLAYID_1_X_STEREO_INTERFACE      = 0x10, /**< Stereo Display Interface */
    XVIDC_DISPLAYID_1_X_TYPE_V_TIMING         = 0x11, /**< Type V Short Timings */
    XVIDC_DISPLAYID_1_X_TILED_TOPOLOGY        = 0x12, /**< Tiled Display Topology */
    XVIDC_DISPLAYID_1_X_TYPE_VI_TIMING        = 0x13, /**< Type VI Detailed Timings */
    XVIDC_DISPLAYID_1_X_CONTAINER_ID          = 0x20, /**< Container ID (in 1.x mapping) */
    XVIDC_DISPLAYID_1_X_VENDOR_SPECIFIC_7E    = 0x7E, /**< Vendor-specific Data */
    XVIDC_DISPLAYID_1_X_VENDOR_SPECIFIC_7F    = 0x7F  /**< Vendor-specific Data */
};

/**
 * EDID detailed timing descriptor structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_detailed_timing_descriptor {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_detailed_timing_descriptor {
#endif
    u16 pixel_clock;                               /**< Pixel clock = value * 10000 */

    u8  horizontal_active_lo;                      /**< Horizontal active pixels low byte */
    u8  horizontal_blanking_lo;                    /**< Horizontal blanking pixels low byte */

    unsigned horizontal_blanking_hi         : 4;   /**< Horizontal blanking pixels high nibble */
    unsigned horizontal_active_hi           : 4;   /**< Horizontal active pixels high nibble */

    u8  vertical_active_lo;                        /**< Vertical active lines low byte */
    u8  vertical_blanking_lo;                      /**< Vertical blanking lines low byte */

    unsigned vertical_blanking_hi           : 4;   /**< Vertical blanking lines high nibble */
    unsigned vertical_active_hi             : 4;   /**< Vertical active lines high nibble */

    u8  horizontal_sync_offset_lo;                 /**< Horizontal sync offset low byte */
    u8  horizontal_sync_pulse_width_lo;            /**< Horizontal sync pulse width low byte */

    unsigned vertical_sync_pulse_width_lo   : 4;   /**< Vertical sync pulse width low nibble */
    unsigned vertical_sync_offset_lo        : 4;   /**< Vertical sync offset low nibble */

    unsigned vertical_sync_pulse_width_hi   : 2;   /**< Vertical sync pulse width high bits */
    unsigned vertical_sync_offset_hi        : 2;   /**< Vertical sync offset high bits */
    unsigned horizontal_sync_pulse_width_hi : 2;   /**< Horizontal sync pulse width high bits */
    unsigned horizontal_sync_offset_hi      : 2;   /**< Horizontal sync offset high bits */

    u8  horizontal_image_size_lo;                  /**< Horizontal image size low byte */
    u8  vertical_image_size_lo;                    /**< Vertical image size low byte */

    unsigned vertical_image_size_hi         : 4;   /**< Vertical image size high nibble */
    unsigned horizontal_image_size_hi       : 4;   /**< Horizontal image size high nibble */

    u8  horizontal_border;                         /**< Horizontal border pixels */
    u8  vertical_border;                           /**< Vertical border lines */

    unsigned stereo_mode_lo                 : 1;   /**< Stereo mode low bit */
    unsigned signal_pulse_polarity          : 1;   /**< Pulse on sync,
                                               composite/horizontal polarity */
    unsigned signal_serration_polarity      : 1;   /**< Serrate on sync, vertical
                                                                    polarity */
    unsigned signal_sync                    : 2;   /**< Signal sync type */
    unsigned stereo_mode_hi                 : 2;   /**< Stereo mode high bits */
    unsigned interlaced                     : 1;   /**< Interlaced flag */
};

/**
 * Extract pixel clock from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Pixel clock in Hz.
 */
static inline u32
xvidc_edid_detailed_timing_pixel_clock
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return dtb->pixel_clock * 10000;
}

/**
 * Extract horizontal blanking from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Horizontal blanking in pixels.
 */
static inline u16
xvidc_edid_detailed_timing_horizontal_blanking
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->horizontal_blanking_hi << 8) | dtb->horizontal_blanking_lo;
}

/**
 * Extract horizontal active pixels from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Horizontal active pixels.
 */
static inline u16
xvidc_edid_detailed_timing_horizontal_active
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->horizontal_active_hi << 8) | dtb->horizontal_active_lo;
}

/**
 * Extract vertical blanking from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Vertical blanking in lines.
 */
static inline u16
xvidc_edid_detailed_timing_vertical_blanking
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->vertical_blanking_hi << 8) | dtb->vertical_blanking_lo;
}

/**
 * Extract vertical active lines from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Vertical active lines.
 */
static inline u16
xvidc_edid_detailed_timing_vertical_active
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->vertical_active_hi << 8) | dtb->vertical_active_lo;
}

/**
 * Extract vertical sync offset from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Vertical sync offset in lines.
 */
static inline u8
xvidc_edid_detailed_timing_vertical_sync_offset
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->vertical_sync_offset_hi << 4) | dtb->vertical_sync_offset_lo;
}

/**
 * Extract vertical sync pulse width from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Vertical sync pulse width in lines.
 */
static inline u8
xvidc_edid_detailed_timing_vertical_sync_pulse_width
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->vertical_sync_pulse_width_hi << 4) |
                                             dtb->vertical_sync_pulse_width_lo;
}

/**
 * Extract horizontal sync offset from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Horizontal sync offset in pixels.
 */
static inline u8
xvidc_edid_detailed_timing_horizontal_sync_offset
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->horizontal_sync_offset_hi << 4) |
                                                dtb->horizontal_sync_offset_lo;
}

/**
 * Extract horizontal sync pulse width from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Horizontal sync pulse width in pixels.
 */
static inline u8
xvidc_edid_detailed_timing_horizontal_sync_pulse_width
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->horizontal_sync_pulse_width_hi << 4) |
                                           dtb->horizontal_sync_pulse_width_lo;
}

/**
 * Extract horizontal image size from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Horizontal image size in millimeters.
 */
static inline u16
xvidc_edid_detailed_timing_horizontal_image_size
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return
          (dtb->horizontal_image_size_hi << 8) | dtb->horizontal_image_size_lo;
}

/**
 * Extract vertical image size from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Vertical image size in millimeters.
 */
static inline u16
xvidc_edid_detailed_timing_vertical_image_size
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->vertical_image_size_hi << 8) | dtb->vertical_image_size_lo;
}

/**
 * Extract stereo mode from EDID detailed timing descriptor.
 *
 * @param dtb Pointer to detailed timing descriptor.
 * @return Stereo mode value.
 */
static inline u8
xvidc_edid_detailed_timing_stereo_mode
            (const struct xvidc_edid_detailed_timing_descriptor * const dtb)
{
    return (dtb->stereo_mode_hi << 2 | dtb->stereo_mode_lo);
}

/**
 * EDID monitor descriptor structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_monitor_descriptor {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_monitor_descriptor {
#endif
    u16 flag0;      /**< Flag byte 0 (should be 0x0000) */
    u8  flag1;      /**< Flag byte 1 (should be 0x00) */
    u8  tag;        /**< Descriptor type tag */
    u8  flag2;      /**< Flag byte 2 (should be 0x00) */
    u8  data[13];   /**< Descriptor data payload */
};

/**
 * Monitor descriptor string type (14 characters including null terminator).
 */
typedef char xvidc_edid_monitor_descriptor_string
            [sizeof(((struct xvidc_edid_monitor_descriptor *)0)->data) + 1];

/**
 * EDID monitor range limits structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_monitor_range_limits {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_monitor_range_limits {
#endif
    u8  minimum_vertical_rate;             /**< Minimum vertical rate in Hz */
    u8  maximum_vertical_rate;             /**< Maximum vertical rate in Hz */
    u8  minimum_horizontal_rate;           /**< Minimum horizontal rate in kHz */
    u8  maximum_horizontal_rate;           /**< Maximum horizontal rate in kHz */
    u8  maximum_supported_pixel_clock;     /**< Maximum pixel clock = (value * 10) MHz
                                                   (round to 10 MHz) */

    /** Secondary timing formula */
    u8  secondary_timing_support;          /**< Secondary timing formula support type */
    u8  reserved;                          /**< Reserved byte (0x00) */
    u8  secondary_curve_start_frequency;   /**< Horizontal frequency / 2 kHz */
    u8  c;                                 /**< C parameter = (value >> 1) */
    u16 m;                                 /**< M parameter for secondary timing */
    u8  k;                                 /**< K parameter for secondary timing */
    u8  j;                                 /**< J parameter = (value >> 1) */
};

/**
 * EDID standard timing descriptor structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_standard_timing_descriptor {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_standard_timing_descriptor {
#endif
    u8  horizontal_active_pixels;         /**< Horizontal active = (value + 31) * 8 */

    unsigned refresh_rate       : 6;           /**< Refresh rate = value + 60 */
    unsigned image_aspect_ratio : 2;           /**< Image aspect ratio code */
};

/**
 * Calculate horizontal active pixels from EDID standard timing descriptor.
 *
 * @param desc Pointer to standard timing descriptor.
 * @return Horizontal active pixels.
 */
inline u32
xvidc_edid_standard_timing_horizontal_active
(const struct xvidc_edid_standard_timing_descriptor * const desc) {
    return ((desc->horizontal_active_pixels + 31) << 3);
}

/**
 * Calculate vertical active lines from EDID standard timing descriptor.
 *
 * @param desc Pointer to standard timing descriptor.
 * @return Vertical active lines.
 */
inline u32
xvidc_edid_standard_timing_vertical_active
(const struct xvidc_edid_standard_timing_descriptor * const desc) {
    const u32 hres = xvidc_edid_standard_timing_horizontal_active(desc);

    switch (desc->image_aspect_ratio) {
    case XVIDC_EDID_ASPECT_RATIO_16_10:
        return ((hres * 10) >> 4);
    case XVIDC_EDID_ASPECT_RATIO_4_3:
        return ((hres * 3) >> 2);
    case XVIDC_EDID_ASPECT_RATIO_5_4:
        return ((hres << 2) / 5);
    case XVIDC_EDID_ASPECT_RATIO_16_9:
        return ((hres * 9) >> 4);
    }

    return hres;
}

/**
 * Extract refresh rate from EDID standard timing descriptor.
 *
 * @param desc Pointer to standard timing descriptor.
 * @return Refresh rate in Hz.
 */
inline u32
xvidc_edid_standard_timing_refresh_rate
(const struct xvidc_edid_standard_timing_descriptor * const desc) {
    return (desc->refresh_rate + 60);
}

/**
 * Main EDID structure (128 bytes).
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) edid {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") edid {
#endif
    /** Header information */
    u8  header[8];                         /**< EDID header (00 FF FF FF FF FF FF 00) */

    /** Vendor/product identification */
    u16 manufacturer;                      /**< Manufacturer ID (compressed ASCII) */                      /**< Manufacturer ID (compressed ASCII) */
    union {
        u16 product_u16;                   /**< Product code as 16-bit value */
        u8  product[2];                    /**< Product code as byte array */
    };
    union {
        u32 serial_number_u32;             /**< Serial number as 32-bit value */
        u8  serial_number[4];              /**< Serial number as byte array */
    };
    u8  manufacture_week;                  /**< Week of manufacture (1-54, 0xFF=not specified) */
    u8  manufacture_year;                  /**< Manufacture year = value + 1990 */

    /** EDID version */
    u8  version;                           /**< EDID version number */
    u8  revision;                          /**< EDID revision number */

    /** Basic display parameters and features */
    union {
#if defined(__GNUC__)
        struct __attribute__ (( packed )) {
#elif defined(__ICCARM__)
	struct _Pragma ("pack()") {
#endif
            unsigned dfp_1x                 : 1;    /**< VESA DFP 1.x */
            unsigned                        : 6;    /**< Reserved bits */
            unsigned digital                : 1;    /**< Digital signal (1=digital, 0=analog) */
        } digital;
#if defined(__GNUC__)
        struct __attribute__ (( packed )) {
#elif defined(__ICCARM__)
	struct _Pragma ("pack()") {
#endif
            unsigned vsync_serration        : 1;    /**< VSync serration required */
            unsigned green_video_sync       : 1;    /**< Sync on green video supported */
            unsigned composite_sync         : 1;    /**< Composite sync supported */
            unsigned separate_sync          : 1;    /**< Separate sync supported */
            unsigned blank_to_black_setup   : 1;    /**< Blank-to-black setup expected */
            unsigned signal_level_standard  : 2;    /**< Video signal level standard */
            unsigned digital                : 1;    /**< Digital signal (1=digital, 0=analog) */
        } analog;
    } video_input_definition;

    u8  maximum_horizontal_image_size;     /**< Maximum horizontal image size in cm */
    u8  maximum_vertical_image_size;       /**< Maximum vertical image size in cm */

    u8  display_transfer_characteristics;  /**< Gamma = (value + 100) / 100 */

#if defined(__GNUC__)
    struct __attribute__ (( packed )) {
#elif defined(__ICCARM__)
    struct _Pragma ("pack()") {
#endif
        unsigned default_gtf                    : 1; /**< Generalised timing
                                                     formula */
        unsigned preferred_timing_mode          : 1; /**< Preferred timing mode specified */
        unsigned standard_default_color_space   : 1; /**< sRGB is default color space */
        unsigned display_type                   : 2; /**< Display color type */
        unsigned active_off                     : 1; /**< Active off/very low power supported */
        unsigned suspend                        : 1; /**< Suspend mode supported */
        unsigned standby                        : 1; /**< Standby mode supported */
    } feature_support;

    /** Color characteristics block */
    unsigned green_y_low    : 2;           /**< Green Y coordinate low bits */
    unsigned green_x_low    : 2;           /**< Green X coordinate low bits */
    unsigned red_y_low      : 2;           /**< Red Y coordinate low bits */
    unsigned red_x_low      : 2;           /**< Red X coordinate low bits */

    unsigned white_y_low    : 2;           /**< White Y coordinate low bits */
    unsigned white_x_low    : 2;           /**< White X coordinate low bits */
    unsigned blue_y_low     : 2;           /**< Blue Y coordinate low bits */
    unsigned blue_x_low     : 2;           /**< Blue X coordinate low bits */

    u8  red_x;                             /**< Red X coordinate high byte */
    u8  red_y;                             /**< Red Y coordinate high byte */
    u8  green_x;                           /**< Green X coordinate high byte */
    u8  green_y;                           /**< Green Y coordinate high byte */
    u8  blue_x;                            /**< Blue X coordinate high byte */
    u8  blue_y;                            /**< Blue Y coordinate high byte */
    u8  white_x;                           /**< White point X coordinate high byte */
    u8  white_y;                           /**< White point Y coordinate high byte */

    /** Established timings */
#if defined(__GNUC__)
    struct __attribute__ (( packed )) {
#elif defined(__ICCARM__)
    struct _Pragma ("pack()") {
#endif
        unsigned timing_800x600_60   : 1;  /**< 800x600 @ 60Hz */
        unsigned timing_800x600_56   : 1;  /**< 800x600 @ 56Hz */
        unsigned timing_640x480_75   : 1;  /**< 640x480 @ 75Hz */
        unsigned timing_640x480_72   : 1;  /**< 640x480 @ 72Hz */
        unsigned timing_640x480_67   : 1;  /**< 640x480 @ 67Hz */
        unsigned timing_640x480_60   : 1;  /**< 640x480 @ 60Hz */
        unsigned timing_720x400_88   : 1;  /**< 720x400 @ 88Hz */
        unsigned timing_720x400_70   : 1;  /**< 720x400 @ 70Hz */

        unsigned timing_1280x1024_75 : 1;  /**< 1280x1024 @ 75Hz */
        unsigned timing_1024x768_75  : 1;  /**< 1024x768 @ 75Hz */
        unsigned timing_1024x768_70  : 1;  /**< 1024x768 @ 70Hz */
        unsigned timing_1024x768_60  : 1;  /**< 1024x768 @ 60Hz */
        unsigned timing_1024x768_87  : 1;  /**< 1024x768 @ 87Hz (interlaced) */
        unsigned timing_832x624_75   : 1;  /**< 832x624 @ 75Hz */
        unsigned timing_800x600_75   : 1;  /**< 800x600 @ 75Hz */
        unsigned timing_800x600_72   : 1;  /**< 800x600 @ 72Hz */
    } established_timings;

#if defined(__GNUC__)
    struct __attribute__ (( packed )) {
#elif defined(__ICCARM__)
    struct _Pragma ("pack()") {
#endif
        unsigned reserved            : 7;  /**< Reserved manufacturer timings */
        unsigned timing_1152x870_75  : 1;  /**< 1152x870 @ 75Hz */
    } manufacturer_timings;

    /** Standard timing identification */
    struct  xvidc_edid_standard_timing_descriptor standard_timing_id[8];  /**< Standard timing descriptors array */

    /** Detailed timing descriptors */
    union {
        struct xvidc_edid_monitor_descriptor         monitor;  /**< Monitor descriptor */
        struct xvidc_edid_detailed_timing_descriptor timing;   /**< Detailed timing descriptor */
    } detailed_timings[4];  /**< Four 18-byte descriptor blocks */

    u8  extensions;         /**< Number of extension blocks */
    u8  checksum;           /**< Checksum (sum of all 128 bytes should equal 0) */
};

/**
 * Extract manufacturer name from EDID structure.
 *
 * @param edid Pointer to EDID structure.
 * @param manufacturer Output buffer for 4-character manufacturer name (3 chars + null terminator).
 */
static inline void
xvidc_edid_manufacturer(const struct edid * const edid, char manufacturer[4])
{
    manufacturer[0] = '@' + ((edid->manufacturer & 0x007c) >> 2);
    manufacturer[1] = '@' + ((((edid->manufacturer & 0x0003) >> 00) << 3) | (((edid->manufacturer & 0xe000) >> 13) << 0));
    manufacturer[2] = '@' + ((edid->manufacturer & 0x1f00) >> 8);
    manufacturer[3] = '\0';
}

/**
 * Extract gamma value from EDID structure.
 *
 * @param edid Pointer to EDID structure.
 * @return Gamma value as floating point.
 */
static inline double
xvidc_edid_gamma(const struct edid * const edid)
{
    return (edid->display_transfer_characteristics + 100) / 100.0;
}

/**
 * Check if detailed timing descriptor is actually a monitor descriptor.
 *
 * @param edid Pointer to EDID structure.
 * @param timing Index of detailed timing descriptor to check.
 * @return True if monitor descriptor, false otherwise.
 */
static inline bool
xvidc_edid_detailed_timing_is_monitor_descriptor(const struct edid * const edid,
                                           const u8 timing)
{
    const struct xvidc_edid_monitor_descriptor * const mon =
        &edid->detailed_timings[timing].monitor;

    Xil_AssertNonvoid(timing < ARRAY_SIZE(edid->detailed_timings));

    return mon->flag0 == 0x0000 && mon->flag1 == 0x00 && mon->flag2 == 0x00;
}

/**
 * EDID color characteristics data structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_color_characteristics_data {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_color_characteristics_data {
#endif
    struct {
        u16 x;  /**< X chromaticity coordinate */
        u16 y;  /**< Y chromaticity coordinate */
    } red,      /**< Red primary chromaticity */
      green,    /**< Green primary chromaticity */
      blue,     /**< Blue primary chromaticity */
      white;    /**< White point chromaticity */
};

static inline struct xvidc_edid_color_characteristics_data
xvidc_edid_color_characteristics(const struct edid * const edid)
{
    const struct xvidc_edid_color_characteristics_data characteristics = {
        .red = {
            .x = (edid->red_x << 2) | edid->red_x_low,
            .y = (edid->red_y << 2) | edid->red_y_low,
        },
        .green = {
            .x = (edid->green_x << 2) | edid->green_x_low,
            .y = (edid->green_y << 2) | edid->green_y_low,
        },
        .blue = {
            .x = (edid->blue_x << 2) | edid->blue_x_low,
            .y = (edid->blue_y << 2) | edid->blue_y_low,
        },
        .white = {
            .x = (edid->white_x << 2) | edid->white_x_low,
            .y = (edid->white_y << 2) | edid->white_y_low,
        },
    };

    return characteristics;
}

/**
 * EDID extension block map structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_block_map {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_block_map {
#endif
    u8 tag;                /**< Block map tag (0xF0) */
    u8 extension_tag[126]; /**< Extension block tags array */
    u8 checksum;           /**< Block checksum */
};

/**
 * EDID extension block structure.
 */
#if defined(__GNUC__)
struct __attribute__ (( packed )) xvidc_edid_extension {
#elif defined(__ICCARM__)
struct _Pragma ("pack()") xvidc_edid_extension {
#endif
    u8 tag;                 /**< Extension tag identifying the extension type */
    u8 revision;            /**< Revision number of the extension */
    u8 extension_data[125]; /**< Extension data payload */
    u8 checksum;            /**< Block checksum */
};


/**
 * Verify EDID block checksum.
 *
 * @param block Pointer to EDID block (128 bytes).
 * @return True if checksum is valid, false otherwise.
 */
static inline bool
xvidc_edid_verify_checksum(const u8 * const block)
{
    u8 checksum = 0;
    int i;

    for (i = 0; i < XVIDC_EDID_BLOCK_SIZE; i++)
        checksum += block[i];

    return (checksum == 0);
}

/**
 * Decode EDID fixed-point value to floating point.
 *
 * @param value Fixed-point value (10-bit fraction).
 * @return Decoded floating point value.
 */
static inline double
xvidc_edid_decode_fixed_point(u16 value)
{
    double result = 0.0;

    Xil_AssertNonvoid((~value & 0xfc00) == 0xfc00);
                                                 /* edid fraction is 10 bits */

    for (u8 i = 0; value && (i < 10); i++, value >>= 1)
        result = result + ((value & 0x1) * (1.0 / (1 << (10 - i))));

    return result;
}

/**
 * Verbosity control enumeration.
 */
typedef enum {
    XVIDC_VERBOSE_DISABLE,  /**< Disable verbose output */
    XVIDC_VERBOSE_ENABLE    /**< Enable verbose output */
} XV_VidC_Verbose;

/**
 * HDMI interface type enumeration.
 */
typedef enum {
    XVIDC_ISDVI,   /**< DVI interface */
    XVIDC_ISHDMI   /**< HDMI interface */
} XV_VidC_IsHdmi;

/**
 * Feature support enumeration.
 */
typedef enum {
    XVIDC_NOT_SUPPORTED,  /**< Feature not supported */
    XVIDC_SUPPORTED       /**< Feature supported */
} XV_VidC_Supp;
#if XVIDC_EDID_VERBOSITY > 1
/**
 * Double precision representation structure (integer and decimal parts).
 */
typedef struct {
	u32 Integer;  /**< Integer part of the value */
	u32 Decimal;  /**< Decimal part of the value */
} XV_VidC_DoubleRep;
#endif

/**
 * Picture aspect ratio structure.
 */
typedef struct {
    u8 width;  /**< Aspect ratio width component */
    u8 height; /**< Aspect ratio height component */
} XV_VidC_PicAspectRatio;

/**
 * Video timing parameters structure.
 */
typedef struct {
    u16 hres;              /**< Horizontal resolution in pixels */
    u16 vres;              /**< Vertical resolution in pixels */
    u16 htotal;            /**< Total horizontal pixels (active + blanking) */
    u16 vtotal;            /**< Total vertical lines (active + blanking) */
    XVidC_VideoFormat vidfrmt; /**< Video format (progressive/interlaced) */
    u32 pixclk;            /**< Pixel clock in Hz */
    u16 hsync_width;       /**< Horizontal sync pulse width */
    u16 vsync_width;       /**< Vertical sync pulse width */
    u16 hfp;               /**< Horizontal front porch */
    u16 vfp;               /**< Vertical front porch */
    u8 vfreq;              /**< Vertical refresh frequency in Hz */
    XV_VidC_PicAspectRatio aspect_ratio; /**< Picture aspect ratio */
    unsigned hsync_polarity : 1; /**< Horizontal sync polarity (0=negative, 1=positive) */
    unsigned vsync_polarity : 1; /**< Vertical sync polarity (0=negative, 1=positive) */
} XV_VidC_TimingParam;

/**
 * EDID control parameter structure containing parsed EDID data.
 */
typedef struct {
	/** Checks whether Sink is able to support HDMI */
	XV_VidC_IsHdmi IsHdmi;                         /**< HDMI support flag */
	/** Color Space Support */
    XV_VidC_Supp   IsYCbCr444Supp;                 /**< YCbCr 4:4:4 color space support */
    XV_VidC_Supp   IsYCbCr420Supp;                 /**< YCbCr 4:2:0 color space support */
    XV_VidC_Supp   IsYCbCr422Supp;                 /**< YCbCr 4:2:2 color space support */
	/** YCbCr444/YCbCr422/RGB444 Deep Color Support */
    XV_VidC_Supp   IsYCbCr444DeepColSupp;          /**< YCbCr 4:4:4/4:2:2/RGB 4:4:4 deep color support */
    XV_VidC_Supp   Is30bppSupp;                    /**< 30 bits per pixel support */
    XV_VidC_Supp   Is36bppSupp;                    /**< 36 bits per pixel support */
    XV_VidC_Supp   Is48bppSupp;                    /**< 48 bits per pixel support */
	/** YCbCr420 Deep Color Support */
    XV_VidC_Supp   IsYCbCr420dc30bppSupp;          /**< YCbCr 4:2:0 deep color 30bpp support */
    XV_VidC_Supp   IsYCbCr420dc36bppSupp;          /**< YCbCr 4:2:0 deep color 36bpp support */
    XV_VidC_Supp   IsYCbCr420dc48bppSupp;          /**< YCbCr 4:2:0 deep color 48bpp support */
	/** SCDC and SCDC ReadRequest Support */
    XV_VidC_Supp   IsSCDCReadRequestReady;         /**< SCDC read request ready support */
    XV_VidC_Supp   IsSCDCPresent;                  /**< SCDC present flag */
	/** Sink Capability Support */
    u8             MaxFrameRateSupp;               /**< Maximum frame rate supported */
    u16            MaxTmdsMhz;                     /**< Maximum TMDS clock in MHz */
    u8             MaxFrlRateSupp;                 /**< Maximum FRL rate supported */
    u8             MaxFrlLineRateSupp;             /**< Maximum FRL line rate supported */
    u8             MaxFrlLanesSupp;                /**< Maximum FRL lanes supported */
	/** CEA 861 Supported VIC Support */
    u8             SuppCeaVIC[32];                 /**< Supported CEA VIC codes array */
	/** VESA Sink Preferred Timing Support */
    XV_VidC_TimingParam PreferedTiming[4];         /**< Preferred timing modes array */
    XV_VidC_Supp	Is3dOsdDisparitySupp;          /**< 3D OSD disparity support */
    XV_VidC_Supp	IsDualViewSupp;                /**< Dual view support */
    XV_VidC_Supp	IsIndependentViewSupp;         /**< Independent view support */
    XV_VidC_Supp	IsLte340McscScamble;           /**< LTE 340 Mcsc scramble support */
    XV_VidC_Supp	IsCCBPCISupp;                  /**< CCBPCI support */
    XV_VidC_Supp	IsCableAssemblyStatusSupp;     /**< Cable assembly status support */
    XV_VidC_Supp	IsFapaStartLocationSupp;       /**< FAPA start location support */
    XV_VidC_Supp	IsAllmSupp;                    /**< ALLM (Auto Low Latency Mode) support */
    XV_VidC_Supp	IsfavSupp;                     /**< FAV support */
    XV_VidC_Supp	IsQmsVrrSupp;                  /**< QMS VRR support */
    XV_VidC_Supp	IsDsc10bpcSupp;                /**< DSC 10 bits per component support */
    XV_VidC_Supp	IsDsc12bpcSupp;                /**< DSC 12 bits per component support */
    XV_VidC_Supp	IsDsc16bpcSupp;                /**< DSC 16 bits per component support */
    XV_VidC_Supp	IsFapaEndExtended;             /**< FAPA end extended support */
    XV_VidC_Supp	IsDscNativeYCbCr420Supp;       /**< DSC native YCbCr 4:2:0 support */
    XV_VidC_Supp	IsVesaDsc12aSupp;              /**< VESA DSC 1.2a support */
    u8			DscTotalChunkBytes;                /**< DSC total chunk bytes */
    u8			Extensions;                        /**< Number of EDID extensions */
    /** DisplayID/EDID 2.0 Support */
    XV_VidC_Supp	IsDispIdPresent;               /**< DisplayID present flag */
    u8			DispIdVersion;                /**< DisplayID version number */
    u8			DispIdProductType;            /**< DisplayID product type */
    u8			DispIdExtensionCount;         /**< DisplayID extension count */
    /** Product Identification */
    u16			ManufacturerId;               /**< Manufacturer ID from base EDID */
    u8			ManufactureWeek;              /**< Manufacture week from base EDID */
    u16			ManufactureYear;              /**< Manufacture year from base EDID */
    u8			DispIdManufacturer[4];        /**< DisplayID manufacturer code */
    u16			DispIdProductCode;            /**< DisplayID product code */
    u32			DispIdSerialNumber;           /**< DisplayID serial number */
    u16			DispIdModelYear;              /**< DisplayID model year */
    u8			DispIdModelWeek;              /**< DisplayID model week */
    u8			DispIdProductString[32];      /**< DisplayID product string */
    u8			DispIdProductStringLen;       /**< DisplayID product string length */
    /** Display Parameters */
    u16			DispIdImageWidthMm;           /**< Image width in 0.1mm units */
    u16			DispIdImageHeightMm;          /**< Image height in 0.1mm units */
    u16			DispIdNativeWidth;            /**< Native horizontal resolution */
    u16			DispIdNativeHeight;           /**< Native vertical resolution */
    u8			DispIdFeatureSupportFlags;    /**< Feature support flags */
    u8			DispIdGamma;                  /**< Gamma value: (value + 100) / 100, 0xFF = not defined */
    u8			DispIdAspectRatio;            /**< Aspect ratio: (value + 100) / 100 */
    u8			DispIdBitDepthNative;         /**< Native bit depth */
    u8			DispIdBitDepthOverall;        /**< Overall bit depth */
    u8			DispIdScanOrientation;        /**< Scan orientation */
    u8			DispIdTechnology;             /**< Display technology type */
    XV_VidC_Supp	DispIdAudioSupport;           /**< Audio support flag */
    XV_VidC_Supp	DispIdSeparateAudio;          /**< Separate audio inputs flag */
    XV_VidC_Supp	DispIdAudioOverride;          /**< Audio input override flag */
    XV_VidC_Supp	DispIdPowerSequenceReq;       /**< Power sequencing required flag */
    XV_VidC_Supp	DispIdFixedPixelFormat;       /**< Fixed pixel format flag */
    XV_VidC_Supp	DispIdDeinterlacing;          /**< De-interlacing support flag */
    /** Color Characteristics */
    u8			DispIdColorDepth;             /**< Color depth */
    u8			DispIdColorEncoding;          /**< Color encoding format */
    /** DisplayID Interface Color Depth Support */
    u8			DispIdRgbColorDepth;          /**< RGB color depth support */
    u8			DispIdYCbCr444ColorDepth;     /**< YCbCr 4:4:4 color depth support */
    u8			DispIdYCbCr422ColorDepth;     /**< YCbCr 4:2:2 color depth support */
    u8			DispIdYCbCr420ColorDepth;     /**< YCbCr 4:2:0 color depth support */
    u16			DispIdPrimaryRedX;            /**< Red primary X coordinate */
    u16			DispIdPrimaryRedY;            /**< Red primary Y coordinate */
    u16			DispIdPrimaryGreenX;          /**< Green primary X coordinate */
    u16			DispIdPrimaryGreenY;          /**< Green primary Y coordinate */
    u16			DispIdPrimaryBlueX;           /**< Blue primary X coordinate */
    u16			DispIdPrimaryBlueY;           /**< Blue primary Y coordinate */
    u16			DispIdWhitePointX;            /**< White point X coordinate */
    u16			DispIdWhitePointY;            /**< White point Y coordinate */
    u16			DispIdMinLuminance;           /**< Minimum luminance */
    u16			DispIdMaxLuminance;           /**< Maximum luminance */
    u16			DispIdMaxFullFrameLum;        /**< Maximum full frame luminance */
    /** Timing Range Data (0x09) */
    u16			DispIdTimingRangeMinHfreq;    /**< Minimum horizontal frequency in kHz */
    u16			DispIdTimingRangeMaxHfreq;    /**< Maximum horizontal frequency in kHz */
    u16			DispIdTimingRangeMinVfreq;    /**< Minimum vertical frequency in Hz */
    u16			DispIdTimingRangeMaxVfreq;    /**< Maximum vertical frequency in Hz */
    u16			DispIdTimingRangeMaxPixclk;   /**< Maximum pixel clock */
    u8			DispIdTimingRangeFlags;       /**< Timing range flags */
    /** Serial Number String (0x0A) */
    u8			DispIdSerialString[256];      /**< Serial number string */
    u16			DispIdSerialStringLen;        /**< Serial string length */
    /** ASCII String (0x0B) */
    u8			DispIdAsciiString[256];       /**< ASCII string */
    u16			DispIdAsciiStringLen;         /**< ASCII string length */
    /** Device Data (0x0C) */
    u8			DispIdDeviceType;             /**< Device type */
    u8			DispIdDeviceColorSpace;       /**< Device color space */
    u16			DispIdDeviceHorSize;          /**< Device horizontal size */
    u16			DispIdDeviceVerSize;          /**< Device vertical size */
    /** Power Sequencing (0x0D) */
    u8			DispIdPowerSeqT1;             /**< Power sequencing T1 timing */
    u8			DispIdPowerSeqT2;             /**< Power sequencing T2 timing */
    u8			DispIdPowerSeqT3;             /**< Power sequencing T3 timing */
    u8			DispIdPowerSeqT4;             /**< Power sequencing T4 timing */
    /** Transfer Characteristics (0x0E) */
    u8			DispIdTransferType;           /**< Transfer characteristics type */
    u8			DispIdTransferFlags;          /**< Transfer characteristics flags */
    u8			DispIdOecfEotfData[2];        /**< OECF/EOTF data */
    /** DisplayID short timing descriptor structure */
    struct {
        u16 HActive;                          /**< Horizontal active pixels */
        u8  RefreshRate;                      /**< Refresh rate in Hz */
        u8  AspectRatio;                      /**< Aspect ratio */
    } DispIdShortTiming[16];              /**< DisplayID short timing descriptors array */
    u8			DispIdNumShortTimings;        /**< Number of short timings */
    /** DMT Timings */
    u8			DispIdDmtTiming[16];          /**< DMT timing codes */
    u8			DispIdNumDmtTimings;          /**< Number of DMT timings */
    /** DisplayID CVT timing descriptor structure */
    struct {
        u16 HActive;                          /**< Horizontal active pixels */
        u8  RefreshRate;                      /**< Refresh rate in Hz */
    } DispIdCvtTiming[8];                 /**< DisplayID CVT timing descriptors array */
    u8			DispIdNumCvtTimings;          /**< Number of CVT timings */
    /** Video Timing Modes */
    XV_VidC_TimingParam DispIdTiming[8];     /**< DisplayID timing modes */
    u8			DispIdNumTimings;             /**< Number of timing modes */
    /** Supported Timing Modes */
    u8			DispIdType6Timing[8];         /**< Type 6 timing codes */
    u8			DispIdNumType6Timings;        /**< Number of type 6 timings */
    u8			DispIdType7Timing[8];         /**< Type 7 timing codes */
    u8			DispIdNumType7Timings;        /**< Number of type 7 timings */
    u8			DispIdType8Timing[8];         /**< Type 8 timing codes */
    u8			DispIdNumType8Timings;        /**< Number of type 8 timings */
    u8			DispIdType9Timing[8];         /**< Type 9 timing codes */
    u8			DispIdNumType9Timings;        /**< Number of type 9 timings */
    u8			DispIdType10Timing[8];        /**< Type 10 timing codes */
    u8			DispIdNumType10Timings;       /**< Number of type 10 timings */
    /** Display Interface Features */
    u8			DispIdInterfaceType;          /**< Interface type */
    u8			DispIdNumLanes;               /**< Number of lanes */
    u8			DispIdInterfaceVersion;       /**< Interface version */
    u8			DispIdContentProtection;      /**< Content protection type */
    u8			DispIdSpreadSpectrum;         /**< Spread spectrum support */
    u8			DispIdColorFormats;           /**< Supported color formats */
    u16			DispIdMinPixelClkMhz;         /**< Minimum pixel clock in MHz */
    u16			DispIdMaxPixelClkMhz;         /**< Maximum pixel clock in MHz */
    /** Extended Interface Features */
    u8			DispIdNumInterfaces;          /**< Number of interfaces */
    u8			DispIdPixelClockRatio;        /**< Pixel clock ratio */
    u16			DispIdMaxPixelClock;          /**< Maximum pixel clock */
    /** Display Parameters Extended */
    u8			DispIdDynamicBpcNative;       /**< Dynamic native bits per color */
    u8			DispIdDynamicBpcOverall;      /**< Dynamic overall bits per color */
    /** Stereo Display Interface */
    u8			DispIdStereoInterface;        /**< Stereo interface type */
    u8			DispIdStereoPolarity;         /**< Stereo polarity */
    /** Tiled Display Topology */
    XV_VidC_Supp	DispIdIsTiled;                /**< Tiled display flag */
    u8			DispIdTileRows;               /**< Number of tile rows */
    u8			DispIdTileCols;               /**< Number of tile columns */
    u8			DispIdTileLocH;               /**< Tile horizontal location */
    u8			DispIdTileLocV;               /**< Tile vertical location */
    u16			DispIdTileWidth;              /**< Tile width in pixels */
    u16			DispIdTileHeight;             /**< Tile height in pixels */
    u8			DispIdTileBezelTop;           /**< Top bezel thickness */
    u8			DispIdTileBezelBottom;        /**< Bottom bezel thickness */
    u8			DispIdTileBezelLeft;          /**< Left bezel thickness */
    u8			DispIdTileBezelRight;         /**< Right bezel thickness */
    u8			DispIdTileCapabilities;       /**< Tile capabilities flags */
    u8			DispIdTileSerialNum[16];      /**< Tile serial number */
    /** Container ID */
    u8			DispIdContainerId[16];        /**< Container ID */
    /** Vendor Specific */
    u8			DispIdVendorTag[3];           /**< Vendor tag */
    u8			DispIdVendorData[28];         /**< Vendor specific data */
    u8			DispIdVendorDataLen;          /**< Vendor data length */
    /** CTA DisplayID Data Block */
    XV_VidC_Supp	DispIdCtaPresent;             /**< CTA data block present flag */
    u8			DispIdCtaVic[16];             /**< CTA VIC codes */
    u8			DispIdNumCtaVic;              /**< Number of CTA VIC codes */
    u8			DispIdCtaBlockTag;            /**< CTA block tag */
    u8			DispIdCtaBlockPayloadSize;    /**< CTA block payload size */
    /** Adaptive Refresh Rate */
    u8			DispIdAdaptiveRefreshFlags;   /**< Adaptive refresh rate flags */
    /** Unicode String */
    u8			DispIdUnicodeString[256];     /**< Unicode string */
    u8			DispIdUnicodeStringLen;       /**< Unicode string length */
    /** Detailed Timing Type 7 */
    u8			DispIdDetailedTiming7Data[32];/**< Detailed timing type 7 data */
    u8			DispIdDetailedTiming7Len;     /**< Detailed timing type 7 length */
} XV_VidC_EdidCntrlParam;

/**
 * Convert EDID detailed timing descriptor to video timing parameters.
 *
 * @param dtb Pointer to detailed timing descriptor structure.
 * @return Video timing parameter structure.
 */
XV_VidC_TimingParam
XV_VidC_timing
           (const struct xvidc_edid_detailed_timing_descriptor * const dtb);
#if XVIDC_EDID_VERBOSITY > 1
/**
 * Convert double precision value to integer representation.
 *
 * @param in_val Input double precision value.
 * @return Double representation structure with integer and decimal parts.
 */
XV_VidC_DoubleRep Double2Int (double in_val);
#endif
/**
 * Initialize EDID control parameter structure with default values.
 *
 * @param EdidCtrlParam Pointer to EDID control parameter structure to initialize.
 */
void XV_VidC_EdidCtrlParamInit (XV_VidC_EdidCntrlParam *EdidCtrlParam);

/**
 * Parse EDID data and populate control parameter structure.
 *
 * @param data Pointer to raw EDID data.
 * @param EdidCtrlParam Pointer to EDID control parameter structure to populate.
 * @param VerboseEn Verbosity enable flag for debug output.
 */
void
XV_VidC_parse_edid(const u8 * const data,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn);

/**
 * Parse EDID extension block data.
 *
 * @param data Pointer to EDID extension block data.
 * @param EdidCtrlParam Pointer to EDID control parameter structure to update.
 * @param VerboseEn Verbosity enable flag for debug output.
 * @param SegmentNum Extension block segment number.
 */
void
XV_VidC_parse_edid_extension(const u8 * const data,
                  XV_VidC_EdidCntrlParam *EdidCtrlParam,
                  XV_VidC_Verbose VerboseEn, u8 SegmentNum);

#ifdef __cplusplus
}
#endif
#endif

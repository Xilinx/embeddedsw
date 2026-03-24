/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_edid_parser.h
* @addtogroup dptxss Overview
* @{
*
* This header file contains declarations for EDID timing parsing functions
* that extract video timings from EDID/DisplayID structures in priority order.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------
* 1.0   amd  03/18/26 Initial release
*
* </pre>
*
******************************************************************************/

#ifndef XVIDC_EDID_PARSER_H_
#define XVIDC_EDID_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xvidc_edid_ext.h"
#include "xdptxss.h"

/************************** Constant Definitions *****************************/

/** Maximum number of timings to return */
#define EDID_MAX_TIMINGS 15

/** @def EDID_DTD_SIZE
 *  @brief EDID Detailed Timing Descriptor size in bytes (18 bytes) */
#define EDID_DTD_SIZE 18

/** EDID parsing limits */
#define MAX_EXTENSIONS_SUPPORTED 16  /**< Maximum EDID extension blocks to process */
#define MAX_SAD_COUNT           32  /**< Maximum Short Audio Descriptors to store */

/** @def CTA861_EXTENSION_TAG
 *  @brief CTA-861 extension block tag (0x02) for audio/video capability data */
#define CTA861_EXTENSION_TAG 		0x02
/** @def DISPLAYID_2_0_EXTENSION_TAG
 *  @brief DisplayID 2.0 extension block tag (0x70) for detailed timing data */
#define DISPLAYID_2_0_EXTENSION_TAG     0x70
/** @def DISPLAYID_TYPE_VII_TAG
 *  @brief DisplayID Type VII timing descriptor tag (0x22) */
#define DISPLAYID_TYPE_VII_TAG          0x22
/** @def CTA861_AUDIO_DATA_BLOCK
 *  @brief CTA-861 audio data block tag (0x01) containing Short Audio Descriptors */
#define CTA861_AUDIO_DATA_BLOCK         0x01
/** @def CTA861_SPKR_ALLOC_DATA_BLOCK
 *  @brief CTA-861 speaker allocation data block tag (0x04) for channel mapping */
#define CTA861_SPKR_ALLOC_DATA_BLOCK    0x04

/** Timing Source Definitions */
#define TIMING_SOURCE_DTD          0  /**< Detailed Timing Descriptor */
#define TIMING_SOURCE_STANDARD     1  /**< Standard Timing */
#define TIMING_SOURCE_ESTABLISHED  2  /**< Established Timing */

/* Short Audio Descriptor (SAD) bit field definitions */
/** @def SAD_AUDIO_FORMAT_SHIFT
 *  @brief Bit shift value (3) to extract audio format code from SAD byte 0 */
#define SAD_AUDIO_FORMAT_SHIFT		3
/** @def SAD_AUDIO_FORMAT_MASK
 *  @brief Bitmask (0x1F) for audio format code field in SAD byte 0 (bits 3-7) */
#define SAD_AUDIO_FORMAT_MASK		0x1F
/** @def SAD_MAX_CHANNELS_MASK
 *  @brief Bitmask (0x07) for maximum channel count in SAD byte 0 (bits 0-2) */
#define SAD_MAX_CHANNELS_MASK		0x07
/** @def SAD_SAMPLING_FREQ_MASK
 *  @brief Bitmask (0x7F) for supported sampling frequencies in SAD byte 1 */
#define SAD_SAMPLING_FREQ_MASK		0x7F
/** @def SAD_BIT_DEPTH_MASK
 *  @brief Bitmask (0x07) for LPCM bit depth flags in SAD byte 2 (16/20/24-bit) */
#define SAD_BIT_DEPTH_MASK		0x07
/** @def SAD_MAX_BITRATE_MASK
 *  @brief Bitmask (0xFF) for maximum bitrate in SAD byte 2 (compressed formats) */
#define SAD_MAX_BITRATE_MASK		0xFF

/************************** Type Definitions *********************************/
/**
 * @brief DisplayID Type VII timing categories for priority-based parsing
 */
typedef enum {
	DISPLAYID_TYPE7_PREFERRED = 0,  /**< Preferred timing (Options bit 7 set) */
	DISPLAYID_TYPE7_NATIVE = 1,     /**< Native timing (Options bit 6 set, bit 7 clear) */
	DISPLAYID_TYPE7_REMAINING = 2   /**< Other timings (both bits clear) */
} XDpTxSs_DisplayIdType7Category;


/**
 * EDID Parse Result Structure
 * Contains detailed information about the parsing operation including
 * timing count, block-level validation statistics
 */
typedef struct {
	u8 TotalBlockCount;           /**< Total blocks of this type found */
	u8 InvalidBlockCount;         /**< Blocks with checksum failures */
} XDpTxSs_ParseResult;

/**
 * Structure to store video timing parameters with pixel clock.
 * This includes all timing fields
 */
typedef struct {
	u16 HActive;          /**< Horizontal active pixels */
	u16 HFrontPorch;      /**< Horizontal front porch pixels */
	u16 HSyncWidth;       /**< Horizontal sync width pixels */
	u16 HBackPorch;       /**< Horizontal back porch pixels */
	u16 HTotal;           /**< Horizontal total pixels */
	u8  HSyncPolarity;    /**< Horizontal sync polarity (0=negative, 1=positive) */
	u16 VActive;          /**< Vertical active lines */
	u16 F0PVFrontPorch;   /**< Field 0/Progressive vertical front porch lines */
	u16 F0PVSyncWidth;    /**< Field 0/Progressive vertical sync width lines */
	u16 F0PVBackPorch;    /**< Field 0/Progressive vertical back porch lines */
	u16 F0PVTotal;        /**< Field 0/Progressive vertical total lines */
	u16 F1VFrontPorch;    /**< Field 1 vertical front porch lines */
	u16 F1VSyncWidth;     /**< Field 1 vertical sync width lines */
	u16 F1VBackPorch;     /**< Field 1 vertical back porch lines */
	u16 F1VTotal;         /**< Field 1 vertical total lines */
	u8  VSyncPolarity;    /**< Vertical sync polarity (0=negative, 1=positive) */
	u64 PixelClockHz;     /**< Pixel clock in Hz (u64 to support high clocks > 4.29 GHz) */
	u8  TimingSource;     /**< Timing source: 0=DTD, 1=Standard, 2=Established */
	u8  AspectRatio;      /**< Aspect ratio (for Standard timings: 0=16:10, 1=4:3, 2=5:4, 3=16:9) */
	u8  RefreshRate;      /**< Refresh rate in Hz  */
} XDpTxSs_VideoTimingParam;


/**
 * Structure to store Short Audio Descriptor (SAD) information
 * from CTA-861 extension blocks (3 bytes per SAD)
 */
typedef struct
{
	u8 AudioFormatCode;	/**< Audio format code (0-15):
					 1=LPCM, 2=AC-3, 3=MPEG-1, 4=MP3,
					 5=MPEG-2, 6=AAC, 7=DTS, 8=ATRAC,
					 9=One-bit, 10=DD+, 11=DTS-HD,
					 12=MAT, 13=DST, 14=WMA Pro, 15=Extended */
	u8 MaxChannels;		/**< Maximum number of audio channels (1-8) */
	u8 SamplingFreqFlags;	/**< Bitmap of supported sampling frequencies:
					 Bit 0: 32 kHz, Bit 1: 44.1 kHz, Bit 2: 48 kHz,
					 Bit 3: 88.2 kHz, Bit 4: 96 kHz, Bit 5: 176.4 kHz,
					 Bit 6: 192 kHz */
	/**
	 * Format-dependent data from SAD byte 2.
	 * For LPCM (format code 1): Use BitDepthFlags for supported bit depths.
	 * For compressed formats (2-14): Use MaxBitrate for maximum bitrate in units of 8 kHz.
	 */
	union {
		u8 BitDepthFlags;	/**< For LPCM (format code 1):
					 Bit 0=16-bit, Bit 1=20-bit, Bit 2=24-bit */
		u8 MaxBitrate;		/**< For compressed formats: Max bitrate / 8 kHz */
	} FormatDependent;
} XDpTxSs_AudioSAD;

/**
 * Structure to store complete sink audio configuration
 * including SADs and speaker allocation
 */
typedef struct
{
	XDpTxSs_AudioSAD SAD_array[MAX_SAD_COUNT];	/**< Fixed array of SADs (no dynamic allocation) */
	u8 Speaker_channel_allocation;			/**< Speaker allocation from CTA-861 Speaker Allocation Data Block*/
	u8 SAD_count;					/**< Number of valid SADs in array (0 to MAX_SAD_COUNT) */
} XDpTxSs_SinkAudioCfg;

/**
 * Structure to store complete sink Audio/Video capabilities
 * Parsed from EDID base block, all CTA-861 extensions, and DisplayID blocks
 */
typedef struct
{
	XDpTxSs_SinkAudioCfg SinkAudioCfg;		/**< Audio configuration with all SADs from all CTA-861 blocks */
	u8 MaxBpc;					/**< Maximum bits per color supported (6/8/10/12/14/16) */
	u16 SupportedColorFormats;			/**< Bitmap of supported color formats:
							 Bit 0: RGB 4:4:4 (always 1)
							 Bit 1: YCbCr 4:4:4
							 Bit 2: YCbCr 4:2:2
							 Bit 3: YCbCr 4:2:0 */
	XDpTxSs_VideoTimingParam TimingList[EDID_MAX_TIMINGS];	/**< Array of video timings (max 15) */
	u8 TimingCount;					/**< Number of valid timings in TimingList (0 to EDID_MAX_TIMINGS) */

	u8 EdidBuffer[128 * MAX_EXTENSIONS_SUPPORTED];	/**< EDID buffer: base block + extensions (2048 bytes max) */
	u8 extensions_count;				/**< Actual number of extensions read (0 to MAX_EXTENSIONS_SUPPORTED-1) */
	u8 checksum_failures;				/**< Number of extension blocks with checksum failures (0 to extensions_count) */
} XDpTxSs_Sink_AV_Capabilities;

/************************** Function Prototypes ******************************/

XDpTxSs_ParseResult XDpTxSs_GetVideoTimingsFromEdid(XDpTxSs_Sink_AV_Capabilities *AVCaps);

int XDpTxSs_get_full_edid(XDpTxSs *InstancePtr, XDpTxSs_Sink_AV_Capabilities *AVCaps);

int XDpTxSs_get_sink_AV_Capabilities(XDpTxSs_Sink_AV_Capabilities *AVCaps);

void XDpTxSs_PrintVideoTiming(const XDpTxSs_VideoTimingParam *Timing, u8 Index);

#ifdef __cplusplus
}
#endif

#endif /* XVIDC_EDID_PARSER_H_ */
/** @} */

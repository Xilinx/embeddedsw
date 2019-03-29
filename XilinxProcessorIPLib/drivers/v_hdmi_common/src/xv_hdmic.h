/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xhdmic.h
 * @addtogroup hdmi_common_v1_0
 * @{
 * @details
 *
 * Contains common structures, definitions, macros, and utility functions that
 * are typically used by hdmi-related drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   EB   21/12/17 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XV_HDMIC_H_  /* Prevent circular inclusions by using protection macros. */
#define XV_HDMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/

#include "xil_types.h"
#include "xil_printf.h"
#include "xvidc.h"
#include "xil_assert.h"

/************************** Constant Definitions ******************************/
#define VICTABLE_SIZE 38
#define AUX_VSIF_TYPE 0x81
#define AUX_AVI_INFOFRAME_TYPE 0x82
#define AUX_GENERAL_CONTROL_PACKET_TYPE 0x3
#define AUX_AUDIO_INFOFRAME_TYPE 0x84

/****************************** Type Definitions ******************************/

/**
* This typedef contains Video identification information in tabular form.
*/
typedef struct {
	XVidC_VideoMode VmId;	/**< Video mode/Resolution ID */
	u8 Vic;			/**< Video Identification code */
} XHdmiC_VicTable;

typedef union {
    u32 Data;   /**< AUX header data field */
    u8 Byte[4]; /**< AUX header byte field */
} XHdmiC_AuxHeader;

typedef union {
    u32 Data[8];    /**< AUX data field */
    u8 Byte[32];    /**< AUX data byte field */
} XHdmiC_AuxData;

typedef struct {
	XHdmiC_AuxHeader Header; /**< AUX header field */
	XHdmiC_AuxData Data;     /**< AUX data field */
} XHdmiC_Aux;

typedef enum {
	XHDMIC_COLORSPACE_RGB,
	XHDMIC_COLORSPACE_YUV422,
	XHDMIC_COLORSPACE_YUV444,
	XHDMIC_COLORSPACE_YUV420,	/**< Version 3 AVI InfoFrame */
	XHDMIC_COLORSPACE_RESERVED,
	XHDMIC_COLORSPACE_IDO_DEFINED = 7,
} XHdmiC_Colorspace;

typedef enum {
	XHDMIC_ACTIVE_ASPECT_16_9_TOP = 2,
	XHDMIC_ACTIVE_ASPECT_14_9_TOP = 3,
	XHDMIC_ACTIVE_ASPECT_16_9_CENTER = 4,
	XHDMIC_ACTIVE_ASPECT_PICTURE = 8,
	XHDMIC_ACTIVE_ASPECT_4_3 = 9,
	XHDMIC_ACTIVE_ASPECT_16_9 = 10,
	XHDMIC_ACTIVE_ASPECT_14_9 = 11,
	XHDMIC_ACTIVE_ASPECT_4_3_SP_14_9 = 13,
	XHDMIC_ACTIVE_ASPECT_16_9_SP_14_9 = 14,
	XHDMIC_ACTIVE_ASPECT_16_9_SP_4_3 = 15
} XHdmiC_ActiveAspectRatio;

typedef enum {
	XHDMIC_BAR_INFO_NA,
	XHDMIC_BAR_INFO_VERT_BAR,
	XHDMIC_BAR_INFO_HORI_BAR,
	XHDMIC_BAR_INFO_VERT_AND_HORI_BAR
} XHdmiC_BarInfo;

typedef enum {
	XHDMIC_SCAN_INFO_NA,
	XHDMIC_SCAN_INFO_OVERSCAN,
	XHDMIC_SCAN_INFO_UNDERSCAN,
	XHDMIC_SCAN_INFO_RESERVED
} XHdmiC_ScanInfo;

typedef enum {
	XHDMIC_COLORIMETRY_NA,
	XHDMIC_COLORIMETRY_ITU_601,
	XHDMIC_COLORIMETRY_ITU709,
	XHDMIC_COLORIMETRY_EXTENDED
} XHdmiC_Colorimetry;

typedef enum {
	XHDMIC_PIC_ASPECT_RATIO_NA,
	XHDMIC_PIC_ASPECT_RATIO_4_3,
	XHDMIC_PIC_ASPECT_RATIO_16_9,
	XHDMIC_PIC_ASPECT_RATIO_RESERVED
} XHdmiC_PicAspectRatio;

typedef enum {
	XHDMIC_EXTENDED_COLORIMETRY_YCC_601,
	XHDMIC_EXTENDED_COLORIMETRY_YCC_709,
	XHDMIC_EXTENDED_COLORIMETRY_S_YCC_601,
	XHDMIC_EXTENDED_COLORIMETRY_ADOBE_YCC_601,
	XHDMIC_EXTENDED_COLORIMETRY_ADOBE_RGB,
	XHDMIC_EXTENDED_COLORIMETRY_RESERVED_1,
	XHDMIC_EXTENDED_COLORIMETRY_RESERVED_2,
	XHDMIC_EXTENDED_COLORIMETRY_RESERVED_3
} XHdmiC_ExtendedColorimetry;

typedef enum {
	XHDMIC_RGB_QUANTIZATION_RANGE_DEFAULT,
	XHDMIC_RGB_QUANTIZATION_RANGE_LIMITED,
	XHDMIC_RGB_QUANTIZATION_RANGE_FULL
} XHdmiC_RGBQuantizationRange;

typedef enum{
	XHDMIC_PIXEL_REPETITION_FACTOR_1,
	XHDMIC_PIXEL_REPETITION_FACTOR_2,
	XHDMIC_PIXEL_REPETITION_FACTOR_3,
	XHDMIC_PIXEL_REPETITION_FACTOR_4,
	XHDMIC_PIXEL_REPETITION_FACTOR_5,
	XHDMIC_PIXEL_REPETITION_FACTOR_6,
	XHDMIC_PIXEL_REPETITION_FACTOR_7,
	XHDMIC_PIXEL_REPETITION_FACTOR_8,
	XHDMIC_PIXEL_REPETITION_FACTOR_9,
	XHDMIC_PIXEL_REPETITION_FACTOR_10
} XHdmiC_PixelRepetitionFactor;

typedef enum {
	XHDMIC_YCC_QUANTIZATION_RANGE_LIMITED,
	XHDMIC_YCC_QUANTIZATION_RANGE_FULL,
	XHDMIC_YCC_QUANTIZATION_RANGE_RESERVED_1,
	XHDMIC_YCC_QUANTIZATION_RANGE_RESERVED_2
} XHdmiC_YccQuantizationRange;

typedef enum {
	XHDMIC_NON_UNIFORM_PICTURE_SCALING_NA,
	XHDMIC_NON_UNIFORM_PICTURE_SCALING_HORI,
	XHDMIC_NON_UNIFORM_PICTURE_SCALING_VERT,
	XHDMIC_NON_UNIFORM_PICTURE_SCALING_VERT_AND_HORI_SCALING
} XHdmiC_NonUniformPictureScaling;

typedef enum {
	XHDMIC_CONTENT_TYPE_GRAPHICS,
	XHDMIC_CONTENT_TYPE_PHOTO,
	XHDMIC_CONTENT_TYPE_CINEMA,
	XHDMIC_CONTENT_TYPE_GAME
} XHdmiC_ContentType;

typedef enum {
	XHDMIC_COLORDEPTH_NOT_INDICATED,
	XHDMIC_COLORDEPTH_24 = 4,
	XHDMIC_COLORDEPTH_30,
	XHDMIC_COLORDEPTH_36,
	XHDMIC_COLORDEPTH_48
} XHdmiC_ColorDepth;

typedef enum {
	XHDMIC_PIXELPACKINGPHASE_4,
	XHDMIC_PIXELPACKINGPHASE_1,
	XHDMIC_PIXELPACKINGPHASE_2,
	XHDMIC_PIXELPACKINGPHASE_3
} XHdmiC_PixelPackingPhase;

typedef enum {
	XHDMIC_SAMPLING_FREQUENCY,
	XHDMIC_SAMPLING_FREQUENCY_32K,
	XHDMIC_SAMPLING_FREQUENCY_44_1K,
	XHDMIC_SAMPLING_FREQUENCY_48K,
	XHDMIC_SAMPLING_FREQUENCY_88_2K,
	XHDMIC_SAMPLING_FREQUENCY_96K,
	XHDMIC_SAMPLING_FREQUENCY_176_4K,
	XHDMIC_SAMPLING_FREQUENCY_192K
} XHdmiC_SamplingFrequency;

typedef enum {
	XHDMIC_SAMPLE_SIZE,
	XHDMIC_SAMPLE_SIZE_16,
	XHDMIC_SAMPLE_SIZE_20,
	XHDMIC_SAMPLE_SIZE_24
} XHdmiC_SampleSize;

typedef enum {
	XHDMIC_AUDIO_CODING_TYPE,
	XHDMIC_AUDIO_CODING_TYPE_PCM,
	XHDMIC_AUDIO_CODING_TYPE_AC3,
	XHDMIC_AUDIO_CODING_TYPE_MPEG1,
	XHDMIC_AUDIO_CODING_TYPE_MP3,
	XHDMIC_AUDIO_CODING_TYPE_MPEG2,
	XHDMIC_AUDIO_CODING_TYPE_AAC,
	XHDMIC_AUDIO_CODING_TYPE_DTS,
	XHDMIC_AUDIO_CODING_TYPE_ATRAC,
	XHDMIC_AUDIO_CODING_TYPE_ONE_BIT_AUDIO,
	XHDMIC_AUDIO_CODING_TYPE_DOLBY_DIGITAL_PLUS,
	XHDMIC_AUDIO_CODING_TYPE_DTS_HD,
	XHDMIC_AUDIO_CODING_TYPE_MAT,
	XHDMIC_AUDIO_CODING_TYPE_DST,
	XHDMIC_AUDIO_CODING_TYPE_WMA_PRO
} XHdmiC_AudioCodingType;

typedef enum {
	XHDMIC_AUDIO_CHANNEL_COUNT,
	XHDMIC_AUDIO_CHANNEL_COUNT_2,
	XHDMIC_AUDIO_CHANNEL_COUNT_3,
	XHDMIC_AUDIO_CHANNEL_COUNT_4,
	XHDMIC_AUDIO_CHANNEL_COUNT_5,
	XHDMIC_AUDIO_CHANNEL_COUNT_6,
	XHDMIC_AUDIO_CHANNEL_COUNT_7,
	XHDMIC_AUDIO_CHANNEL_COUNT_8
} XHdmiC_AudioChannelCount;

typedef enum {
	XHDMIC_LFEPBL,
	XHDMIC_LFEPBL_0,
	XHDMIC_LFEPBL_10,
} XHdmiC_LFEPlaybackLevel;

typedef enum {
	XHDMIC_LSV_0,
	XHDMIC_LSV_1,
	XHDMIC_LSV_2,
	XHDMIC_LSV_3,
	XHDMIC_LSV_4,
	XHDMIC_LSV_5,
	XHDMIC_LSV_6,
	XHDMIC_LSV_7,
	XHDMIC_LSV_8,
	XHDMIC_LSV_9,
	XHDMIC_LSV_10,
	XHDMIC_LSV_11,
	XHDMIC_LSV_12,
	XHDMIC_LSV_13,
	XHDMIC_LSV_14,
	XHDMIC_LSV_15
} XHdmiC_LevelShiftValue;

/**
 * This typedef contains the data structure for Auxiliary Video Information Info frame
 */
typedef struct XHDMIC_AVI_InfoFrame {
	unsigned char Version;
	XHdmiC_Colorspace ColorSpace;
	u8 ActiveFormatDataPresent;
	XHdmiC_BarInfo BarInfo;
	XHdmiC_ScanInfo ScanInfo;
	XHdmiC_Colorimetry Colorimetry;
	XHdmiC_PicAspectRatio PicAspectRatio;
	XHdmiC_ActiveAspectRatio ActiveAspectRatio;
	unsigned char Itc;
	XHdmiC_ExtendedColorimetry ExtendedColorimetry;
	XHdmiC_RGBQuantizationRange QuantizationRange;
	XHdmiC_NonUniformPictureScaling NonUniformPictureScaling;
	unsigned char VIC;
	XHdmiC_YccQuantizationRange YccQuantizationRange;
	XHdmiC_ContentType ContentType;
	XHdmiC_PixelRepetitionFactor PixelRepetition;
	u16 TopBar;
	u16 BottomBar;
	u16 LeftBar;
	u16 RightBar;
} XHdmiC_AVI_InfoFrame;

/**
 * This typedef contains the data structure for General Control Packet
 */
typedef struct XHdmiC_GCP_Packet {
	unsigned char Set_AVMUTE;
	unsigned char Clear_AVMUTE;
	XHdmiC_ColorDepth ColorDepth;
	XHdmiC_PixelPackingPhase PixelPackingPhase;
	unsigned char Default_Phase;
} XHdmiC_GeneralControlPacket;

/**
 * This typedef contains the data structure for Audio Infoframe
 */
typedef struct XHdmiC_Audio_InfoFrame {
	unsigned char Version;
	XHdmiC_AudioChannelCount ChannelCount;
	XHdmiC_AudioCodingType CodingType;
	XHdmiC_SampleSize SampleSize;
	XHdmiC_SamplingFrequency SampleFrequency;
	u8 CodingTypeExt;
	u8 ChannelAllocation;
	XHdmiC_LFEPlaybackLevel LFE_Playback_Level;
	XHdmiC_LevelShiftValue LevelShiftVal;
	unsigned char Downmix_Inhibit;
} XHdmiC_AudioInfoFrame;

/*************************** Variable Declarations ****************************/
extern const XHdmiC_VicTable VicTable[VICTABLE_SIZE];


/************************** Function Prototypes ******************************/

void XV_HdmiC_ParseAVIInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AVI_InfoFrame *infoFramePtr);
void XV_HdmiC_ParseGCP(XHdmiC_Aux *AuxPtr, XHdmiC_GeneralControlPacket *GcpPtr);
void XV_HdmiC_ParseAudioInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AudioInfoFrame *AudIFPtr);
XHdmiC_Aux XV_HdmiC_AVIIF_GeneratePacket(XHdmiC_AVI_InfoFrame *infoFramePtr);
XHdmiC_Aux XV_HdmiC_AudioIF_GeneratePacket(XHdmiC_AudioInfoFrame *AudioInfoFrame);
XHdmiC_Colorspace XV_HdmiC_XVidC_To_IfColorformat(XVidC_ColorFormat ColorFormat);
XVidC_AspectRatio XV_HdmiC_IFAspectRatio_To_XVidC(XHdmiC_PicAspectRatio AR);

#ifdef __cplusplus
}
#endif

#endif /* XHDMIC_H_ */
/** @} */

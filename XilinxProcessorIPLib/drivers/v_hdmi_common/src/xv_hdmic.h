/*******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmic.h
 * @addtogroup hdmi_common Overview
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
 * 1.2   EB   15/08/19 Added enumeration for HDMI 2.1 Support
 *       mmo  15/08/19 Updated the VIC table to support HDMI 2.1 Resolution
 *                     Added Audio ACR CTS/N Enumeration and Library
 * 1.3   EB   02/12/19 Added 3D Audio Enumerations and APIs
 * 1.6   kp   15/07/21 Updated the VIC table as per latest CTA spec
 *       kp   30/08/21 Increased the VIC table size
 * </pre>
 *
*******************************************************************************/

#ifndef XV_HDMIC_H_  /* Prevent circular inclusions by
                              using protection macros. */
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
#define VICTABLE_SIZE 166
#define AUX_VSIF_TYPE 0x81
#define AUX_AVI_INFOFRAME_TYPE 0x82
#define AUX_GENERAL_CONTROL_PACKET_TYPE 0x3
#define AUX_AUDIO_INFOFRAME_TYPE 0x84
#define AUX_AUDIO_METADATA_PACKET_TYPE 0x0D
#define AUX_SPD_INFOFRAME_TYPE 0x83
#define AUX_DRM_INFOFRAME_TYPE 0x87

/****************************** Type Definitions ******************************/

/**
 * This typedef contains the different background colors available
 */
typedef enum
{
  XV_BKGND_BLACK = 0,
  XV_BKGND_WHITE,
  XV_BKGND_RED,
  XV_BKGND_GREEN,
  XV_BKGND_BLUE,
  XV_BKGND_NOISE,
  XV_BKGND_LAST
}XVMaskColorId;

/**
* This typedef contains Video identification information in tabular form.
*/
typedef enum {
	XHDMIC_PIC_ASPECT_RATIO_NA,
	XHDMIC_PIC_ASPECT_RATIO_4_3,
	XHDMIC_PIC_ASPECT_RATIO_16_9,
	XHDMIC_PIC_ASPECT_RATIO_64_27,
	XHDMIC_PIC_ASPECT_RATIO_256_135,
	XHDMIC_PIC_ASPECT_RATIO_RESERVED
} XHdmiC_PicAspectRatio;

/**
* This typedef contains FrameRate code information as per the HDMI 2.1b spec.
*/
typedef enum {
	XHDMIC_FR_NO_DATA,
	XHDMIC_FR_23_98Hz,
	XHDMIC_FR_24Hz,
	XHDMIC_FR_25Hz,
	XHDMIC_FR_29_97Hz,
	XHDMIC_FR_30Hz,
	XHDMIC_FR_47_95Hz,
	XHDMIC_FR_48Hz,
	XHDMIC_FR_50Hz,
	XHDMIC_FR_59_94Hz,
	XHDMIC_FR_60Hz,
	XHDMIC_FR_100Hz,
	XHDMIC_FR_119_88Hz,
	XHDMIC_FR_120Hz,
	XHDMIC_FR_143_86Hz,
	XHDMIC_FR_144Hz,
	XHDMIC_FR_200Hz,
	XHDMIC_FR_239_76Hz,
	XHDMIC_FR_240Hz,
	XHDMIC_FR_300Hz,
	XHDMIC_FR_359_64Hz,
	XHDMIC_FR_360Hz,
	XHDMIC_FR_400Hz,
	XHDMIC_FR_479_52Hz,
	XHDMIC_FR_480Hz,
	XHDMIC_FR_RESERVED
} XHdmiC_FrameRateCode;

/**
* This typedef contains Resolution ID's information as per the HDMI 2.1b spec.
*/
typedef enum {
	XHDMIC_VM_1280x720P,
	XHDMIC_VM_1680x720P,
	XHDMIC_VM_1920x1080P,
	XHDMIC_VM_2560x1080P,
	XHDMIC_VM_3840x1080P,
	XHDMIC_VM_2560x1440P,
	XHDMIC_VM_3440x1440P,
	XHDMIC_VM_5120x1440P,
	XHDMIC_VM_3840x2160P,
	XHDMIC_VM_5120x2160P,
	XHDMIC_VM_7680x2160P,
	XHDMIC_VM_5120x2880P,
	XHDMIC_VM_6880x2880P,
	XHDMIC_VM_10240x2880P,
	XHDMIC_VM_7680x4320P,
	XHDMIC_VM_10240x4320P,
	XHDMIC_VM_15360x4320P,
	XHDMIC_VM_11520x6480P,
	XHDMIC_VM_15360x6480P,
	XHDMIC_VM_15360x8640P,
	XHDMIC_VM_20480x8640P
} XHdmiC_ResolutionID;

typedef struct {
	XVidC_VideoMode VmId;	/**< Video mode/Resolution ID */
	XHdmiC_PicAspectRatio PicAspectRatio;   /**< Picture Aspect Ratio */
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

typedef union {
    u32 Data[32];    /**< PPS data field */
    u8 Byte[128];    /**< PPS data byte field */
} XHdmiC_DscPpsData;

typedef enum {
	XHDMIC_COLORSPACE_RGB,
	XHDMIC_COLORSPACE_YUV422,
	XHDMIC_COLORSPACE_YUV444,
	XHDMIC_COLORSPACE_YUV420,	/**< Version 3 AVI InfoFrame */
	XHDMIC_COLORSPACE_RESERVED4,
	XHDMIC_COLORSPACE_RESERVED5,
	XHDMIC_COLORSPACE_RESERVED6,
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
	XHDMIC_ADD_EXTENDED_COLORIMETRY_P3D65,
	XHDMIC_ADD_EXTENDED_COLORIMETRY_P3DCI,
	XHDMIC_ADD_EXTENDED_COLORIMETRY_ICTCP,
	XHDMIC_ADD_EXTENDED_COLORIMETRY_STANDARD_RGB,
	XHDMIC_ADD_EXTENDED_COLORIMETRY_DEFAULT_RGB,
	XHDMIC_ADD_EXTENDED_COLORIMETRY_RESERVED
} XHdmiC_AdditionalColorimetryExtension;

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
	XHDMIC_SAMPLING_FREQ             = 0,
	XHDMIC_SAMPLING_FREQ_32K         = 32000,
	XHDMIC_SAMPLING_FREQ_64K         = 32000,
	XHDMIC_SAMPLING_FREQ_128K        = 128000,
	XHDMIC_SAMPLING_FREQ_256K        = 128000,
	XHDMIC_SAMPLING_FREQ_512K        = 128000,
	XHDMIC_SAMPLING_FREQ_1024K       = 1024000,

	XHDMIC_SAMPLING_FREQ_44_1K       = 44100,
	XHDMIC_SAMPLING_FREQ_88_2K       = 88200,
	XHDMIC_SAMPLING_FREQ_176_4K      = 176400,
	XHDMIC_SAMPLING_FREQ_352_8K      = 352800,
	XHDMIC_SAMPLING_FREQ_705_6K      = 705600,
	XHDMIC_SAMPLING_FREQ_1411_2K     = 1411200,

	XHDMIC_SAMPLING_FREQ_48K         = 48000,
	XHDMIC_SAMPLING_FREQ_96K         = 96000,
	XHDMIC_SAMPLING_FREQ_192K        = 192000,
	XHDMIC_SAMPLING_FREQ_384K        = 384000,
	XHDMIC_SAMPLING_FREQ_768K        = 768000,
	XHDMIC_SAMPLING_FREQ_1536K       = 1536000,
} XHdmiC_SamplingFrequencyVal;

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
* FRL Character Rate Enumeration
*/
typedef enum {
	R_166_667 = 0,	/*  3 Gbps */
	R_333_333,		/*  6 Gbps */
	R_444_444,		/*  8 Gbps */
	R_555_556,		/* 10 Gbps */
	R_666_667,		/* 12 Gbps */
	XHDMIC_FRLCHARRATE_SIZE,
} XHdmiC_FRLCharRate;

/**
* FRL Rate Enumeration
*/
typedef enum {
	XHDMIC_MAXFRLRATE_NOT_SUPPORTED,
	XHDMIC_MAXFRLRATE_3X3GBITSPS,
	XHDMIC_MAXFRLRATE_3X6GBITSPS,
	XHDMIC_MAXFRLRATE_4X6GBITSPS,
	XHDMIC_MAXFRLRATE_4X8GBITSPS,
	XHDMIC_MAXFRLRATE_4X10GBITSPS,
	XHDMIC_MAXFRLRATE_4X12GBITSPS,
	XHDMIC_MAXFRLRATE_SIZE,
	XHDMIC_MAXFRLRATE_TMDSONLY = XHDMIC_MAXFRLRATE_NOT_SUPPORTED
} XHdmiC_MaxFrlRate;

/**
* FRL CTS/N Value Table
*/
typedef struct {
	u32 ACR_CTSVal;
	u32 ACR_NVal[6];	/* multiply of 6 of the initial Sample Freq */
} XHdmiC_FRL_CTS_N_Val;

/**
* TMDS Char/N Value Table
*/
typedef struct {
  u32 TMDSCharRate;
  u32 ACR_NVal[7];
} XHdmiC_TMDS_N_Table;

/**
* This typedef contains translations of FRL_Rate to Lanes and Line Rates.
*/
typedef struct {
	u8 Lanes;		/**< No of Lanes */
	u8 LineRate;		/**< Line Rate */
} XHdmiC_FrlRate;

typedef enum {
	XHDMIC_SPD_UNKNOWN = 0x0,
	XHDMIC_SPD_DIGITAL_STB,
	XHDMIC_SPD_DVD_PLAYER,
	XHDMIC_SPD_DVHS,
	XHDMIC_SPD_HDD_VIDEORECORDER,
	XHDMIC_SPD_DVC,
	XHDMIC_SPD_DSC,
	XHDMIC_SPD_VIDEOCD,
	XHDMIC_SPD_GAME,
	XHDMIC_SPD_PC_GENERAL,
	XHDMIC_SPD_BLURAY_DISC,
	XHDMIC_SPD_SUPER_AUDIO_CD,
	XHDMIC_SPD_HD_DVD,
	XHDMIC_SPD_PMP
} XHdmiC_SPD_SourceInfo;

typedef enum {
	XHDMIC_DRM_TRADITIONAL_GAMMA_SDR = 0x0,
	XHDMIC_DRM_TRADITIONAL_GAMMA_HDR,
	XHDMIC_DRM_SMPTE_ST_2084,
	XHDMIC_DRM_HLG
} XHdmiC_DRM_EOTF;

typedef enum {
	XHDMIC_DRM_STATIC_METADATA_TYPE1 = 0
} XHdmiC_DRM_Static_Metadata_Descp_Id;

/**
 * This typedef contains the data structure for
 * Auxiliary Video Information Info frame
 */
typedef struct XHDMIC_AVI_InfoFrame {
	unsigned char Version;
	u8 DataLength;
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
	XHdmiC_AdditionalColorimetryExtension AdditionalColorimetryExtension;
	XHdmiC_FrameRateCode FrameRateCode;
	XHdmiC_ResolutionID ResolutionID;
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

/**
 * This typedef contains the data structure for Audio Metadata Infoframe
 */
typedef struct XHdmiC_Audio_Metadata_Packet {
	u8 Audio3D;
	u8 Num_Audio_Str;
	u8 Num_Views;
	u8 Audio3D_ChannelCount; /* 5 bits */
	u8 ACAT; /* 4 bits - Audio Channel Allication Standard */
	u8 Audio3D_ChannelAllocation; /* 8 bits */
} XHdmiC_AudioMetadata;

/**
 * This typedef contains the data structure for Source Product Descriptor
 * Infoframe
 */
typedef struct XHdmiC_SPD_InfoFrame {
	unsigned char Version;
	unsigned char VN1;
	unsigned char VN2;
	unsigned char VN3;
	unsigned char VN4;
	unsigned char VN5;
	unsigned char VN6;
	unsigned char VN7;
	unsigned char VN8;
	unsigned char PD1;
	unsigned char PD2;
	unsigned char PD3;
	unsigned char PD4;
	unsigned char PD5;
	unsigned char PD6;
	unsigned char PD7;
	unsigned char PD8;
	unsigned char PD9;
	unsigned char PD10;
	unsigned char PD11;
	unsigned char PD12;
	unsigned char PD13;
	unsigned char PD14;
	unsigned char PD15;
	unsigned char PD16;
	XHdmiC_SPD_SourceInfo SourceInfo;
} XHdmiC_SPDInfoFrame;

/**
 * This typedef contains the data structure for Dynamic Range and Mastering
 * Infoframe
 */
typedef struct XHdmiC_DRM_InfoFrame {
	XHdmiC_DRM_EOTF EOTF;
	XHdmiC_DRM_Static_Metadata_Descp_Id Static_Metadata_Descriptor_ID;
	struct {
		u16 x,y;
	} disp_primaries[3];
	struct {
		u16 x,y;
	} white_point;
	u16 Max_Disp_Mastering_Luminance;
	u16 Min_Disp_Mastering_Luminance;
	u16 Max_Content_Light_Level;
	u16 Max_Frame_Average_Light_Level;
} XHdmiC_DRMInfoFrame;

/**
* This typedef contains Video Timing Extended Metadata
* specific data structure.
*/
typedef struct {
	u8	VRREnabled;
	u8 	MConstEnabled;
	u8	FVAFactorMinus1;
	u8	BaseVFront;
	u16	BaseRefreshRate;
	u8	RBEnabled;
	u8	QMSEnabled;
	u8	NextTransferRate;
} XV_HdmiC_VideoTimingExtMeta;

typedef struct {
	u8	Version;
	u8	FreeSyncSupported;
	u8	FreeSyncEnabled;
	u8	FreeSyncActive;
	u8	FreeSyncMinRefreshRate;
	u8	FreeSyncMaxRefreshRate;
} XV_HdmiC_FreeSync;

typedef struct {
	XV_HdmiC_FreeSync FreeSync;
	u8	NativeColorSpaceActive;
	u8	BrightnessControlActive;
	u8	LocalDimControlActive;
	u8	sRGBEOTFActive;
	u8	BT709EOTFActive;
	u8	Gamma22EOTFActive;
	u8	Gamma26EOTFActive;
	u8	PQEOTFActive;
	u8	BrightnessControl;
} XV_HdmiC_FreeSyncPro;

/**
* This typedef contains AMD Source Product Descriptor Infoframe
* specific data structure.
*/
typedef union {
	XV_HdmiC_FreeSync FreeSync;
	XV_HdmiC_FreeSyncPro FreeSyncPro;
} XV_HdmiC_SrcProdDescIF;

typedef enum {
	XV_HDMIC_VRRINFO_TYPE_NONE,
	XV_HDMIC_VRRINFO_TYPE_VTEM,
	XV_HDMIC_VRRINFO_TYPE_SPDIF,
} XV_HdmiC_VrrInfoframeType;

typedef struct {
	XV_HdmiC_VrrInfoframeType VrrIfType;
	union {
		XV_HdmiC_VideoTimingExtMeta VidTimingExtMeta;
		XV_HdmiC_SrcProdDescIF SrcProdDescIF;
	};
} XV_HdmiC_VrrInfoFrame;

/*************************** Variable Declarations ****************************/
extern const XHdmiC_VicTable VicTable[VICTABLE_SIZE];
extern const XHdmiC_FrlRate FrlRateTable[];

/************************** Function Prototypes ******************************/

void XV_HdmiC_ParseAVIInfoFrame(XHdmiC_Aux *AuxPtr,
			XHdmiC_AVI_InfoFrame *infoFramePtr);
void XV_HdmiC_ParseGCP(XHdmiC_Aux *AuxPtr,
			XHdmiC_GeneralControlPacket *GcpPtr);
void XV_HdmiC_ParseAudioInfoFrame(XHdmiC_Aux *AuxPtr,
			XHdmiC_AudioInfoFrame *AudIFPtr);
void XV_HdmiC_ParseDRMIF(XHdmiC_Aux *AuxPtr,
			XHdmiC_DRMInfoFrame *DRMInfoFrame);
XHdmiC_Aux XV_HdmiC_AVIIF_GeneratePacket(XHdmiC_AVI_InfoFrame *infoFramePtr);
XHdmiC_Aux
	XV_HdmiC_AudioIF_GeneratePacket(XHdmiC_AudioInfoFrame *AudioInfoFrame);
XHdmiC_Colorspace
		XV_HdmiC_XVidC_To_IfColorformat(XVidC_ColorFormat ColorFormat);
XHdmiC_Aux XV_HdmiC_AudioMetadata_GeneratePacket(XHdmiC_AudioMetadata
		*AudMetadata);
XHdmiC_Aux XV_HdmiC_SPDIF_GeneratePacket(XHdmiC_SPDInfoFrame *SPDInfoFrame);
void XV_HdmiC_DRMIF_GeneratePacket(XHdmiC_DRMInfoFrame *DRMInfoFrame,
					XHdmiC_Aux *aux);
XVidC_AspectRatio XV_HdmiC_IFAspectRatio_To_XVidC(XHdmiC_PicAspectRatio AR);

u32 XHdmiC_FRL_GetNVal(XHdmiC_FRLCharRate FRLCharRate,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal);
u32 XHdmiC_TMDS_GetNVal(u32 TMDSCharRate,
		XHdmiC_SamplingFrequency AudSampleFreq);
XHdmiC_SamplingFrequencyVal
	XHdmiC_FRL_GetAudSampFreq(XHdmiC_FRLCharRate FRLCharRate,
		u32 CTS, u32 N);
XHdmiC_SamplingFrequency XHdmiC_TMDS_GetAudSampFreq(u32 TMDSCharRate,
		u32 N, u32 CTSVal);
XHdmiC_SamplingFrequency
	XHdmiC_GetAudIFSampFreq (XHdmiC_SamplingFrequencyVal AudSampFreqVal);
XHdmiC_SamplingFrequencyVal
	XHdmiC_GetAudSampFreqVal(XHdmiC_SamplingFrequency AudSampFreqVal);

#ifdef __cplusplus
}
#endif

#endif /* XHDMIC_H_ */
/** @} */

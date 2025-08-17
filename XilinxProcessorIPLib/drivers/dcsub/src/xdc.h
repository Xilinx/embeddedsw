/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdc.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25 Initial Release.
 * </pre>
 *
*******************************************************************************/
#ifndef XDC_H_
/* Prevent circular inclusions by using protection macros. */
#define XDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xdc_hw.h"
/****************************** Type Definitions ******************************/

/**
 * This structure stores the XDC configuration information.
**/
typedef struct {
	const char *Name;
	u32 BaseAddr;
} XDc_Config;

/**
 * This typedef describes all the Video Formats supported by the driver
 */
typedef enum {
	/* Non-Live Video Formats */
	CbY0CrY1,
	CrY0CbY1,
	Y0CrY1Cb,
	Y0CbY1Cr,
	YV16,
	YV24,
	YV16Ci,
	MONOCHROME,
	YV16Ci2,
	YUV444,
	RGB888,
	RGBA8880,
	RGB888_10BPC,
	YUV444_10BPC,
	YV16Ci2_10BPC,
	YV16Ci_10BPC,
	YV16_10BPC,
	YV24_10BPC,
	MONOCHROME_10BPC,
	YV16_420,
	YV16Ci_420,
	YV16Ci2_420,
	YV16_420_10BPC,
	YV16Ci_420_10BPC,
	YV16Ci2_420_10BPC,
	RGBA8888,
	ABGR8888,
	RGB888_GFX,
	BGR888,
	RGBA5551,
	RGBA4444,
	RGB565,
	RGB888dc,
	RGB888dcm_10BPC,
	RGB888dcl_10BPC,
	RGB888dcm_12BPC,
	RGB888dcl_12BPC,
	RGB10A2g,
	RGBA8888dcm_10BPC,
	RGBA8888dcl_10BPC,
	RGBA8888dcm_12BPC,
	RGBA8888dcl_12BPC,
	YUV444g,
	YUV444dc,
	YUV444g_10BPC,
	YUV444dcm_10BPC,
	YUV444dcl_10BPC,
	YUV444dcm_12BPC,
	YUV444dcl_12BPC,
	YUVA444g,
	YV24dc,
	YV24dcm_10BPC,
	YV24dcl_10BPC,
	YV24dcm_12BPC,
	YV24dcl_12BPC,
	YV16CIdc,
	YV16CI2dc,
	YV16CIdcm_10BPC,
	YV16CIdcl_10BPC,
	YV16CI2dcm_10BPC,
	YV16CI2dcl_10BPC,
	YV16CIdcm_12BPC,
	YV16CIdcl_12BPC,
	YV16CI2dcm_12BPC,
	YV16CI2dcl_12BPC,
	YV16CIdc_420,
	YV16CI2dc_420,
	YV16CIdcm_420_10BPC,
	YV16CIdcl_420_10BPC,
	YV16CI2dcm_420_10BPC,
	YV16CI2dcl_420_10BPC,
	YV16CIdcm_420_12BPC,
	YV16CIdcl_420_12BPC,
	YV16CI2dcm_420_12BPC,
	YV16CI2dcl_420_12BPC,
	Ydc_ONLY,
	Ydcm_ONLY_10BPC,
	Ydcl_ONLY_10BPC,
	Ydcm_ONLY_12BPC,
	Ydcl_ONLY_12BPC,
	/* Live Input/Output Formats */
	Y_ONLY_8BPC,
	Y_ONLY_10BPC,
	Y_ONLY_12BPC,
	RGB_6BPC,
	RGB_8BPC,
	RGB_10BPC,
	RGB_12BPC,
	YCbCr444_6BPC,
	YCbCr444_8BPC,
	YCbCr444_10BPC,
	YCbCr444_12BPC,
	YCbCr422_8BPC,
	YCbCr422_10BPC,
	YCbCr422_12BPC,
} XDc_VideoFormat;

/**
 * This typedef describes the DC operational modes
 */
typedef enum {
	XDC_VID_FUNCTIONAL = 0,
	XDC_VID_BYPASS = 1,
} XDc_VidInterface;

/**
 * This typedef describes the DC operational modes
 */
typedef enum {
	XDC_AUD_FUNCTIONAL = 0,
	XDC_AUD_BYPASS = 1,
} XDc_AudInterface;

/**
 * This typedef describes the video source list
 */
typedef enum {
	XDC_VIDSTREAM1_LIVE = 0x00,
	XDC_VIDSTREAM1_NONLIVE = 0x01,
	XDC_VIDSTREAM1_NONE = 0x02,
	XDC_VIDSTREAM1_NONE_BLACK_FRAMES = 0x03,
} XDc_VideoStream1;

/**
 * This typedef describes the graphics source list
 */
typedef enum {
	XDC_VIDSTREAM2_LIVE = 0x20,
	XDC_VIDSTREAM2_NONLIVE = 0x04,
	XDC_VIDSTREAM2_NONE = 0x08,
	XDC_VIDSTREAM2_NONE_BLACK_FRAMES = 0xC,
} XDc_VideoStream2;

/**
 * This typedef describes the audio source list
 */
typedef enum {
	XDC_AUDSTREAM_LIVE = 0x00,
	XDC_AUDSTREAM_DISABLE_AUDIO = 0x01,
	XDC_AUDSTREAM_NONLIVE = 0x02,
	XDC_AUDSTREAM_NONE = 0x03,
} XDc_AudioStream;

/**
 *  This typedef enables alpha blend
 * */
typedef enum {
	ALPHA_DISABLE,
	ALPHA_ENABLE,
} XDc_AlphaBlend;

/**
 *  This typedef enables chroma keying
 * */
typedef enum {
	CHROMA_DISABLE,
	CHROMA_ENABLE,
} XDc_ChromaBlend;

/**
 *  This typedef enables chroma key master stream
 * */
typedef enum {
	MASTER_STREAM1 = 0,
	MASTER_STREAM2 = 1,
} XDc_ChromaMaster;

typedef enum {
	Interleaved,
	SemiPlanar,
	Planar,
	Tiled,
} XDc_VideoModes;

/**
* This typedef describes the SDP source list
*/
typedef enum {
	SDP_PL = 0x00,
	SDP_DMA = 0x01,
} XDc_Sdp;

/**
 *  This typedef enables audio
 * */
typedef enum {
	XDC_AUD_DISABLE = 0x0,
	XDC_AUD_ENABLE  = 0x1,
} XDc_AudEn;


/**
 *  This typedef enables partial blend
 * */
typedef enum {
	PB_DISABLE,
	PB_ENABLE,
} XDc_PartialBlendEn;

/**
* This structure stores the Chroma keys max, min value for each
* component.
*
**/
typedef struct {
	u16 RMax;
	u16 RMin;
	u16 GMax;
	u16 GMin;
	u16 BMax;
	u16 BMin;
} XDc_ChromaKey;

/**
 *  This typedef enables cursor blend
 * */
typedef enum {
	CB_DISABLE,
	CB_ENABLE,
} XDc_CursorBlend;

typedef struct {
	XDc_VideoFormat VideoFormat;
	u8 Id;
	XDc_VideoModes Mode;
	u32 ScaleFactors[3];
	u8 SubSample;
	u8 IsRGB;
	u8 IsGraphics;
	u8 CbFirst;
	u8 BPP; //bits per pixel
} XDc_VideoAttribute;

/**
 * This typedef stores the data associated with the Audio Video input modes.
 */
typedef struct {
	XDc_VideoStream1 VideoSrc1;
	XDc_VideoStream2 VideoSrc2;
	XDc_AudioStream  AudSrc;

	XDc_VideoAttribute *NonLiveVideo1;
	XDc_VideoAttribute *NonLiveVideo2;

	XDc_VideoAttribute *LiveVideo1;
	XDc_VideoAttribute *LiveVideo2;

	XDc_VideoAttribute *OutputVideo;

} XDc_AVModes;

typedef struct {
	u16 RCr;
	u16 GY;
	u16 BCb;
} XDc_BlenderBgClr;

/**
 * This typedef describes the cursor blend coords
 * */
typedef struct {

	u8 CoordY;
	u8 CoordX;
	u8 SizeY;
	u8 SizeX;

	XDc_VideoAttribute *CursorAttribute;

} XDc_Cursor;

/**
 * This typedef describes the parital blend coords
 * */
typedef struct {

	u16 CoordX;
	u16 CoordY;
	u16 SizeX;
	u16 SizeY;
	u16 OffsetX;
	u16 OffsetY;

} XDc_PartialBlend;

/**
 * This structure stores the XDC Timing configuration.
**/
typedef struct {

	u32 HTotal;
	u32 HSWidth;
	u32 HRes;
	u32 HStart;

	u32 VTotal;
	u32 VSWidth;
	u32 VRes;
	u32 VStart;

} XDc_VideoTiming;

/**
 * This typedef stores all the attributes associated to the Blender block of the
 * DisplayPort Subsystem
 */
typedef struct {

	XDc_AlphaBlend AlphaEnable;
	u8 Alpha;
	u32 *Stream1ScaleFactors;
	u32 *Stream2ScaleFactors;
	u32 *Stream1CSCCoeff;
	u32 *Stream1CSCOffset;
	u32 *Stream2CSCCoeff;
	u32 *Stream2CSCOffset;

	u32 *OutCSCCoeff;
	u32 *OutCSCOffset;

	u8 Stream1BlendBypass;
	u8 Stream2BlendBypass;

	XDc_ChromaBlend ChromaEnable;
	XDc_ChromaMaster ChromaMasterSel;
	XDc_ChromaKey *ChromaKey;

	XDc_CursorBlend CursorEnable;
	XDc_Cursor Cursor;

	XDc_PartialBlendEn Stream1PbEn;
	XDc_PartialBlend *Stream1PbCoords;

	XDc_PartialBlendEn Stream2PbEn;
	XDc_PartialBlend *Stream2PbCoords;

} XDc_Blender;

typedef struct {

	XDc_Config Config;
	XDc_VidInterface VidInterface;
	XDc_AudInterface AudInterface;

	XDc_BlenderBgClr *BgClr;
	XDc_Blender Blender;
	XDc_AVModes AVMode;

	u8 VideoClk;
	u8 AudioClk;
	u8 VidTimingSrc;

	u8 VidClkSelect;
	u8 AudClkSelect;

	u8 Stream1ChannelEn;
	u8 Stream1BurstLen;
	u8 Stream2ChannelEn;
	u8 Stream2BurstLen;

	u8 AudLineRstDisable;
	u8 AudBypassExtraBS;
	u8 AudChannelEn;
	u8 AudBurstLen;
	u8 AudChannelSel;
	u16 AudSampleRate;
	XDc_AudEn AudioEnable;
	u16 AudChCtrl;
	u8 AudSegmentedMode;

	u8 SdpEnable;
	XDc_Sdp Sdp;
	u8 SdpEmptyThreshold;
	u8 SdpChannelEn;
	u8 SdpBurstLen;
	u16 RdyInterval;
	u8 SdpAckSel;

	u32 VidFrameSwitchCtrl;

	u16 NonLiveLatency;

	u32 StcEn;
	u32 StcInitVal;
	u32 StcAdjVal;
	u64 StcVSyncTs;
	u64 StcExtVSyncTs;
	u64 StcCustomEventTs;
	u64 StcCustomEvent2Ts;
	u64 StcSnapshot;

	XDc_VideoTiming VideoTiming;
} XDc;

/**************************** Function Prototypes *****************************/
void XDc_CfgInitialize(XDc *InstancePtr, u32 BaseAddr);
void XDc_Initialize(XDc *InstancePtr);
void XDc_WriteProtEnable(XDc *InstancePtr);
void XDc_WriteProtDisable(XDc *InstancePtr);
void XDc_SetVidInterfaceMode(XDc *InstancePtr);
void XDc_SetBlenderBgColor(XDc *InstancePtr);
void XDc_SetGlobalAlpha(XDc *InstancePtr);
void XDc_SetInputVideoSelect(XDc *InstancePtr);
void XDc_SetInputAudioSelect(XDc *InstancePtr);
XDc_VideoAttribute *XDc_GetNonLiveVideoAttribute
(XDc_VideoFormat Format);
void XDc_SetNonLiveInputFormat(XDc *InstancePtr, u8 VideoSrc,
			       XDc_VideoAttribute *Video);
XDc_VideoAttribute *XDc_GetLiveVideoAttribute
(XDc_VideoFormat Format);
void XDc_SetOutputVideoFormat(XDc *InstancePtr);
XDc_VideoAttribute *XDc_GetOutputVideoAttribute
(XDc_VideoFormat Format, XDc_VidInterface Mode);
void XDc_SetOutputVideoFormat(XDc *InstancePtr);
void XDc_EnablePartialBlend(XDc *InstancePtr);
u32 XDc_ConfigureStream(XDc *InstancePtr, u8 StreamSrc);
void XDc_SetOutputCSC(XDc *InstancePtr);
void XDc_SetChromaKey(XDc *InstancePtr);
void XDc_SetCursorBlend(XDc *InstancePtr);
void XDc_SetAudioVideoClkSrc(XDc *InstancePtr);
void XDc_VideoSoftReset(XDc *InstancePtr);
void XDc_VidClkSelect(XDc *InstancePtr);
void XDc_InputAudioSelect(XDc *InstancePtr);
void XDc_EnableStream1Buffers(XDc *InstancePtr);
void XDc_EnableStream2Buffers(XDc *InstancePtr);
void XDc_AudLineResetDisable(XDc *InstancePtr);
void XDc_AudioSoftReset(XDc *InstancePtr);
void XDc_AudExtraBSControl(XDc *InstancePtr);
void XDc_SetSdpSource(XDc *InstancePtr);
void XDc_SetSdpEmptyThreshold(XDc *InstancePtr);
void XDc_SetSdpCursorBuffers(XDc *InstancePtr);
void XDc_SetVidFrameSwitch(XDc *InstancePtr);
void XDc_SetNonLiveLatency(XDc *InstancePtr);
void XDc_SetStcCtrl(XDc *InstancePtr);
void XDc_SetStcLoad(XDc *InstancePtr);
void XDc_SetStcAdjust(XDc *InstancePtr);
u64 XDc_GetStcVsyncTs(XDc *InstancePtr);
u64 XDc_GetStcExtVsyncTs(XDc *InstancePtr);
u64 XDc_GetStcCustomEventTs(XDc *InstancePtr);
u64 XDc_GetStcCustomEvent2Ts(XDc *InstancePtr);
u64 XDc_GetStcSnapshot(XDc *InstancePtr);
void XDc_EnableAudioBuffer(XDc *InstancePtr);
void XDc_AudioChannelSelect(XDc *InstancePtr);
void XDc_AudClkSelect(XDc *InstancePtr);
void XDc_SetAudInterfaceMode(XDc *InstancePtr);
void XDc_EnableAudio(XDc *InstancePtr);
void XDc_DisableAudio(XDc *InstancePtr);
void XDc_SetVideoTiming(XDc *InstancePtr);
void XDc_SetAudioChCtrl(XDc *InstancePtr);
void XDc_SetAudioSegmentedMode(XDc *InstancePtr);
void XDc_SetCursorSdpRdyInterval(XDc *InstancePtr);
void XDc_SetSdpAckSel(XDc *InstancePtr);

#endif /* __XDC_H__ */

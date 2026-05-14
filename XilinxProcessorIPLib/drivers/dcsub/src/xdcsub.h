/*******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdcsub.h
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   ck  03/14/25    Initial Version
*</pre>
*
******************************************************************************/

/******************************* Include Files ********************************/
#ifndef __XDCSUB_H__
#define __XDCSUB_H__

#include "xdc.h"
#include "xdcdma.h"

/****************************** Variables ******************************/
/* Shared audio channel limits used by dcsub driver and examples */
#define XDCSUB_AUDIO_CHANNELS_MIN	1U
#define XDCSUB_AUDIO_CHANNELS_MAX	8U

/****************************** Type Definitions ******************************/
typedef struct {

	XDc_Config DcConfig;
	XDcDma_Config DmaConfig;

} XDcSub_Config;

typedef struct {

	XDcSub_Config XDcSubCfg;
	XDc *DcPtr;
	XDcDma *DmaPtr;

} XDcSub;

/**
 * The DC Subsystem can work in functional or in bypass mode.
 * In functional mode, the 2 video streams are blended together and sent to the DisplayPort.
 * In bypass mode, the input video stream is directly forwarded to the DisplayPort.
 */
/**
 * @def XDCSUB_OPMODE_FUNCTIONAL
 * @brief Functional mode supports blending videos
 */
#define XDCSUB_OPMODE_FUNCTIONAL	(0U)
/**
 * @def XDCSUB_OPMODE_BYPASS
 * @brief Bypass mode makes DC in pass through mode
 */
#define XDCSUB_OPMODE_BYPASS		(1U)

/**
 * The DC Subsystem supports three presentation modes: Non-live, Live, and Mixed.
 * In non-live mode, the video streams are fetched from the memory.
 * In live mode, the video streams are sourced from the live video ports.
 * In mixed mode, one video stream is sourced from memory and
 * the other video stream is sourced from one of the live video ports.
 */
/**
 * @def XDCSUB_PPTMODE_NONLIVE
 * @brief In Non live mode, video streams are fetched from memory.
 */
#define XDCSUB_PPTMODE_NONLIVE		(0U)
/**
 * @def XDCSUB_PPTMODE_LIVE
 * @brief In Live mode, video streams come from live video ports.
 */
#define XDCSUB_PPTMODE_LIVE		(1U)
/**
 * @def XDCSUB_PPTMODE_MIXED
 * @brief In Mixed mode, one video come from live source
 * and another from memory.
 */
#define XDCSUB_PPTMODE_MIXED		(2U)

/**
 * In mixed mode, one video stream is sourced from memory and
 * the other video stream is sourced from one of the live video ports.
 * These macros are to denote which of the live video ports are selected as the video source.
 * In live mode, both the video ports are active.
 */
 /**
  * @def XDCSUB_LIVVID_SEL_V01
  * @brief In mixed mode, live video port V01 is selected as video source.
  */
#define XDCSUB_LIVVID_SEL_V01		(0U)
/**
 * @def XDCSUB_LIVVID_SEL_V02
 * @brief In mixed mode, live video port V02 is selected as video source.
 */
#define XDCSUB_LIVVID_SEL_V02		(1U)
/**
 * @def XDCSUB_LIVVID_SEL_BOTH
 * @brief In live mode, both live video ports are active.
 */
#define XDCSUB_LIVVID_SEL_BOTH		(2U)
/**
 * @def XDCSUB_LIVVID_SEL_NONE
 * @brief In non-live mode, both live video ports are disabled.
 */
#define XDCSUB_LIVVID_SEL_NONE		(3U)

/**
 * The first video port V01 supports both audio and video.
 * The second video port V02 supports video only.
 * These macros are to denote the mode of operation of the live video ports.
 */
 /**
  * @def XDCSUB_VIDMODE_AV
  * @brief Live video port V01 supports both audio and video.
  */
#define XDCSUB_VIDMODE_AV		(0U)
 /**
  * @def XDCSUB_VIDMODE_VONLY
  * @brief Live video port V02 supports video only.
  */
#define XDCSUB_VIDMODE_VONLY		(1U)
/**
  * @def XDCSUB_VIDMODE_NONE
  * @brief No video port.
  */
#define XDCSUB_VIDMODE_NONE		(2U)

/**
 * These defines are used to denote
 * if alpha channel is enabled or disabled for live video input
 */
 /**
  * @def XDCSUB_LIVVID_ALPHA_EN
  * @brief Alpha channel is enabled for live video input.
  */
#define XDCSUB_LIVVID_ALPHA_EN		(1U)
 /**
  * @def XDCSUB_LIVVID_ALPHA_DIS
  * @brief Alpha channel is disabled for live video input.
  */
#define XDCSUB_LIVVID_ALPHA_DIS		(0U)

/**
 * This defines if SDP is enabled or disabled for live video input
 */
/**
 * @def XDCSUB_LIVVID_SDP_EN
 * @brief SDP is enabled for live video input.
 */
#define XDCSUB_LIVVID_SDP_EN		(1U)
/**
 * @def XDCSUB_LIVVID_SDP_DIS
 * @brief SDP is disabled for live video input.
 */
#define XDCSUB_LIVVID_SDP_DIS		(0U)

extern XDcSub_Config XDcSub_ConfigTable[];

/****************************** Function Prototypes ******************************/
u32 XDcSub_CfgInitialize(XDcSub *InstancePtr);
u32 XDcSub_Initialize(XDcSub *InstancePtr);
u32 XDcSub_SetVidInterfaceMode(XDcSub *InstancePtr, XDc_VidInterface Mode);
u32 XDcSub_SetBlenderBgColor(XDcSub *InstancePtr, XDc_BlenderBgClr *Color);
u32 XDcSub_SetGlobalAlpha(XDcSub *InstancePtr, XDc_AlphaBlend AlphaEnable, u8 Alpha);
u32 XDcSub_SetVidStreamSrc(XDcSub *InstancePtr, XDc_VideoStream1 VidStream1,
			   XDc_VideoStream2 VidStream2);
u32 XDcSub_SetInputNonLiveVideoFormat(XDcSub *InstancePtr, XDc_VideoFormat Format1,
				      XDc_VideoFormat Format2);
u32 XDcSub_SetInputLiveStreamFormat(XDcSub *InstancePtr, XDc_VideoFormat Format1,
				    XDc_VideoFormat Format2);
u32 XDcSub_SetOutputVideoFormat(XDcSub *InstancePtr, XDc_VideoFormat Format);
u32 XDcSub_SetStreamPixelScaling(XDcSub *InstancePtr, u32 *Stream1_ScaleFactors,
				 u32 *Stream2_ScaleFactors);
u32 XDcSub_SetInputStreamCSC(XDcSub *InstancePtr, u32 *Stream1CSCCoeff,
			     u32 *Stream1CSCOffset, u32 *Stream2CSCCoeff, u32 *Stream2CS);
u32 XDcSub_SetOutputStreamCSC(XDcSub *InstancePtr, u32 *OutCSCCoeff, u32 *OutCSCOffset);
u32 XDcSub_SetInputStreamLayerControl(XDcSub *InstancePtr, u8 BlendBypass1, u8 BlendBypass2);
u32 XDcSub_SetChromaKey(XDcSub *InstancePtr, u8 ChromaEnable,
			u8 ChromaMasterSel, XDc_ChromaKey *Key);
u32 XDcSub_SetCursorBlend(XDcSub *InstancePtr, XDc_CursorBlend Enable, XDc_Cursor *Cursor);
u32 XDcSub_SetAudioVideoClkSrc(XDcSub *InstancePtr);
u32 XDcSub_VidClkSelect(XDcSub *InstancePtr, u8 Stream1Sel, u8 Stream2Sel);
u32 XDcSub_EnableStream1Buffers(XDcSub *InstancePtr, u8 Enable, u8 Burst);
u32 XDcSub_EnableStream2Buffers(XDcSub *InstancePtr, u8 Enable, u8 Burst);
u32 XDcSub_AudLineResetDisable(XDcSub *InstancePtr, u8 Disable);
u32 XDcSub_AudExtraBSControl(XDcSub *InstancePtr, u8 Bypass);
u32 XDcSub_SetSdp(XDcSub *InstancePtr, XDc_Sdp Sdp);
u32 XDcSub_SetSdpEmptyThreshold(XDcSub *InstancePtr, u8 Threshold);
u32 XDcSub_SetSdpCursorBuffers(XDcSub *InstancePtr, u8 Enable, u8 BurstLen);
u32 XDcSub_SetStreamPartialBlend(XDcSub *InstancePtr, XDc_PartialBlendEn Enable1,
				 XDc_PartialBlend *Coords1, XDc_PartialBlendEn Enable2, XDc_PartialBlend *Coords2);
u32 XDcSub_SetVidFrameSwitch(XDcSub *InstancePtr, u32 Control);
u32 XDcSub_SetNonLiveLatency(XDcSub *InstancePtr, u16 Nl_Latency, u8 V_Line_Latency);
u32 XDcSub_EnableStcCtrl(XDcSub *InstancePtr);
u32 XDcSub_ClearStcCtrl(XDcSub *InstancePtr);
u32 XDcSub_SetStcLoad(XDcSub *InstancePtr, u64 InitVal);
u32 XDcSub_SetStcAdjust(XDcSub *InstancePtr, u32 InitVal);
u64 XDcSub_GetStcVSyncTs(XDcSub *InstancePtr);
u64 XDcSub_GetStcExtVSyncTs(XDcSub *InstancePtr);
u64 XDcSub_GetStcCustomEventTs(XDcSub *InstancePtr);
u64 XDcSub_GetStcCustomEvent2Ts(XDcSub *InstancePtr);
u64 XDcSub_GetStcSnapshot(XDcSub *InstancePtr);
u32 XDcSub_EnableAudioBuffer(XDcSub *InstancePtr, u8 Enable, u8 BurstLen);
u32 XDcSub_AudioChannelSelect(XDcSub *InstancePtr, u8 ChannelSel, u16 SampleRate);
u32 XDcSub_AudClkSelect(XDcSub *InstancePtr, u32 Select);
u32 XDcSub_SetAudInterfaceMode(XDcSub *InstancePtr, XDc_AudInterface Mode);
u32 XDcSub_EnableAudio(XDcSub *InstancePtr);
u32 XDcSub_DisableAudio(XDcSub *InstancePtr);
u32 XDcSub_SetInputAudioSelect(XDcSub *InstancePtr, XDc_AudioStream AudStream);
u32 XDcSub_ConfigureDcVideo(XDc *InstancePtr);
u32 XDcSub_SetAudioChCtrl(XDcSub *InstancePtr, u8 AudioChannels);
u32 XDcSub_SetAudioSegmentedMode(XDcSub *InstancePtr, u8 AudSegmentedMode);
void XDcSub_EnableSdp(XDcSub *InstancePtr);
void XDcSub_SetSdpAckSel(XDcSub *InstancePtr, u8 AckSel);
u32 XDcSub_SetCursorSdpRdyInterval(XDcSub *InstancePtr, u16 RdyInterval);

#endif /* __XDCSUB_H__ */

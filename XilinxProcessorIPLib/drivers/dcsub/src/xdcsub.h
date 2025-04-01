/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
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
u32 XDcSub_SetNonLiveLatency(XDcSub *InstancePtr, u16 Latency);
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
u32 XDcSub_AudioChannelSelect(XDcSub *InstancePtr, u8 ChannelSel, u8 SampleRate);
u32 XDcSub_AudClkSelect(XDcSub *InstancePtr, u32 Select);
u32 XDcSub_SetAudInterfaceMode(XDcSub *InstancePtr, XDc_AudInterface Mode);
u32 XDcSub_EnableAudio(XDcSub *InstancePtr);
u32 XDcSub_DisableAudio(XDcSub *InstancePtr);
u32 XDcSub_ConfigureDcVideo(XDc *InstancePtr);

#endif /* __XDCSUB_H__ */

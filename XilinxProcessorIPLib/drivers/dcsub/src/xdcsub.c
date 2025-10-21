/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdcsub.c
 *
 * This file implements all the functions related to the Video Pipeline of the
 * DisplayPort Subsystem. See xdc.h for the detailed description of the
 * driver.
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0    ck   03/14/2025 Initial Version
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <xstatus.h>

#include "xdc.h"
#include "xdcsub.h"

/******************************************************************************/
/**
 * This function intializes the configuration of all instances part of DC
 * subsystem i.e DC, DCDMA, Interrupts.
 *
 * @param       InstancePtr is a pointer to the XDcSub instance.
 * @param       DeviceId is the id of the device from the design.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        Base address and DeviceId is same as the DP Core driver.
 *
*******************************************************************************/
u32 XDcSub_CfgInitialize(XDcSub *InstancePtr)
{
	u32 Status;
	XDc *DcPtr;
	XDcSub_Config DcSubCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcPtr = InstancePtr->DcPtr;
	DcSubCfg = InstancePtr->XDcSubCfg;

	XDc_CfgInitialize(DcPtr, DcSubCfg.DcConfig.BaseAddr);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function initializes all the data structures of the XDcSub
 * Instance.
 *
 * @param       InstancePtr is a pointer to the XDcSub instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_Initialize(XDcSub *InstancePtr)
{
	u32 Status;
	XDc *DcPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcPtr = InstancePtr->DcPtr;

	XDc_WriteProtDisable(DcPtr);

	XDc_AudioSoftReset(DcPtr);

	XDc_SetVidInterfaceMode(DcPtr);

	if ((DcPtr->Blender.AlphaEnable == ALPHA_ENABLE) &&
	    (DcPtr->Blender.ChromaEnable == CHROMA_ENABLE)) {
		xil_printf("Enable AlphaBlending or CursorBlending. Not both\n");
		return XST_FAILURE;
	}

	if (DcPtr->Blender.Stream1PbEn && DcPtr->Blender.Stream2PbEn) {
		xil_printf("Enable Partial Blend for only one stream. Not both\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the Interface mode for DCSub DC Instance.
 * DC can be either in bypass mode or functional mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Mode is the enum val to enable or diasable bypass mode.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetVidInterfaceMode(XDcSub *InstancePtr, XDc_VidInterface Mode)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);
	Xil_AssertNonvoid((Mode != XDC_VID_BYPASS) |
			  (Mode != XDC_VID_FUNCTIONAL));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->VidInterface = Mode;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the blender background color
 *
 * @param       InstancePtr is an pointer to the XDcSub Instance.
 * @param       Color is a pointer to the structure XDc_BlenderBgClr
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
******************************************************************************/
u32 XDcSub_SetBlenderBgColor(XDcSub *InstancePtr, XDc_BlenderBgClr *Color)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Color != NULL);

	InstancePtr->DcPtr->BgClr = Color;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables or disables global alpha
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        GlobalAlphaEn = 1, enables the global alpha.
 *              GlobalAlphaEn = 0, disables the global alpha.
 *              Alpha = 0, transparent
 *              Alpha = 255, Opaque
 *              Alpha = 8:1 (Alpha value)
******************************************************************************/
u32 XDcSub_SetGlobalAlpha(XDcSub *InstancePtr, XDc_AlphaBlend AlphaEnable, u8 Alpha)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((AlphaEnable != ALPHA_ENABLE) |
			  (AlphaEnable != ALPHA_DISABLE));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.AlphaEnable = AlphaEnable;
	DcConfigPtr->Blender.Alpha = Alpha;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function selects the source type for the Video and Graphics input streams
 * that are passed on to the blender block.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       VidStream1 selects the stream coming from the video source1
 * @param       VidStream2 selects the stream coming from the video source2
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetVidStreamSrc(XDcSub *InstancePtr, XDc_VideoStream1 VidStream1,
			   XDc_VideoStream2 VidStream2)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Xil_AssertNonvoid((VidStream1 != XDC_VIDSTREAM1_LIVE) |
			  (VidStream1 != XDC_VIDSTREAM1_NONLIVE) |
			  (VidStream1 != XDC_VIDSTREAM1_NONE) |
			  (VidStream1 != XDC_VIDSTREAM1_NONE_BLACK_FRAMES));

	Xil_AssertNonvoid((VidStream2 != XDC_VIDSTREAM2_LIVE) |
			  (VidStream2 != XDC_VIDSTREAM2_NONLIVE) |
			  (VidStream2 != XDC_VIDSTREAM2_NONE) |
			  (VidStream2 != XDC_VIDSTREAM2_NONE_BLACK_FRAMES));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AVMode.VideoSrc1 = VidStream1;
	DcConfigPtr->AVMode.VideoSrc2 = VidStream2;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function selects the source type for audio stream
 *
 * @param       InstancePtr is a pointer to the XDcSub instance.
 * @param       AudStream selects the audio stream source type
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetInputAudioSelect(XDcSub *InstancePtr, XDc_AudioStream AudStream)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Xil_AssertNonvoid((AudStream != XDC_AUDSTREAM_LIVE) |
			  (AudStream != XDC_AUDSTREAM_NONLIVE) |
			  (AudStream != XDC_AUDSTREAM_NONE) |
			  (AudStream != XDC_AUDSTREAM_DISABLE_AUDIO));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AVMode.AudSrc = AudStream;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function selects the source type for audio stream
 *
 * @param       InstancePtr is a pointer to the XDcSub instance.
 * @param       Format1 selects the nonlive video1 format
 * @param       Format2 selects the nonlive video2 format
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetInputNonLiveVideoFormat(XDcSub *InstancePtr, XDc_VideoFormat Format1,
				      XDc_VideoFormat Format2)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AVMode.NonLiveVideo1 =
		XDc_GetNonLiveVideoAttribute(Format1);

	DcConfigPtr->AVMode.NonLiveVideo2 =
		XDc_GetNonLiveVideoAttribute(Format2);

	if (!DcConfigPtr->AVMode.NonLiveVideo1 &&
	    !DcConfigPtr->AVMode.NonLiveVideo2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the stream1 format for the live video
 *
 * @param       InstancePtr is a pointer to the XDcSub instance.
 * @param       Format1 is the enum for the live video1 format
 * @param       Format2 is the enum for the live video2 format
 *
 * @return      XST_SUCCESS if the correct format has been set.
 *              XST_FAILURE if the format is invalid.
 *
 * @note        None.
*******************************************************************************/
u32 XDcSub_SetInputLiveStreamFormat(XDcSub *InstancePtr,
				    XDc_VideoFormat Format1, XDc_VideoFormat Format2)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Format1 >= Y_ONLY_8BPC) | (Format1 <= YCbCr422_12BPC));
	Xil_AssertNonvoid((Format2 >= Y_ONLY_8BPC) | (Format2 <= YCbCr422_12BPC));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AVMode.LiveVideo1 = XDc_GetLiveVideoAttribute(Format1);
	DcConfigPtr->AVMode.LiveVideo2 =
		XDc_GetLiveVideoAttribute(Format2);

	if (!DcConfigPtr->AVMode.LiveVideo1 &&
	    !DcConfigPtr->AVMode.LiveVideo2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the Output Video Format
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Format is the enum for the non-live video format
 *
 * @return      XST_SUCCESS if the correct format has been set.
 *              XST_FAILURE if the format is invalid.
 *
 * @note        None.
*******************************************************************************/
u32 XDcSub_SetOutputVideoFormat(XDcSub *InstancePtr, XDc_VideoFormat Format)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Format >= Y_ONLY_8BPC) | (Format <= YCbCr422_12BPC));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AVMode.OutputVideo =
		XDc_GetOutputVideoAttribute(Format, DcConfigPtr->VidInterface);

	if (DcConfigPtr->AVMode.OutputVideo == NULL) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the scaling factors depending on the source video stream.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Scaling Factors is a pointer to the scaling factors needed for
 *              scaling colors to 12 BPC.
 * @param       Scaling Factors is a pointer to the scaling factors needed for
 *              scaling colors to 12 BPC.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetStreamPixelScaling(XDcSub *InstancePtr, u32 *Stream1_ScaleFactors,
				 u32 *Stream2_ScaleFactors)
{
	XDc *DcConfigPtr;
	XDc_VideoAttribute *Video1;
	XDc_VideoAttribute *Video2;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	if (DcConfigPtr->AVMode.VideoSrc1 == XDC_VIDSTREAM1_LIVE) {
		Video1 = DcConfigPtr->AVMode.LiveVideo1;
	} else if (DcConfigPtr->AVMode.VideoSrc1 ==
		   XDC_VIDSTREAM1_NONLIVE) {
		Video1 = DcConfigPtr->AVMode.NonLiveVideo1;
	} else {
		return XST_FAILURE;
	}

	if (DcConfigPtr->AVMode.VideoSrc2 == XDC_VIDSTREAM2_LIVE) {
		Video2 = DcConfigPtr->AVMode.LiveVideo2;
	} else if (DcConfigPtr->AVMode.VideoSrc2 ==
		   XDC_VIDSTREAM2_NONLIVE) {
		Video2 = DcConfigPtr->AVMode.NonLiveVideo2;
	} else {
		return XST_FAILURE;
	}

	if (Stream1_ScaleFactors == NULL)
		DcConfigPtr->Blender.Stream1ScaleFactors
			= Video1->ScaleFactors;
	else
		DcConfigPtr->Blender.Stream1ScaleFactors
			= Stream1_ScaleFactors;

	if (Stream2_ScaleFactors == NULL)
		DcConfigPtr->Blender.Stream2ScaleFactors
			= Video2->ScaleFactors;
	else
		DcConfigPtr->Blender.Stream2ScaleFactors
			= Stream2_ScaleFactors;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the Stream Color space conversion matrix.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Stream1CSCCoeff is a pointer to the CSCCoeff values
 * @param       Stream1CSCOffset is a pointer to the CSCCOffset values
 * @param       Stream2CSCCoeff is a pointer to the CSCCoeff values
 * @param       Stream2CSCOffset is a pointer to the CSCCOffset values

 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetInputStreamCSC(XDcSub *InstancePtr, u32 *Stream1CSCCoeff,
			     u32 *Stream1CSCOffset, u32 *Stream2CSCCoeff, u32 *Stream2CSCOffset)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Stream1CSCCoeff != NULL);
	Xil_AssertNonvoid(Stream1CSCOffset != NULL);
	Xil_AssertNonvoid(Stream2CSCCoeff != NULL);
	Xil_AssertNonvoid(Stream2CSCOffset != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.Stream1CSCCoeff = Stream1CSCCoeff;
	DcConfigPtr->Blender.Stream1CSCOffset = Stream1CSCOffset;

	DcConfigPtr->Blender.Stream2CSCCoeff = Stream2CSCCoeff;
	DcConfigPtr->Blender.Stream2CSCOffset = Stream2CSCOffset;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the Output Stream Color space conversion matrix.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       OutCSCCoeff is a pointer to the CSCCoeff values
 * @param       OutCSCOffset is a pointer to the CSCOffset values
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetOutputStreamCSC(XDcSub *InstancePtr, u32 *OutCSCCoeff,
			      u32 *OutCSCOffset)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(OutCSCCoeff != NULL);
	Xil_AssertNonvoid(OutCSCOffset != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.OutCSCCoeff = OutCSCCoeff;
	DcConfigPtr->Blender.OutCSCOffset = OutCSCOffset;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the Stream1 Blend enable, disable option.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       BlendBypass1 is enables, disables blending for stream
 * @param       BlendBypass2 is enables, disables blending for stream
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XDcSub_SetInputStreamLayerControl(XDcSub *InstancePtr, u8 BlendBypass1, u8 BlendBypass2)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.Stream1BlendBypass = BlendBypass1;
	DcConfigPtr->Blender.Stream2BlendBypass = BlendBypass2;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function enables Chroma Keying
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
******************************************************************************/
u32 XDcSub_SetChromaKey(XDcSub *InstancePtr, u8 ChromaEnable, u8 ChromaMasterSel,
			XDc_ChromaKey *Key)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((ChromaEnable != CHROMA_ENABLE) |
			  (ChromaEnable != CHROMA_DISABLE));
	Xil_AssertNonvoid((ChromaMasterSel != MASTER_STREAM1) |
			  (ChromaMasterSel != MASTER_STREAM2));
	Xil_AssertNonvoid(Key != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.ChromaEnable = ChromaEnable;
	DcConfigPtr->Blender.ChromaMasterSel = ChromaMasterSel;
	DcConfigPtr->Blender.ChromaKey = Key;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables Cursor Blending
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Cursor Coordinates.
 * @param       Cursor Video Attributes.
 *

 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_SetCursorBlend(XDcSub *InstancePtr, XDc_CursorBlend Enable,
			  XDc_Cursor *Cursor)
{
	XDc_Blender *DcBlender;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Enable != CB_ENABLE);
	Xil_AssertNonvoid(Cursor != NULL);
	Xil_AssertNonvoid(Cursor->CursorAttribute->VideoFormat
			  != RGBA4444);

	DcBlender = &InstancePtr->DcPtr->Blender;

	DcBlender->CursorEnable = Enable;

	DcBlender->Cursor.CursorAttribute =
		Cursor->CursorAttribute;

	DcBlender->Cursor.CoordY = Cursor->CoordY;
	DcBlender->Cursor.CoordX = Cursor->CoordX;
	DcBlender->Cursor.SizeY = Cursor->SizeY;
	DcBlender->Cursor.SizeX = Cursor->SizeX;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the Audio and Video Clock Source and the video timing
 * source.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        System uses PL Clock for Video when Live source is in use.
 *
*******************************************************************************/
u32 XDcSub_SetAudioVideoClkSrc(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	if ((DcConfigPtr->AVMode.VideoSrc1 != XDC_VIDSTREAM1_LIVE) &&
	    (DcConfigPtr->AVMode.VideoSrc2 != XDC_VIDSTREAM2_LIVE)) {
		DcConfigPtr->VideoClk = XDC_PS_CLK;
		DcConfigPtr->AudioClk = XDC_PS_CLK;
		DcConfigPtr->VidTimingSrc = 1;
	} else if ((DcConfigPtr->AVMode.VideoSrc1 == XDC_VIDSTREAM1_LIVE) ||
		   (DcConfigPtr->AVMode.VideoSrc2 == XDC_VIDSTREAM2_LIVE)) {
		DcConfigPtr->VideoClk = XDC_PL_CLK;
		DcConfigPtr->AudioClk = XDC_PL_CLK;
		DcConfigPtr->VidTimingSrc = 0;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/** This function configures DC Video Clock Select
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Select configures Video Clock mode
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_VidClkSelect(XDcSub *InstancePtr, u8 Stream1Sel, u8 Stream2Sel)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream1Sel == 0x0) || (Stream1Sel == 0x1));
	Xil_AssertNonvoid((Stream2Sel == 0x0) || (Stream2Sel == 0x1));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->VidClkSelect = Stream1Sel | (Stream2Sel << 0x1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the video channel interface between the DCDMA
 * and the
 * DC
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Enable sets the corresponding buffers.
 * @param       Burst programs burstlength.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_EnableStream1Buffers(XDcSub *InstancePtr, u8 Enable, u8 Burst)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Stream1ChannelEn = Enable;
	DcConfigPtr->Stream1BurstLen = Burst;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the video channel interface between the DCDMA
 * and the
 * DC
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Enable sets the corresponding buffers.
 * @param       Burst programs burstlength.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_EnableStream2Buffers(XDcSub *InstancePtr, u8 Enable, u8 Burst)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Stream2ChannelEn = Enable;
	DcConfigPtr->Stream2BurstLen = Burst;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables End of Line Reset for reduced blanking
 * resolutions.
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Disable is to be set while using Reduced Blanking
 * Resolutions.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_AudLineResetDisable(XDcSub *InstancePtr, u8 Disable)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudLineRstDisable = Disable;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables Extra BS Control for AUDIO_SOFT_RESET
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Bypass is to be override the extra BS on link.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_AudExtraBSControl(XDcSub *InstancePtr, u8 Bypass)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudBypassExtraBS = Bypass;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables SDP from DMA or PL
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Enable sets the corresponding buffers.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_SetSdp(XDcSub *InstancePtr, XDc_Sdp Sdp)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Sdp = Sdp;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets SDP Empty Threshold value
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Empty Threshold value.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_SetSdpEmptyThreshold(XDcSub *InstancePtr, u8 Threshold)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->SdpEmptyThreshold = Threshold;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables SDP & Cursor buffers
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Enable.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_SetSdpCursorBuffers(XDcSub *InstancePtr, u8 Enable, u8 BurstLen)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->SdpChannelEn = Enable;
	DcConfigPtr->SdpBurstLen = BurstLen;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables Partial Blend for both video streams
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       Stream1_Enable.
 * @param       Stream2_Enable.
 * @param       Stream1 Partial Blend Coordinates.
 * @param       Stream2 Partial Blend Coordinates.
 *

 * @return      XST_SUCCESS or XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
u32 XDcSub_SetStreamPartialBlend(XDcSub *InstancePtr, XDc_PartialBlendEn Enable1,
				 XDc_PartialBlend *Coords1, XDc_PartialBlendEn Enable2, XDc_PartialBlend *Coords2)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->Blender.Stream1PbEn = Enable1;
	DcConfigPtr->Blender.Stream1PbCoords = Coords1;

	DcConfigPtr->Blender.Stream2PbEn = Enable2;
	DcConfigPtr->Blender.Stream2PbCoords = Coords2;

	return XST_SUCCESS;
}

/******************************************************************************/
/** This function configures the register to gate input video on/off at video
 * frame boundary.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Val bits set different control signals
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetVidFrameSwitch(XDcSub *InstancePtr, u32 Control)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->VidFrameSwitchCtrl = Control;

	return XST_SUCCESS;
}

/******************************************************************************/
/** This function configures the memory fetch latency
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Val bits sets the offset for early timing
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetNonLiveLatency(XDcSub *InstancePtr, u32 Latency)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->NonLiveLatency = Latency;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function enables STC Control
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_EnableStcCtrl(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->StcEn = 0x0;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function clears STC Control
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_ClearStcCtrl(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->StcEn = 0x1;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function loads STC Init value
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcSub_SetStcLoad(XDcSub *InstancePtr, u64 InitVal)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->StcInitVal = InitVal;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function Adjust STC value
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcSub_SetStcAdjust(XDcSub *InstancePtr, u32 InitVal)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->StcInitVal = InitVal;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function reads VSync STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u64 XDcSub_GetStcVSyncTs(XDcSub *InstancePtr)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	return DcConfigPtr->StcVSyncTs;

}

/******************************************************************************/
/** This function reads External VSync STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u64 XDcSub_GetStcExtVSyncTs(XDcSub *InstancePtr)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	return DcConfigPtr->StcExtVSyncTs;

}

/******************************************************************************/
/** This function reads Custom VSync Event STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u64 XDcSub_GetStcCustomEventTs(XDcSub *InstancePtr)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	return DcConfigPtr->StcCustomEventTs;

}

/******************************************************************************/
/** This function reads Custom VSync Event STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u64 XDcSub_GetStcCustomEvent2Ts(XDcSub *InstancePtr)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	return DcConfigPtr->StcCustomEvent2Ts;

}

/******************************************************************************/
/** This function  returns StcSnapshot value
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u64 XDcSub_GetStcSnapshot(XDcSub *InstancePtr)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	return DcConfigPtr->StcSnapshot;

}

/******************************************************************************/
/** This function Enable Audio Buffer channel and sets the burst length
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_EnableAudioBuffer(XDcSub *InstancePtr, u8 Enable, u8 BurstLen)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudChannelEn = Enable;
	DcConfigPtr->AudBurstLen = BurstLen;

	return XST_SUCCESS;
}

/******************************************************************************/
/** This function enables audio channel select and programs sample rate
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       ChannelSel selects the different audio channels.
 * @param       SampleRate selects the different sample rates.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_AudioChannelSelect(XDcSub *InstancePtr, u8 ChannelSel, u16 SampleRate)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudChannelSel = ChannelSel;
	DcConfigPtr->AudSampleRate = SampleRate;

	return XST_SUCCESS;
}

/******************************************************************************/
/** This function configures DC Audio Clock Select
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Select configures Audio Clock mode
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_AudClkSelect(XDcSub *InstancePtr, u32 Select)
{

	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudClkSelect = Select;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets the Interface mode for DCSub DC Instance.
 * DC can be either in bypass mode or functional mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       Mode is the enum val to enable or diasable bypass mode.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetAudInterfaceMode(XDcSub *InstancePtr, XDc_AudInterface Mode)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);
	Xil_AssertNonvoid((Mode != XDC_AUD_BYPASS) |
			  (Mode != XDC_AUD_FUNCTIONAL));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudInterface = Mode;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function enables DC Aud mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_EnableAudio(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudioEnable = 0x1;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function disable DC Aud mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_DisableAudio(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudioEnable = 0x0;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function allows user to disable channelX preamble and status.
 * Set 1 to disable, 0 to enable at bits 0-7 for corresponding channel.
 * Set bit 8 to load user_bit adn channel status.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetAudioChCtrl(XDcSub *InstancePtr, u16 AudChCtrl)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudChCtrl = AudChCtrl;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function sets  Audio Segmented Mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_SetAudioSegmentedMode(XDcSub *InstancePtr, u8 AudSegmentedMode)
{
	XDc *DcConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->AudSegmentedMode = AudSegmentedMode;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function sets Cursor Sdp Ready Interval.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u32 XDcSub_SetCursorSdpRdyInterval(XDcSub *InstancePtr, u16 RdyInterval)
{
	XDc *DcConfigPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->RdyInterval = RdyInterval;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function sets SDP ACK from either DP or PL.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDcSub_SetSdpAckSel(XDcSub *InstancePtr, u8 AckSel)
{
	XDc *DcConfigPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);
	Xil_AssertNonvoid((AckSel == XDC_SDP_DP_ACK) ||
			  (AckSel == XDC_SDP_PL_ACK));

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->SdpAckSel = AckSel;

	return XST_SUCCESS;

}

/******************************************************************************/
/** This function enable DC SDP.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDcSub_EnableSdp(XDcSub *InstancePtr)
{
	XDc *DcConfigPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->DcPtr != NULL);

	DcConfigPtr = InstancePtr->DcPtr;

	DcConfigPtr->SdpEnable = 0x1;

	return XST_SUCCESS;

}
/******************************************************************************/
/**
 * This function configures Dc Video registers in the required sequence.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
 *
*******************************************************************************/
u32 XDcSub_ConfigureDcVideo(XDc *InstancePtr)
{
	XDc_VideoStream1 VideoSrc1;
	XDc_VideoStream2 VideoSrc2;

	VideoSrc1 = InstancePtr->AVMode.VideoSrc1;
	VideoSrc2 = InstancePtr->AVMode.VideoSrc2;

	XDc_ConfigureStream(InstancePtr, VideoSrc1);
	XDc_ConfigureStream(InstancePtr, VideoSrc2);

	XDc_SetOutputCSC(InstancePtr);

	XDc_SetNonLiveLatency(InstancePtr);

	XDc_SetOutputVideoFormat(InstancePtr);

	if (InstancePtr->Blender.AlphaEnable == ALPHA_ENABLE) {
		XDc_SetGlobalAlpha(InstancePtr);
	}

	if (InstancePtr->Blender.ChromaEnable == CHROMA_ENABLE) {
		XDc_SetChromaKey(InstancePtr);
	}

	if (InstancePtr->Blender.CursorEnable == CB_ENABLE) {
		XDc_SetCursorBlend(InstancePtr);
		XDc_SetSdpCursorBuffers(InstancePtr);
	}

	XDc_EnablePartialBlend(InstancePtr);

	if (InstancePtr->SdpEnable) {
		XDc_SetSdpSource(InstancePtr);
		XDc_SetCursorSdpRdyInterval(InstancePtr);
		XDc_SetSdpAckSel(InstancePtr);
		XDc_SetSdpCursorBuffers(InstancePtr);
	}

	if (InstancePtr->AudioEnable) {
		XDc_EnableAudio(InstancePtr);
		XDc_EnableAudioBuffer(InstancePtr);
		XDc_AudioChannelSelect(InstancePtr);
	}

	XDc_SetAudioVideoClkSrc(InstancePtr);

	if (VideoSrc1 != XDC_VIDSTREAM1_NONE) {
		XDc_EnableStream1Buffers(InstancePtr);
	}

	if (VideoSrc2 != XDC_VIDSTREAM2_NONE) {
		XDc_EnableStream2Buffers(InstancePtr);
	}

	XDc_SetInputVideoSelect(InstancePtr);

	if (InstancePtr->AudioEnable) {
		XDc_SetInputAudioSelect(InstancePtr);
	}

	return XST_SUCCESS;
}

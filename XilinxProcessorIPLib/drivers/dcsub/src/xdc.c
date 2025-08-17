/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdc.c
 * @addtogroup mmi_dc Overview
 * @{
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
 * 1.0 	 ck    03/14/25  Initial Release
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/
#include <stdlib.h>
#include "xdc.h"
#include <xstatus.h>

/******************************* Constant Definitions  ********************************/
extern const XDc_VideoAttribute XDc_SupportedFormats[XDC_NUM_SUPPORTED];

/******************************* Macro Definitions  ********************************/
#define XDC_MAX_SF_REG_COUNT		3
#define XDC_MAX_CSC_COEFF_REG_COUNT	9
#define XDC_MAX_CSC_OFFSET_REG_COUNT	3
#define XDC_TOTAL_CSC_REG_COUNT		12

/******************************************************************************/
/**
 * This function intializes the configuration for the DC Instance.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       BaseAddr sets the base address of the DC instance
 * @param       DeviceId is the id of the device from the design.
 *
 * @return      None.
 *
 * @note        Base address and DeviceId is same as the DP Core driver.
 *
*******************************************************************************/
void XDc_CfgInitialize(XDc *InstancePtr, u32 BaseAddr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Config.BaseAddr = BaseAddr;

}

/******************************************************************************/
/**
 * This function initializes all the data structures of the XDc
 * Instance.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XDc_Initialize(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->VidInterface = XDC_VID_FUNCTIONAL;
	InstancePtr->AudInterface = XDC_AUD_FUNCTIONAL;

	InstancePtr->AVMode.VideoSrc1 = XDC_VIDSTREAM1_NONE;
	InstancePtr->AVMode.VideoSrc2 = XDC_VIDSTREAM2_NONE;

	InstancePtr->Blender.AlphaEnable = ALPHA_ENABLE;
	InstancePtr->Blender.Alpha = 0;

	InstancePtr->Blender.ChromaEnable = CHROMA_DISABLE;
	InstancePtr->Blender.ChromaMasterSel = MASTER_STREAM1;

	InstancePtr->Blender.CursorEnable = CB_DISABLE;

	InstancePtr->Blender.Stream1PbEn = PB_DISABLE;
	InstancePtr->Blender.Stream2PbEn = PB_DISABLE;

	InstancePtr->Sdp = SDP_PL;

	InstancePtr->StcEn = 0x1;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AUD_SOFT_RST, 0);

}

/******************************************************************************/
/** This function disables writes to DC
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_WriteProtEnable(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_WPROTS, XDC_WPROTS_ENABLE);

}

/******************************************************************************/
/** This function enables writes to DC
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_WriteProtDisable(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_WPROTS, XDC_WPROTS_DISABLE);

}

/******************************************************************************/
/**
 * This function sets the mode for DC Instance. DC can be either in bypass mode
 * or functional mode
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetVidInterfaceMode(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_DC_BYPASS,
		     InstancePtr->VidInterface);
}

/******************************************************************************/
/**
 * This function sets the blender background color
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 * @param       Color is a pointer to the structure XDc_BlenderBgClr
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
void XDc_SetBlenderBgColor(XDc *InstancePtr)
{
	XDc_BlenderBgClr *BgClr;

	Xil_AssertVoid(InstancePtr != NULL);

	BgClr = InstancePtr->BgClr;

	Xil_AssertVoid(BgClr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_V_BLEND_BG_CLR_0,
		     BgClr->RCr);
	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_V_BLEND_BG_CLR_1,
		     BgClr->GY);
	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_V_BLEND_BG_CLR_2,
		     BgClr->BCb);

}

/******************************************************************************/
/**
 * This function enables or disables global alpha
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
 * @note        GlobalAlphaEn = 1, enables the global alpha.
 *              GlobalAlphaEn = 0, disables the global alpha.
 *              Alpha = 0, transparent
 *              Alpha = 255, Opaque
 *              Alpha = 8:1 (Alpha value)
******************************************************************************/
void XDc_SetGlobalAlpha(XDc *InstancePtr)
{
	u32 RegVal;
	XDc_Blender *Blender;

	Xil_AssertVoid(InstancePtr != NULL);

	Blender = &InstancePtr->Blender;

	RegVal = Blender->AlphaEnable;
	RegVal |= Blender->Alpha <<
		  XDC_V_BLEND_SET_GLOBAL_ALPHA_REG_VALUE_SHIFT;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_SET_GLOBAL_ALPHA_REG, RegVal);

}

/******************************************************************************/
/**
 * This function selects the source type for the Video and Graphics input streams
 * that are passed on to the blender block.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XDc_SetInputVideoSelect(XDc *InstancePtr)
{

	u32 RegVal;

	XDc_VideoStream1 VidStream1;
	XDc_VideoStream2 VidStream2;

	Xil_AssertVoid(InstancePtr != NULL);

	VidStream1 = InstancePtr->AVMode.VideoSrc1;
	VidStream2 = InstancePtr->AVMode.VideoSrc2;

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_AV_BUF_OUTPUT_AUD_VID_SELECT);

	RegVal &= ~(XDC_AV_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM1_SEL_MASK);
	RegVal &= ~(XDC_AV_BUF_OUTPUT_AUD_VID_SELECT_VID_STREAM2_SEL_MASK);

	RegVal = VidStream1 | VidStream2;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_OUTPUT_AUD_VID_SELECT, RegVal);

}

/******************************************************************************/
/**
 * This function selects the source type for audio stream
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XDc_SetInputAudioSelect(XDc *InstancePtr)
{
	u32 RegVal;
	XDc_AudioStream AudStream;

	Xil_AssertVoid(InstancePtr != NULL);

	AudStream = InstancePtr->AVMode.AudSrc;

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_AV_BUF_OUTPUT_AUD_VID_SELECT);

	RegVal &= ~(XDC_AV_BUF_OUTPUT_AUD_VID_SELECT_AUD_STREAM1_SEL_MASK);

	RegVal |= (AudStream <<
		   XDC_AV_BUF_OUTPUT_AUD_VID_SELECT_AUD_STREAM1_SEL_SHIFT);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_OUTPUT_AUD_VID_SELECT, RegVal);

}

/******************************************************************************/
/**
 * This function looks up if the video format is valid NonLiveVideo format or
 * not and returns a pointer to the attributes of the video.
 *
 * @param       Format takes in the video format for which attributes are being
 *              requested.
 *
 * @return      A pointer to the structure XDc_VideoAttribute if the video
 *              format is valid, else returns NULL.
 * @note        None.
*******************************************************************************/
XDc_VideoAttribute *XDc_GetNonLiveVideoAttribute(XDc_VideoFormat Format)
{
	u8 Index;

	for (Index = CbY0CrY1; Index <= Ydcl_ONLY_12BPC; Index++) {
		XDc_VideoAttribute *VideoAttribute;
		VideoAttribute = &XDc_SupportedFormats[Index];
		if (Format == VideoAttribute->VideoFormat) {
			return VideoAttribute;
		}
	}
	return NULL;
}

/******************************************************************************/
/**
 * This function applies Attributes for Non - Live source(Stream1/Stream2).
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
******************************************************************************/
void XDc_SetNonLiveInputFormat(XDc *InstancePtr, u8 VideoSrc,
			       XDc_VideoAttribute *Video)
{
	u32 RegVal = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr, XDC_AV_BUF_FORMAT);

	if (VideoSrc == XDC_VIDSTREAM1_NONLIVE) {
		RegVal &= ~XDC_AV_BUF_FORMAT_NL_VID0_FORMAT_MASK;
		RegVal |= Video->Id;
	} else if (VideoSrc == XDC_VIDSTREAM2_NONLIVE) {
		RegVal &= ~XDC_AV_BUF_FORMAT_NL_VID1_FORMAT_MASK;
		RegVal |= (Video->Id) <<
			  XDC_AV_BUF_FORMAT_NL_VID1_FORMAT_SHIFT;
	}

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AV_BUF_FORMAT,
		     RegVal);

}

/******************************************************************************/
/**
 * This function looks up if the video format is valid LiveVideo format or not
 * and returns a pointer to the attributes of the video.
 *
 * @param       Format takes in the video format for which attributes are being
 *              requested.
 *
 * @return      A pointer to the structure XDc_VideoAttribute if the video
 *              format is valid, else returns NULL.
 *
 * @note        None.
*******************************************************************************/
XDc_VideoAttribute *XDc_GetLiveVideoAttribute(XDc_VideoFormat Format)
{
	u8 Index;

	for (Index = Y_ONLY_8BPC; Index <= YCbCr422_12BPC; Index++) {
		XDc_VideoAttribute *VideoAttribute;
		VideoAttribute = &XDc_SupportedFormats[Index];
		if (Format == VideoAttribute->VideoFormat) {
			return VideoAttribute;
		}
	}
	return NULL;
}

/******************************************************************************/
/**
 * This function applies Attributes for Live source(Video/Graphics).
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 * @param	RegConfig points to LIVE_VID_CONFIG or LIVE_GFX_CONFIG reg
 * @param       Video is a pointer to the attributes of the video to be applied
 *
 * @return      None.
 *
 * @Note 	BPC values to be programmed are
 * 		001: 8BPC
 * 		010: 10BPC
 * 		011: 12BPC
 * 		XDc_VideoFormats stores bits per pixel(BPP) value i.e
 *		24 for 8BPC, 30 for 10BPC, 36 for 12BPC.
 * 		Dividing the BPP by 6 and subtractig with 3
 * 		generates the required above values to be programmed.
 *
******************************************************************************/
static void XDc_SetLiveInputFormat(XDc *InstancePtr, u32 RegConfig,
				   XDc_VideoAttribute *Video)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal |= Video->Id << XDC_AV_BUF_LIVE_VID_CFG_FORMAT_SHIFT;
	RegVal |= Video->BPP / 6 - 3;
	RegVal |= Video->CbFirst << XDC_AV_BUF_LIVE_VID_CFG_CB_FIRST_SHIFT;

	XDc_WriteReg(InstancePtr->Config.BaseAddr, RegConfig, RegVal);

}

/******************************************************************************/
/**
 * This function looks up if the output video format is valid or not and returns a
 * pointer to the attributes of the video.
 *
 * @param       Format takes in the output video format for which attributes are being
 *              requested.
 *
 * @return      A pointer to the structure XDc_VideoAttribute if the video
 *              format is valid, else returns NULL.
 * @note        None.
*******************************************************************************/
XDc_VideoAttribute *XDc_GetOutputVideoAttribute(XDc_VideoFormat Format,
	XDc_VidInterface Mode)
{
	u8 Index;

	Index = (Mode == XDC_VID_BYPASS) ? RGB_6BPC : Y_ONLY_8BPC;

	for (; Index <= YCbCr422_12BPC; Index++) {
		XDc_VideoAttribute *VideoAttribute;
		VideoAttribute = (XDc_VideoAttribute *)
				 &XDc_SupportedFormats[Index];
		if (Format == VideoAttribute->VideoFormat) {
			return VideoAttribute;
		}
	}
	return NULL;
}

/******************************************************************************/
/**
 * This function configures the Output of the Video Pipeline
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
void XDc_SetOutputVideoFormat(XDc *InstancePtr)
{
	u32 RegVal;

	XDc_VideoAttribute *OutVideo;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->AVMode.OutputVideo != NULL);

	OutVideo = InstancePtr->AVMode.OutputVideo;

	RegVal |= OutVideo->SubSample <<
		  XDC_V_BLEND_OUTPUT_VID_FORMAT_EN_DOWNSAMPLE_SHIFT;
	RegVal |= OutVideo->Id;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_OUTPUT_VID_FORMAT, RegVal);

}

/******************************************************************************/
/**
 * This function sets the scaling factors depending on the source video
 * stream.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param	RegOffset
 * @param       ScaleFactors is an array with per component scale
 *             factor to scale to 12 BPC
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
static void XDc_SetScalingFactors(XDc *InstancePtr, u32 RegOffset, u32 *ScalingFactors)
{

	u8 Index;

	for (Index = 0; Index < XDC_MAX_SF_REG_COUNT; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4), ScalingFactors[Index]);
	}

}

/******************************************************************************/
/**
 * This function programs the coeffitients for Input Color Space Conversion.
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 * @param       RegOffset is a register offset for Video or Graphics
 *              config register
 * @param       CSCCoeffs is matrix with coefficient values
 * @param       CSCOffset is offset matrix for Y, Cb, Cr for pre-post
 *              matrix multiplication
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
static void XDc_SetInputCSC(XDc *InstancePtr, u32 RegOffset, u32 *CSCCoeffs,
			    u32 *CSCOffset)
{
	u8 Index;

	/* Program Colorspace conversion coefficients */
	for (Index = XDC_MAX_CSC_COEFF_REG_COUNT; Index < XDC_TOTAL_CSC_REG_COUNT; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4),
			     CSCOffset[Index - XDC_MAX_CSC_COEFF_REG_COUNT]);
	}

	/* Program Colorspace conversion matrix */
	for (Index = 0; Index < XDC_MAX_CSC_COEFF_REG_COUNT; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4),
			     CSCCoeffs[Index]);
	}

}

/******************************************************************************/
/**
 * This function sets the Layer Control for Streams.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       RegOffset is the register offset of Video Layer or
 *              Graphics Layer from the base address
 * @param       blendbypass is a 8 bit value to set layer as passed
 *              through as the blender out
 * @param       Video is a pointer to the XDc_VideoAttribute struct
 * which
 *              has been configured for the particular layer
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
static void XDc_SetLayerControl(XDc *InstancePtr, u32 RegOffset, u8
				BlendBypass, XDc_VideoAttribute *Video)
{
	u32 RegVal;

	RegVal = (Video->IsRGB <<
		  XDC_V_BLEND_LAYER0_CONTROL_RGB_MODE_SHIFT) |
		 (BlendBypass <<
		  XDC_V_BLEND_LAYER0_CONTROL_BYPASS_SHIFT) |
		 Video->SubSample;

	XDc_WriteReg(InstancePtr->Config.BaseAddr, RegOffset,
		     RegVal);
}

/******************************************************************************/
/**
 * This function enables Partial Blend for both video streams
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 * @param       RegOffset address to program
 * @param       Video contains partial blend coords to set.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
static void XDc_SetPartialBlend(XDc *InstancePtr, u32 RegOffset,
				XDc_PartialBlend *Video )
{
	u32 RegVal;
	u32 PBSize;
	u32 PBOffset;

	RegVal = XDC_V0_PB_SELF_CLEAR_MASK;
	RegVal |= 0x1 << XDC_V0_PB_ENABLE_SHIFT;
	RegVal |= Video->CoordY << XDC_V0_PB_COORD_Y_SHIFT;
	RegVal |= Video->CoordX << XDC_V0_PB_COORD_X_SHIFT;

	PBSize = Video->SizeY << XDC_V0_PARTIALBLEND_SIZE_Y_SHIFT;
	PBSize |= Video->SizeX;

	PBOffset = Video->OffsetY <<
		   XDC_V0_PARTIALBLEND_COORD_OFFSET_Y_SHIFT;
	PBOffset |= Video->OffsetX;

	XDc_WriteReg(InstancePtr->Config.BaseAddr, RegOffset + 8, PBOffset);
	XDc_WriteReg(InstancePtr->Config.BaseAddr, RegOffset + 4, PBSize);
	XDc_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function enables Partial Blend for corresponding video stream
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_EnablePartialBlend(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Blender.Stream1PbEn) {
		XDc_SetPartialBlend(InstancePtr, XDC_V0, InstancePtr->Blender.Stream1PbCoords);
	} else if (InstancePtr->Blender.Stream2PbEn) {
		XDc_SetPartialBlend(InstancePtr, XDC_V1, InstancePtr->Blender.Stream2PbCoords);
	}

}

/******************************************************************************/
/** This function configures DC Blender
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 * @param       VideoSrc points to the stream source
 *
 * @return      None.
 *
*******************************************************************************/
u32 XDc_ConfigureStream(XDc *InstancePtr, u8 StreamSrc)
{

	u32 ScalingOffset = 0;
	u32 CSCOffset = 0;
	u32 LayerOffset = 0;
	u32 RegOffset = 0;

	u8 Stream_BlendBypass = 0;
	u32 *ScalingFactors = NULL;
	u32 *Stream_CSCCoeff = NULL;
	u32 *Stream_CSCOffset = NULL;
	XDc_VideoAttribute *Video = NULL;

	Xil_AssertNonvoid(InstancePtr != NULL);

	switch (StreamSrc) {
		case XDC_VIDSTREAM1_LIVE:
			ScalingOffset = XDC_AV_BUF_LIVE_VID_COMP0_SF;
			ScalingFactors = InstancePtr->Blender.Stream1ScaleFactors;
			CSCOffset = XDC_V_BLEND_IN1CSC_COEFF0;
			Stream_CSCCoeff = InstancePtr->Blender.Stream1CSCCoeff;
			Stream_CSCOffset = InstancePtr->Blender.Stream1CSCOffset;
			LayerOffset = XDC_V_BLEND_LAYER0_CONTROL;
			Stream_BlendBypass = InstancePtr->Blender.Stream1BlendBypass;
			RegOffset = XDC_AV_BUF_LIVE_VID_CFG;
			Video = InstancePtr->AVMode.LiveVideo1;
			XDc_SetLiveInputFormat(InstancePtr, RegOffset, Video);
			break;

		case XDC_VIDSTREAM1_NONLIVE:
			ScalingOffset = XDC_AV_BUF_VID_COMP0_SCALE_FACTOR;
			ScalingFactors = InstancePtr->Blender.Stream1ScaleFactors;
			CSCOffset = XDC_V_BLEND_IN1CSC_COEFF0;
			Stream_CSCCoeff = InstancePtr->Blender.Stream1CSCCoeff;
			Stream_CSCOffset = InstancePtr->Blender.Stream1CSCOffset;
			LayerOffset = XDC_V_BLEND_LAYER0_CONTROL;
			Stream_BlendBypass = InstancePtr->Blender.Stream1BlendBypass;
			Video = InstancePtr->AVMode.NonLiveVideo1;
			XDc_SetNonLiveInputFormat(InstancePtr, StreamSrc, Video);
			break;

		case XDC_VIDSTREAM2_LIVE:
			ScalingOffset = XDC_AV_BUF_LIVE_GFX_COMP0_SF;
			ScalingFactors = InstancePtr->Blender.Stream2ScaleFactors;
			CSCOffset = XDC_V_BLEND_IN2CSC_COEFF0;
			Stream_CSCCoeff = InstancePtr->Blender.Stream2CSCCoeff;
			Stream_CSCOffset = InstancePtr->Blender.Stream2CSCOffset;
			LayerOffset = XDC_V_BLEND_LAYER1_CONTROL;
			Stream_BlendBypass = InstancePtr->Blender.Stream2BlendBypass;
			RegOffset = XDC_AV_BUF_LIVE_GFX_CFG;
			Video = InstancePtr->AVMode.LiveVideo2;
			XDc_SetLiveInputFormat(InstancePtr, RegOffset, Video);

			break;

		case XDC_VIDSTREAM2_NONLIVE:
			ScalingOffset = XDC_AV_BUF_GRAPHICS_COMP0_SCALE_FACTOR;
			ScalingFactors = InstancePtr->Blender.Stream2ScaleFactors;
			CSCOffset = XDC_V_BLEND_IN2CSC_COEFF0;
			Stream_CSCCoeff = InstancePtr->Blender.Stream2CSCCoeff;
			Stream_CSCOffset = InstancePtr->Blender.Stream2CSCOffset;
			LayerOffset = XDC_V_BLEND_LAYER1_CONTROL;
			Stream_BlendBypass = InstancePtr->Blender.Stream2BlendBypass;
			Video = InstancePtr->AVMode.NonLiveVideo2;
			XDc_SetNonLiveInputFormat(InstancePtr, StreamSrc, Video);
			break;

		default:
			return XST_FAILURE;
	}

	XDc_SetScalingFactors(InstancePtr, ScalingOffset, ScalingFactors);
	XDc_SetInputCSC(InstancePtr, CSCOffset,
			Stream_CSCCoeff, Stream_CSCOffset);
	XDc_SetLayerControl(InstancePtr, LayerOffset,
			    Stream_BlendBypass, Video);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function converts the Blender output to the desired output
 * format.
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
void XDc_SetOutputCSC(XDc *InstancePtr)
{
	u32 Index;
	u32 RegOffset;
	u32 ColorOffset;
	u32 *CSCCoeff;
	u32 *CSCOffset;

	Xil_AssertVoid(InstancePtr != NULL);

	RegOffset = XDC_V_BLEND_RGB2YCBCR_COEFF0;

	ColorOffset = XDC_V_BLEND_LUMA_OUTCSC_OFFSET;

	CSCCoeff = InstancePtr->Blender.OutCSCCoeff;
	CSCOffset = InstancePtr->Blender.OutCSCOffset;

	for (Index = 0; Index < XDC_MAX_CSC_COEFF_REG_COUNT; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4), CSCCoeff[Index]);
	}

	for (Index = 0; Index < XDC_MAX_CSC_OFFSET_REG_COUNT; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     ColorOffset + (Index * 4),
			     (CSCOffset[Index] <<
			      XDC_V_BLEND_LUMA_IN1CSC_OFFSET_POST_OFFSET_SHIFT));
	}

}

/******************************************************************************/
/**
 * This function enables Chroma Keying
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
void XDc_SetChromaKey(XDc *InstancePtr)
{
	u32 ChromaEn;
	u32 KeyComp1;
	u32 KeyComp2;
	u32 KeyComp3;

	Xil_AssertVoid(InstancePtr != NULL);

	ChromaEn = InstancePtr->Blender.ChromaEnable;
	ChromaEn |= InstancePtr->Blender.ChromaMasterSel <<
		    XDC_V_BLEND_CHROMA_KEY_ENABLE_M_SEL_SHIFT;

	KeyComp1 = InstancePtr->Blender.ChromaKey->RMin;
	KeyComp1 |= InstancePtr->Blender.ChromaKey->RMax <<
		    XDC_V_BLEND_CHROMA_KEY_COMP1_MAX_SHIFT;

	KeyComp2 = InstancePtr->Blender.ChromaKey->GMin;
	KeyComp2 |= InstancePtr->Blender.ChromaKey->GMax <<
		    XDC_V_BLEND_CHROMA_KEY_COMP2_MAX_SHIFT;

	KeyComp3 = InstancePtr->Blender.ChromaKey->BMin;
	KeyComp3 |= InstancePtr->Blender.ChromaKey->BMax <<
		    XDC_V_BLEND_CHROMA_KEY_COMP3_MAX_SHIFT;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_CHROMA_KEY_ENABLE, ChromaEn);
	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_CHROMA_KEY_COMP1, KeyComp1);
	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_CHROMA_KEY_COMP2, KeyComp2);
	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_V_BLEND_CHROMA_KEY_COMP3, KeyComp3);

}

/******************************************************************************/
/**
 * This function enables Cursor blending
 *
 * @param       InstancePtr is an pointer to the XDc Instance.
 *
 * @return      None.
 *
 * @note        None.
******************************************************************************/
void XDc_SetCursorBlend(XDc *InstancePtr)
{
	u32 CoordVal;
	u32 SizeVal;
	XDc_Cursor *Cursor;

	Xil_AssertVoid(InstancePtr != NULL);

	Cursor = &InstancePtr->Blender.Cursor;

	CoordVal = XDC_CURSOR_ENABLE_MASK;
	CoordVal |= Cursor->CoordY << XDC_CURSOR_COORD_Y_SHIFT;
	CoordVal |= Cursor->CoordX;

	SizeVal = Cursor->SizeY << XDC_CURSOR_SIZE_Y_SHIFT;
	SizeVal |= Cursor->SizeX;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_CURSOR_COORDINATE, CoordVal);
	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_CURSOR_SIZE, SizeVal);

}

/******************************************************************************/
/**
 * This function sets the Audio and Video Clock Source and the video timing
 * source.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        System uses PL Clock for Video when Live source is in use.
 *
*******************************************************************************/
void XDc_SetAudioVideoClkSrc(XDc *InstancePtr)
{

	u32 RegVal = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal |= (InstancePtr->VideoClk <<
		   XDC_AV_BUF_AUD_VID_CLK_SOURCE_VID_CLK_SRC_SHIFT) |
		  (InstancePtr->AudioClk <<
		   XDC_AV_BUF_AUD_VID_CLK_SOURCE_AUD_CLK_SRC_SHIFT) |
		  (InstancePtr->VidTimingSrc << XDC_AV_BUF_AUD_VID_CLK_SOURCE_VID_TIMING_SRC_SHIFT);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_AUD_VID_CLK_SOURCE, RegVal);

}

/******************************************************************************/
/**
 * This function applies a soft reset to the Video pipeline.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XDc_VideoSoftReset(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AV_BUF_SRST_REG,
		     XDC_AV_BUF_SRST_REG_VID_RST_MASK);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AV_BUF_SRST_REG, 0);

}

/******************************************************************************/
/** This function configures DC Video Clock Select
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDc_VidClkSelect(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_VID_CLK,
		     InstancePtr->VidClkSelect);

}

/******************************************************************************/
/**
 * This function enables the video channel interface between the DCDMA
 * and the
 * DC
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_EnableStream1Buffers(XDc *InstancePtr)
{
	u32 RegVal;
	u32 RegOffset;
	u8 NumChannels;
	u8 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	NumChannels = InstancePtr->AVMode.NonLiveVideo1->Mode;

	RegOffset = XDC_AV_CHBUF0;

	RegVal = (InstancePtr->Stream1BurstLen << XDC_AV_CHBUF0_BURST_LEN_SHIFT) |
		 (XDC_AV_CHBUF0_FLUSH_MASK);

	for (Index = 0; Index <= NumChannels; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4), RegVal);
	}

	if (InstancePtr->Stream1ChannelEn) {
		RegVal = (InstancePtr->Stream1BurstLen << XDC_AV_CHBUF0_BURST_LEN_SHIFT) |
			 XDC_AV_CHBUF0_EN_MASK;
		for (Index = 0; Index <= NumChannels; Index++) {
			XDc_WriteReg(InstancePtr->Config.BaseAddr,
				     RegOffset + (Index * 4),
				     RegVal);
		}
	}

}

/******************************************************************************/
/**
 * This function enables the video channel interface between the DCDMA
 * and the
 * DC
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_EnableStream2Buffers(XDc *InstancePtr)
{
	u32 RegVal;
	u32 RegOffset;
	u8 NumChannels;
	u8 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	NumChannels = InstancePtr->AVMode.NonLiveVideo2->Mode;

	RegOffset = XDC_AV_CHBUF3;

	RegVal = (InstancePtr->Stream2BurstLen << XDC_AV_CHBUF3_BURST_LEN_SHIFT) |
		 (XDC_AV_CHBUF3_FLUSH_MASK);

	for (Index = 0; Index <= NumChannels; Index++) {
		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     RegOffset + (Index * 4), RegVal);
	}

	if (InstancePtr->Stream2ChannelEn) {
		RegVal = (InstancePtr->Stream2BurstLen << XDC_AV_CHBUF3_BURST_LEN_SHIFT) |
			 XDC_AV_CHBUF3_EN_MASK;
		for (Index = 0; Index <= NumChannels; Index++) {
			XDc_WriteReg(InstancePtr->Config.BaseAddr,
				     RegOffset + (Index * 4),
				     RegVal);
		}
	}

}

/******************************************************************************/
/**
 * This function applies a soft reset to the Audio pipeline.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XDc_AudioSoftReset(XDc *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_AUD_SOFT_RST);
	RegVal |= XDC_AUD_SOFT_RST_AUD_SRST_MASK;

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AUD_SOFT_RST,
		     RegVal);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AUD_SOFT_RST, 0);

}

/******************************************************************************/
/**
 * This function enables End of Line Reset for reduced blanking
 * resolutions.
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_AudLineResetDisable(XDc *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_AUD_SOFT_RST);
	if (InstancePtr->AudLineRstDisable) {
		RegVal |= XDC_AUD_SOFT_RST_LINE_RST_DISABLE_MASK;
	} else {
		RegVal &= ~XDC_AUD_SOFT_RST_LINE_RST_DISABLE_MASK;
	}

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AUD_SOFT_RST,
		     RegVal);

}

/******************************************************************************/
/**
 * This function enables Extra BS Control for AUDIO_SOFT_RESET
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_AudExtraBSControl(XDc *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_AUD_SOFT_RST);
	if (InstancePtr->AudBypassExtraBS) {
		RegVal |= XDC_AUD_SOFT_RST_EXTRA_BS_CONTROL_MASK;
	} else {
		RegVal &= ~XDC_AUD_SOFT_RST_EXTRA_BS_CONTROL_MASK;
	}

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_AUD_SOFT_RST,
		     RegVal);

}

/******************************************************************************/
/**
 * This function enables SDP from DMA or PL
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_SetSdpSource(XDc *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			     XDC_SDP);
	RegVal &= ~ XDC_SDP_SELECT_MASK;
	RegVal |= InstancePtr->Sdp;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_SDP, RegVal);
}

/******************************************************************************/
/**
 * This function sets SDP Empty Threshold value
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_SetSdpEmptyThreshold(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_SDP_EMPTY, InstancePtr->SdpEmptyThreshold);

}

/******************************************************************************/
/**
 * This function enables SDP & Cursor buffers
 *
 * @param       InstancePtr is a  pointer to the XDc Instance.
 *
 * @returns     None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDc_SetSdpCursorBuffers(XDc *InstancePtr)
{

	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_CHBUF_CURSOR_SDP,
		     XDC_AV_CHBUF_CURSOR_SDP_FLUSH_MASK);

	if (InstancePtr->SdpChannelEn) {
		RegVal = (InstancePtr->SdpBurstLen
			  << XDC_AV_CHBUF_CURSOR_SDP_BURST_LENGTH_SHIFT)
			 | XDC_AV_CHBUF_CURSOR_SDP_EN_MASK;

		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     XDC_AV_CHBUF_CURSOR_SDP, RegVal);

	}

}

/******************************************************************************/
/** This function configures the register to gate input video on/off at video
 * frame boundary.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetVidFrameSwitch(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_VIDEO_FRAME_SWITCH,
		     InstancePtr->VidFrameSwitchCtrl);

}

/******************************************************************************/
/** This function configures the memory fetch latency
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetNonLiveLatency(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_NON_LIVE_LATENCY,
		     InstancePtr->NonLiveLatency);

}

/******************************************************************************/
/** This function enables STC Control
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetStcCtrl(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_STC_CONTROL,
		     InstancePtr->StcEn);
}

/******************************************************************************/
/** This function loads STC Init value
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDc_SetStcLoad(XDc *InstancePtr)
{

	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = ((u64)(InstancePtr->StcInitVal)) >> 32;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_STC_INIT_VALUE0,
		     LOWER_32_BITS(InstancePtr->StcInitVal));

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_STC_INIT_VALUE1, RegVal);

}

/******************************************************************************/
/** This function Adjusts STC Control value
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetStcAdjust(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_BUF_STC_ADJ,
		     InstancePtr->StcAdjVal);
}

/******************************************************************************/
/** This function reads VSync STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u64 XDc_GetStcVSyncTs(XDc *InstancePtr)
{

	u64 RegVal0;
	u64 RegVal1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal0 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_VID_VSYNC_TS_REG0);

	RegVal1 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_VID_VSYNC_TS_REG1);

	InstancePtr->StcVSyncTs = (RegVal1 << 32) | (RegVal0);

	return InstancePtr->StcVSyncTs;

}

/******************************************************************************/
/** This function reads External VSync STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u64 XDc_GetStcExtVSyncTs(XDc *InstancePtr)
{

	u64 RegVal0;
	u64 RegVal1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal0 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_EXT_VSYNC_TS_REG0);

	RegVal1 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_EXT_VSYNC_TS_REG1);

	InstancePtr->StcExtVSyncTs =  (RegVal1 << 32) | (RegVal0);

	return InstancePtr->StcExtVSyncTs;

}

/******************************************************************************/
/** This function reads Custom Event STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u64 XDc_GetStcCustomEventTs(XDc *InstancePtr)
{

	u64 RegVal0;
	u64 RegVal1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal0 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_CUSTOM_EVENT_TS_REG0);

	RegVal1 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_CUSTOM_EVENT_TS_REG1);

	InstancePtr->StcCustomEventTs =  (RegVal1 << 32) | (RegVal0);

	return InstancePtr->StcCustomEventTs;

}

/******************************************************************************/
/** This function reads Custom Event2 STC value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u64 XDc_GetStcCustomEvent2Ts(XDc *InstancePtr)
{

	u64 RegVal0;
	u64 RegVal1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal0 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_CUSTOM_EVENT2_TS_REG0);

	RegVal1 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_CUSTOM_EVENT2_TS_REG1);

	InstancePtr->StcCustomEvent2Ts =  (RegVal1 << 32) | (RegVal0);

	return InstancePtr->StcCustomEvent2Ts;

}

/******************************************************************************/
/** This function reads STC Snapshot value and returns it
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
u64 XDc_GetStcSnapshot(XDc *InstancePtr)
{

	u64 RegVal0;
	u64 RegVal1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal0 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_SNAPSHOT0);

	RegVal1 = XDc_ReadReg(InstancePtr->Config.BaseAddr,
			      XDC_AV_BUF_STC_SNAPSHOT1);

	InstancePtr->StcSnapshot =  (RegVal1 << 32) | (RegVal0);

	return InstancePtr->StcSnapshot;

}

/******************************************************************************/
/** This function Enable Audio Buffer channel and sets the burst length
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_EnableAudioBuffer(XDc *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AV_CHBUF_AUD, XDC_AV_CHBUF_AUD_FLUSH_MASK);

	if (InstancePtr->AudChannelEn) {
		RegVal = (InstancePtr->AudBurstLen
			  << XDC_AV_CHBUF_AUD_BURST_LENGTH_SHIFT)
			 | (XDC_AV_CHBUF_AUD_EN_MASK);

		XDc_WriteReg(InstancePtr->Config.BaseAddr,
			     XDC_AV_CHBUF_AUD, RegVal);

	}

}

/******************************************************************************/
/** This function enables audio channel select and programs sample rate
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_AudioChannelSelect(XDc *InstancePtr)
{

	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = InstancePtr->AudChannelSel;
	RegVal |= InstancePtr->AudSampleRate
		  << XDC_NL_AUD_SAMPLE_RATE_SHIFT;

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_NL_AUD, RegVal);

}

/******************************************************************************/
/** This function configures DC Audio Clock Select
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_AudClkSelect(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AUD_CLK, InstancePtr->AudClkSelect);

}

/******************************************************************************/
/**
 * This function sets the audio mode for DC Instance.
 * Audio can be either in bypass mode
 * or functional mode
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetAudInterface(XDc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_DC_AUD,
		     InstancePtr->AudInterface);
}

/******************************************************************************/
/** This function enables audio to DC
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_EnableAudio(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_DC_AUDIO,
		     XDC_AUDIO_ENABLE);
}

/******************************************************************************/
/** This function disables audio to DC
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_DisableAudio(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr, XDC_DC_AUDIO,
		     XDC_AUDIO_DISABLE);
}

/******************************************************************************/
/** This function configures DC Timings register
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetVideoTiming(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_VideoTiming *VidTiming = &InstancePtr->VideoTiming;

	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_HTOTAL, VidTiming->HTotal);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_VTOTAL, VidTiming->VTotal);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_HSWIDTH, VidTiming->HSWidth);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_VSWIDTH, VidTiming->VSWidth);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_HRES, VidTiming->HRes);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_VRES, VidTiming->VRes);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_HSTART, VidTiming->HStart);
	XDc_WriteReg(XDC_TIMING_BASEADDR, XDC_TIMING_MAIN_STREAM_VSTART, VidTiming->VStart);

}

/******************************************************************************/
/** This function controls the Audio channel preamble and status bits.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetAudioChCtrl(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AUD_MIXER_VOLUME_CONTROL, InstancePtr->AudChCtrl);

}

/******************************************************************************/
/** This function sets Audio Segmented Mode.
 *
 * @param       InstancePtr is a pointer to the XDc instance.
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XDc_SetAudioSegmentedMode(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_AUD_MIXER_META_DATA, InstancePtr->AudSegmentedMode);

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
void XDc_SetCursorSdpRdyInterval(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_RDY_INTERVAL, InstancePtr->RdyInterval);

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
void XDc_SetSdpAckSel(XDc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDc_WriteReg(InstancePtr->Config.BaseAddr,
		     XDC_DP, InstancePtr->SdpAckSel);

}
/** @} */

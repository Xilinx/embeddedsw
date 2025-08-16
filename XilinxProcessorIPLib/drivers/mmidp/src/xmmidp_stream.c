/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_stream.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25	Initial release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <xstatus.h>
#include <sleep.h>

#include "xmmidp.h"

#define XMMIDP_STREAM_OFFSET	0x10000

/******************************************************************************/
/**
 * This function disables video stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_DisableVideoStream(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	XMmiDp_RegReadModifyWrite(InstancePtr, RegOffset,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_MASK,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT,
				  0x0);

}

/******************************************************************************/
/**
 * This function enabless video stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_EnableVideoStream(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	XMmiDp_RegReadModifyWrite(InstancePtr, RegOffset,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_MASK,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT,
				  0x1);

}

/******************************************************************************/
/**
 * This function clears MSA values
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearMsaValues(XMmiDp *InstancePtr, u8 Stream)
{

	u32 MsaRegOffset;
	u32 StreamOffset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	StreamOffset = (Stream - 1) * XMMIDP_STREAM_OFFSET ;

	for (Index = 0; Index < 3; Index++) {
		MsaRegOffset = (XMMIDP_VIDEO_MSA1 + (Index * 4)) + StreamOffset;
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, MsaRegOffset, 0x0);
	}

}

/******************************************************************************/
/**
 * This function clears out Video Config Register values
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearVideoConfigValues(XMmiDp *InstancePtr, u8 Stream)
{
	u32 Index;
	u32 StreamOffset;
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	StreamOffset = (Stream - 1) * XMMIDP_STREAM_OFFSET ;

	for (Index = 0; Index < 6; Index++) {
		RegOffset =  (XMMIDP_VINPUT_POLARITY_CTRL + (Index * 4)) + StreamOffset;
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, 0x0);
	}

}

/******************************************************************************/
/**
 * This function sets the video input polarity control
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVInputPolarityCtrl(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VINPUT_POLARITY_CTRL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VSyncInPolarity;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].HSyncInPolarity
		  << XMMIDP_VINPUT_POLARITY_CTRL_HSYNC_IN_POLARITY_SHIFT;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].DeInPolarity
		  << XMMIDP_VINPUT_POLARITY_CTRL_DIE_IN_POLARITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config1 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig1(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG1 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].RVBlankInOsc;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].I_P
		  << XMMIDP_VIDEO_CONFIG1_I_P_SHIFT ;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].HBlank
		  << XMMIDP_VIDEO_CONFIG1_HBLANK_SHIFT;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].HActive
		  << XMMIDP_VIDEO_CONFIG1_HACTIVE_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config2 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig2(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG2 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VActive;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].VBlank
		  << XMMIDP_VIDEO_CONFIG2_VBLANK_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config3 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig3(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG3 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].HSyncWidth
		 << XMMIDP_VIDEO_CONFIG3_HSYNC_WIDTH_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config4 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig4(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG4 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VSyncWidth
		 << XMMIDP_VIDEO_CONFIG4_VSYNC_WIDTH_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config5 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig5(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG5 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].AvgBytesPerTu;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].InitThreshold
		  << XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].AvgBytesPerTuFrac
		  << XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_FRAC_SHIFT;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].En3DFrameFieldSeq
		  << XMMIDP_VIDEO_CONFIG5_EN_3D_FRAME_FILED_SEQ_SHIFT;

	RegVal |= InstancePtr->VideoConfig[Stream - 1].InitThresholdHi
		  << XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_HI_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the vsample register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVSampleCtrl(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal |= InstancePtr->VSampleCtrl[Stream - 1].PixModeSel
		  << XMMIDP_VSAMPLE_CTRL_PIXEL_MODE_SELECT_SHIFT;

	RegVal |= InstancePtr->VSampleCtrl[Stream - 1].VideoMapping
		  << XMMIDP_VSAMPLE_CTRL_VIDEO_MAPPING_SHIFT;

	RegVal |= InstancePtr->VSampleCtrl[Stream - 1].VidStreamEn
		  << XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa1 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa1(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA1 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal |= InstancePtr->MsaConfig[Stream - 1].VStart
		  << XMMIDP_VIDEO_MSA1_VSTART_SHIFT;

	RegVal |= InstancePtr->MsaConfig[Stream - 1].HStart;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa2 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa2(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA2 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal |= InstancePtr->MsaConfig[Stream - 1].Misc0
		  << XMMIDP_VIDEO_MSA2_MISC0_SHIFT;

	RegVal |= InstancePtr->MsaConfig[Stream - 1].MVid;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa3 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa3(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA3 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal |= InstancePtr->MsaConfig[Stream - 1].Misc1
		  << XMMIDP_VIDEO_MSA3_MISC1_SHIFT;

	RegVal |= InstancePtr->MsaConfig[Stream - 1].NVid;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video HBlankInterval register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetHBlankInterval(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_HBLANK_INTERVAL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal |= InstancePtr->MsaConfig[Stream - 1].HBlankInterval;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This is helper api that programs the VideoController MSA, VideoConfig registers
 * for a specific stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ConfigureVideoController(XMmiDp *InstancePtr, u8 Stream)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	XMmiDp_SetVSampleCtrl(InstancePtr, Stream);

	XMmiDp_SetVInputPolarityCtrl(InstancePtr, Stream);
	XMmiDp_SetVideoConfig1(InstancePtr, Stream);
	XMmiDp_SetVideoConfig2(InstancePtr, Stream);
	XMmiDp_SetVideoConfig3(InstancePtr, Stream);
	XMmiDp_SetVideoConfig4(InstancePtr, Stream);
	XMmiDp_SetVideoConfig5(InstancePtr, Stream);

	XMmiDp_SetVideoMsa1(InstancePtr, Stream);
	XMmiDp_SetVideoMsa2(InstancePtr, Stream);
	XMmiDp_SetVideoMsa3(InstancePtr, Stream);
	XMmiDp_SetHBlankInterval(InstancePtr, Stream);

}

/******************************************************************************/
/**
 * This is helper api that clears the VideoController MSA, VideoConfig registers
 * for a specific stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearVideoController(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Clear MSA config values */
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID1);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID2);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID3);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID4);

	/* Clear Video Config values */
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID1);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID2);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID3);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID4);

}

/******************************************************************************/
/**
 * This function sets the bits per color value of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       BitsPerColor is the new number of bits per color to use.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetMsaBpc(XMmiDp *InstancePtr, u8 Stream, u8 BitsPerColor)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((BitsPerColor == 6) || (BitsPerColor == 8) ||
		       (BitsPerColor == 10) || (BitsPerColor == 12));

	InstancePtr->MsaConfig[Stream - 1].BitsPerColor = BitsPerColor;
}

/******************************************************************************/
/**
 * This function sets the pixel mode of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       PixModeSel.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetPixModeSel(XMmiDp *InstancePtr, u8 Stream, XMmiDp_PPC PixModeSel)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->VSampleCtrl[Stream - 1].PixModeSel = PixModeSel;
}

/******************************************************************************/
/**
 * This function sets the video format of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Video input mapping.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMapping(XMmiDp *InstancePtr, u8 Stream, XMmiDp_VidMap VidMap)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->VSampleCtrl[Stream - 1].VideoMapping = VidMap;
}

/******************************************************************************/
/**
 * This function sets the video stream to enable or disable.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable or Disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetVidStreamEnable(XMmiDp *InstancePtr, u8 Stream, u8 StreamEnable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((StreamEnable == 0) || (StreamEnable == 1));

	InstancePtr->VSampleCtrl[Stream - 1].VidStreamEn = StreamEnable;
}

/******************************************************************************/
/**
 * This function sets the audio stream interface.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio interface I2S or SPDIF .
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudStreamInterfaceSel(XMmiDp *InstancePtr, u8 Stream, u8 InterfaceSel)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((InterfaceSel == XMMIDP_AUD_INF_I2S) || (InterfaceSel == XMMIDP_AUD_INF_SPDIF));

	InstancePtr->AudCfg[Stream - 1].InterfaceSel = InterfaceSel;

}

/******************************************************************************/
/**
 * This function sets the active audio data input.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Active data input on different channels.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudDataInputEn(XMmiDp *InstancePtr, u8 Stream, u8 ActiveDataInput)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].DataInEn = ActiveDataInput;

}

/******************************************************************************/
/**
 * This function sets the audio data width val(16 bit - 24 bit).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio Data Width.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudDataWidth(XMmiDp *InstancePtr, u8 Stream, u8 DataWidth)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((DataWidth == XMMIDP_AUDIO_INPUT_16_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_17_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_18_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_19_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_20_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_21_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_22_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_23_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_24_BIT));

	InstancePtr->AudCfg[Stream - 1].DataWidth = DataWidth;

}

/******************************************************************************/
/**
 * This function sets the audio input is operating in HBR mode or not.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio HBR mode en or disable
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudHbrModeEn(XMmiDp *InstancePtr, u8 Stream, u8 HbrModeEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((HbrModeEn == 0) || (HbrModeEn == 1));

	InstancePtr->AudCfg[Stream - 1].HbrModeEn = HbrModeEn;

}

/******************************************************************************/
/**
 * This function sets the audio num of channels used.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio num channels
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudNumChannels(XMmiDp *InstancePtr, u8 Stream, u8 NumChannels)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((Stream == XMMIDP_AUDIO_1_CHANNEL) ||
		       (Stream == XMMIDP_AUDIO_2_CHANNEL) ||
		       (Stream == XMMIDP_AUDIO_8_CHANNEL));

	InstancePtr->AudCfg[Stream - 1].NumChannels = NumChannels;

}

/******************************************************************************/
/**
 * This function controls the audio mute flag in VB-ID
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio mute clear or set
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudMuteFlag(XMmiDp *InstancePtr, u8 Stream, u8 AudioMute)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((Stream == XMMIDP_CLEAR_AUDIOMUTE) ||
		       (Stream == XMMIDP_SET_AUDIOMUTE));

	InstancePtr->AudCfg[Stream - 1].AudioMute = AudioMute;

}

/******************************************************************************/
/**
 * This function defines first byte of audio SDP and audio timestamp SDP header.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio Packet Id
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudPktId(XMmiDp *InstancePtr, u8 Stream, u8 PktId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].PktId = PktId;

}

/******************************************************************************/
/**
 * This function defines audio timestamp version num.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp version number
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudTimeStampVerNum(XMmiDp *InstancePtr, u8 Stream, u8 VerNum)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].TimeStampVerNum = VerNum;

}

/******************************************************************************/
/**
 * This function defines audio clock frequency multiplier in relation to
 * audio sample rate Fs.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio clock frequency multiplier
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudClkMultFs(XMmiDp *InstancePtr, u8 Stream, u8 ClkMultFs)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Xil_AssertVoid((Stream == XMMIDP_AUDIO_CLK_512FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_256FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_128FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_64FS));

	InstancePtr->AudCfg[Stream - 1].ClkMultFs = ClkMultFs;

}

/******************************************************************************/
/**
 * This function programs the AUDIO_CONFIG1 registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudioConfig1(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegVal;
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_AUD_CONFIG1 + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->AudCfg[Stream - 1].InterfaceSel;
	RegVal |= InstancePtr->AudCfg[Stream - 1].DataInEn
		  << XMMIDP_AUD_CONFIG1_AUDIO_DATA_IN_EN_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].DataWidth
		  << XMMIDP_AUD_CONFIG1_AUDIO_DATA_WIDTH_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].HbrModeEn
		  << XMMIDP_AUD_CONFIG1_HBR_MODE_EN_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].NumChannels
		  << XMMIDP_AUD_CONFIG1_NUM_CHANNELS_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].AudioMute
		  << XMMIDP_AUD_CONFIG1_AUDIO_MUTE_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].PktId
		  << XMMIDP_AUD_CONFIG1_PACKET_ID_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].TimeStampVerNum
		  << XMMIDP_AUD_CONFIG1_TIMESTAMP_VER_NUM_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].ClkMultFs
		  << XMMIDP_AUD_CONFIG1_CLK_MULT_FS_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function enables audio timestamp for sdp vertical ctrl.
 * When set, dptx sends out audio timestamp SDP once every video frame
 * during vertical blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp sdp enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertAudTimeStampEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudTimeStamp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnAudioTimeStampSdp = EnAudTimeStamp;

}

/******************************************************************************/
/**
 * This function enables audio stream during vertical  blanking .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio stream enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertAudStreamEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudStream)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnAudioStreamSdp = EnAudStream;

}

/******************************************************************************/
/**
 * This function sets SDP to be sent out during vertical intervals using data
 * from SDP_REGISTER_BANK .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable SDP during vertical intervals.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertEn(XMmiDp *InstancePtr, u8 Stream, u32 EnVerticalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnVerticalSdp = EnVerticalSdp;

}

/******************************************************************************/
/**
 * This function sets 128 bytes of SDP by using SDP_REGISTER_BANK 1,2,3 and 4.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable 128BytesSdp.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertEn128Bytes(XMmiDp *InstancePtr, u8 Stream, u8 En128Bytes)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].En128Bytes = En128Bytes;

}

/******************************************************************************/
/**
 * This function disables external SDP when set.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable external SDP.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetDisableExternalSdp(XMmiDp *InstancePtr, u8 Stream, u8 DisExternalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].DisableExtSdp = DisExternalSdp;

}

/******************************************************************************/
/**
 * This function sets scheduler searches for next SDP to transmit
 * based on fixed priority.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable fixed priority SDP scheduler.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertFixedPriority(XMmiDp *InstancePtr, u8 Stream, u8 EnFixedPriority)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].FixedPriorityArb = EnFixedPriority;

}

/******************************************************************************/
/**
 * This function programs the SDP_VERTICAL_CTRL registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVerticalCtrl(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegVal;
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_SDP_VERTICAL_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->SdpVertCtrl[Stream - 1].EnAudioTimeStampSdp;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].EnAudioStreamSdp
		  << XMMIDP_SDP_VERTICAL_EN_AUDSTREAM_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].EnVerticalSdp
		  << XMMIDP_SDP_VERTICAL_EN_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].En128Bytes
		  << XMMIDP_SDP_VERTICAL_EN_128BYTES_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].DisableExtSdp
		  << XMMIDP_SDP_VERTICAL_DISABLE_EXT_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].FixedPriorityArb
		  << XMMIDP_SDP_VERTICAL_FIXED_PRIORITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}
/******************************************************************************/
/**
 * This function enables audio timestamp for sdp horizontal ctrl.
 * When set, dptx sends out audio timestamp SDP once every video frame
 * during horizontal blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp sdp enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorAudTimeStampEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudTimeStamp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnAudioTimeStampSdp = EnAudTimeStamp;

}

/******************************************************************************/
/**
 * This function enables audio stream during horizontal blanking .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio stream enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorAudStreamEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudStream)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnAudioStreamSdp = EnAudStream;

}

/******************************************************************************/
/**
 * This function sets SDP to be sent out during horizontal intervals using data
 * from SDP_REGISTER_BANK .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable SDP during horizontal intervals.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorizontalEn(XMmiDp *InstancePtr, u8 Stream, u32 EnHorizontalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnHorizontalSdp = EnHorizontalSdp;

}

/******************************************************************************/
/**
 * This function sets scheduler searches for next SDP to transmit
 * based on fixed priority.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable fixed priority SDP scheduler.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorFixedPriority(XMmiDp *InstancePtr, u8 Stream, u8 EnFixedPriority)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].FixedPriorityArb = EnFixedPriority;

}

/******************************************************************************/
/**
 * This function programs the SDP_HORIZONTAL_CTRL registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorizontalCtrl(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegVal;
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_SDP_HORIZONTAL_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->SdpHorCtrl[Stream - 1].EnAudioTimeStampSdp;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].EnAudioStreamSdp
		  << XMMIDP_SDP_HORIZONTAL_EN_AUDSTREAM_SHIFT;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].EnHorizontalSdp
		  << XMMIDP_SDP_HORIZONTAL_EN_SHIFT;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].FixedPriorityArb
		  << XMMIDP_SDP_HORIZONTAL_FIXED_PRIORITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function programs DpTx Audio Controller
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_ConfigureAudioController(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_SetSdpVerticalCtrl(InstancePtr, Stream);
	XMmiDp_SetSdpHorizontalCtrl(InstancePtr, Stream);
	XMmiDp_SetAudioConfig1(InstancePtr, Stream);

}
/** @} */

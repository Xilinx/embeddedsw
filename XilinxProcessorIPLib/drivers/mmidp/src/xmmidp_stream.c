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
	u32 RegOffset = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * 0x10000);

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
	u32 RegOffset = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * 0x10000);

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

	u32 Msa1RegOffset = 0x0;
	u32 Msa2RegOffset = 0x0;
	u32 Msa3RegOffset = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	Msa1RegOffset = XMMIDP_VIDEO_MSA1 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, Msa1RegOffset, 0x0);

	Msa2RegOffset = XMMIDP_VIDEO_MSA2 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, Msa2RegOffset, 0x0);

	Msa3RegOffset = XMMIDP_VIDEO_MSA3 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, Msa3RegOffset, 0x0);

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

	u32 VinputPolarityCtrlOffset = 0x0;
	u32 VideoConfig1Offset = 0x0;
	u32 VideoConfig2Offset = 0x0;
	u32 VideoConfig3Offset = 0x0;
	u32 VideoConfig4Offset = 0x0;
	u32 VideoConfig5Offset = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	VinputPolarityCtrlOffset = XMMIDP_VINPUT_POLARITY_CTRL + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VinputPolarityCtrlOffset, 0x0);

	VideoConfig1Offset = XMMIDP_VIDEO_CONFIG1 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VideoConfig1Offset, 0x0);

	VideoConfig2Offset = XMMIDP_VIDEO_CONFIG2 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VideoConfig2Offset, 0x0);

	VideoConfig3Offset = XMMIDP_VIDEO_CONFIG3 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VideoConfig3Offset, 0x0);

	VideoConfig4Offset = XMMIDP_VIDEO_CONFIG4 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VideoConfig4Offset, 0x0);

	VideoConfig5Offset = XMMIDP_VIDEO_CONFIG5 + ((Stream - 1) * 0x10000);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, VideoConfig5Offset, 0x0);
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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VINPUT_POLARITY_CTRL +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG1 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG2 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG3 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG4 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG5 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA1 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA2 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA3 +
		    ((Stream - 1) * 0x10000);

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

	u32 RegOffset = 0x0;
	u32 RegVal = 0x0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_HBLANK_INTERVAL +
		    ((Stream - 1) * 0x10000);

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
		       (BitsPerColor == 10) || (BitsPerColor == 12) ||
		       (BitsPerColor == 16));

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

	Xil_AssertVoid((StreamEnable == 0x0) || (StreamEnable == 0x1));

	InstancePtr->VSampleCtrl[Stream - 1].VidStreamEn = StreamEnable;
}
/** @} */

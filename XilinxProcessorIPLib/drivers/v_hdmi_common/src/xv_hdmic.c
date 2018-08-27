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
 * @file xhdmic.c
 * @addtogroup hdmi_common_v1_0
 * @{
 *
 * Contains common utility functions that are typically used by hdmi-related
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   EB  21/12/17 Initial release.
 * 1.1   EB  10/04/18 Fixed a bug in XV_HdmiC_ParseAudioInfoFrame
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/
#include "xv_hdmic.h"

/************************** Constant Definitions ******************************/

/*****************************************************************************/
/**
* This table contains the attributes for various standard resolutions.
* Each entry is of the format:
* 1) Resolution ID
* 2) Video Identification Code.
*/
const XHdmiC_VicTable VicTable[VICTABLE_SIZE] = {
    {XVIDC_VM_640x480_60_P, 1},     // Vic 1
    {XVIDC_VM_720x480_60_P, 2},     // Vic 2
    {XVIDC_VM_720x480_60_P, 3},     // Vic 3
    {XVIDC_VM_1280x720_60_P, 4},    // Vic 4
    {XVIDC_VM_1920x1080_60_I, 5},   // Vic 5
    {XVIDC_VM_1440x480_60_I, 6},    // Vic 6
    {XVIDC_VM_1440x480_60_I, 7},    // Vic 7

    {XVIDC_VM_1920x1080_60_P, 16},  // Vic 16
    {XVIDC_VM_720x576_50_P, 17},    // Vic 17
    {XVIDC_VM_720x576_50_P, 18},    // Vic 18
    {XVIDC_VM_1280x720_50_P, 19},   // Vic 19
    {XVIDC_VM_1920x1080_50_I, 20},  // Vic 20
    {XVIDC_VM_1440x576_50_I, 21},   // Vic 21
    {XVIDC_VM_1440x576_50_I, 22},   // Vic 22

    // 1680 x 720
    {XVIDC_VM_1680x720_50_P, 82},   // Vic 82
    {XVIDC_VM_1680x720_60_P, 83},   // Vic 83
    {XVIDC_VM_1680x720_100_P, 84},  // Vic 84
    {XVIDC_VM_1680x720_120_P, 85},  // Vic 85

    // 1920 x 1080
    {XVIDC_VM_1920x1080_24_P, 32},  // Vic 32
    {XVIDC_VM_1920x1080_25_P, 33},  // Vic 33
    {XVIDC_VM_1920x1080_30_P, 34},  // Vic 34
    {XVIDC_VM_1920x1080_50_P, 31},  // Vic 31
    {XVIDC_VM_1920x1080_100_P, 64}, // Vic 64
    {XVIDC_VM_1920x1080_120_P, 63}, // Vic 63

    // 2560 x 1080
    {XVIDC_VM_2560x1080_50_P, 89},  // Vic 89
    {XVIDC_VM_2560x1080_60_P, 90},  // Vic 89
    {XVIDC_VM_2560x1080_100_P, 91}, // Vic 91
    {XVIDC_VM_2560x1080_120_P, 92}, // Vic 92

    // 3840 x 2160
    {XVIDC_VM_3840x2160_24_P, 93},  // Vic 93
    {XVIDC_VM_3840x2160_25_P, 94},  // Vic 94
    {XVIDC_VM_3840x2160_30_P, 95},  // Vic 95
    {XVIDC_VM_3840x2160_50_P, 96},  // Vic 96
    {XVIDC_VM_3840x2160_60_P, 97},  // Vic 97

    // 4096 x 2160
    {XVIDC_VM_4096x2160_24_P, 98},  // Vic 98
    {XVIDC_VM_4096x2160_25_P, 99},  // Vic 99
    {XVIDC_VM_4096x2160_30_P, 100}, // Vic 100
    {XVIDC_VM_4096x2160_50_P, 101}, // Vic 101
    {XVIDC_VM_4096x2160_60_P, 102}  // Vic 102
};

/*************************** Function Definitions *****************************/

/**
*
* This function retrieves the Auxiliary Video Information Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseAVIInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AVI_InfoFrame *infoFramePtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_AVI_INFOFRAME_TYPE) {

		/* Header, Version */
		infoFramePtr->Version = AuxPtr->Header.Byte[1];

		/* PB1 */
		infoFramePtr->ColorSpace = (AuxPtr->Data.Byte[1] >> 5) & 0x7;
		infoFramePtr->ActiveFormatDataPresent = (AuxPtr->Data.Byte[1] >> 4) & 0x1;
		infoFramePtr->BarInfo = (AuxPtr->Data.Byte[1] >> 2) & 0x3;
		infoFramePtr->ScanInfo = AuxPtr->Data.Byte[1] & 0x3;

		/* PB2 */
		infoFramePtr->Colorimetry = (AuxPtr->Data.Byte[2] >> 6) & 0x3;
		infoFramePtr->PicAspectRatio = (AuxPtr->Data.Byte[2] >> 4) & 0x3;
		infoFramePtr->ActiveAspectRatio = AuxPtr->Data.Byte[2] & 0xf;

		/* PB3 */
		infoFramePtr->Itc = (AuxPtr->Data.Byte[3] >> 7) & 0x1;
		infoFramePtr->ExtendedColorimetry = (AuxPtr->Data.Byte[3] >> 4) & 0x7;
		infoFramePtr->QuantizationRange = (AuxPtr->Data.Byte[3] >> 2) & 0x3;
		infoFramePtr->NonUniformPictureScaling = AuxPtr->Data.Byte[3] & 0x3;

		/* PB4 */
		infoFramePtr->VIC = AuxPtr->Data.Byte[4] & 0x7f;

		/* PB5 */
		infoFramePtr->YccQuantizationRange = (AuxPtr->Data.Byte[5] >> 6) & 0x3;
		infoFramePtr->ContentType = (AuxPtr->Data.Byte[5] >> 4) & 0x3;
		infoFramePtr->PixelRepetition = AuxPtr->Data.Byte[5] & 0xf;

		/* PB6/7 */
		infoFramePtr->TopBar = (AuxPtr->Data.Byte[8] << 8) | AuxPtr->Data.Byte[6];

		/* PB8/9 */
		infoFramePtr->BottomBar = (AuxPtr->Data.Byte[10] << 8) | AuxPtr->Data.Byte[9];

		/* PB10/11 */
		infoFramePtr->LeftBar = (AuxPtr->Data.Byte[12] << 8) | AuxPtr->Data.Byte[11];

		/* PB12/13 */
		infoFramePtr->RightBar = (AuxPtr->Data.Byte[14] << 8) | AuxPtr->Data.Byte[13];

	}
}

/*****************************************************************************/
/**
*
* This function retrieves the General Control Packet.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseGCP(XHdmiC_Aux *AuxPtr, XHdmiC_GeneralControlPacket *GcpPtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_GENERAL_CONTROL_PACKET_TYPE) {

		/* SB0 */
		GcpPtr->Clear_AVMUTE = (AuxPtr->Data.Byte[0] >> 4) & 0x1;
		GcpPtr->Set_AVMUTE = AuxPtr->Data.Byte[0] & 0x1;

		/* SB1 */
		GcpPtr->PixelPackingPhase = (AuxPtr->Data.Byte[1] >> 4) & 0xf;
		GcpPtr->ColorDepth = AuxPtr->Data.Byte[1] & 0xf;

		/* SB2 */
		GcpPtr->Default_Phase = AuxPtr->Data.Byte[2] & 0x1;
	}
}

/*****************************************************************************/
/**
*
* This function retrieves the Audio Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseAudioInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AudioInfoFrame *AudIFPtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_AUDIO_INFOFRAME_TYPE) {

		/* HB1, Version */
		AudIFPtr->Version = AuxPtr->Header.Byte[1];

		/* PB1 */
		AudIFPtr->CodingType = (AuxPtr->Data.Byte[1] >> 4) & 0xf;
		AudIFPtr->ChannelCount = (AuxPtr->Data.Byte[1]) & 0x7;

		/* PB2 */
		AudIFPtr->SampleFrequency = (AuxPtr->Data.Byte[2] >> 2) & 0x7;
		AudIFPtr->SampleSize = AuxPtr->Data.Byte[2] & 0x3;

		/* PB4 */
		AudIFPtr->ChannelAllocation = AuxPtr->Data.Byte[4];

		/* PB5 */
		AudIFPtr->Downmix_Inhibit = (AuxPtr->Data.Byte[5] >> 7) & 0x1;
		AudIFPtr->LevelShiftVal = (AuxPtr->Data.Byte[5] >> 4) & 0xf;
		AudIFPtr->LFE_Playback_Level = AuxPtr->Data.Byte[5] & 0x3;
	}
}

/*****************************************************************************/
/**
*
* This function generates and sends Auxilliary Video Infoframes
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_AVIIF_GeneratePacket(XHdmiC_AVI_InfoFrame *infoFramePtr)
{
//	Xil_AssertNonvoid(infoFramePtr != NULL);

	u8 Index;
	u8 Crc;
	XHdmiC_Aux aux;

	/* Header, Packet type*/
	aux.Header.Byte[0] = AUX_AVI_INFOFRAME_TYPE;

	/* Version */
	aux.Header.Byte[1] = infoFramePtr->Version;

	/* Length */
	aux.Header.Byte[2] = 13;

	/* Checksum (this will be calculated by the HDMI TX IP) */
	aux.Header.Byte[3] = 0;

	/* PB1 */
   aux.Data.Byte[1] = (infoFramePtr->ColorSpace & 0x7) << 5 |
		   ((infoFramePtr->ActiveFormatDataPresent << 4) & 0x10) |
		   ((infoFramePtr->BarInfo << 2) & 0xc) |
		   (infoFramePtr->ScanInfo & 0x3);

   /* PB2 */
   aux.Data.Byte[2] = ((infoFramePtr->Colorimetry & 0x3) << 6  |
		   ((infoFramePtr->PicAspectRatio << 4) & 0x30) |
		   (infoFramePtr->ActiveAspectRatio & 0xf));

   /* PB3 */
   aux.Data.Byte[3] = (infoFramePtr->Itc & 0x1) << 7 |
		   ((infoFramePtr->ExtendedColorimetry << 4) & 0x70) |
		   ((infoFramePtr->QuantizationRange << 2) & 0xc) |
		   (infoFramePtr->NonUniformPictureScaling & 0x3);

   /* PB4 */
   aux.Data.Byte[4] = infoFramePtr->VIC;

   /* PB5 */
   aux.Data.Byte[5] = (infoFramePtr->YccQuantizationRange & 0x3) << 6 |
		   ((infoFramePtr->ContentType << 4) & 0x30) |
		   (infoFramePtr->PixelRepetition & 0xf);

   /* PB6 */
   aux.Data.Byte[6] = infoFramePtr->TopBar & 0xff;

   aux.Data.Byte[7] = 0;

   /* PB8 */
   aux.Data.Byte[8] = (infoFramePtr->TopBar & 0xff00) >> 8;

   /* PB9 */
   aux.Data.Byte[9] = infoFramePtr->BottomBar & 0xff;

   /* PB10 */
   aux.Data.Byte[10] = (infoFramePtr->BottomBar & 0xff00) >> 8;

   /* PB11 */
   aux.Data.Byte[11] = infoFramePtr->LeftBar & 0xff;

   /* PB12 */
   aux.Data.Byte[12] = (infoFramePtr->LeftBar & 0xff00) >> 8;

   /* PB13 */
   aux.Data.Byte[13] = infoFramePtr->RightBar & 0xff;

   /* PB14 */
   aux.Data.Byte[14] = (infoFramePtr->RightBar & 0xff00) >> 8;

   /* Index references the length to calculate start of loop from where values are reserved */
   for (Index = aux.Header.Byte[2] + 2; Index < 32; Index++) {
	   aux.Data.Byte[Index] = 0;
   }

   /* Calculate AVI infoframe checksum */
   Crc = 0;

   /* Header */
   for (Index = 0; Index < 3; Index++) {
     Crc += aux.Header.Byte[Index];
   }

   /* Data */
   for (Index = 1; Index < (aux.Header.Byte[2] + 2); Index++) {
     Crc += aux.Data.Byte[Index];
   }

   Crc = 256 - Crc;

   /* PB0 */
   aux.Data.Byte[0] = Crc;

   return aux;
}

/*****************************************************************************/
/**
*
* This function generates and sends Audio Infoframes
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_AudioIF_GeneratePacket(XHdmiC_AudioInfoFrame *AudioInfoFrame)
{
//	Xil_AssertNonvoid(AudioInfoFrame != NULL);

	u8 Index;
	u8 Crc;
	XHdmiC_Aux aux;

	/* Header, Packet Type */
	aux.Header.Byte[0] = AUX_AUDIO_INFOFRAME_TYPE;

	/* Version */
	aux.Header.Byte[1] = 0x01;

	/* Length */
	aux.Header.Byte[2] = 0x0A;

	aux.Header.Byte[3] = 0;

	/* PB1 */
	aux.Data.Byte[1] = AudioInfoFrame->CodingType << 4 |
			(AudioInfoFrame->ChannelCount & 0x7);

	/* PB2 */
	aux.Data.Byte[2] = ((AudioInfoFrame->SampleFrequency << 2) & 0x1c) |
			(AudioInfoFrame->SampleSize & 0x3);

	/* PB3 */
	aux.Data.Byte[3] = 0;


	/* PB4 */
	aux.Data.Byte[4] = AudioInfoFrame->ChannelAllocation;

	/* PB5 */
	aux.Data.Byte[5] = (AudioInfoFrame->Downmix_Inhibit << 7) |
			((AudioInfoFrame->LevelShiftVal << 3) & 0x78) |
			(AudioInfoFrame->LFE_Playback_Level & 0x3);

	for (Index = 6; Index < 32; Index++)
	{
		aux.Data.Byte[Index] = 0;
	}

	/* Calculate Audio infoframe checksum */
	  Crc = 0;

	  /* Header */
	  for (Index = 0; Index < 3; Index++) {
	    Crc += aux.Header.Byte[Index];
	  }

	  /* Data */
	  for (Index = 1; Index < aux.Header.Byte[2] + 1; Index++) {
		  Crc += aux.Data.Byte[Index];
	  }

	  Crc = 256 - Crc;

	  aux.Data.Byte[0] = Crc;

	  return aux;
}

/*****************************************************************************/
/**
*
* This function converts the XVidC_ColorFormat to XHdmiC_Colorspace
*
* @param  ColorFormat is the XVidC_ColorFormat value to be converted
*
* @return XHdmiC_Colorspace value.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Colorspace XV_HdmiC_XVidC_To_IfColorformat(XVidC_ColorFormat ColorFormat) {
	XHdmiC_Colorspace Colorspace;

	switch(ColorFormat) {
		case XVIDC_CSF_RGB :
			Colorspace = XHDMIC_COLORSPACE_RGB;
			break;

		case XVIDC_CSF_YCRCB_422 :
			Colorspace = XHDMIC_COLORSPACE_YUV422;
			break;

		case XVIDC_CSF_YCRCB_444 :
			Colorspace = XHDMIC_COLORSPACE_YUV444;
			break;

		case XVIDC_CSF_YCRCB_420 :
			Colorspace = XHDMIC_COLORSPACE_YUV420;
			break;

		default:
			Colorspace = XHDMIC_COLORSPACE_RESERVED;
			break;
	}

	return Colorspace;
}

/*****************************************************************************/
/**
*
* This function converts the XVidC_ColorDepth to XHdmiC_ColorDepth
*
* @param  Bpc is the XVidC_ColorDepth value to be converted
*
* @return XHdmiC_ColorDepth value.
*
* @note   None.
*
******************************************************************************/
XVidC_AspectRatio XV_HdmiC_IFAspectRatio_To_XVidC(XHdmiC_PicAspectRatio AR) {
	XVidC_AspectRatio AspectRatio;

	switch(AR) {
		case XHDMIC_PIC_ASPECT_RATIO_4_3 :
			AspectRatio = XVIDC_AR_4_3;
			break;

		case XHDMIC_PIC_ASPECT_RATIO_16_9 :
			AspectRatio = XVIDC_AR_16_9;
			break;

		default:
			AspectRatio = XVIDC_AR_16_9;
			break;
	}

	return AspectRatio;
}

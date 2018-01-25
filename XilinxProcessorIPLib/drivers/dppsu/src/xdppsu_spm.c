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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
 * @file xdppsu_spm.c
 *
 * This file contains the stream policy maker functions for the XDpPsu driver.
 * These functions set up the DisplayPort TX core's main stream attributes that
 * determine how a video stream will be displayed.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/17/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xdppsu_hw.h"
#include "xstatus.h"
#include <math.h>

/**************************** Function Definitions ****************************/
/******************************************************************************/
/**
 * This function sets the output Video format for the DP core
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	ColorEncode is an enumarator for output color encoding schemes
 *
 * @return	None.
 *
 * @note	The MsaConfig structure is modified with the new, calculated
 *		values. The main stream attributes that were used to derive the
 *		calculated values are untouched in the MsaConfig structure.
 *
*******************************************************************************/
void XDpPsu_SetColorEncode(XDpPsu *InstancePtr, XDpPsu_ColorEncoding ColorEncode)
{
	Xil_AssertVoid(InstancePtr != NULL);
	XDpPsu_MainStreamAttributes *MsaConfig = &InstancePtr->MsaConfig;

	MsaConfig->YCbCrColorimetry = 0;
	MsaConfig->DynamicRange     = 0;
	MsaConfig->ComponentFormat  = 0;
	MsaConfig->Misc0	    = 0;
	MsaConfig->Misc1	    = 0;

	switch (ColorEncode) {
	case XDPPSU_CENC_RGB:
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_RGB;
		break;
	case XDPPSU_CENC_XVYCC_422_BT601:
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422;
		break;
	case XDPPSU_CENC_XVYCC_422_BT709:
		MsaConfig->YCbCrColorimetry = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422;
		break;
	case XDPPSU_CENC_XVYCC_444_BT601:
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR444;
		break;
	case XDPPSU_CENC_XVYCC_444_BT709:
		MsaConfig->YCbCrColorimetry = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR444;
		break;
	case XDPPSU_CENC_YCBCR_422_BT601:
		MsaConfig->DynamicRange    = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422;
		break;
	case XDPPSU_CENC_YCBCR_422_BT709:
		MsaConfig->YCbCrColorimetry = 1;
		MsaConfig->DynamicRange    = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422;
		break;
	case XDPPSU_CENC_YCBCR_444_BT601:
		MsaConfig->DynamicRange    = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR444;
		break;
	case XDPPSU_CENC_YCBCR_444_BT709:
		MsaConfig->YCbCrColorimetry = 1;
		MsaConfig->DynamicRange    = 1;
		MsaConfig->ComponentFormat =
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR444;
		break;
	case XDPPSU_CENC_YONLY:
		MsaConfig->Misc1 = XDPPSU_MAIN_STREAM_MISC1_Y_ONLY_EN_MASK;
		break;
	default:
		break;
	}
}

/******************************************************************************/
/**
 * This function calculates the following Main Stream Attributes (MSA):
 *	- Transfer unit size
 *	- User pixel width
 *	- Horizontal start
 *	- Vertical start
 *	- Horizontal total clock
 *	- Vertical total clock
 *	- Misc0
 *	- Misc1
 *	- Data per lane
 *	- Average number of bytes per transfer unit
 *	- Number of initial wait cycles
 * These values are derived from:
 *	- Bits per color
 *	- Horizontal resolution
 *	- Vertical resolution
 *	- Pixel clock (in KHz)
 *	- Horizontal sync polarity
 *	- Vertical sync polarity
 *	- Horizontal front porch
 *	- Horizontal sync pulse width
 *	- Horizontal back porch
 *	- Vertical front porch
 *	- Vertical sync pulse width
 *	- Vertical back porch
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	The MsaConfig structure is modified with the new, calculated
 *		values. The main stream attributes that were used to derive the
 *		calculated values are untouched in the MsaConfig structure.
 *
*******************************************************************************/
void XDpPsu_CfgMsaRecalculate(XDpPsu *InstancePtr)
{
	u32 VideoBw;
	u32 LinkBw;
	u32 WordsPerLine;
	u8 BitsPerPixel;
	XDpPsu_MainStreamAttributes *MsaConfig;
	XDpPsu_LinkConfig *LinkConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	MsaConfig = &InstancePtr->MsaConfig;
	LinkConfig = &InstancePtr->LinkConfig;

	/* Verify the rest of the values used. */
	Xil_AssertVoid((LinkConfig->LinkRate == XDPPSU_LINK_BW_SET_162GBPS) ||
			(LinkConfig->LinkRate == XDPPSU_LINK_BW_SET_270GBPS) ||
			(LinkConfig->LinkRate == XDPPSU_LINK_BW_SET_540GBPS));
	Xil_AssertVoid((LinkConfig->LaneCount == XDPPSU_LANE_COUNT_SET_1) ||
			(LinkConfig->LaneCount == XDPPSU_LANE_COUNT_SET_2));
	Xil_AssertVoid((MsaConfig->SynchronousClockMode == 0) ||
				(MsaConfig->SynchronousClockMode == 1));
	Xil_AssertVoid((MsaConfig->DynamicRange == 0) ||
					(MsaConfig->DynamicRange == 1));
	Xil_AssertVoid((MsaConfig->YCbCrColorimetry == 0) ||
					(MsaConfig->YCbCrColorimetry == 1));
	Xil_AssertVoid((MsaConfig->BitsPerColor == 6) ||
					(MsaConfig->BitsPerColor == 8) ||
					(MsaConfig->BitsPerColor == 10) ||
					(MsaConfig->BitsPerColor == 12) ||
					(MsaConfig->BitsPerColor == 16));

	MsaConfig->UserPixelWidth = 1;

	/* Compute the rest of the MSA values. */
	MsaConfig->NVid = 27 * 1000 * LinkConfig->LinkRate;
	MsaConfig->HStart = MsaConfig->Vtm.Timing.HSyncWidth +
					MsaConfig->Vtm.Timing.HBackPorch;
	MsaConfig->VStart = MsaConfig->Vtm.Timing.F0PVSyncWidth +
					MsaConfig->Vtm.Timing.F0PVBackPorch;

	/* Miscellaneous attributes. */
	if (MsaConfig->BitsPerColor == 6) {
		MsaConfig->Misc0 = XDPPSU_MAIN_STREAM_MISC0_BDC_6BPC;
	}
	else if (MsaConfig->BitsPerColor == 8) {
		MsaConfig->Misc0 = XDPPSU_MAIN_STREAM_MISC0_BDC_8BPC;
	}
	else if (MsaConfig->BitsPerColor == 10) {
		MsaConfig->Misc0 = XDPPSU_MAIN_STREAM_MISC0_BDC_10BPC;
	}
	else if (MsaConfig->BitsPerColor == 12) {
		MsaConfig->Misc0 = XDPPSU_MAIN_STREAM_MISC0_BDC_12BPC;
	}
	else if (MsaConfig->BitsPerColor == 16) {
		MsaConfig->Misc0 = XDPPSU_MAIN_STREAM_MISC0_BDC_16BPC;
	}
	MsaConfig->Misc0 <<= XDPPSU_MAIN_STREAM_MISC0_BDC_SHIFT;

	/* Need to set this. */
	MsaConfig->Misc0 |= MsaConfig->ComponentFormat <<
			    XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT;

	MsaConfig->Misc0 |= MsaConfig->DynamicRange <<
			    XDPPSU_MAIN_STREAM_MISC0_DYNAMIC_RANGE_SHIFT;

	MsaConfig->Misc0 |= MsaConfig->YCbCrColorimetry <<
			    XDPPSU_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_SHIFT;

	MsaConfig->Misc0 |= MsaConfig->SynchronousClockMode;

	/* Determine the number of bits per pixel for the specified color
	 * component format. */
	if (MsaConfig->Misc1 == XDPPSU_MAIN_STREAM_MISC1_Y_ONLY_EN_MASK) {
		BitsPerPixel = MsaConfig->BitsPerColor;
	}
	else if (MsaConfig->ComponentFormat ==
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422) {
		/* YCbCr422 color component format. */
		BitsPerPixel = MsaConfig->BitsPerColor * 2;
	}
	else {
		/* RGB or YCbCr 4:4:4 color component format. */
		BitsPerPixel = MsaConfig->BitsPerColor * 3;
	}

	/* Calculate the data per lane. */
	WordsPerLine = (MsaConfig->Vtm.Timing.HActive * BitsPerPixel);
	if ((WordsPerLine % 16) != 0) {
		WordsPerLine += 16;
	}
	WordsPerLine /= 16;

	MsaConfig->DataPerLane = WordsPerLine - LinkConfig->LaneCount;
	if ((WordsPerLine % LinkConfig->LaneCount) != 0) {
		MsaConfig->DataPerLane +=
					(WordsPerLine % LinkConfig->LaneCount);
	}

	/* Allocate a fixed size for single-stream transport (SST) operation. */
	MsaConfig->TransferUnitSize = 64;

	/* Calculate the average number of bytes per transfer unit.
	 * Note: Both the integer and the fractional part is stored in
	 * AvgBytesPerTU. */
	VideoBw = ((MsaConfig->PixelClockHz / 1000) * BitsPerPixel) / 8;
	LinkBw = (LinkConfig->LaneCount * LinkConfig->LinkRate * 27);
	MsaConfig->AvgBytesPerTU = ((10 *
		(VideoBw * MsaConfig->TransferUnitSize) / LinkBw) + 5) / 10;

	/* The number of initial wait cycles at the start of a new line by the
	 * framing logic. This allows enough data to be buffered in the input
	 * FIFO before video is sent. */
	if ((MsaConfig->AvgBytesPerTU / 1000) <= 4) {
		MsaConfig->InitWait = 64;
	}
	else {
		MsaConfig->InitWait = MsaConfig->TransferUnitSize -
					(MsaConfig->AvgBytesPerTU / 1000);
	}
}

/******************************************************************************/
/**
 * This function sets the Main Stream Attribute (MSA) values in the
 * configuration structure to match one of the standard display mode timings
 * from the XDpPsu_DmtModes[] standard Display Monitor Timing (DMT) table. The
 * XDpPsu_VideoMode enumeration in xdppsu.h lists the available video modes.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	VideoMode is one of the enumerated standard video modes that is
 *		used to determine the MSA values to be used.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		MSA values associated to the specified video mode.
 *
*******************************************************************************/
void XDpPsu_CfgMsaUseStandardVideoMode(XDpPsu *InstancePtr,
						XVidC_VideoMode VideoMode)
{
	XDpPsu_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VideoMode < XVIDC_VM_NUM_SUPPORTED);

	MsaConfig = &InstancePtr->MsaConfig;

	/* Configure the MSA values from the display monitor DMT table. */
	MsaConfig->Vtm.VmId = XVidC_VideoTimingModes[VideoMode].VmId;
	MsaConfig->Vtm.FrameRate = XVidC_VideoTimingModes[VideoMode].FrameRate;
	MsaConfig->Vtm.Timing.HActive =
			XVidC_VideoTimingModes[VideoMode].Timing.HActive;
	MsaConfig->Vtm.Timing.HFrontPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.HFrontPorch;
	MsaConfig->Vtm.Timing.HSyncWidth =
			XVidC_VideoTimingModes[VideoMode].Timing.HSyncWidth;
	MsaConfig->Vtm.Timing.HBackPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.HBackPorch;
	MsaConfig->Vtm.Timing.HTotal =
			XVidC_VideoTimingModes[VideoMode].Timing.HTotal;
	MsaConfig->Vtm.Timing.HSyncPolarity =
			XVidC_VideoTimingModes[VideoMode].Timing.HSyncPolarity;
	MsaConfig->Vtm.Timing.VActive =
			XVidC_VideoTimingModes[VideoMode].Timing.VActive;
	MsaConfig->Vtm.Timing.F0PVFrontPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.F0PVFrontPorch;
	MsaConfig->Vtm.Timing.F0PVSyncWidth =
			XVidC_VideoTimingModes[VideoMode].Timing.F0PVSyncWidth;
	MsaConfig->Vtm.Timing.F0PVBackPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.F0PVBackPorch;
	MsaConfig->Vtm.Timing.F0PVTotal =
			XVidC_VideoTimingModes[VideoMode].Timing.F0PVTotal;
	MsaConfig->Vtm.Timing.F1VFrontPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.F1VFrontPorch;
	MsaConfig->Vtm.Timing.F1VSyncWidth =
			XVidC_VideoTimingModes[VideoMode].Timing.F1VSyncWidth;
	MsaConfig->Vtm.Timing.F1VBackPorch =
			XVidC_VideoTimingModes[VideoMode].Timing.F1VBackPorch;
	MsaConfig->Vtm.Timing.F1VTotal =
			XVidC_VideoTimingModes[VideoMode].Timing.F1VTotal;
	MsaConfig->Vtm.Timing.VSyncPolarity =
			XVidC_VideoTimingModes[VideoMode].Timing.VSyncPolarity;

	/* Calculate the pixel clock frequency. */
	MsaConfig->PixelClockHz =
			XVidC_GetPixelClockHzByVmId(MsaConfig->Vtm.VmId);

	/* Calculate the rest of the MSA values. */
	XDpPsu_CfgMsaRecalculate(InstancePtr);
}

/******************************************************************************/
/**
 * This function sets the main stream attribute values in the configuration
 * structure to match the preferred timing of the sink monitor. This Preferred
 * Timing Mode (PTM) information is stored in the sink's Extended Display
 * Identification Data (EDID).
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance
 * @param	Edid is a pointer to the Edid to use for the specified stream.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		main stream attribute values associated to the preferred timing
 *		of the sink monitor.
 *
*******************************************************************************/
void XDpPsu_CfgMsaUseEdidPreferredTiming(XDpPsu *InstancePtr, u8 *Edid)
{
	XDpPsu_MainStreamAttributes *MsaConfig;
	u8 *Ptm;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Edid != NULL);

	MsaConfig = &InstancePtr->MsaConfig;
	Ptm = &Edid[XDPPSU_EDID_PTM];

	/* Configure the MSA values with the PTM information as
	 * specified by the preferred Detailed Timing Descriptor (DTD) of the
	 * monitor's EDID.
	 * Note, the PTM is only required for EDID versions 1.3 a newer. Earlier
	 * versions may not contain this information. */
	u16 HBlank = ((Ptm[XDPPSU_EDID_DTD_HRES_HBLANK_U4] &
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDPPSU_EDID_DTD_HBLANK_LSB];

	u16 VBlank = ((Ptm[XDPPSU_EDID_DTD_VRES_VBLANK_U4] &
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDPPSU_EDID_DTD_VBLANK_LSB];

	MsaConfig->Vtm.Timing.HActive =
			(((Ptm[XDPPSU_EDID_DTD_HRES_HBLANK_U4] &
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDPPSU_EDID_DTD_HRES_LSB];

	MsaConfig->Vtm.Timing.VActive =
			(((Ptm[XDPPSU_EDID_DTD_VRES_VBLANK_U4] &
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDPPSU_EDID_DTD_VRES_LSB];

	MsaConfig->PixelClockHz = (((Ptm[XDPPSU_EDID_DTD_PIXEL_CLK_KHZ_MSB] <<
		8) | Ptm[XDPPSU_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

	MsaConfig->Vtm.Timing.HFrontPorch =
			(((Ptm[XDPPSU_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
			Ptm[XDPPSU_EDID_DTD_HFPORCH_LSB];

	MsaConfig->Vtm.Timing.HSyncWidth =
			(((Ptm[XDPPSU_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			Ptm[XDPPSU_EDID_DTD_HSPW_LSB];

	MsaConfig->Vtm.Timing.F0PVFrontPorch =
			(((Ptm[XDPPSU_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
			((Ptm[XDPPSU_EDID_DTD_VFPORCH_VSPW_L4] &
			XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
			XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	MsaConfig->Vtm.Timing.F0PVSyncWidth =
			((Ptm[XDPPSU_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
			(Ptm[XDPPSU_EDID_DTD_VFPORCH_VSPW_L4] &
			XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

	/* Compute video mode timing values. */
	MsaConfig->Vtm.Timing.HBackPorch = HBlank -
					(MsaConfig->Vtm.Timing.HFrontPorch +
					MsaConfig->Vtm.Timing.HSyncWidth);

	MsaConfig->Vtm.Timing.F0PVBackPorch = VBlank -
					(MsaConfig->Vtm.Timing.F0PVFrontPorch +
					MsaConfig->Vtm.Timing.F0PVSyncWidth);

	MsaConfig->Vtm.Timing.HTotal = (MsaConfig->Vtm.Timing.HSyncWidth +
					MsaConfig->Vtm.Timing.HFrontPorch +
					MsaConfig->Vtm.Timing.HActive +
					MsaConfig->Vtm.Timing.HBackPorch);

	MsaConfig->Vtm.Timing.F0PVTotal = (MsaConfig->Vtm.Timing.F0PVSyncWidth +
					MsaConfig->Vtm.Timing.F0PVFrontPorch +
					MsaConfig->Vtm.Timing.VActive +
					MsaConfig->Vtm.Timing.F0PVBackPorch);

	MsaConfig->Vtm.FrameRate = nearbyint((double )MsaConfig->PixelClockHz /
					(MsaConfig->Vtm.Timing.HTotal *
					MsaConfig->Vtm.Timing.F0PVTotal));

	MsaConfig->Vtm.Timing.HSyncPolarity = !((Ptm[XDPPSU_EDID_DTD_SIGNAL] &
					XDPPSU_EDID_DTD_SIGNAL_HPOLARITY_MASK) >>
					XDPPSU_EDID_DTD_SIGNAL_HPOLARITY_SHIFT);
	MsaConfig->Vtm.Timing.VSyncPolarity = !((Ptm[XDPPSU_EDID_DTD_SIGNAL] &
					XDPPSU_EDID_DTD_SIGNAL_VPOLARITY_MASK) >>
					XDPPSU_EDID_DTD_SIGNAL_VPOLARITY_SHIFT);

	MsaConfig->Vtm.VmId = XVIDC_VM_USE_EDID_PREFERRED;

	/* Calculate the rest of the MSA values. */
	XDpPsu_CfgMsaRecalculate(InstancePtr);
}

/******************************************************************************/
/**
 * This function takes a the main stream attributes from MsaConfigCustom and
 * copies them into InstancePtr->MsaConfig. If desired, given a base set of
 * attributes, the rest of the attributes may be derived. The minimal required
 * main stream attributes (MSA) that must be contained in the MsaConfigCustom
 * structure are:
 *	- Pixel clock (in Hz)
 *	- Horizontal sync polarity
 *	- Vertical sync polarity
 *	- Horizontal sync pulse width
 *	- Vertical sync pulse width
 *	- Horizontal resolution
 *	- Vertical resolution
 *	- Vertical back porch
 *	- Vertical front porch
 *	- Horizontal back porch
 *	- Horizontal front porch
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	MsaConfigCustom is the structure that will be used to copy the
 *		main stream attributes from (into InstancePtr->MsaConfig).
 * @param	Recalculate is a boolean enable that determines whether or not
 *		the main stream attributes should be recalculated.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified with the new
 *		values.
 *
*******************************************************************************/
void XDpPsu_CfgMsaUseCustom(XDpPsu *InstancePtr,
		XDpPsu_MainStreamAttributes *MsaConfigCustom, u8 Recalculate)
{
	XDpPsu_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsaConfigCustom != NULL);

	MsaConfig = &InstancePtr->MsaConfig;

	/* Copy the MSA values from the user configuration structure. */
	MsaConfig->PixelClockHz = MsaConfigCustom->PixelClockHz;
	MsaConfig->Vtm.VmId = MsaConfigCustom->Vtm.VmId;
	MsaConfig->Vtm.FrameRate = MsaConfigCustom->Vtm.FrameRate;
	MsaConfig->Vtm.Timing.HActive =
				MsaConfigCustom->Vtm.Timing.HActive;
	MsaConfig->Vtm.Timing.HFrontPorch =
				MsaConfigCustom->Vtm.Timing.HFrontPorch;
	MsaConfig->Vtm.Timing.HSyncWidth =
				MsaConfigCustom->Vtm.Timing.HSyncWidth;
	MsaConfig->Vtm.Timing.HBackPorch =
				MsaConfigCustom->Vtm.Timing.HBackPorch;
	MsaConfig->Vtm.Timing.HTotal =
				MsaConfigCustom->Vtm.Timing.HTotal;
	MsaConfig->Vtm.Timing.HSyncPolarity =
				MsaConfigCustom->Vtm.Timing.HSyncPolarity;
	MsaConfig->Vtm.Timing.VActive =
				MsaConfigCustom->Vtm.Timing.VActive;
	MsaConfig->Vtm.Timing.F0PVFrontPorch =
				MsaConfigCustom->Vtm.Timing.F0PVFrontPorch;
	MsaConfig->Vtm.Timing.F0PVSyncWidth =
				MsaConfigCustom->Vtm.Timing.F0PVSyncWidth;
	MsaConfig->Vtm.Timing.F0PVBackPorch =
				MsaConfigCustom->Vtm.Timing.F0PVBackPorch;
	MsaConfig->Vtm.Timing.F0PVTotal =
				MsaConfigCustom->Vtm.Timing.F0PVTotal;
	MsaConfig->Vtm.Timing.F1VFrontPorch =
				MsaConfigCustom->Vtm.Timing.F1VFrontPorch;
	MsaConfig->Vtm.Timing.F1VSyncWidth =
				MsaConfigCustom->Vtm.Timing.F1VSyncWidth;
	MsaConfig->Vtm.Timing.F1VBackPorch =
				MsaConfigCustom->Vtm.Timing.F1VBackPorch;
	MsaConfig->Vtm.Timing.F1VTotal =
				MsaConfigCustom->Vtm.Timing.F1VTotal;
	MsaConfig->Vtm.Timing.VSyncPolarity =
				MsaConfigCustom->Vtm.Timing.VSyncPolarity;

	if (Recalculate) {
		/* Calculate the rest of the MSA values. */
		XDpPsu_CfgMsaRecalculate(InstancePtr);
	}
	else {
		/* Use the custom values for the rest. */
		MsaConfig->TransferUnitSize = MsaConfigCustom->TransferUnitSize;
		MsaConfig->UserPixelWidth = 1;
		MsaConfig->NVid = MsaConfigCustom->NVid;
		MsaConfig->HStart = MsaConfigCustom->HStart;
		MsaConfig->VStart = MsaConfigCustom->VStart;
		MsaConfig->Misc0 = MsaConfigCustom->Misc0;
		MsaConfig->Misc1 = MsaConfigCustom->Misc1;
		MsaConfig->DataPerLane = MsaConfigCustom->DataPerLane;
		MsaConfig->AvgBytesPerTU = MsaConfigCustom->AvgBytesPerTU;
		MsaConfig->InitWait = MsaConfigCustom->InitWait;
	}
}

/******************************************************************************/
/**
 * This function sets the bits per color value of the video stream.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance
 * @param	BitsPerColor is the new number of bits per color to use.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		new main stream attributes associated with a new bits per color
 *		value.
 *
*******************************************************************************/
void XDpPsu_CfgMsaSetBpc(XDpPsu *InstancePtr, u8 BitsPerColor)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((BitsPerColor == 6) || (BitsPerColor == 8) ||
				(BitsPerColor == 10) || (BitsPerColor == 12) ||
				(BitsPerColor == 16));

	InstancePtr->MsaConfig.BitsPerColor = BitsPerColor;

	/* Calculate the rest of the MSA values. */
	XDpPsu_CfgMsaRecalculate(InstancePtr);
}

/******************************************************************************/
/**
 * This function enables or disables synchronous clock mode for a video stream.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance
 * @param	Enable if set to 1, will enable synchronous clock mode.
 *		Otherwise, if set to 0, synchronous clock mode will be disabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_CfgMsaEnSynchClkMode(XDpPsu *InstancePtr, u8 Enable)
{
	XDpPsu_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Enable == 0) || (Enable == 1));

	MsaConfig = &InstancePtr->MsaConfig;

        MsaConfig->SynchronousClockMode = Enable;

	if (Enable == 1) {
		MsaConfig->Misc0 |= (1 <<
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT);
	}
	else {
		MsaConfig->Misc0 &= ~(1 <<
			XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT);
	}
}

/******************************************************************************/
/**
 * This function clears the main stream attributes registers of the DisplayPort
 * TX core and sets them to the values specified in the main stream attributes
 * configuration structure.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_SetVideoMode(XDpPsu *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XDpPsu_SetMsaValues(InstancePtr);
}

/******************************************************************************/
/**
 * This function sets the main stream attributes registers of the DisplayPort TX
 * core with the values specified in the main stream attributes configuration
 * structure.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_SetMsaValues(XDpPsu *InstancePtr)
{
	XDpPsu_Config *Config;
	XDpPsu_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Config = &InstancePtr->Config;
	MsaConfig = &InstancePtr->MsaConfig;

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_HTOTAL,
				MsaConfig->Vtm.Timing.HTotal);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_VTOTAL,
				MsaConfig->Vtm.Timing.F0PVTotal);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_POLARITY,
				MsaConfig->Vtm.Timing.HSyncPolarity |
				(MsaConfig->Vtm.Timing.VSyncPolarity <<
				XDPPSU_MAIN_STREAM_POLARITY_VSYNC_POL_SHIFT));
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_HSWIDTH,
				MsaConfig->Vtm.Timing.HSyncWidth);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_VSWIDTH,
				MsaConfig->Vtm.Timing.F0PVSyncWidth);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_HRES,
				MsaConfig->Vtm.Timing.HActive);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_VRES,
				MsaConfig->Vtm.Timing.VActive);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_HSTART,
				MsaConfig->HStart);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_VSTART,
				MsaConfig->VStart);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_MISC0,
				MsaConfig->Misc0);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MAIN_STREAM_MISC1,
				MsaConfig->Misc1);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_M_VID,
				MsaConfig->PixelClockHz / 1000);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_N_VID, MsaConfig->NVid);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_USER_PIXEL_WIDTH,
				MsaConfig->UserPixelWidth);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_USER_DATA_COUNT_PER_LANE,
				MsaConfig->DataPerLane);

	/* Set the transfer unit values to the associated DisplayPort TX core
	 * registers. */
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_TU_SIZE,
				MsaConfig->TransferUnitSize);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_MIN_BYTES_PER_TU,
				MsaConfig->AvgBytesPerTU / 1000);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_FRAC_BYTES_PER_TU,
				(MsaConfig->AvgBytesPerTU % 1000) * 1000);
	XDpPsu_WriteReg(Config->BaseAddr, XDPPSU_INIT_WAIT, MsaConfig->InitWait);
}

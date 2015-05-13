/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xdptx_spm.c
 *
 * This file contains the stream policy maker functions for the XDptx driver.
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
 * 1.0   als  05/17/14 Initial release.
 *       als  08/03/14 Initial MST addition.
 * 3.0   als  12/16/14 Updated to use common video library.
 *                     Stream naming now starts at 1 to follow IP.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xdptx_hw.h"
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static void XDptx_CalculateTs(XDptx *InstancePtr, u8 Stream, u8 BitsPerPixel);

/**************************** Function Definitions ****************************/

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
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number for which to calculate the MSA
 *		values.
 *
 * @return	None.
 *
 * @note	The MsaConfig structure is modified with the new, calculated
 *		values. The main stream attributes that were used to derive the
 *		calculated values are untouched in the MsaConfig structure.
 *
*******************************************************************************/
void XDptx_CfgMsaRecalculate(XDptx *InstancePtr, u8 Stream)
{
	u32 VideoBw;
	u32 LinkBw;
	u32 WordsPerLine;
	u8 BitsPerPixel;
	XDptx_MainStreamAttributes *MsaConfig;
	XDptx_LinkConfig *LinkConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	LinkConfig = &InstancePtr->LinkConfig;

	/* Verify the rest of the values used. */
	Xil_AssertVoid((LinkConfig->LinkRate == XDPTX_LINK_BW_SET_162GBPS) ||
			(LinkConfig->LinkRate == XDPTX_LINK_BW_SET_270GBPS) ||
			(LinkConfig->LinkRate == XDPTX_LINK_BW_SET_540GBPS));
	Xil_AssertVoid((LinkConfig->LaneCount == XDPTX_LANE_COUNT_SET_1) ||
			(LinkConfig->LaneCount == XDPTX_LANE_COUNT_SET_2) ||
			(LinkConfig->LaneCount == XDPTX_LANE_COUNT_SET_4));
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

	/* Set the user pixel width to handle clocks that exceed the
	 * capabilities of the DisplayPort TX core. */
	if (MsaConfig->OverrideUserPixelWidth == 0) {
		if ((MsaConfig->PixelClockHz > 300000000) &&
			(LinkConfig->LaneCount == XDPTX_LANE_COUNT_SET_4)) {
			MsaConfig->UserPixelWidth = 4;
		}
		else if ((MsaConfig->PixelClockHz > 75000000) &&
			(LinkConfig->LaneCount != XDPTX_LANE_COUNT_SET_1)) {
			MsaConfig->UserPixelWidth = 2;
		}
		else {
			MsaConfig->UserPixelWidth = 1;
		}
	}

	/* Compute the rest of the MSA values. */
	MsaConfig->NVid = 27 * 1000 * LinkConfig->LinkRate;
	MsaConfig->HStart = MsaConfig->Vtm.Timing.HSyncWidth +
					MsaConfig->Vtm.Timing.HBackPorch;
	MsaConfig->VStart = MsaConfig->Vtm.Timing.F0PVSyncWidth +
					MsaConfig->Vtm.Timing.F0PVBackPorch;

	/* Miscellaneous attributes. */
	if (MsaConfig->BitsPerColor == 6) {
		MsaConfig->Misc0 = XDPTX_MAIN_STREAMX_MISC0_BDC_6BPC;
	}
	else if (MsaConfig->BitsPerColor == 8) {
		MsaConfig->Misc0 = XDPTX_MAIN_STREAMX_MISC0_BDC_8BPC;
	}
	else if (MsaConfig->BitsPerColor == 10) {
		MsaConfig->Misc0 = XDPTX_MAIN_STREAMX_MISC0_BDC_10BPC;
	}
	else if (MsaConfig->BitsPerColor == 12) {
		MsaConfig->Misc0 = XDPTX_MAIN_STREAMX_MISC0_BDC_12BPC;
	}
	else if (MsaConfig->BitsPerColor == 16) {
		MsaConfig->Misc0 = XDPTX_MAIN_STREAMX_MISC0_BDC_16BPC;
	}
	MsaConfig->Misc0 = (MsaConfig->Misc0 <<
			XDPTX_MAIN_STREAMX_MISC0_BDC_SHIFT) |
			(MsaConfig->YCbCrColorimetry <<
			XDPTX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_SHIFT) |
			(MsaConfig->DynamicRange <<
			XDPTX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_SHIFT) |
			(MsaConfig->ComponentFormat <<
			XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT) |
			(MsaConfig->SynchronousClockMode);
	MsaConfig->Misc1 = 0;

	/* Determine the number of bits per pixel for the specified color
	 * component format. */
	if (MsaConfig->ComponentFormat ==
			XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
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
		MsaConfig->DataPerLane += (WordsPerLine % LinkConfig->LaneCount);
	}

	if (InstancePtr->MstEnable == 1) {
		/* Do time slot (and payload bandwidth number) calculations for
		 * MST. */
		XDptx_CalculateTs(InstancePtr, Stream, BitsPerPixel);

		MsaConfig->InitWait = 0;
	}
	else {
		/* Allocate a fixed size for single-stream transport (SST)
		 * operation. */
		MsaConfig->TransferUnitSize = 64;

		/* Calculate the average number of bytes per transfer unit.
		 * Note: Both the integer and the fractional part is stored in
		 * AvgBytesPerTU. */
		VideoBw = ((MsaConfig->PixelClockHz / 1000) * BitsPerPixel) / 8;
		LinkBw = (LinkConfig->LaneCount * LinkConfig->LinkRate * 27);
		MsaConfig->AvgBytesPerTU = (VideoBw *
					MsaConfig->TransferUnitSize) / LinkBw;

		/* The number of initial wait cycles at the start of a new line
		 * by the framing logic. This allows enough data to be buffered
		 * in the input FIFO before video is sent. */
		if ((MsaConfig->AvgBytesPerTU / 1000) <= 4) {
			MsaConfig->InitWait = 64;
		}
		else {
			MsaConfig->InitWait = MsaConfig->TransferUnitSize -
					(MsaConfig->AvgBytesPerTU / 1000);
		}
	}
}

/******************************************************************************/
/**
 * This function sets the Main Stream Attribute (MSA) values in the
 * configuration structure to match one of the standard display mode timings
 * from the XDptx_DmtModes[] standard Display Monitor Timing (DMT) table. The
 * XDptx_VideoMode enumeration in xdptx.h lists the available video modes.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number for which the MSA values will be
 *		used for.
 * @param	VideoMode is one of the enumerated standard video modes that is
 *		used to determine the MSA values to be used.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		MSA values associated to the specified video mode.
 *
*******************************************************************************/
void XDptx_CfgMsaUseStandardVideoMode(XDptx *InstancePtr, u8 Stream,
						XVidC_VideoMode VideoMode)
{
	XDptx_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VideoMode < XVIDC_VM_NUM_SUPPORTED);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

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
	XDptx_CfgMsaRecalculate(InstancePtr, Stream);
}

/******************************************************************************/
/**
 * This function sets the main stream attribute values in the configuration
 * structure to match the preferred timing of the sink monitor. This Preferred
 * Timing Mode (PTM) information is stored in the sink's Extended Display
 * Identification Data (EDID).
 *
 * @param	InstancePtr is a pointer to the XDptx instance
 * @param	Stream is the stream number for which the MSA values will be
 *		used for.
 * @param	Edid is a pointer to the Edid to use for the specified stream.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		main stream attribute values associated to the preferred timing
 *		of the sink monitor.
 *
*******************************************************************************/
void XDptx_CfgMsaUseEdidPreferredTiming(XDptx *InstancePtr, u8 Stream, u8 *Edid)
{
	XDptx_MainStreamAttributes *MsaConfig;
	u8 *Ptm;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));
	Xil_AssertVoid(Edid != NULL);

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	Ptm = &Edid[XDPTX_EDID_PTM];

	/* Configure the MSA values with the PTM information as
	 * specified by the preferred Detailed Timing Descriptor (DTD) of the
	 * monitor's EDID.
	 * Note, the PTM is only required for EDID versions 1.3 a newer. Earlier
	 * versions may not contain this information. */
	u16 HBlank = ((Ptm[XDPTX_EDID_DTD_HRES_HBLANK_U4] &
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDPTX_EDID_DTD_HBLANK_LSB];

	u16 VBlank = ((Ptm[XDPTX_EDID_DTD_VRES_VBLANK_U4] &
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDPTX_EDID_DTD_VBLANK_LSB];

	MsaConfig->Vtm.Timing.HActive =
			(((Ptm[XDPTX_EDID_DTD_HRES_HBLANK_U4] &
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDPTX_EDID_DTD_HRES_LSB];

	MsaConfig->Vtm.Timing.VActive =
			(((Ptm[XDPTX_EDID_DTD_VRES_VBLANK_U4] &
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDPTX_EDID_DTD_VRES_LSB];

	MsaConfig->PixelClockHz = (((Ptm[XDPTX_EDID_DTD_PIXEL_CLK_KHZ_MSB] <<
		8) | Ptm[XDPTX_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

	MsaConfig->Vtm.Timing.HFrontPorch =
			(((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
			Ptm[XDPTX_EDID_DTD_HFPORCH_LSB];

	MsaConfig->Vtm.Timing.HSyncWidth =
			(((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			Ptm[XDPTX_EDID_DTD_HSPW_LSB];

	MsaConfig->Vtm.Timing.F0PVFrontPorch =
			(((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
			((Ptm[XDPTX_EDID_DTD_VFPORCH_VSPW_L4] &
			XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
			XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	MsaConfig->Vtm.Timing.F0PVSyncWidth =
			((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
			XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
			(Ptm[XDPTX_EDID_DTD_VFPORCH_VSPW_L4] &
			XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

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

	MsaConfig->Vtm.FrameRate = MsaConfig->PixelClockHz /
					(MsaConfig->Vtm.Timing.HTotal *
					MsaConfig->Vtm.Timing.F0PVTotal);

	MsaConfig->Vtm.VmId = XVIDC_VM_USE_EDID_PREFERRED;

	/* Calculate the rest of the MSA values. */
	XDptx_CfgMsaRecalculate(InstancePtr, Stream);
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
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number for which the MSA values will be
 *		used for.
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
void XDptx_CfgMsaUseCustom(XDptx *InstancePtr, u8 Stream,
		XDptx_MainStreamAttributes *MsaConfigCustom, u8 Recalculate)
{
	XDptx_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));
	Xil_AssertVoid(MsaConfigCustom != NULL);

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

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
		XDptx_CfgMsaRecalculate(InstancePtr, Stream);
	}
	else {
		/* Use the custom values for the rest. */
		MsaConfig->TransferUnitSize = MsaConfigCustom->TransferUnitSize;
		MsaConfig->UserPixelWidth = MsaConfigCustom->UserPixelWidth;
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
 * @param	InstancePtr is a pointer to the XDptx instance
 * @param	Stream is the stream number for which to set the color depth.
 * @param	BitsPerColor is the new number of bits per color to use.
 *
 * @return	None.
 *
 * @note	The InstancePtr->MsaConfig structure is modified to reflect the
 *		new main stream attributes associated with a new bits per color
 *		value.
 *
*******************************************************************************/
void XDptx_CfgMsaSetBpc(XDptx *InstancePtr, u8 Stream, u8 BitsPerColor)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));
	Xil_AssertVoid((BitsPerColor == 6) || (BitsPerColor == 8) ||
				(BitsPerColor == 10) || (BitsPerColor == 12) ||
				(BitsPerColor == 16));

	InstancePtr->MsaConfig[Stream - 1].BitsPerColor = BitsPerColor;

	/* Calculate the rest of the MSA values. */
	XDptx_CfgMsaRecalculate(InstancePtr, Stream);
}

/******************************************************************************/
/**
 * This function enables or disables synchronous clock mode for a video stream.
 *
 * @param	InstancePtr is a pointer to the XDptx instance
 * @param	Stream is the stream number for which to enable or disable
 *		synchronous clock mode.
 * @param	Enable if set to 1, will enable synchronous clock mode.
 *		Otherwise, if set to 0, synchronous clock mode will be disabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_CfgMsaEnSynchClkMode(XDptx *InstancePtr, u8 Stream, u8 Enable)
{
	XDptx_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));
	Xil_AssertVoid((Enable == 0) || (Enable == 1));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

        MsaConfig->SynchronousClockMode = Enable;

	if (Enable == 1) {
		MsaConfig->Misc0 |= (1 <<
			XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT);
	}
	else {
		MsaConfig->Misc0 &= ~(1 <<
			XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT);
	}
}

/******************************************************************************/
/**
 * This function clears the main stream attributes registers of the DisplayPort
 * TX core and sets them to the values specified in the main stream attributes
 * configuration structure.
 *
 * @param	InstancePtr is a pointer to the XDptx instance
 * @param	Stream is the stream number for which to set the MSA values for.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_SetVideoMode(XDptx *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	XDptx_ClearMsaValues(InstancePtr, Stream);
	XDptx_SetMsaValues(InstancePtr, Stream);
}

/******************************************************************************/
/**
 * This function clears the main stream attributes registers of the DisplayPort
 * TX core.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number for which to clear the MSA values.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_ClearMsaValues(XDptx *InstancePtr, u8 Stream)
{
	XDptx_Config *Config;
	u32 StreamOffset[4] = {0, XDPTX_STREAM2_MSA_START_OFFSET,
		XDPTX_STREAM3_MSA_START_OFFSET, XDPTX_STREAM4_MSA_START_OFFSET};

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	Config = &InstancePtr->Config;

	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HTOTAL +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VTOTAL +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_POLARITY +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HSWIDTH +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VSWIDTH +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HRES +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VRES +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HSTART +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VSTART +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_MISC0 +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_MISC1 +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_USER_PIXEL_WIDTH +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_USER_DATA_COUNT_PER_LANE +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_M_VID +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_N_VID +
						StreamOffset[Stream - 1], 0);

        XDptx_WriteReg(Config->BaseAddr, XDPTX_STREAM1 + (Stream - 1) * 4, 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_TU_SIZE +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MIN_BYTES_PER_TU +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_FRAC_BYTES_PER_TU +
						StreamOffset[Stream - 1], 0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_INIT_WAIT +
						StreamOffset[Stream - 1], 0);
}

/******************************************************************************/
/**
 * This function sets the main stream attributes registers of the DisplayPort TX
 * core with the values specified in the main stream attributes configuration
 * structure.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number for which to set the MSA values for.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_SetMsaValues(XDptx *InstancePtr, u8 Stream)
{
	XDptx_Config *Config;
	XDptx_MainStreamAttributes *MsaConfig;
	u32 StreamOffset[4] = {0, XDPTX_STREAM2_MSA_START_OFFSET,
		XDPTX_STREAM3_MSA_START_OFFSET, XDPTX_STREAM4_MSA_START_OFFSET};

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	Config = &InstancePtr->Config;
	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HTOTAL +
			StreamOffset[Stream - 1], MsaConfig->Vtm.Timing.HTotal);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VTOTAL +
			StreamOffset[Stream - 1],
			MsaConfig->Vtm.Timing.F0PVTotal);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_POLARITY +
			StreamOffset[Stream - 1],
			MsaConfig->Vtm.Timing.HSyncPolarity |
			(MsaConfig->Vtm.Timing.VSyncPolarity <<
			XDPTX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HSWIDTH +
		StreamOffset[Stream - 1], MsaConfig->Vtm.Timing.HSyncWidth);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VSWIDTH +
		StreamOffset[Stream - 1], MsaConfig->Vtm.Timing.F0PVSyncWidth);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HRES +
			StreamOffset[Stream - 1],
			MsaConfig->Vtm.Timing.HActive);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VRES +
			StreamOffset[Stream - 1],
			MsaConfig->Vtm.Timing.VActive);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_HSTART +
			StreamOffset[Stream - 1], MsaConfig->HStart);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_VSTART +
			StreamOffset[Stream - 1], MsaConfig->VStart);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_MISC0 +
			StreamOffset[Stream - 1], MsaConfig->Misc0);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MAIN_STREAM_MISC1 +
			StreamOffset[Stream - 1], MsaConfig->Misc1);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_M_VID +
			StreamOffset[Stream - 1],
			MsaConfig->PixelClockHz / 1000);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_N_VID +
			StreamOffset[Stream - 1], MsaConfig->NVid);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_USER_PIXEL_WIDTH +
			StreamOffset[Stream - 1], MsaConfig->UserPixelWidth);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_USER_DATA_COUNT_PER_LANE +
			StreamOffset[Stream - 1], MsaConfig->DataPerLane);

	/* Set the transfer unit values to the associated DisplayPort TX core
	 * registers. */
        if (InstancePtr->MstEnable == 1) {
                XDptx_WriteReg(Config->BaseAddr,
			XDPTX_STREAM1 + (Stream - 1) * 4,
                        ((MsaConfig->AvgBytesPerTU / 1000) << 16) |
                        (MsaConfig->AvgBytesPerTU % 1000));
        }
	XDptx_WriteReg(Config->BaseAddr, XDPTX_TU_SIZE +
			StreamOffset[Stream - 1], MsaConfig->TransferUnitSize);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_MIN_BYTES_PER_TU +
		StreamOffset[Stream - 1], MsaConfig->AvgBytesPerTU / 1000);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_FRAC_BYTES_PER_TU +
		StreamOffset[Stream - 1], MsaConfig->AvgBytesPerTU % 1000);
	XDptx_WriteReg(Config->BaseAddr, XDPTX_INIT_WAIT +
			StreamOffset[Stream - 1], MsaConfig->InitWait);
}

/******************************************************************************/
/**
 * When the driver is in multi-stream transport (MST) mode, this function will
 * make the necessary calculations to describe a stream in MST mode. The key
 * values are the payload bandwidth number (PBN), the number of timeslots
 * required for allocating the bandwidth, and the average bytes per transfer
 * unit (both the integer and the fractional part).
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number to make the calculations for.
 * @param	BitsPerPixel is the number of bits that is used to store one
 *		pixel.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDptx_CalculateTs(XDptx *InstancePtr, u8 Stream, u8 BitsPerPixel)
{
	XDptx_MainStreamAttributes *MsaConfig =
					&InstancePtr->MsaConfig[Stream - 1];
	XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;
	double PeakPixelBw;
	u32 LinkBw;
	double Average_StreamSymbolTimeSlotsPerMTP;
	double Target_Average_StreamSymbolTimeSlotsPerMTP;
	double MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP;
	u32 TsInt;
	u32 TsFrac;

	PeakPixelBw = ((double)MsaConfig->PixelClockHz / 1000000) *
						((double)BitsPerPixel / 8);
	LinkBw = (LinkConfig->LaneCount * LinkConfig->LinkRate * 27);

	/* Calculate the payload bandiwdth number (PBN).  */
	InstancePtr->MstStreamConfig[Stream - 1].MstPbn =
					1.006 * PeakPixelBw * ((double)64 / 54);
	/* Ceil - round up if required, avoiding overhead of math.h. */
	if ((double)(1.006 * PeakPixelBw * ((double)64 / 54)) >
		((double)InstancePtr->MstStreamConfig[Stream - 1].MstPbn)) {
		InstancePtr->MstStreamConfig[Stream - 1].MstPbn++;
	}

	/* Calculate the average stream symbol time slots per MTP. */
	Average_StreamSymbolTimeSlotsPerMTP = (64.0 * PeakPixelBw / LinkBw);
	MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP = (54.0 *
		((double)InstancePtr->MstStreamConfig[Stream - 1].MstPbn /
		LinkBw));

	/* The target value to be found needs to follow the condition:
	 *	Average_StreamSymbolTimeSlotsPerMTP <=
	 *		Target_Average_StreamSymbolTimeSlotsPerMTP
	 *	>= MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP
	 * Obtain the greatest target value that satisfies the above condition
	 * and still a multiple of 1/TsFrac_Denominator.
	 * Note: TsFrac_Denominator = 8. */
	/* Round down. */
	Target_Average_StreamSymbolTimeSlotsPerMTP =
				(u32)Average_StreamSymbolTimeSlotsPerMTP;
	/* Find the greatest multiple that is less than the maximum. */
	Target_Average_StreamSymbolTimeSlotsPerMTP += ((1.0 / 8.0) * (u32)(8.0 *
			(MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP -
			Target_Average_StreamSymbolTimeSlotsPerMTP)));

	/* Determine the integer and the fractional part of the number of time
	 * slots that will be allocated for the stream. */
	TsInt = Target_Average_StreamSymbolTimeSlotsPerMTP;
	TsFrac = (((double)Target_Average_StreamSymbolTimeSlotsPerMTP * 1000) -
								(TsInt * 1000));

	/* Store TsInt and TsFrac in AvgBytesPerTU. */
	MsaConfig->AvgBytesPerTU = TsInt * 1000 + TsFrac;

	/* Set the number of time slots to allocate for this stream. */
	MsaConfig->TransferUnitSize = TsInt;
	if (TsFrac != 0) {
		/* Round up. */
		MsaConfig->TransferUnitSize++;
	}
	if ((InstancePtr->Config.PayloadDataWidth == 4) &&
				(MsaConfig->TransferUnitSize % 4) != 0) {
		/* Set to a multiple of 4 boundary. */
		MsaConfig->TransferUnitSize += (4 -
					(MsaConfig->TransferUnitSize % 4));
	}
	else if ((MsaConfig->TransferUnitSize % 2) != 0) {
		/* Set to an even boundary. */
		MsaConfig->TransferUnitSize++;
	}

	/* Determine the PBN for the stream. */
	InstancePtr->MstStreamConfig[Stream - 1].MstPbn =
			MsaConfig->TransferUnitSize *
			(LinkConfig->LaneCount * LinkConfig->LinkRate / 2);
}

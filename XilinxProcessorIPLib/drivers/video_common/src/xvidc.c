/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * @file xvidc.c
 *
 * Contains common utility functions that are typically used by video-related
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   rc,  01/10/15 Initial release.
 *       als
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xil_printf.h"
#include "xvidc.h"

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * This function calculates pixel clock based on the inputs.
 *
 * @param	HTotal specifies horizontal total.
 * @param	VTotal specifies vertical total.
 * @param	FrameRate specifies rate at which frames are generated.
 *
 * @return	Pixel clock in Hz.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XVidC_GetPixelClockHzByHVFr(u32 HTotal, u32 VTotal, u8 FrameRate)
{
	return (HTotal * VTotal * FrameRate);
}

/******************************************************************************/
/**
 * This function calculates pixel clock from video mode.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Pixel clock in Hz.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XVidC_GetPixelClockHzByVmId(XVidC_VideoMode VmId)
{
	u32 ClkHz;
	const XVidC_VideoTimingMode *VmPtr;

	VmPtr = &XVidC_VideoTimingModes[VmId];

	if (XVidC_IsInterlaced(VmId)) {
		/* For interlaced mode, use both frame 0 and frame 1 vertical
		 * totals. */
		ClkHz = VmPtr->Timing.F0PVTotal + VmPtr->Timing.F1VTotal;

		/* Multiply the number of pixels by the frame rate of each
		 * individual frame (half of the total frame rate). */
		ClkHz *= VmPtr->FrameRate / 2;
	}
	else {
		/* For progressive mode, use only frame 0 vertical total. */
		ClkHz = VmPtr->Timing.F0PVTotal;

		/* Multiply the number of pixels by the frame rate. */
		ClkHz *= VmPtr->FrameRate;
	}

	/* Multiply the vertical total by the horizontal total for number of
	 * pixels. */
	ClkHz *= VmPtr->Timing.HTotal;

	return ClkHz;
}

/******************************************************************************/
/**
 * This function checks if the input video mode is interlaced/progressive based
 * on its ID from the video timings table.
 *
 * @param	VmId specifies the resolution ID from the video timings table.
 *
 * @return	Video format.
 *		- XVIDC_VF_PROGRESSIVE
 *		- XVIDC_VF_INTERLACED
 *
 * @note	None.
 *
*******************************************************************************/
XVidC_VideoFormat XVidC_GetVideoFormat(XVidC_VideoMode VmId)
{
	if (XVidC_VideoTimingModes[VmId].Timing.F1VTotal == 0) {
		return (XVIDC_VF_PROGRESSIVE);
	}

	return (XVIDC_VF_INTERLACED);
}

/******************************************************************************/
/**
 * This function checks if the input video mode is interlaced based on its ID
 * from the video timings table.
 *
 * @param	VmId specifies the resolution ID from the video timings table.
 *
 * @return
 *		- 1 if the video timing with the supplied table ID is
 *		  interlaced.
 *		- 0 if the video timing is progressive.
 *
 * @note	None.
 *
*******************************************************************************/
u8 XVidC_IsInterlaced(XVidC_VideoMode VmId)
{
	if (XVidC_GetVideoFormat(VmId) == XVIDC_VF_INTERLACED) {
		return 1;
	}

	return 0;
}

/******************************************************************************/
/**
 * This function returns the Video Mode ID that matches the detected input
 * width, height, frame rate and I/P flag
 *
 * @param	Width specifies the number pixels per scanline.
 * @param	Height specifies the number of scanline's.
 * @param	FrameRate specifies refresh rate in HZ
 * @param	IsInterlaced is flag.
 *		- 0 = Progressive
 *		- 1 = Interlaced.
 *
 * @return	Id of a supported video mode.
 *
 * @note	None.
 *
*******************************************************************************/
XVidC_VideoMode XVidC_GetVideoModeId(u32 Width, u32 Height, u32 FrameRate,
					u8 IsInterlaced)
{
	u32 Low;
	u32 High;
	u32 Mid;
	u32 HActive;
	u32 VActive;
	u32 Rate;
	u32 ResFound = (FALSE);
	XVidC_VideoMode Mode;

	if (IsInterlaced) {
		Low = (XVIDC_VM_INTL_START);
		High = (XVIDC_VM_INTL_END);
	}
	else {
		Low = (XVIDC_VM_PROG_START);
		High = (XVIDC_VM_PROG_END);
	}

	HActive = VActive = Rate = 0;

	/* Binary search finds item in sorted array.
	 * And returns index (zero based) of item
	 * If item is not found returns flag remains
	 * FALSE. Search key is "width or HActive"
	 */
	while (Low <= High) {
		Mid = (Low + High) / 2;
		HActive = XVidC_VideoTimingModes[Mid].Timing.HActive;
		if (Width == HActive) {
			ResFound = (TRUE);
			break;
		}
		else if (Width < HActive) {
			High = Mid - 1;
		}
		else {
			Low = Mid + 1;
		}
	}

	 /* HActive matched at middle */
	if (ResFound) {
		/* Rewind to start index of mode with matching width */
		while ((Mid > 0) &&
			(XVidC_VideoTimingModes[Mid - 1].Timing.HActive ==
								Width)) {
			--Mid;
		}

		ResFound = (FALSE);
		VActive = XVidC_VideoTimingModes[Mid].Timing.VActive;
		Rate = XVidC_VideoTimingModes[Mid].FrameRate;

		/* Now do a linear search for matching VActive and Frame
		 * Rate
		 */
		while (HActive == Width) {
			/* check current entry */
			if ((VActive == Height) && (Rate == FrameRate)) {
				ResFound = (TRUE);
				break;
			}
			/* Check next entry */
			else {
				Mid = Mid + 1;
				HActive =
				XVidC_VideoTimingModes[Mid].Timing.HActive;
				VActive =
				XVidC_VideoTimingModes[Mid].Timing.VActive;
				Rate = XVidC_VideoTimingModes[Mid].FrameRate;
			}
		}
		Mode =
		(ResFound) ? (XVidC_VideoMode)Mid : (XVIDC_VM_NOT_SUPPORTED);
	}
	else {
		Mode = (XVIDC_VM_NOT_SUPPORTED);
	}

	return (Mode);
}

/******************************************************************************/
/**
 * This function returns the pointer to video mode data at index provided.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Pointer to XVidC_VideoTimingMode structure based on the given
 *		video mode.
 *
 * @note	None.
 *
*******************************************************************************/
const XVidC_VideoTimingMode *XVidC_GetVideoModeData(XVidC_VideoMode VmId)
{
	if (VmId < (XVIDC_VM_NUM_SUPPORTED)) {
		return (&XVidC_VideoTimingModes[VmId]);
	}
	else {
		return (NULL);
	}
}

/******************************************************************************/
/**
 *
 * This function returns the resolution name for index specified.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Pointer to a resolution name string.
 *
 * @note	None.
 *
*******************************************************************************/
const char *XVidC_GetVideoModeStr(XVidC_VideoMode VmId)
{
	if (VmId < (XVIDC_VM_NUM_SUPPORTED)) {
		return (XVidC_VideoTimingModes[VmId].Name);
	}
	else {
		return ("Video mode not supported");
	}
}

/******************************************************************************/
/**
 * This function returns the frame rate name for index specified.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Pointer to a frame rate name string.
 *
 * @note	None.
 *
*******************************************************************************/
char *XVidC_GetFrameRateStr(XVidC_VideoMode VmId)
{
	if (VmId < (XVIDC_VM_NUM_SUPPORTED)) {
		switch (XVidC_VideoTimingModes[VmId].FrameRate) {
			case (XVIDC_FR_24HZ):
				return ("24Hz");

			case (XVIDC_FR_25HZ):
				return ("25Hz");

			case (XVIDC_FR_30HZ):
				return ("30Hz");

			case (XVIDC_FR_50HZ):
				return ("50Hz");

			case (XVIDC_FR_56HZ):
				return ("56Hz");

			case (XVIDC_FR_60HZ):
				return ("60Hz");

			case (XVIDC_FR_65HZ):
				return ("65Hz");

			case (XVIDC_FR_67HZ):
				return ("67Hz");

			case (XVIDC_FR_70HZ):
				return("70Hz");

			case (XVIDC_FR_72HZ):
				return ("72Hz");

			case (XVIDC_FR_75HZ):
				return ("75Hz");

			case (XVIDC_FR_85HZ):
				return ("85Hz");

			case (XVIDC_FR_87HZ):
				return ("87Hz");

			case (XVIDC_FR_88HZ):
				return ("88Hz");

			case (XVIDC_FR_100HZ):
				return("100Hz");

			case (XVIDC_FR_120HZ):
				return ("120Hz");

			default:
				return ("Frame rate not supported");
		}
	}
	else {
		return ("Video mode not supported");
	}
}

/******************************************************************************/
/**
 * This function returns the color format name for index specified.
 *
 * @param	ColorFormatId specifies the index of color format space.
 *		0 = XVIDC_CSF_RGB
 *		1 = XVIDC_CSF_YCRCB_444,
 *		2 = XVIDC_CSF_YCRCB_422,
 *		3 = XVIDC_CSF_YCRCB_420,
 *
 * @return	Pointer to a color space name string.
 *
 * @note	None.
 *
*******************************************************************************/
char *XVidC_GetColorFormatStr(XVidC_ColorFormat ColorFormatId)
{
	switch (ColorFormatId) {
		case (XVIDC_CSF_RGB):
			return ("RGB");

		case (XVIDC_CSF_YCRCB_444):
			return ("YUV_444");

		case (XVIDC_CSF_YCRCB_422):
			return ("YUV_422");

		case (XVIDC_CSF_YCRCB_420):
			return ("YUV_420");

		default:
			return ("Color space format not supported");
	}
}

/******************************************************************************/
/**
 * This function returns the frame rate for index specified.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Frame rate in Hz.
 *
 * @note	None.
 *
*******************************************************************************/
XVidC_FrameRate XVidC_GetFrameRate(XVidC_VideoMode VmId)
{
	if (VmId < (XVIDC_VM_NUM_SUPPORTED)) {
		return (XVidC_VideoTimingModes[VmId].FrameRate);
	}
	else {
		return (XVIDC_FR_NUM_SUPPORTED);
	}
}

/******************************************************************************/
/**
 * This function returns the timing parameters for specified resolution.
 *
 * @param	VmId specifies the resolution id.
 *
 * @return	Pointer to a XVidC_VideoTiming structure.
 *
 * @note	None.
 *
*******************************************************************************/
const XVidC_VideoTiming *XVidC_GetTimingInfo(XVidC_VideoMode VmId)
{
	if (VmId < (XVIDC_VM_NUM_SUPPORTED)) {
		return (&XVidC_VideoTimingModes[VmId].Timing);
	}
	else {
		return (NULL);
	}
}

/******************************************************************************/
/**
 * This function prints the stream information on STDIO/UART console.
 *
 * @param	Stream is a pointer to video stream.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XVidC_ReportStreamInfo(XVidC_VideoStream *Stream)
{
	xil_printf("\tColor Space Format:%s\r\n",
				XVidC_GetColorFormatStr(Stream->ColorFormatId));
	xil_printf("\tColor Depth:%d\r\n", Stream->ColorDepth);
	xil_printf("\tPixels Per Clock:%d\r\n", Stream->PixPerClk);
	xil_printf("\tFrame Rate:%s\r\n", XVidC_GetFrameRateStr(Stream->VmId));
	xil_printf("\tMode:%s\r\n",
			Stream->IsInterlaced ? "Interlaced" : "Progressive" );
	xil_printf("\tResolution:%s\r\n", XVidC_GetVideoModeStr(Stream->VmId));
	xil_printf("\tPixel Clock:%d\r\n",
				XVidC_GetPixelClockHzByVmId(Stream->VmId));
}

/******************************************************************************/
/**
 * This function prints timing information on STDIO/Uart console.
 *
 * @param	Timing is a pointer to Video Timing structure of the stream.
 * @param	IsInterlaced is a TRUE/FALSE flag that denotes the timing
 *		parameter is for interlaced/progressive stream.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XVidC_ReportTiming(XVidC_VideoTiming *Timing, u8 IsInterlaced)
{
	xil_printf("\r\n\tHSYNC Timing: hav=%04d, hfp=%02d, hsw=%02d(hsp=%d), "
			"hbp=%03d, htot=%04d \n\r", Timing->HActive,
			Timing->HFrontPorch, Timing->HSyncWidth,
			Timing->HSyncPolarity,
			Timing->HBackPorch, Timing->HTotal);

	/* Interlaced */
	if (IsInterlaced) {
		xil_printf("\tVSYNC Timing (Field 0): vav=%04d, vfp=%02d, "
			"vsw=%02d(vsp=%d), vbp=%03d, vtot=%04d\n\r",
			Timing->VActive, Timing->F0PVFrontPorch,
			Timing->F0PVSyncWidth, Timing->VSyncPolarity,
			Timing->F0PVBackPorch, Timing->F0PVTotal);
	xil_printf("\tVSYNC Timing (Field 1): vav=%04d, vfp=%02d, "
			"vsw=%02d(vsp=%d), vbp=%03d, vtot=%04d\n\r",
			Timing->VActive, Timing->F1VFrontPorch,
			Timing->F1VSyncWidth, Timing->VSyncPolarity,
			Timing->F1VBackPorch, Timing->F1VTotal);
	}
	/* Progressive */
	else {
		xil_printf("\tVSYNC Timing: vav=%04d, vfp=%02d, "
			"vsw=%02d(vsp=%d), vbp=%03d, vtot=%04d\n\r",
			Timing->VActive, Timing->F0PVFrontPorch,
			Timing->F0PVSyncWidth, Timing->VSyncPolarity,
			Timing->F0PVBackPorch, Timing->F0PVTotal);
	}
}

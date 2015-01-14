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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xvidc_edid.c
 *
 * Contains function definitions related to the Extended Display Identification
 * Data (EDID) structure which is present in all monitors. All content in this
 * file is agnostic of communication interface protocol.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  11/09/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvidc_edid.h"

/**************************** Function Prototypes *****************************/

static u32 XVidC_EdidIsVideoTimingSupportedPreferredTiming(u8 *EdidRaw,
						XVidC_VideoTimingMode *VtMode);
static u32 XVidC_EdidIsVideoTimingSupportedEstablishedTimings(u8 *EdidRaw,
						XVidC_VideoTimingMode *VtMode);
static u32 XVidC_EdidIsVideoTimingSupportedStandardTimings(u8 *EdidRaw,
						XVidC_VideoTimingMode *VtMode);
static float XVidC_CalculatePower(float Base, u8 Power);
static float XVidC_CalculateBinaryFraction(u16 Val, u8 DecPtIndex);

/**************************** Function Definitions ****************************/

void XVidC_EdidGetVpiIdManName(u8 *EdidRaw, char ManName[4])
{
	ManName[0] = 0x40 + ((EdidRaw[XVIDC_EDID_VPI_ID_MAN_NAME0] &
				XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR0_MASK) >>
				XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR0_SHIFT);
	ManName[1] = 0x40 + (((EdidRaw[XVIDC_EDID_VPI_ID_MAN_NAME0] &
				XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR1_MASK) <<
				XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR1_POS) |
				(EdidRaw[XVIDC_EDID_VPI_ID_MAN_NAME1] >>
				XVIDC_EDID_VPI_ID_MAN_NAME1_CHAR1_SHIFT));
	ManName[2] = 0x40 + (EdidRaw[XVIDC_EDID_VPI_ID_MAN_NAME1] &
				XVIDC_EDID_VPI_ID_MAN_NAME1_CHAR2_MASK);
	ManName[3] = '\0';
}

u8 XVidC_EdidGetBDispVidDigBpc(u8 *EdidRaw)
{
	u8 Bpc;

	switch (((EdidRaw[XVIDC_EDID_BDISP_VID] &
			XVIDC_EDID_BDISP_VID_DIG_BPC_MASK) >>
					XVIDC_EDID_BDISP_VID_DIG_BPC_SHIFT)) {
		case XVIDC_EDID_BDISP_VID_DIG_BPC_6:
			Bpc = 6;
			break;

		case XVIDC_EDID_BDISP_VID_DIG_BPC_8:
			Bpc = 8;
			break;

		case XVIDC_EDID_BDISP_VID_DIG_BPC_10:
			Bpc = 10;
			break;

		case XVIDC_EDID_BDISP_VID_DIG_BPC_12:
			Bpc = 12;
			break;

		case XVIDC_EDID_BDISP_VID_DIG_BPC_14:
			Bpc = 14;
			break;

		case XVIDC_EDID_BDISP_VID_DIG_BPC_16:
			Bpc = 16;
			break;

		default:
			Bpc = XVIDC_EDID_BDISP_VID_DIG_BPC_UNDEF;
			break;
	}

	return Bpc;
}

float XVidC_EdidGetCcRedX(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_REDX_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | (EdidRaw[XVIDC_EDID_CC_RG_LOW] >>
		XVIDC_EDID_CC_RBX_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcRedY(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_REDY_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | ((EdidRaw[XVIDC_EDID_CC_RG_LOW] &
		XVIDC_EDID_CC_RBY_LOW_MASK) >>
		XVIDC_EDID_CC_RBY_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcGreenX(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_GREENX_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | ((EdidRaw[XVIDC_EDID_CC_RG_LOW] &
		XVIDC_EDID_CC_GWX_LOW_MASK) >>
		XVIDC_EDID_CC_GWX_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcGreenY(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_GREENY_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | (EdidRaw[XVIDC_EDID_CC_RG_LOW] &
		XVIDC_EDID_CC_GWY_LOW_MASK), 9);
}

float XVidC_EdidGetCcBlueX(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_BLUEX_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | (EdidRaw[XVIDC_EDID_CC_BW_LOW] >>
		XVIDC_EDID_CC_RBX_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcBlueY(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_BLUEY_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | ((EdidRaw[XVIDC_EDID_CC_BW_LOW] &
		XVIDC_EDID_CC_RBY_LOW_MASK) >> XVIDC_EDID_CC_RBY_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcWhiteX(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_WHITEX_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | ((EdidRaw[XVIDC_EDID_CC_BW_LOW] &
		XVIDC_EDID_CC_GWX_LOW_MASK) >> XVIDC_EDID_CC_GWX_LOW_SHIFT), 9);
}

float XVidC_EdidGetCcWhiteY(u8 *EdidRaw)
{
	return XVidC_CalculateBinaryFraction(
		(EdidRaw[XVIDC_EDID_CC_WHITEY_HIGH] <<
		XVIDC_EDID_CC_HIGH_SHIFT) | (EdidRaw[XVIDC_EDID_CC_BW_LOW] &
		XVIDC_EDID_CC_GWY_LOW_MASK), 9);
}

u16 XVidC_EdidGetStdTimingsV(u8 *EdidRaw, u8 StdTimingsNum)
{
	u16 V;

	switch (XVidC_EdidGetStdTimingsAr(EdidRaw, StdTimingsNum)) {
		case XVIDC_EDID_STD_TIMINGS_AR_16_10:
			V = (10 * XVidC_EdidGetStdTimingsH(EdidRaw,
							StdTimingsNum)) / 16;
			break;

		case XVIDC_EDID_STD_TIMINGS_AR_4_3:
			V = (3 * XVidC_EdidGetStdTimingsH(EdidRaw,
							StdTimingsNum)) / 4;
			break;

		case XVIDC_EDID_STD_TIMINGS_AR_5_4:
			V = (4 * XVidC_EdidGetStdTimingsH(EdidRaw,
							StdTimingsNum)) / 5;
			break;

		default:
			V = (9 * XVidC_EdidGetStdTimingsH(EdidRaw,
							StdTimingsNum)) / 16;
			break;
	}

	return V;
}

u32 XVidC_EdidIsVideoTimingSupported(u8 *EdidRaw, XVidC_VideoTimingMode *VtMode)
{
	u32 Status;

	/* Check if the video mode is the preferred timing. */
	Status = XVidC_EdidIsVideoTimingSupportedPreferredTiming(EdidRaw,
									VtMode);
	if (Status == XST_SUCCESS) {
		return Status;
	}

	/* Check established timings I, II, and III. */
	Status = XVidC_EdidIsVideoTimingSupportedEstablishedTimings(EdidRaw,
									VtMode);
	if (Status == XST_SUCCESS) {
		return Status;
	}

	/* Check in standard timings support. */
	Status = XVidC_EdidIsVideoTimingSupportedStandardTimings(EdidRaw,
									VtMode);

	return Status;
}

static u32 XVidC_EdidIsVideoTimingSupportedPreferredTiming(u8 *EdidRaw,
						XVidC_VideoTimingMode *VtMode)
{
	u8 *Ptm;

	Ptm = &EdidRaw[XVIDC_EDID_PTM];

	u32 HActive =
			(((Ptm[XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4] &
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK) >>
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XVIDC_EDID_DTD_PTM_HRES_LSB];

	u32 VActive =
			(((Ptm[XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4] &
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK) >>
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XVIDC_EDID_DTD_PTM_VRES_LSB];

	if (VtMode->Timing.F1VTotal != XVidC_EdidIsDtdPtmInterlaced(EdidRaw)) {
		return (XST_FAILURE);
	}
	else if ((VtMode->Timing.HActive == HActive) &&
			(VtMode->Timing.VActive == VActive)) {
		return (XST_SUCCESS);
	}

	return XST_FAILURE;
}

static u32 XVidC_EdidIsVideoTimingSupportedEstablishedTimings(u8 *EdidRaw,
						XVidC_VideoTimingMode *VtMode)
{
	u32 Status = XST_FAILURE;

	/* First, check established timings I, II, and III. */
	if ((VtMode->Timing.HActive == 800) &&
			(VtMode->Timing.VActive == 640) &&
			(VtMode->FrameRate == XVIDC_FR_56HZ) &&
			XVidC_EdidSuppEstTimings800x600_56(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 640) &&
			(VtMode->Timing.VActive == 480) &&
			(VtMode->FrameRate == XVIDC_FR_60HZ) &&
			XVidC_EdidSuppEstTimings640x480_60(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 800) &&
			(VtMode->Timing.VActive == 600) &&
			(VtMode->FrameRate == XVIDC_FR_60HZ) &&
			XVidC_EdidSuppEstTimings800x600_60(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1024) &&
			(VtMode->Timing.VActive == 768) &&
			(VtMode->FrameRate == XVIDC_FR_60HZ) &&
			XVidC_EdidSuppEstTimings1024x768_60(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 640) &&
			(VtMode->Timing.VActive == 480) &&
			(VtMode->FrameRate == XVIDC_FR_67HZ) &&
			XVidC_EdidSuppEstTimings640x480_67(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 720) &&
			(VtMode->Timing.VActive == 400) &&
			(VtMode->FrameRate == XVIDC_FR_70HZ) &&
			XVidC_EdidSuppEstTimings720x400_70(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1024) &&
			(VtMode->Timing.VActive == 768) &&
			(VtMode->FrameRate == XVIDC_FR_70HZ) &&
			XVidC_EdidSuppEstTimings1024x768_70(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 640) &&
			(VtMode->Timing.VActive == 480) &&
			(VtMode->FrameRate == XVIDC_FR_72HZ) &&
			XVidC_EdidSuppEstTimings640x480_72(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 800) &&
			(VtMode->Timing.VActive == 600) &&
			(VtMode->FrameRate == XVIDC_FR_72HZ) &&
			XVidC_EdidSuppEstTimings800x600_72(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 640) &&
			(VtMode->Timing.VActive == 480) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings640x480_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 800) &&
			(VtMode->Timing.VActive == 600) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings800x600_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 832) &&
			(VtMode->Timing.VActive == 624) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings832x624_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1024) &&
			(VtMode->Timing.VActive == 768) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings1024x768_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1152) &&
			(VtMode->Timing.VActive == 870) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings1152x870_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1280) &&
			(VtMode->Timing.VActive == 1024) &&
			(VtMode->FrameRate == XVIDC_FR_75HZ) &&
			XVidC_EdidSuppEstTimings1280x1024_75(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 1024) &&
			(VtMode->Timing.VActive == 768) &&
			(VtMode->FrameRate == XVIDC_FR_87HZ) &&
			XVidC_EdidSuppEstTimings1024x768_87(EdidRaw)) {
		Status = XST_SUCCESS;
	}
	else if ((VtMode->Timing.HActive == 720) &&
			(VtMode->Timing.VActive == 400) &&
			(VtMode->FrameRate == XVIDC_FR_88HZ) &&
			XVidC_EdidSuppEstTimings720x400_88(EdidRaw)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static u32 XVidC_EdidIsVideoTimingSupportedStandardTimings(u8 *EdidRaw,
			XVidC_VideoTimingMode *VtMode)
{
	u8 Index;

	for (Index = 0; Index < 8; Index++) {
		if ((VtMode->Timing.HActive ==
			XVidC_EdidGetStdTimingsH(EdidRaw, Index + 1)) &&
			(VtMode->Timing.VActive ==
				XVidC_EdidGetStdTimingsV(EdidRaw, Index + 1)) &&
			(VtMode->FrameRate == (u8)XVidC_EdidGetStdTimingsFrr(
							EdidRaw, Index + 1))) {
			return XST_SUCCESS;
		}
	}

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * Perform a power operation.
 *
 * @param	Base is b in the power operation, b^n.
 * @param	Power is n in the power operation, b^n.
 *
 * @return	Base^Power (Base to the power of Power).
 *
 * @note	None.
 *
*******************************************************************************/
static float XVidC_CalculatePower(float Base, u8 Power)
{
	u8 Index;
	float Res = 1.0;

	for (Index = 0; Index < Power; Index++) {
		Res *= Base;
	}

	return Res;
}

/******************************************************************************/
/**
 * Convert a fractional binary number into a decimal number. Binary digits to
 * the right of the decimal point represent 2^-1 to 2^-(DecPtIndex+1). Binary
 * digits to the left of the decimal point represent 2^0, 2^1, etc.
 *
 * @param	Val is the binary representation of the fraction.
 * @param	DecPtIndex is the index of the decimal point in the binary
 *		number. The decimal point is between the binary digits at Val's
 *		indices (DecPtIndex) and (DecPtIndex + 1).
 *
 * @return	Base^Power (Base to the power of Power).
 *
 * @note	None.
 *
*******************************************************************************/
static float XVidC_CalculateBinaryFraction(u16 Val, u8 DecPtIndex)
{
	int Index;
	float Res;

	for (Index = DecPtIndex, Res = 0; Index >= 0; Index--) {
		if (((Val >> Index) & 0x1) == 1) {
			Res += XVidC_CalculatePower(
						0.5, DecPtIndex - Index + 1);
		}
	}

	return (Val >> (DecPtIndex + 1)) + Res;
}

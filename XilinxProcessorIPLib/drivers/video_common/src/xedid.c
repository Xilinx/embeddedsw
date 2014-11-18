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
 * @file xedid.c
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

#include "xedid.h"

/**************************** Function Prototypes *****************************/

static float XVid_CalculatePower(float Base, u8 Power);
static float XVid_CalculateBinaryFraction(u16 Val, u8 DecPtIndex);

/**************************** Function Definitions ****************************/

void XEDID_GET_VPI_ID_MAN_NAME(u8 *EdidRaw, char ManName[4])
{
	ManName[0] = 0x40 + ((EdidRaw[XEDID_VPI_ID_MAN_NAME0] &
				XEDID_VPI_ID_MAN_NAME0_CHAR0_MASK) >>
				XEDID_VPI_ID_MAN_NAME0_CHAR0_SHIFT);
	ManName[1] = 0x40 + (((EdidRaw[XEDID_VPI_ID_MAN_NAME0] &
				XEDID_VPI_ID_MAN_NAME0_CHAR1_MASK) <<
				XEDID_VPI_ID_MAN_NAME0_CHAR1_POS) |
				(EdidRaw[XEDID_VPI_ID_MAN_NAME1] >>
				XEDID_VPI_ID_MAN_NAME1_CHAR1_SHIFT));
	ManName[2] = 0x40 + (EdidRaw[XEDID_VPI_ID_MAN_NAME1] &
				XEDID_VPI_ID_MAN_NAME1_CHAR2_MASK);
	ManName[3] = '\0';
}

u8 XEDID_GET_BDISP_VID_DIG_BPC(u8 *EdidRaw)
{
	u8 Bpc;

	switch (((EdidRaw[XEDID_BDISP_VID] & XEDID_BDISP_VID_DIG_BPC_MASK) >>
					XEDID_BDISP_VID_DIG_BPC_SHIFT)) {
		case XEDID_BDISP_VID_DIG_BPC_6:
			Bpc = 6;
			break;
		case XEDID_BDISP_VID_DIG_BPC_8:
			Bpc = 8;
			break;
		case XEDID_BDISP_VID_DIG_BPC_10:
			Bpc = 10;
			break;
		case XEDID_BDISP_VID_DIG_BPC_12:
			Bpc = 12;
			break;
		case XEDID_BDISP_VID_DIG_BPC_14:
			Bpc = 14;
			break;
		case XEDID_BDISP_VID_DIG_BPC_16:
			Bpc = 16;
			break;
		default:
			Bpc = XEDID_BDISP_VID_DIG_BPC_UNDEF;
			break;
	}

	return Bpc;
}

float XEDID_GET_CC_REDX(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_REDX_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | (EdidRaw[XEDID_CC_RG_LOW] >>
		XEDID_CC_RBX_LOW_SHIFT), 9);
}

float XEDID_GET_CC_REDY(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_REDY_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | ((EdidRaw[XEDID_CC_RG_LOW] &
		XEDID_CC_RBY_LOW_MASK) >> XEDID_CC_RBY_LOW_SHIFT), 9);
}

float XEDID_GET_CC_GREENX(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_GREENX_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | ((EdidRaw[XEDID_CC_RG_LOW] &
		XEDID_CC_GWX_LOW_MASK) >> XEDID_CC_GWX_LOW_SHIFT), 9);
}

float XEDID_GET_CC_GREENY(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_GREENY_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | (EdidRaw[XEDID_CC_RG_LOW] &
		XEDID_CC_GWY_LOW_MASK), 9);
}

float XEDID_GET_CC_BLUEX(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_BLUEX_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | (EdidRaw[XEDID_CC_BW_LOW] >>
		XEDID_CC_RBX_LOW_SHIFT), 9);
}

float XEDID_GET_CC_BLUEY(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_BLUEY_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | ((EdidRaw[XEDID_CC_BW_LOW] &
		XEDID_CC_RBY_LOW_MASK) >> XEDID_CC_RBY_LOW_SHIFT), 9);
}

float XEDID_GET_CC_WHITEX(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_WHITEX_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | ((EdidRaw[XEDID_CC_BW_LOW] &
		XEDID_CC_GWX_LOW_MASK) >> XEDID_CC_GWX_LOW_SHIFT), 9);
}

float XEDID_GET_CC_WHITEY(u8 *EdidRaw)
{
	return XVid_CalculateBinaryFraction((EdidRaw[XEDID_CC_WHITEY_HIGH] <<
		XEDID_CC_HIGH_SHIFT) | (EdidRaw[XEDID_CC_BW_LOW] &
		XEDID_CC_GWY_LOW_MASK), 9);
}

u16 XEDID_GET_STD_TIMINGS_V(u8 *EdidRaw, u8 StdTimingsNum)
{
	u16 V;

	switch (XEDID_GET_STD_TIMINGS_AR(EdidRaw, StdTimingsNum)) {
	case XEDID_STD_TIMINGS_AR_16_10:
		V = (10 * XEDID_GET_STD_TIMINGS_H(EdidRaw, StdTimingsNum)) / 16;
		break;
	case XEDID_STD_TIMINGS_AR_4_3:
		V = (3 * XEDID_GET_STD_TIMINGS_H(EdidRaw, StdTimingsNum)) / 4;
		break;
	case XEDID_STD_TIMINGS_AR_5_4:
		V = (4 * XEDID_GET_STD_TIMINGS_H(EdidRaw, StdTimingsNum)) / 5;
		break;
	default:
		V = (9 * XEDID_GET_STD_TIMINGS_H(EdidRaw, StdTimingsNum)) / 16;
		break;
	}

	return V;
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
static float XVid_CalculatePower(float Base, u8 Power)
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
static float XVid_CalculateBinaryFraction(u16 Val, u8 DecPtIndex)
{
	int Index;
	float Res;

	for (Index = DecPtIndex, Res = 0; Index >= 0; Index--) {
		if (((Val >> Index) & 0x1) == 1) {
			Res += XVid_CalculatePower(0.5, DecPtIndex - Index + 1);
		}
	}

	return (Val >> (DecPtIndex + 1)) + Res;
}

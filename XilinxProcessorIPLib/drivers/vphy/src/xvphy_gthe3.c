/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
 * @file xvphy_gthe3.c
 *
 * Contains a minimal set of functions for the XVphy driver that allow access
 * to all of the Video PHY core's functionality. See xvphy.h for a detailed
 * description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * 1.1   gm   03/18/16 Added XVphy_Gthe3RxPllRefClkDiv1Reconfig function
 * 1.2   gm   08/26/16 Suppressed warning messages due to unused arguments
 * 1.4   gm   29/11/16 Added preprocessor directives for sw footprint reduction
 *                     Changed TX reconfig hook from TxPllRefClkDiv1Reconfig to
 *                       TxChReconfig
 *                     Added TX datawidth dynamic reconfiguration
 *                     Added N2=8 divider for CPLL for DP
 *                     Added CPLL_CFGx reconfiguration in
 *                       XVphy_Gthe3ClkChReconfig API
 *                     Corrected the default return value of DRP encoding
 *                       APIs to prevent overwritting the reserved bits
 * 1.6   gm   12/06/17 Changed XVphy_DrpRead with XVphy_DrpRd
 *                     Changed XVphy_DrpWrite with XVphy_DrpWr
 *                     Improved status return of APIs with DRP Rd and Wr
 *                     Added N2=8 divider for CPLL for HDMI
 * 1.7   gm   13/09/17 Disabled intelligent clock sel in QPLL0/1 configuration
 *                     Updated DP CDR config for 8.1 Gbps
 *                     Updated XVPHY_QPLL0_MAX to 16375000000LL
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xvphy_gt.h"
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE3)
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static u8 XVphy_MToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
static u16 XVphy_NToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 NId);
static u8 XVphy_DToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);
static u8 XVphy_DrpEncodeQpllMCpllMN2(u8 AttrEncode);
static u8 XVphy_DrpEncodeCpllN1(u8 AttrEncode);
static u8 XVphy_DrpEncodeCpllTxRxD(u8 AttrEncode);
static u16 XVphy_DrpEncodeQpllN(u8 AttrEncode);
static u8 Xvphy_DrpEncodeDataWidth(u8 AttrEncode);
static u8 Xvphy_DrpEncodeIntDataWidth(u8 AttrEncode);
static u16 XVphy_DrpEncodeClk25(u32 RefClkFreqHz);

u32 XVphy_Gthe3CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_Gthe3CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XVphy_Gthe3OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);
u32 XVphy_Gthe3ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gthe3ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId);
u32 XVphy_Gthe3TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gthe3RxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gthe3TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gthe3RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
#define XVPHY_DRP_RXCDR_CFG(n)			(0x0E + n)

#define XVPHY_DRP_CPLL_FBDIV       		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV  		0x2A
#define XVPHY_DRP_RXOUT_DIV       		0x63
#define XVPHY_DRP_RXCLK25       		0x6D
#define XVPHY_DRP_TXCLK25       		0x7A
#define XVPHY_DRP_TXOUT_DIV        		0x7C
#define XVPHY_DRP_QPLL1_FBDIV      		0x94
#define XVPHY_DRP_QPLL1_REFCLK_DIV 		0x98
#define XVPHY_DRP_RXCDR_CFG_WORD0  		0x0E
#define XVPHY_DRP_RXCDR_CFG_WORD1  		0x0F
#define XVPHY_DRP_RXCDR_CFG_WORD2  		0x10
#define XVPHY_DRP_RXCDR_CFG_WORD3  		0x11
#define XVPHY_DRP_RXCDR_CFG_WORD4  		0x12

/* PLL operating ranges. */
#define XVPHY_QPLL0_MIN			 9800000000LL
#define XVPHY_QPLL0_MAX			16375000000LL
#define XVPHY_QPLL1_MIN			 8000000000LL
#define XVPHY_QPLL1_MAX			13000000000LL
#define XVPHY_CPLL_MIN			 2000000000LL
#define XVPHY_CPLL_MAX			 6250000000LL

const u8 Gthe3CpllDivsM[]	= {1, 2, 0};
const u8 Gthe3CpllDivsN1[]	= {4, 5, 0};
const u8 Gthe3CpllDivsN2[]	= {1, 2, 3, 4, 5, 8, 0};
const u8 Gthe3CpllDivsD[]	= {1, 2, 4, 8, 0};

const u8 Gthe3QpllDivsM[]	= {4, 3, 2, 1, 0};
const u8 Gthe3QpllDivsN1[]	= {16, 20, 32, 40, 60, 64, 66, 75, 80, 84, 90,
					96, 100, 112, 120, 125, 150, 160, 0};
const u8 Gthe3QpllDivsN2[]	= {1, 0};
const u8 Gthe3QpllDivsD[]	= {16, 8, 4, 2, 1, 0};

const XVphy_GtConfig Gthe3Config = {
	.CfgSetCdr = XVphy_Gthe3CfgSetCdr,
	.CheckPllOpRange = XVphy_Gthe3CheckPllOpRange,
	.OutDivChReconfig = XVphy_Gthe3OutDivChReconfig,
	.ClkChReconfig = XVphy_Gthe3ClkChReconfig,
	.ClkCmnReconfig = XVphy_Gthe3ClkCmnReconfig,
	.RxChReconfig = XVphy_Gthe3RxChReconfig,
	.TxChReconfig = XVphy_Gthe3TxChReconfig,

	.CpllDivs = {
		.M = Gthe3CpllDivsM,
		.N1 = Gthe3CpllDivsN1,
		.N2 = Gthe3CpllDivsN2,
		.D = Gthe3CpllDivsD,
	},
	.QpllDivs = {
		.M = Gthe3QpllDivsM,
		.N1 = Gthe3QpllDivsN1,
		.N2 = Gthe3QpllDivsN2,
		.D = Gthe3QpllDivsD,
	},
};

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will set the clock and data recovery (CDR) values for a given
* channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr;
	u32 Status = XST_SUCCESS;
	u64 LineRateHz;

	/* Set CDR values only for CPLLs. */
	if ((ChId < XVPHY_CHANNEL_ID_CH1) || (ChId > XVPHY_CHANNEL_ID_CH4)) {
		return XST_FAILURE;
	}

	/* This is DP specific. */
	ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)];

	ChPtr->PllParams.Cdr[0] = 0x0000;
	ChPtr->PllParams.Cdr[1] = 0x0000;
	ChPtr->PllParams.Cdr[3] = 0x0000;
	ChPtr->PllParams.Cdr[4] = 0x0000;
	if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP) {

		LineRateHz = XVphy_GetLineRateHz(InstancePtr, QuadId, ChId);

		if(LineRateHz==XVPHY_DP_LINK_RATE_HZ_810GBPS) {
		  ChPtr->PllParams.Cdr[2] = 0x0742;
		} else if(LineRateHz==XVPHY_DP_LINK_RATE_HZ_540GBPS) {
			ChPtr->PllParams.Cdr[2] = 0x0742;
		} else if(LineRateHz==XVPHY_DP_LINK_RATE_HZ_270GBPS) {
			ChPtr->PllParams.Cdr[2] = 0x0721;
		} else {
			ChPtr->PllParams.Cdr[2] = 0x0721;
		}
	}
	else if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI) {
		/* RxOutDiv = 1  => Cdr[2] = 0x07E4
		 * RxOutDiv = 2  => Cdr[2] = 0x07D4
		 * RxOutDiv = 4  => Cdr[2] = 0x07C4
		 * RxOutDiv = 8  => Cdr[2] = 0x07B4
		 * RxOutDiv = 16 => Cdr[2] = 0x07A4 */
		ChPtr->PllParams.Cdr[2] = 0x07E4;
		while (ChPtr->RxOutDiv >>= 1) {
			ChPtr->PllParams.Cdr[2] -= 0x10;
		}
		/* Restore RxOutDiv. */
		ChPtr->RxOutDiv = 1 << ((0x7E4 - ChPtr->PllParams.Cdr[2]) >> 4);
	}
	else {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will check if a given PLL output frequency is within the
* operating range of the PLL for the GT type.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	PllClkOutFreqHz is the frequency to check.
*
* @return
*		- XST_SUCCESS if the frequency resides within the PLL's range.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz)
{
	u32 Status = XST_FAILURE;

	/* Suppress Warning Messages */
	InstancePtr = InstancePtr;
	QuadId = QuadId;

	if (((ChId == XVPHY_CHANNEL_ID_CMN0) &&
				(XVPHY_QPLL0_MIN <= PllClkOutFreqHz) &&
				(PllClkOutFreqHz <= XVPHY_QPLL0_MAX)) ||
			((ChId == XVPHY_CHANNEL_ID_CMN1) &&
				(XVPHY_QPLL1_MIN <= PllClkOutFreqHz) &&
				(PllClkOutFreqHz <= XVPHY_QPLL1_MAX)) ||
			((ChId >= XVPHY_CHANNEL_ID_CH1) &&
			 (ChId <= XVPHY_CHANNEL_ID_CH4) &&
				(XVPHY_CPLL_MIN <= PllClkOutFreqHz) &&
				(PllClkOutFreqHz <= XVPHY_CPLL_MAX))) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will set the output divider logic for a given channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	if (Dir == XVPHY_DIR_RX) {
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x63, &DrpVal);
		/* Mask out RX_OUT_DIV. */
		DrpVal &= ~0x07;
		/* Set RX_OUT_DIV. */
		WriteVal = (XVphy_DToDrpEncoding(InstancePtr, QuadId, ChId,
				XVPHY_DIR_RX) & 0x7);
		DrpVal |= WriteVal;
		/* Write new DRP register value for RX dividers. */
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x63, DrpVal);
	}
	else {
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x7C, &DrpVal);
		/* Mask out TX_OUT_DIV. */
		DrpVal &= ~0x700;
		/* Set TX_OUT_DIV. */
		WriteVal = (XVphy_DToDrpEncoding(InstancePtr, QuadId, ChId,
				XVPHY_DIR_TX) & 0x7);
		DrpVal |= (WriteVal << 8);
		/* Write new DRP register value for RX dividers. */
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x7C, DrpVal);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel clock settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u16 WriteVal;
	u32 CpllxVcoRateMHz;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for PLL dividers. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x28, &DrpVal);
	/* Mask out clock divider bits. */
	DrpVal &= ~(0xFF80);
	/* Set CPLL_FBDIV. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, ChId, 2) & 0xFF);
	DrpVal |= (WriteVal << 8);
	/* Set CPLL_FBDIV_45. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, ChId, 1) & 0x1);
	DrpVal |= (WriteVal << 7);
	/* Write new DRP register value for PLL dividers. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x28, DrpVal);

	/* Write CPLL Ref Clk Div. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x2A, &DrpVal);
	/* Mask out clock divider bits. */
	DrpVal &= ~(0xF800);
	/* Set CPLL_REFCLKDIV. */
	WriteVal = (XVphy_MToDrpEncoding(InstancePtr, QuadId, ChId) & 0x1F);
	DrpVal |= (WriteVal << 11);
	/* Write new DRP register value for PLL dividers. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x2A, DrpVal);

	CpllxVcoRateMHz = XVphy_GetPllVcoFreqHz(InstancePtr, QuadId, ChId,
			XVphy_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
					XVPHY_DIR_TX : XVPHY_DIR_RX) / 1000000;

	/* CPLL_CFG0 */
	if (CpllxVcoRateMHz <= 3000) {
		DrpVal = 0x67F8;
	}
	else if (CpllxVcoRateMHz <= 4250) {
		DrpVal = 0x21F8;
	}
	else {
		DrpVal = 0x23FC;
	}
	/* Write new DRP register value for CPLL_CFG0. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0xCB, DrpVal);

	/* CPLL_CFG1 */
	/* No need to change CFG1. This is solely based if silicon is prod */


	/* CPLL_CFG2 */
	/* Obtain current DRP register value for CPLL_CFG2. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0xBC, &DrpVal);
	/* Mask out clock divider bits. */
	DrpVal &= ~(0x7);

	if (CpllxVcoRateMHz <= 3000) {
		DrpVal |= 0x7;
	}
	else {
		DrpVal |= 0x4;
	}
	/* Write new DRP register value for CPLL_CFG2. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0xBC, DrpVal);

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the common channel clock settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	CmnId is the common channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for QPLLx_FBDIV. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
		(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x14 : 0x94, &DrpVal);
	/* Mask out QPLLx_FBDIV. */
	DrpVal &= ~(0xFF);
	/* Set QPLLx_FBDIV. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, CmnId, 0) & 0xFF);
	DrpVal |= WriteVal;
	/* Write new DRP register value for QPLLx_FBDIV. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
		(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x14 : 0x94, DrpVal);

	/* Obtain current DRP register value for QPLLx_REFCLK_DIV. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
		(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x18 : 0x98, &DrpVal);
	/* Mask out QPLLx_REFCLK_DIV. */
	DrpVal &= ~(0xF80);
	/* Disable Intelligent Reference Clock Selection */
	DrpVal |= (1 << 6);
	/* Set QPLLx_REFCLK_DIV. */
	WriteVal = (XVphy_MToDrpEncoding(InstancePtr, QuadId, CmnId) & 0x1F);
	DrpVal |= (WriteVal << 7);
	/* Write new DRP register value for QPLLx_REFCLK_DIV. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
		(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x18 : 0x98, DrpVal);

	if ((InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) ||
		(InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI)) {
		/* QPLLx_LPF */
		switch (InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].
				PllParams.NFbDiv) {
		case 40:
			DrpVal = 0x3FF;
			break;
		case 80:
			DrpVal = 0x3F4;
			break;
		case 160:
			DrpVal = 0x3FC;
			break;
		default:
			DrpVal = 0x3FE;
			break;
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x19 : 0x99, DrpVal);

		/* QPLLx_CP */
		switch (InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].
				PllParams.NFbDiv) {
		case 160:
			DrpVal = 0x1FF;
			break;
		default:
			DrpVal = (CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x1F : 0x7F;
			break;
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x16 : 0x96, DrpVal);

		/* QPLLx_CFG4 */
		DrpVal = 0x1B;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x30 : 0xB0, DrpVal);
		/* QPLLx_LOCK_CFG */
		DrpVal = 0x25E8;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x12 : 0x92, DrpVal);
		/* QPLLx_LOCK_CFG_G3 */
		Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x1D : 0x9D, DrpVal);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3RxChReconfig(XVphy *InstancePtr, u8 QuadId,
            XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr;
	u16 DrpVal;
	u16 WriteVal;
	u8 CfgIndex;
    u32 Status = XST_SUCCESS;

	ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)];

	/* RXCDR_CFG(CfgIndex) */
	for (CfgIndex = 0; CfgIndex < 5; CfgIndex++) {
		DrpVal = ChPtr->PllParams.Cdr[CfgIndex];
		if (!DrpVal) {
			/* Don't modify RX_CDR configuration. */
			continue;
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId,
			XVPHY_DRP_RXCDR_CFG(CfgIndex), DrpVal);
	}

	if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI) {
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x66, &DrpVal);
		DrpVal &= ~(0x3);
		WriteVal = (Xvphy_DrpEncodeIntDataWidth(ChPtr->RxIntDataWidth) & 0x3);
		DrpVal |= WriteVal;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x66, DrpVal);

		/* RX_DATA_WIDTH */
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x03, &DrpVal);
		DrpVal &= ~(0x1E0);
		WriteVal = (Xvphy_DrpEncodeDataWidth(ChPtr->RxDataWidth) & 0xF);
		WriteVal <<= 5;
		DrpVal |= WriteVal;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x03, DrpVal);
	}

	Status |= XVphy_Gthe3RxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr;
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)];

	if (InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) {
		/* TX_INT_DATAWIDTH */
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x85, &DrpVal);
		DrpVal &= ~(0x3 << 10);
		WriteVal = ((Xvphy_DrpEncodeIntDataWidth(ChPtr->
						TxIntDataWidth) & 0x3) << 10);
		DrpVal |= WriteVal;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x85, DrpVal);

		/* TX_DATA_WIDTH */
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x7A, &DrpVal);
		DrpVal &= ~(0xF);
		WriteVal = (Xvphy_DrpEncodeDataWidth(ChPtr->TxDataWidth) & 0xF);
		DrpVal |= WriteVal;
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x7A, DrpVal);
	}

    Status |= XVphy_Gthe3TxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX CLKDIV1 settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u32 TxRefClkHz;
    u32 Status = XST_SUCCESS;
	XVphy_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XVPHY_CH2IDX(ChId)];

	if (InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) {
		TxRefClkHz = InstancePtr->HdmiTxRefClkHz;
	}
	else {
		TxRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
								PllPtr->PllRefClkSel);
	}

	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, XVPHY_DRP_TXCLK25,
                    &DrpVal);
	DrpVal &= ~(0xF800);
	DrpVal |= XVphy_DrpEncodeClk25(TxRefClkHz) << 11;
    Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_DRP_TXCLK25,
			DrpVal);
	return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX CLKDIV1 settings.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gthe3RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u32 RxRefClkHz;
    u32 Status = XST_SUCCESS;
	XVphy_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XVPHY_CH2IDX(ChId)];

	if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI) {
		RxRefClkHz = InstancePtr->HdmiRxRefClkHz;
	}
	else {
		RxRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
								PllPtr->PllRefClkSel);
	}

	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, XVPHY_DRP_RXCLK25,
            &DrpVal);
	DrpVal &= ~(0x00F8);
	DrpVal |= XVphy_DrpEncodeClk25(RxRefClkHz) << 3;
    Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_DRP_RXCLK25,
			DrpVal);

	return Status;
}

/*****************************************************************************/
/**
* This function will translate the configured M value to DRP encoding.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return	The DRP encoding for M.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_MToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u8 MRefClkDiv;
	u8 DrpEncode;

	if ((ChId >= XVPHY_CHANNEL_ID_CH1) && (ChId <= XVPHY_CHANNEL_ID_CH4)) {
		MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)]
			.PllParams.MRefClkDiv;
	}
	else if ((ChId == XVPHY_CHANNEL_ID_CMN0) ||
			(ChId == XVPHY_CHANNEL_ID_CMN1)) {
		MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)]
			.PllParams.MRefClkDiv;
	}
	else {
		MRefClkDiv = 0;
	}

	DrpEncode = XVphy_DrpEncodeQpllMCpllMN2(MRefClkDiv);

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured D value to DRP encoding.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return	The DRP encoding for D.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_DToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir)
{
	u8 OutDiv;
	u8 DrpEncode;

	OutDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
		OutDiv[Dir];

	DrpEncode = XVphy_DrpEncodeCpllTxRxD(OutDiv);

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured N1/N2 value to DRP encoding.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	NId specified to operate on N1 (if == 1) or N2 (if == 2).
*
* @return	The DRP encoding for N1/N2.
*
* @note		None.
*
******************************************************************************/
static u16 XVphy_NToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 NId)
{
	u8 NFbDiv;
	u16 DrpEncode;

	if ((ChId == XVPHY_CHANNEL_ID_CMN0) ||
			(ChId == XVPHY_CHANNEL_ID_CMN1)) {
		NFbDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
			PllParams.NFbDiv;
		DrpEncode = XVphy_DrpEncodeQpllN(NFbDiv);
	}
	else if (NId == 1) {
		NFbDiv = InstancePtr->Quads[QuadId].Plls[
			XVPHY_CH2IDX(ChId)].PllParams.N1FbDiv;
		DrpEncode = XVphy_DrpEncodeCpllN1(NFbDiv);
	}
	else {
		NFbDiv = InstancePtr->Quads[QuadId].Plls[
			XVPHY_CH2IDX(ChId)].PllParams.N2FbDiv;
		DrpEncode = XVphy_DrpEncodeQpllMCpllMN2(NFbDiv);
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured QPLL's M or CPLL's M or N2
* values to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the QPLL's M or CPLL's M or N2 values.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_DrpEncodeQpllMCpllMN2(u8 AttrEncode)
{
	u8 DrpEncode;

	switch (AttrEncode) {
	case 1:
		DrpEncode = 16;
		break;
	case 6:
		DrpEncode = 5;
		break;
	case 10:
		DrpEncode = 7;
		break;
	case 12:
		DrpEncode = 13;
		break;
	case 20:
		DrpEncode = 15;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 8:
	case 16:
		DrpEncode = (AttrEncode - 2);
		break;
	default:
		DrpEncode = 0xF;
		break;
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CPLL's N1 value to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the CPLL's N1 value.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_DrpEncodeCpllN1(u8 AttrEncode)
{
	u8 DrpEncode;

	DrpEncode = (AttrEncode - 4) & 0x1;

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CPLL's D values to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the CPLL's D value.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_DrpEncodeCpllTxRxD(u8 AttrEncode)
{
	u8 DrpEncode;

	switch (AttrEncode) {
	case 1:
		DrpEncode = 0;
		break;
	case 2:
		DrpEncode = 1;
		break;
	case 4:
		DrpEncode = 2;
		break;
	case 8:
		DrpEncode = 3;
		break;
	case 16:
		DrpEncode = 4;
		break;
	default:
		DrpEncode = 0x4;
		break;
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured QPLL's N value to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the QPLL's N value.
*
* @note		None.
*
******************************************************************************/
static u16 XVphy_DrpEncodeQpllN(u8 AttrEncode)
{
	u16 DrpEncode;

	if ((16 <= AttrEncode) && (AttrEncode <= 160)) {
		DrpEncode = AttrEncode - 2;
	}
	else {
		DrpEncode = 0xFF;
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured RXDATAWIDTH to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the RXDATAWIDTH value.
*
* @note		None.
*
******************************************************************************/
static u8 Xvphy_DrpEncodeDataWidth(u8 AttrEncode)
{
	u8 DrpEncode;

	switch (AttrEncode) {
	case 16:
		DrpEncode = 2;
		break;
	case 20:
		DrpEncode = 3;
		break;
	case 32:
		DrpEncode = 4;
		break;
	case 40:
		DrpEncode = 5;
		break;
	case 64:
		DrpEncode = 6;
		break;
	case 80:
		DrpEncode = 7;
		break;
	case 128:
		DrpEncode = 8;
		break;
	case 160:
		DrpEncode = 9;
		break;
	default:
		DrpEncode = 0xF;
		break;
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured RXINTDATAWIDTH to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the RXINTDATAWIDTH value.
*
* @note		None.
*
******************************************************************************/
static u8 Xvphy_DrpEncodeIntDataWidth(u8 AttrEncode)
{
	u8 DrpEncode;

	switch (AttrEncode) {
	case 2:
		DrpEncode = 0;
		break;
	case 4:
		DrpEncode = 1;
		break;
	default:
		DrpEncode = 2;
		break;
	}

	return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CLK25 to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the CLK25 value.
*
* @note		None.
*
******************************************************************************/
static u16 XVphy_DrpEncodeClk25(u32 RefClkFreqHz)
{
	u16 DrpEncode;
	u32 RefClkFreqMHz = RefClkFreqHz / 1000000;

	DrpEncode = ((RefClkFreqMHz / 25) +
			(((RefClkFreqMHz % 25) > 0) ? 1 : 0)) - 1;

	return (DrpEncode & 0x1F);
}
#endif

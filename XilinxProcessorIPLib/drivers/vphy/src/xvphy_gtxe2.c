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
 * @file xvphy_gtxe2.c
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
 * 1.1   gm   12/07/15 Corrected PllParams.Cdr[1] values for DP and HDMI
 *       gm   03/18/16 Added XVphy_Gtxe2RxPllRefClkDiv1Reconfig function
 * 1.4   gm   29/11/16 Added preprocessor directives for sw footprint reduction
 *                     Changed TX reconfig hook from TxPllRefClkDiv1Reconfig to
 *                       TxChReconfig
 *                     Added TX datawidth dynamic reconfiguration
 *                     Corrected the default return value of DRP encoding
 *                       APIs to prevent overwritting the reserved bits
 * 1.6   gm   12/06/17 Changed XVphy_DrpRead with XVphy_DrpRd
 *                     Changed XVphy_DrpWrite with XVphy_DrpWr
 *                     Improved status return of APIs with DRP Rd and Wr
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xvphy_gt.h"
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
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
static u16 XVphy_DrpEncodeClk25(u32 RefClkFreqHz);

u32 XVphy_Gtxe2CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_Gtxe2CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XVphy_Gtxe2OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);
u32 XVphy_Gtxe2ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtxe2ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId);
u32 XVphy_Gtxe2TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtxe2RxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtxe2TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtxe2RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
#define XVPHY_DRP_RXCDR_CFG(n)			(0xA8 + n)

#define XVPHY_DRP_RXDATAWIDTH            	0x11
#define XVPHY_DRP_CPLL_PROG              	0x5E
#define XVPHY_DRP_OUT_DIV_PROG           	0x88
#define XVPHY_DRP_QPLL_CFG               	0x32
#define XVPHY_DRP_QPLL_REFCLK_DIV_PROG   	0x33
#define XVPHY_DRP_QPLL_FBDIV_PROG        	0x36
#define XVPHY_DRP_QPLL_FBDIV_RATIO_PROG  	0x37
#define XVPHY_DRP_TXCLK25        	        0x6A
#define XVPHY_DRP_TXDATAWIDTH               0x6B
#define XVPHY_DRP_RXCDR_CFG_WORD0        	0xA8
#define XVPHY_DRP_RXCDR_CFG_WORD1        	0xA9
#define XVPHY_DRP_RXCDR_CFG_WORD2        	0xAA
#define XVPHY_DRP_RXCDR_CFG_WORD3        	0xAB
#define XVPHY_DRP_RXCDR_CFG_WORD4        	0xAC

/* PLL operating ranges. */
#define XVPHY_QPLL_LB_MIN		 5930000000LL
#define XVPHY_QPLL_LB_MAX		 8000000000LL
#define XVPHY_QPLL_UB_MIN		 9800000000LL
#define XVPHY_QPLL_UB_MAX		12500000000LL
#define XVPHY_CPLL_MIN			 1600000000LL
#define XVPHY_CPLL_MAX			 3300000000LL

const u8 Gtxe2CpllDivsM[]	= {1, 2, 0};
const u8 Gtxe2CpllDivsN1[]	= {4, 5, 0};
const u8 Gtxe2CpllDivsN2[]	= {1, 2, 3, 4, 5, 0};
const u8 Gtxe2CpllDivsD[]	= {1, 2, 4, 8, 0};

const u8 Gtxe2QpllDivsM[]	= {4, 3, 2, 1, 0};
const u8 Gtxe2QpllDivsN1[]	= {16, 20, 32, 40, 64, 66, 80, 100, 0};
const u8 Gtxe2QpllDivsN2[]	= {1, 0};
const u8 Gtxe2QpllDivsD[]	= {8, 4, 2, 1, 0};

const XVphy_GtConfig Gtxe2Config = {
	.CfgSetCdr = XVphy_Gtxe2CfgSetCdr,
	.CheckPllOpRange = XVphy_Gtxe2CheckPllOpRange,
	.OutDivChReconfig = XVphy_Gtxe2OutDivChReconfig,
	.ClkChReconfig = XVphy_Gtxe2ClkChReconfig,
	.ClkCmnReconfig = XVphy_Gtxe2ClkCmnReconfig,
	.RxChReconfig = XVphy_Gtxe2RxChReconfig,
	.TxChReconfig = XVphy_Gtxe2TxChReconfig,

	.CpllDivs = {
		.M = Gtxe2CpllDivsM,
		.N1 = Gtxe2CpllDivsN1,
		.N2 = Gtxe2CpllDivsN2,
		.D = Gtxe2CpllDivsD,
	},
	.QpllDivs = {
		.M = Gtxe2QpllDivsM,
		.N1 = Gtxe2QpllDivsN1,
		.N2 = Gtxe2QpllDivsN2,
		.D = Gtxe2QpllDivsD,
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
u32 XVphy_Gtxe2CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr;

	/* Set CDR values only for CPLLs. */
	if ((ChId < XVPHY_CHANNEL_ID_CH1) || (ChId > XVPHY_CHANNEL_ID_CH4)) {
		return XST_FAILURE;
	}

	ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)];

	ChPtr->PllParams.Cdr[0] = 0x0020;
	ChPtr->PllParams.Cdr[2] = 0x23FF;
	ChPtr->PllParams.Cdr[3] =
		(InstancePtr->Config.RxProtocol != XVPHY_PROTOCOL_HDMI) ?
		0x0000 : 0x8000;
	ChPtr->PllParams.Cdr[4] = 0x0003;

	/* Update the RXCDR_CFG2 settings. */
	switch (ChPtr->RxOutDiv) {
	case 1:
		ChPtr->PllParams.Cdr[1] = 0x2040;
		break;
	case 2:
		ChPtr->PllParams.Cdr[1] = 0x4020;
		break;
	case 4:
		ChPtr->PllParams.Cdr[1] = 0x4010;
		break;
	case 8:
		ChPtr->PllParams.Cdr[1] = 0x4008;
		break;
	default:
		ChPtr->PllParams.Cdr[1] = 0x4010;
		break;
	}

	return XST_SUCCESS;
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
u32 XVphy_Gtxe2CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz)
{
	u32 Status = XST_FAILURE;

	if ((ChId == XVPHY_CHANNEL_ID_CMN0) &&
			((XVPHY_QPLL_LB_MIN <= PllClkOutFreqHz) &&
			(PllClkOutFreqHz <= XVPHY_QPLL_LB_MAX))) {
		InstancePtr->Quads[QuadId].Cmn0.PllParams.IsLowerBand = 1;
		Status = XST_SUCCESS;
	}
	else if ((ChId == XVPHY_CHANNEL_ID_CMN0) &&
			((XVPHY_QPLL_UB_MIN <= PllClkOutFreqHz) &&
				(PllClkOutFreqHz <= XVPHY_QPLL_UB_MAX) &&
			((InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP) ||
			 (InstancePtr->Config.TxProtocol ==	XVPHY_PROTOCOL_DP)))) {
		InstancePtr->Quads[QuadId].Cmn0.PllParams.IsLowerBand = 0;
			Status = XST_SUCCESS;
	}
	else if ((ChId >= XVPHY_CHANNEL_ID_CH1) &&
			(ChId <= XVPHY_CHANNEL_ID_CH4) &&
			(XVPHY_CPLL_MIN <= PllClkOutFreqHz) &&
				(PllClkOutFreqHz <= XVPHY_CPLL_MAX)) {
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
u32 XVphy_Gtxe2OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for TX/RX dividers. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x88, &DrpVal);

	if (Dir == XVPHY_DIR_RX) {
		/* Mask out RX_OUT_DIV. */
		DrpVal &= ~0x07;
		/* Set RX_OUT_DIV. */
		WriteVal = (XVphy_DToDrpEncoding(InstancePtr, QuadId, ChId,
				XVPHY_DIR_RX) & 0x7);
		DrpVal |= WriteVal;
	}
	else {
		/* Mask out TX_OUT_DIV. */
		DrpVal &= ~0x70;
		/* Set TX_OUT_DIV. */
		WriteVal = (XVphy_DToDrpEncoding(InstancePtr, QuadId, ChId,
				XVPHY_DIR_TX) & 0x7);
		DrpVal |= (WriteVal << 4);
	}
	/* Write new DRP register value for TX/RX dividers. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x88, DrpVal);

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
u32 XVphy_Gtxe2ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for PLL dividers. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, 0x5E, &DrpVal);
	/* Mask out clock divider bits. */
	DrpVal &= ~(0x1FFF);
	/* Set CPLL_FBDIV. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, ChId, 2) & 0x7F);
	DrpVal |= WriteVal;
	/* Set CPLL_FBDIV_45. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, ChId, 1) & 0x1);
	DrpVal |= (WriteVal << 7);
	/* Set CPLL_REFCLKDIV. */
	WriteVal = (XVphy_MToDrpEncoding(InstancePtr, QuadId, ChId) & 0x1F);
	DrpVal |= (WriteVal << 8);
	/* Write new DRP register value for PLL dividers. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x5E, DrpVal);

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
u32 XVphy_Gtxe2ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for QPLL_CFG. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, CmnId, 0x32, &DrpVal);
	/* Mask out QPLL_CFG. */
	DrpVal &= ~(1 << 6);
	/* Set QPLL_CFG lower/upper band setting to hardware. */
	WriteVal = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].
		PllParams.IsLowerBand;
	DrpVal |= (WriteVal << 6);
	/* Write new DRP register value for QPLL_CFG. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, CmnId, 0x32, DrpVal);

	/* Obtain current DRP register value for QPLL_REFCLK_DIV. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, CmnId, 0x33, &DrpVal);
	/* Mask out QPLL_REFCLK_DIV. */
	DrpVal &= ~(0xF800);
	/* Set QPLL_REFCLK_DIV. */
	WriteVal = (XVphy_MToDrpEncoding(InstancePtr, QuadId, CmnId) & 0x1F);
	DrpVal |= (WriteVal << 11);
	/* Write new DRP register value for QPLL_REFCLK_DIV. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, CmnId, 0x33, DrpVal);

	/* Obtain current DRP register value for QPLL_FBDIV. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, CmnId, 0x36, &DrpVal);
	/* Mask out QPLL_FBDIV. */
	DrpVal &= ~(0x3FFF);
	/* Set QPLL_FBDIV. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, CmnId, 0) & 0x3FF);
	DrpVal |= WriteVal;
	/* Write new DRP register value for QPLL_FBDIV. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, CmnId, 0x36, DrpVal);

	/* Obtain current DRP register value for QPLL_FBDIV_RATIO. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, CmnId, 0x37, &DrpVal);
	/* Mask out QPLL_FBDIV_RATIO. */
	DrpVal &= ~(1 << 6);
	/* Set QPLL_FBDIV_RATIO. */
	if (InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].PllParams.
			NFbDiv != 66) {
		DrpVal |= (1 << 6);
	}
	/* Write new DRP register value for QPLL_FBDIV_RATIO. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, CmnId, 0x37, DrpVal);

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
u32 XVphy_Gtxe2RxChReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
/* Missing: RXCDR_LOCK_CFG, RX_LPM_*_CFG, RX_DFE_*_CFG */
	XVphy_Channel *ChPtr;
	u16 DrpVal;
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
		/* Set internal Data width */
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId,
				XVPHY_DRP_RXDATAWIDTH, &DrpVal);
		DrpVal &= ~(0x7800);
		if ((InstancePtr->HdmiRxDruIsEnabled) ||
			(InstancePtr->Config.TransceiverWidth == 2)) {
			/* Set internal Data width of the RX GT to 2-byte */
			DrpVal |= (0 << 14);

			/* Set RX Data width of the RX GT to 20 bits */
			DrpVal |= (3 << 11);
		}
		else {
			/* Set internal Data width of the RX GT to 4-byte */
			DrpVal |= (1 << 14);

			/* Set RX Data width of the RX GT to 40 bits */
			DrpVal |= (5 << 11);
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId,
                        XVPHY_DRP_RXDATAWIDTH, DrpVal);

		/* Set QPLL Lower Band */
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x99, 0x8480);
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x9A, 0x1);

	}

	Status |= XVphy_Gtxe2RxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

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
u32 XVphy_Gtxe2TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
    u32 Status = XST_SUCCESS;

	if (InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) {
        Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId,
                XVPHY_DRP_TXDATAWIDTH, &DrpVal);
        DrpVal &= ~(0x0017);

		if (InstancePtr->Config.TransceiverWidth == 2) {
			/* Set internal Data width of the RX GT to 2-byte */
			DrpVal |= (0 << 4);

			/* Set TX Data width of the TX GT to 20 bits */
			DrpVal |= 3;
		}
		else {
			/* Set internal Data width of the TX GT to 4-byte */
			DrpVal |= (1 << 4);

			/* Set TX Data width of the TX GT to 40 bits */
			DrpVal |= 5;
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId,
                        XVPHY_DRP_TXDATAWIDTH, DrpVal);
	}
    Status |= XVphy_Gtxe2TxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

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
u32 XVphy_Gtxe2TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
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

	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, XVPHY_DRP_TXCLK25, &DrpVal);
	DrpVal &= ~(0x1F);
	DrpVal |= XVphy_DrpEncodeClk25(TxRefClkHz);
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
u32 XVphy_Gtxe2RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
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

	Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId, XVPHY_DRP_RXDATAWIDTH,
                    &DrpVal);
	DrpVal &= ~(0x07C0);
	DrpVal |= XVphy_DrpEncodeClk25(RxRefClkHz) << 6;
	Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_DRP_RXDATAWIDTH,
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

	switch (AttrEncode) {
	case 16:
		DrpEncode = 0x020;
		break;
	case 20:
		DrpEncode = 0x030;
		break;
	case 32:
		DrpEncode = 0x060;
		break;
	case 40:
		DrpEncode = 0x080;
		break;
	case 64:
		DrpEncode = 0x0E0;
		break;
	case 66:
		DrpEncode = 0x140;
		break;
	case 80:
		DrpEncode = 0x120;
		break;
	case 100:
		DrpEncode = 0x170;
		break;
	default:
		DrpEncode = 0x3FF;
		break;
	}

	return DrpEncode;
}

static u16 XVphy_DrpEncodeClk25(u32 RefClkFreqHz)
{
	u16 DrpEncode;
	u32 RefClkFreqMHz = RefClkFreqHz / 1000000;

	DrpEncode = ((RefClkFreqMHz / 25) +
			(((RefClkFreqMHz % 25) > 0) ? 1 : 0)) - 1;

	return (DrpEncode & 0x1F);
}
#endif

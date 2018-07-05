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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xvphy_gtpe2.c
 *
 * Contains a minimal set of functions for the XVphy driver that allow access
 * to all of the Video PHY core's functionality. See xvphy.h for a detailed
 * description of the driver.
 *
 * @note    None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * 1.1   gm   03/18/16 Added XVphy_Gtpe2RxPllRefClkDiv1Reconfig function
 * 1.2   gm   08/26/16 Suppressed warning messages due to unused arguments
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
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static u8 XVphy_MToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
static u16 XVphy_NToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 NId);
static u8 XVphy_DToDrpEncoding(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);
static u8 XVphy_DrpEncodePllMN2(u8 AttrEncode);
static u8 XVphy_DrpEncodePllN1(u8 AttrEncode);
static u8 XVphy_DrpEncodeCpllTxRxD(u8 AttrEncode);
static u16 XVphy_DrpEncodeClk25(u32 RefClkFreqHz);

u32 XVphy_Gtpe2CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_Gtpe2CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XVphy_Gtpe2OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);
u32 XVphy_Gtpe2ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtpe2ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId);
u32 XVphy_Gtpe2TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtpe2RxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtpe2TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
u32 XVphy_Gtpe2RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
#define XVPHY_DRP_RXCDR_CFG(n)		(0xA8 + n)

#define XVPHY_DRP_RXDATAWIDTH		0x11
#define XVPHY_DRP_OUT_DIV_PROG		0x88
#define XVPHY_DRP_TXCLK25		    0x6A
#define XVPHY_DRP_TXDATAWIDTH	    0x6B
#define XVPHY_DRP_RXCDR_CFG_WORD0	0xA8
#define XVPHY_DRP_RXCDR_CFG_WORD1	0xA9
#define XVPHY_DRP_RXCDR_CFG_WORD2	0xAA
#define XVPHY_DRP_RXCDR_CFG_WORD3	0xAB
#define XVPHY_DRP_RXCDR_CFG_WORD4	0xAC

/* PLL operating ranges. */
#define XVPHY_CPLL_MIN		1600000000LL
#define XVPHY_CPLL_MAX		3300000000LL

const u8 Gtpe2CpllDivsM[]	= {1, 2, 0};
const u8 Gtpe2CpllDivsN1[]	= {4, 5, 0};
const u8 Gtpe2CpllDivsN2[]	= {1, 2, 3, 4, 5, 0};
const u8 Gtpe2CpllDivsD[]	= {1, 2, 4, 8, 0};

const XVphy_GtConfig Gtpe2Config = {
	.CfgSetCdr = XVphy_Gtpe2CfgSetCdr,
	.CheckPllOpRange = XVphy_Gtpe2CheckPllOpRange,
	.OutDivChReconfig = XVphy_Gtpe2OutDivChReconfig,
	.ClkChReconfig = XVphy_Gtpe2ClkChReconfig,
	.ClkCmnReconfig = XVphy_Gtpe2ClkCmnReconfig,
	.RxChReconfig = XVphy_Gtpe2RxChReconfig,
	.TxChReconfig = XVphy_Gtpe2TxChReconfig,

	.CpllDivs = {
		.M = Gtpe2CpllDivsM,
		.N1 = Gtpe2CpllDivsN1,
		.N2 = Gtpe2CpllDivsN2,
		.D = Gtpe2CpllDivsD,
	},
	.QpllDivs = {
		.M = Gtpe2CpllDivsM,
		.N1 = Gtpe2CpllDivsN1,
		.N2 = Gtpe2CpllDivsN2,
		.D = Gtpe2CpllDivsD,
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
u32 XVphy_Gtpe2CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr, *CmnPtr;
	XVphy_ChannelId CmnId;
	XVphy_PllType PllType;
	u32 PllClkInFreqHz;

	/* Set CDR values only for CPLLs. */
	if ((ChId < XVPHY_CHANNEL_ID_CH1) || (ChId > XVPHY_CHANNEL_ID_CH4)) {
		return XST_FAILURE;
	}

	ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)];

	ChPtr->PllParams.Cdr[0] = 0x1010;
	ChPtr->PllParams.Cdr[1] = 0x2104;
	ChPtr->PllParams.Cdr[3] = 0x07FE;

	if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP) {
		PllType = XVphy_GetPllType(InstancePtr, QuadId, XVPHY_DIR_RX, ChId);
		CmnId = XVphy_GetRcfgChId(InstancePtr, QuadId, XVPHY_DIR_RX, PllType);
		CmnPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)];
		PllClkInFreqHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
					CmnPtr->CpllRefClkSel);

		if (PllClkInFreqHz == 270000000) {
		    ChPtr->PllParams.Cdr[2] = 0x4060;
			ChPtr->PllParams.Cdr[4] = 0x0001;
		}
		else if (PllClkInFreqHz == 135000000) {
		    ChPtr->PllParams.Cdr[2] = 0x2060;
			ChPtr->PllParams.Cdr[4] = 0x0011;
		}
		/* RBR does not use DP159 forwarded clock and expects 162MHz. */
		else {
		    ChPtr->PllParams.Cdr[2] = 0x2060;
			ChPtr->PllParams.Cdr[4] = 0x0001;
		}
	}

	else if (XVphy_IsHDMI(InstancePtr, XVPHY_DIR_RX)) {

		ChPtr->PllParams.Cdr[4] = 0x0001;

		/* Update the RXCDR_CFG2 settings. */
		switch (ChPtr->RxOutDiv) {
			case 1:
				ChPtr->PllParams.Cdr[2] = 0x4060;
				break;
			case 2:
				ChPtr->PllParams.Cdr[2] = 0x2060;
				break;
			case 4:
				ChPtr->PllParams.Cdr[2] = 0x1060;
				break;
			case 8:
				ChPtr->PllParams.Cdr[2] = 0x0860;
				break;
			default:
				ChPtr->PllParams.Cdr[2] = 0x1060;
				break;
		}
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
u32 XVphy_Gtpe2CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u64 PllClkOutFreqHz)
{
	u32 Status = XST_FAILURE;

	/* Suppress Warning Messages */
	InstancePtr = InstancePtr;
	QuadId = QuadId;
	ChId = ChId;

	if ((XVPHY_CPLL_MIN <= PllClkOutFreqHz) &&
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
u32 XVphy_Gtpe2OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
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
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Gtpe2ClkChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	/* Suppress Warning Messages */
	InstancePtr = InstancePtr;
	QuadId = QuadId;
	ChId = ChId;

	/* GTPE2 doesn't have channel PLL. */
	return XST_FAILURE;
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
u32 XVphy_Gtpe2ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId CmnId)
{
	u16 DrpVal;
	u16 WriteVal;
    u32 Status = XST_SUCCESS;

	/* Obtain current DRP register value for PLL dividers. */
	Status |= XVphy_DrpRd(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x04 : 0x2B, &DrpVal);
	/* Mask out clock divider bits. */
	DrpVal &= ~(0x1FFF);
	/* Set CPLL_FBDIV. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, CmnId, 2) & 0x3F);
	DrpVal |= WriteVal;
	/* Set CPLL_FBDIV_45. */
	WriteVal = (XVphy_NToDrpEncoding(InstancePtr, QuadId, CmnId, 1) & 0x1);
	DrpVal |= (WriteVal << 7);
	/* Set CPLL_REFCLKDIV. */
	WriteVal = (XVphy_MToDrpEncoding(InstancePtr, QuadId, CmnId) & 0x1F);
	DrpVal |= (WriteVal << 9);
	/* Write new DRP register value for PLL dividers. */
	Status |= XVphy_DrpWr(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN,
			(CmnId == XVPHY_CHANNEL_ID_CMN0) ? 0x04 : 0x2B, DrpVal);

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
u32 XVphy_Gtpe2RxChReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
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

	if ((XVphy_IsHDMI(InstancePtr, XVPHY_DIR_RX)) &&
	    (InstancePtr->HdmiRxDruIsEnabled)) {
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x2A,0x0);
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x2B,0x0);

		/* Set RX_DATA_WIDTH to 20 bits */
		Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId,
				XVPHY_DRP_RXDATAWIDTH, &DrpVal);
		DrpVal &= ~0x3800;
		DrpVal |= (3 << 11);
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId,
                XVPHY_DRP_RXDATAWIDTH, DrpVal);
	}

	Status |= XVphy_Gtpe2RxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

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
u32 XVphy_Gtpe2TxChReconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
    u32 Status = XST_SUCCESS;

	if (XVphy_IsHDMI(InstancePtr, XVPHY_DIR_TX)) {
        Status |= XVphy_DrpRd(InstancePtr, QuadId, ChId,
                XVPHY_DRP_TXDATAWIDTH, &DrpVal);
        DrpVal &= ~(0x07);

		if (InstancePtr->Config.TransceiverWidth == 2) {
			/* Set TX Data width of the TX GT to 20 bits */
			DrpVal |= 3;
		}
		else {
			/* Set TX Data width of the TX GT to 40 bits */
			DrpVal |= 5;
		}
		Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId,
                    XVPHY_DRP_TXDATAWIDTH, DrpVal);
	}
    Status |= XVphy_Gtpe2TxPllRefClkDiv1Reconfig(InstancePtr, QuadId, ChId);

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
u32 XVphy_Gtpe2TxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u32 TxRefClkHz;
	XVphy_Channel *CmnPtr;
	XVphy_ChannelId CmnId;
	XVphy_PllType PllType;
    u32 Status = XST_SUCCESS;

	if (XVphy_IsHDMI(InstancePtr, XVPHY_DIR_TX)) {
		TxRefClkHz = InstancePtr->HdmiTxRefClkHz;
	}
	else {
		PllType = XVphy_GetPllType(InstancePtr, QuadId, XVPHY_DIR_TX, ChId);
		CmnId = XVphy_GetRcfgChId(InstancePtr, QuadId, XVPHY_DIR_TX, PllType);
		CmnPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)];
		TxRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
								CmnPtr->PllRefClkSel);
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
u32 XVphy_Gtpe2RxPllRefClkDiv1Reconfig(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u16 DrpVal;
	u32 RxRefClkHz;
	XVphy_Channel *CmnPtr;
	XVphy_ChannelId CmnId;
	XVphy_PllType PllType;
    u32 Status = XST_SUCCESS;

	if (XVphy_IsHDMI(InstancePtr, XVPHY_DIR_RX)) {
		RxRefClkHz = InstancePtr->HdmiRxRefClkHz;
	}
	else {
		PllType = XVphy_GetPllType(InstancePtr, QuadId, XVPHY_DIR_RX, ChId);
		CmnId = XVphy_GetRcfgChId(InstancePtr, QuadId, XVPHY_DIR_RX, PllType);
		CmnPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)];
		RxRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
								CmnPtr->PllRefClkSel);
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

	if ((ChId == XVPHY_CHANNEL_ID_CMN0) ||
			(ChId == XVPHY_CHANNEL_ID_CMN1)) {
		MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)]
				.PllParams.MRefClkDiv;
	}
	else {
		MRefClkDiv = 0;
	}

	DrpEncode = XVphy_DrpEncodePllMN2(MRefClkDiv);

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

	if (NId == 1) {
		NFbDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
				PllParams.N1FbDiv;
		DrpEncode = XVphy_DrpEncodePllN1(NFbDiv);
	}
	else {
		NFbDiv = InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
			PllParams.N2FbDiv;
		DrpEncode = XVphy_DrpEncodePllMN2(NFbDiv);
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
static u8 XVphy_DrpEncodePllMN2(u8 AttrEncode)
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
* This function will translate the configured PLL's N1 value to DRP encoding.
*
* @param	AttrEncode is the attribute to encode.
*
* @return	The DRP encoding for the PLL's N1 value.
*
* @note		None.
*
******************************************************************************/
static u8 XVphy_DrpEncodePllN1(u8 AttrEncode)
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

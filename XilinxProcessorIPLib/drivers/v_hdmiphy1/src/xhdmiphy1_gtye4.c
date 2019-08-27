/*******************************************************************************
 *
 * Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
 * @file xhdmiphy1_gtye4.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note    None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xhdmiphy1_gt.h"
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static u8 XHdmiphy1_MToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
static u16 XHdmiphy1_NToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 NId);
static u8 XHdmiphy1_DToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
static u8 XHdmiphy1_DrpEncodeQpllMCpllMN2(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeCpllN1(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeCpllTxRxD(u8 AttrEncode);
static u16 XHdmiphy1_DrpEncodeQpllN(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeDataWidth(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeIntDataWidth(u8 AttrEncode);
static u16 XHdmiphy1_DrpEncodeClk25(u32 RefClkFreqHz);

u32 XHdmiphy1_Gtye4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XHdmiphy1_Gtye4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_Gtye4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId);
u32 XHdmiphy1_Gtye4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
#define XHDMIPHY1_DRP_RXCDR_CFG(n)       (0x0E + n)
#define XHDMIPHY1_DRP_RXCDR_CFG_GEN3(n)  (0xA2 + n)
#define XHDMIPHY1_DRP_RXCDR_CFG2_GEN2    0x135

#define XHDMIPHY1_DRP_CPLL_FBDIV         0x28
#define XHDMIPHY1_DRP_CPLL_REFCLK_DIV    0x2A
#define XHDMIPHY1_DRP_RXOUT_DIV          0x63
#define XHDMIPHY1_DRP_RXCLK25            0x6D
#define XHDMIPHY1_DRP_TXCLK25            0x7A
#define XHDMIPHY1_DRP_TXOUT_DIV          0x7C
#define XHDMIPHY1_DRP_QPLL1_FBDIV        0x94
#define XHDMIPHY1_DRP_QPLL1_REFCLK_DIV   0x98
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD0    0x0E
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD1    0x0F
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD2    0x10
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD3    0x11
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD4    0x12

/* PLL operating ranges. */
#define XHDMIPHY1_QPLL0_MIN       9800000000LL
#define XHDMIPHY1_QPLL0_MAX      16375000000LL
#define XHDMIPHY1_QPLL1_MIN       8000000000LL
#define XHDMIPHY1_QPLL1_MAX      13000000000LL
#define XHDMIPHY1_CPLL_MIN        2000000000LL
#define XHDMIPHY1_CPLL_MAX        6250000000LL

const u8 Gtye4CpllDivsM[]   = {1, 2, 0};
const u8 Gtye4CpllDivsN1[]  = {4, 5, 0};
const u8 Gtye4CpllDivsN2[]  = {1, 2, 3, 4, 5, 8, 0};
const u8 Gtye4CpllDivsD[]   = {1, 2, 4, 8, 0};

const u8 Gtye4QpllDivsM[]   = {4, 3, 2, 1, 0};
const u8 Gtye4QpllDivsN1[]  = {16, 20, 32, 40, 60, 64, 66, 75, 80, 84, 90,
                   96, 100, 112, 120, 125, 150, 160, 0};
const u8 Gtye4QpllDivsN2[]  = {1, 0};
const u8 Gtye4QpllDivsD[]   = {32, 16, 8, 4, 2, 1, 0};

const XHdmiphy1_GtConfig Gtye4Config = {
    .CfgSetCdr = XHdmiphy1_Gtye4CfgSetCdr,
    .CheckPllOpRange = XHdmiphy1_Gtye4CheckPllOpRange,
    .OutDivChReconfig = XHdmiphy1_Gtye4OutDivChReconfig,
    .ClkChReconfig = XHdmiphy1_Gtye4ClkChReconfig,
    .ClkCmnReconfig = XHdmiphy1_Gtye4ClkCmnReconfig,
    .RxChReconfig = XHdmiphy1_Gtye4RxChReconfig,
    .TxChReconfig = XHdmiphy1_Gtye4TxChReconfig,

    .CpllDivs = {
        .M = Gtye4CpllDivsM,
        .N1 = Gtye4CpllDivsN1,
        .N2 = Gtye4CpllDivsN2,
        .D = Gtye4CpllDivsD,
    },
    .QpllDivs = {
        .M = Gtye4QpllDivsM,
        .N1 = Gtye4QpllDivsN1,
        .N2 = Gtye4QpllDivsN2,
        .D = Gtye4QpllDivsD,
    },
};

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will set the clock and data recovery (CDR) values for a given
* channel.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u32 Status = XST_SUCCESS;

    /* Set CDR values only for CPLLs. */
    if ((ChId < XHDMIPHY1_CHANNEL_ID_CH1) ||
        (ChId > XHDMIPHY1_CHANNEL_ID_CH4)) {
        return XST_FAILURE;
    }

    ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];

    ChPtr->PllParams.Cdr[0] = 0x0000;
    ChPtr->PllParams.Cdr[1] = 0x0000;
    ChPtr->PllParams.Cdr[3] = 0x0000;
    ChPtr->PllParams.Cdr[4] = 0x0000;
    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        /* RxOutDiv = 1  => Cdr[2] = 0x0269
         * RxOutDiv = 2  => Cdr[2] = 0x0259
         * RxOutDiv = 4  => Cdr[2] = 0x0249
         * RxOutDiv = 8  => Cdr[2] = 0x0239
         * RxOutDiv = 16 => Cdr[2] = 0x0229 */
        ChPtr->PllParams.Cdr[2] = 0x0269;
        while (ChPtr->RxOutDiv >>= 1) {
            ChPtr->PllParams.Cdr[2] -= 0x10;
        }
        /* Restore RxOutDiv. */
        ChPtr->RxOutDiv = 1 << ((0x0269 - ChPtr->PllParams.Cdr[2]) >> 4);
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
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    PllClkOutFreqHz is the frequency to check.
*
* @return
*       - XST_SUCCESS if the frequency resides within the PLL's range.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz)
{
    u32 Status = XST_FAILURE;

    /* Suppress Warning Messages */
    InstancePtr = InstancePtr;
    QuadId = QuadId;

    if (((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) &&
            (XHDMIPHY1_QPLL0_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_QPLL0_MAX)) ||
        ((ChId == XHDMIPHY1_CHANNEL_ID_CMN1) &&
            (XHDMIPHY1_QPLL1_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_QPLL1_MAX)) ||
        ((ChId >= XHDMIPHY1_CHANNEL_ID_CH1) &&
            (ChId <= XHDMIPHY1_CHANNEL_ID_CH4) &&
            (XHDMIPHY1_CPLL_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_CPLL_MAX))) {
        Status = XST_SUCCESS;
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will set the output divider logic for a given channel.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    Dir is an indicator for RX or TX.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 Status = XST_SUCCESS;

    if (Dir == XHDMIPHY1_DIR_RX) {
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x63, &DrpVal);
        /* Mask out RX_OUT_DIV. */
        DrpVal &= ~0x07;
        /* Set RX_OUT_DIV. */
        WriteVal = (XHdmiphy1_DToDrpEncoding(InstancePtr, QuadId, ChId,
                        XHDMIPHY1_DIR_RX) & 0x7);
        DrpVal |= WriteVal;
        /* Write new DRP register value for RX dividers. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x63, DrpVal);
    }
    else {
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x7C, &DrpVal);
        /* Mask out TX_OUT_DIV. */
        DrpVal &= ~0x700;
        /* Set TX_OUT_DIV. */
        WriteVal = (XHdmiphy1_DToDrpEncoding(InstancePtr, QuadId, ChId,
                        XHDMIPHY1_DIR_TX) & 0x7);
        DrpVal |= (WriteVal << 8);
        /* Write new DRP register value for RX dividers. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x7C, DrpVal);
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel clock settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u16 WriteVal;
    u16 CPLL_CFG0, CPLL_CFG1, CPLL_CFG2;
    u32 PllxVcoRateMHz;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u32 QPllxClkOutMHz;
    u32 Status = XST_SUCCESS;

    /* Obtain current DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x28, &DrpVal);
    /* Mask out clock divider bits. */
    DrpVal &= ~(0xFF80);
    /* Set CPLL_FBDIV. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, ChId, 2) & 0xFF);
    DrpVal |= (WriteVal << 8);
    /* Set CPLL_FBDIV_45. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, ChId, 1) & 0x1);
    DrpVal |= (WriteVal << 7);
    /* Write new DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x28, DrpVal);

    /* Write CPLL Ref Clk Div. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x2A, &DrpVal);
    /* Mask out clock divider bits. */
    DrpVal &= ~(0xF800);
    /* Set CPLL_REFCLK_DIV. */
    WriteVal = (XHdmiphy1_MToDrpEncoding(InstancePtr, QuadId, ChId) & 0x1F);
    DrpVal |= (WriteVal << 11);
    /* Write new DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x2A, DrpVal);

    /* Configure CPLL Calibration Registers */
    XHdmiphy1_CfgCpllCalPeriodandTol(InstancePtr, QuadId, ChId,
            (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
                XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX),
            InstancePtr->Config.DrpClkFreq);

    PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChId,
					XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
							XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX) / 1000000;
    /* CPLL VCO = Pll ClkOut */
    PllxClkOutMHz = PllxVcoRateMHz;

    /* CPLL_CFGx */
    if (PllxVcoRateMHz <= 3000) {
	CPLL_CFG0 = 0x01FA;
	CPLL_CFG1 = 0x002B;
	CPLL_CFG2 = 0x0002;
    }
    else if (PllxVcoRateMHz <= 4250) {
	CPLL_CFG0 = 0x0FFA;
	CPLL_CFG1 = 0x0029;
	CPLL_CFG2 = 0x0202;
    }
    else {
	CPLL_CFG0 = 0x03FE;
	CPLL_CFG1 = 0x0029;
	CPLL_CFG2 = 0x0203;
    }
    /* Write new DRP register value for CPLL_CFGx. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x9F, CPLL_CFG0);
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xA0, CPLL_CFG1);
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xBC, CPLL_CFG2);

    /* RTX_BUF_CML_CTRL */
    /* Determine which QPLL (0/1) is active and calculate the PllClkOut */
	/* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId,
			(XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, ChId) ?
					XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX), ChId);

    /* Determine which channel(s) to operate on. */
    switch (PllType) {
        case XHDMIPHY1_PLL_TYPE_QPLL:
        case XHDMIPHY1_PLL_TYPE_QPLL0:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN0;
            PllxClkOutDiv = 2;
            break;
        case XHDMIPHY1_PLL_TYPE_QPLL1:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN1;
            PllxClkOutDiv = 2;
            break;
        default:
            ChIdPll = ChId;
            PllxClkOutDiv = 1;
            break;
    }

    /* Get QPLLx VCO rate */
    PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChIdPll,
						(XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, ChId) ?
					XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX)) / 1000000;
    QPllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;

	/* Update PllxClkOutMHz value */
	PllxClkOutMHz = (PllxClkOutMHz > QPllxClkOutMHz) ?
						PllxClkOutMHz : QPllxClkOutMHz;

	/* Get RTX_BUF_CML_CTRL DRP Reg Value */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xDD, &DrpVal);
    /* Mask out RTX_BUF_CML_CTRL bits */
    DrpVal &= ~0x7;

    if (PllxClkOutMHz < 5500) {
	DrpVal |= 0x3;
    }
    else if (PllxClkOutMHz < 7500) {
	DrpVal |= 0x4;
    }
    else if (PllxClkOutMHz < 9500) {
	DrpVal |= 0x5;
    }
    else if (PllxClkOutMHz < 12500) {
	DrpVal |= 0x6;
    }
    else {
	DrpVal |= 0x7;
    }
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xDD, DrpVal);


    return Status;
}

/*****************************************************************************/
/**
* This function will configure the common channel clock settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    CmnId is the common channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId)
{
    u16 DrpVal;
    u16 WriteVal;
    u8  QpllxFbdiv;
    u32 QpllxVcoRateMHz;
    u32 QpllxClkOutMHz;
    u32 Status = XST_SUCCESS;
    u16 QPLLx_CFG0;
    u16 QPLLx_CFG23;
    u16 QPLLx_LPF;

    /* Obtain current DRP register value for QPLLx_FBDIV. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x14 : 0x94, &DrpVal);
    /* Mask out QPLLx_FBDIV. */
    DrpVal &= ~(0xFF);
    /* Set QPLLx_FBDIV. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId,
                    CmnId, 0) & 0xFF);
    DrpVal |= WriteVal;
    /* Write new DRP register value for QPLLx_FBDIV. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x14 : 0x94, DrpVal);

    /* Obtain current DRP register value for QPLLx_REFCLK_DIV. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x18 : 0x98, &DrpVal);
    /* Mask out QPLLx_REFCLK_DIV. */
    DrpVal &= ~(0xF80);
    /* Disable Intelligent Reference Clock Selection */
    if (XHdmiphy1_GetRefClkSourcesCount(InstancePtr) > 1) {
        DrpVal |= (1 << 6);
    }
    /* Set QPLLx_REFCLK_DIV. */
    WriteVal = (XHdmiphy1_MToDrpEncoding(InstancePtr, QuadId, CmnId) & 0x1F);
    DrpVal |= (WriteVal << 7);
    /* Write new DRP register value for QPLLx_REFCLK_DIV. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x18 : 0x98, DrpVal);

    if ((XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) ||
        (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX))) {

        QpllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, CmnId,
                        XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, CmnId) ?
                        XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX) / 1000000;
        QpllxClkOutMHz = QpllxVcoRateMHz / 2;

        /* PPFx_CFG */
        if (QpllxVcoRateMHz >= 16375) {
            DrpVal = 0x0F00;
        }
        else if (QpllxVcoRateMHz >= 14000) {
            DrpVal = 0x0B00;
        }
        else if (QpllxVcoRateMHz >= 13000) {
            DrpVal = 0x0900;
        }
        else if (QpllxVcoRateMHz >= 12500) {
            DrpVal = 0x0800;
        }
        else if (QpllxVcoRateMHz >= 10312) {
            DrpVal = 0x0600;
        }
        else {
            DrpVal = 0x0400;
        }
        /* Write new DRP register value for PPFx_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x0D : 0x8D,
                    DrpVal);

        /* QPLLx_CFG2 & QPLLx_CFG3*/
        QpllxFbdiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                            PllParams.NFbDiv;
        QPLLx_CFG0 = 0x331C;
        if (QpllxFbdiv <= 30) {
            if (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) {
                if (QpllxVcoRateMHz <= 11000) {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x27F;
                }
                else if (QpllxVcoRateMHz <= 14000) {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x37F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x2FF;
                    QPLLx_CFG0  = 0x333C;
                }
            }
            else {
                QPLLx_CFG23 = 0x0FC1;
                QPLLx_LPF   = 0x2FF;
            }
        }
        else if (QpllxFbdiv <= 50) {
            if (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) {
                if (QpllxVcoRateMHz <= 14000) {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x37F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x31D;
                    QPLLx_CFG0  = 0x333C;
                }
            }
            else {
                QPLLx_CFG23 = 0x0FC1;
                if (QpllxVcoRateMHz <= 11500) {
                    QPLLx_LPF   = 0x33F;
                }
                else {
                    QPLLx_LPF   = 0x37F;
                }
            }
        }
        else if (QpllxFbdiv <= 70) {
            if (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) {
                if (QpllxVcoRateMHz <= 11000) {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x23F;
                }
                else if (QpllxVcoRateMHz <= 14000) {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x33F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x29D;
                    QPLLx_CFG0  = 0x333C;
                }
            }
            else {
                if (QpllxVcoRateMHz <= 11500) {
                    QPLLx_CFG23 = 0x0FC3;
                    QPLLx_LPF   = 0x21F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x33F;
                }
            }
        }
        else if (QpllxFbdiv <= 90) {
            if (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) {
                if (QpllxVcoRateMHz <= 14000) {
                    QPLLx_CFG23 = 0x0FC3;
                    QPLLx_LPF   = 0x21F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x37F;
                }
            }
            else {
                if (QpllxVcoRateMHz <= 9000) {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x21F;
                }
                else if (QpllxVcoRateMHz <= 11500) {
                    QPLLx_CFG23 = 0x0FC3;
                    QPLLx_LPF   = 0x21F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x33F;
                }
            }
        }
        else {
            if (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) {
                if (QpllxVcoRateMHz <= 11000) {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x21F;
                }
                else if (QpllxVcoRateMHz <= 14000) {
                    QPLLx_CFG23 = 0x0FC3;
                    QPLLx_LPF   = 0x21F;
                }
                else {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x37F;
                }
            }
            else {
                if (QpllxVcoRateMHz <= 9000) {
                    QPLLx_CFG23 = 0x0FC3;
                    QPLLx_LPF   = 0x21D;
                }
                else if (QpllxVcoRateMHz <= 11500) {
                    QPLLx_CFG23 = 0x0FC1;
                    QPLLx_LPF   = 0x21D;
                }
                else {
                    QPLLx_CFG23 = 0x0FC0;
                    QPLLx_LPF   = 0x33F;
                }
            }
        }
        /* Write new DRP register value for QPLLx_CFG0. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x08 : 0x88,
                    QPLLx_CFG0);
        /* Write new DRP register value for QPLLx_CFG2. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x11 : 0x91,
                    QPLLx_CFG23);
        /* Write new DRP register value for QPLLx_CFG2_G3. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x1B : 0x9B,
                    QPLLx_CFG23);
        /* Write new DRP register value for QPLLx_LPF. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x19 : 0x99,
                    QPLLx_LPF);

        /* QPLLx_CFG4 */
        if (QpllxClkOutMHz >= 14000) {
            DrpVal = 0x0086;
        }
        else if (QpllxClkOutMHz >= 12500) {
            DrpVal = 0x0084;
        }
        else if (QpllxClkOutMHz >= 8187) {
            DrpVal = 0x0003;
        }
        else if (QpllxClkOutMHz >= 5156) {
            DrpVal = 0x0002;
        }
        else {
            DrpVal = 0x0001;
        }
        /* Write new DRP register value for QPLLx_CFG4. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x30 : 0xB0,
                    DrpVal);
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u16 DrpVal, DrpVal2;
    u16 WriteVal;
    u8 CfgIndex;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxVcoRateMHz;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u32 Status = XST_SUCCESS;
	u64 LineRateHz;

    ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];

    /* RXCDR_CFG(CfgIndex) */
    for (CfgIndex = 0; CfgIndex < 5; CfgIndex++) {
        DrpVal = ChPtr->PllParams.Cdr[CfgIndex];
        if (!DrpVal) {
            /* Don't modify RX_CDR configuration. */
            continue;
        }
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId,
                    XHDMIPHY1_DRP_RXCDR_CFG(CfgIndex), DrpVal);
        if (CfgIndex == 2) {
            Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId,
                        XHDMIPHY1_DRP_RXCDR_CFG_GEN3(CfgIndex), DrpVal);
        /* RXCDR_CFG2_GEN2 */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x135, &DrpVal);
        DrpVal &= ~(0x3FF);
        DrpVal |= ChPtr->PllParams.Cdr[CfgIndex] & 0x3FF;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x135, DrpVal);
        }
    }

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		LineRateHz = XHdmiphy1_GetLineRateHz(InstancePtr, QuadId, ChId);
        /* ADAPT_CFG1 */
		if(LineRateHz <= 8000000000) {
			DrpVal = 0xF81C;
		} else {
			DrpVal = 0xFB1C;
		}
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x91, DrpVal);

        /* RX_XMODE_SEL */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xD3, &DrpVal);
        DrpVal &= ~(0x2);
		if(LineRateHz <= 10312500000) {
			DrpVal |= 0x1 << 1;
		} else {
			DrpVal |= 0x0;
		}
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xD3, DrpVal);

        /* RX_WIDEMODE_CDR Encoding */
        switch (ChPtr->RxDataWidth) {
        case 80:
		WriteVal = 0x2 << 2;
            break;
        case 64:
		if(LineRateHz > 16375000000) {
			WriteVal = 0x2 << 2;
		} else {
			WriteVal = 0x1 << 2;
		}
            break;
        case 40:
		if(LineRateHz > 10312500000) {
			WriteVal = 0x1 << 2;
		} else {
			WriteVal = 0x0;
		}
            break;
        case 32:
		if(LineRateHz > 8000000000) {
			WriteVal = 0x1 << 2;
		} else {
			WriteVal = 0x0;
		}
            break;
        default:
		WriteVal = 0x0;
            break;
        }

        /* RX_INT_DATAWIDTH & RX_WIDEMODE_CDR*/
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x66, &DrpVal);
        DrpVal &= ~(0xF);
        /* Update RX_WIDEMODE_CDR Value */
        DrpVal |= WriteVal & 0xC;
        WriteVal = (XHdmiphy1_DrpEncodeIntDataWidth(ChPtr->RxIntDataWidth) &
                        0x3);
        /* Update RX_INT_DATAWIDTH Value*/
        DrpVal |= WriteVal & 0x3;

        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x66, DrpVal);

        /* RX_DATA_WIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x03, &DrpVal);
        DrpVal &= ~(0x1E0);
        WriteVal = (XHdmiphy1_DrpEncodeDataWidth(ChPtr->RxDataWidth) & 0xF);
        WriteVal <<= 5;
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x03, DrpVal);

		/* Determine PLL type. */
        PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX,
                    ChId);
        /* Determine which channel(s) to operate on. */
        switch (PllType) {
            case XHDMIPHY1_PLL_TYPE_QPLL:
            case XHDMIPHY1_PLL_TYPE_QPLL0:
                ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN0;
                PllxClkOutDiv = 2;
                break;
            case XHDMIPHY1_PLL_TYPE_QPLL1:
                ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN1;
                PllxClkOutDiv = 2;
                break;
            default:
                ChIdPll = ChId;
                PllxClkOutDiv = 1;
                break;
        }
        PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId,
                            ChIdPll, XHDMIPHY1_DIR_RX) / 1000000;
        PllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;

        /* CH_HSPMUX_RX */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x116, &DrpVal);
        DrpVal &= ~(0x00FF);
        if (PllxClkOutMHz >= 14000) {
            DrpVal |= 0xD0;
        }
        else if (PllxClkOutMHz >= 12500) {
            DrpVal |= 0x90;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal |= 0x60;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal |= 0x40;
        }
        else {
            DrpVal |= 0x20;
        }
        /* Write new DRP register value for CH_HSPMUX_RX. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x116, DrpVal);

        /* PREIQ_FREQ_BST */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xFB, &DrpVal);
        DrpVal &= ~(0x0030);
        if (PllxClkOutMHz >= 12500) {
        DrpVal |= 3 << 4;
        }
        else if (PllxClkOutMHz >= 8187) {
        DrpVal |= 2 << 4; /* LPM Mode */
        }
        else if (PllxClkOutMHz >= 5156) {
        DrpVal |= 1 << 4;
        }
        /* Write new DRP register value for PREIQ_FREQ_BST. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xFB, DrpVal);

        /* RXPI_CFG0 */
        if (PllxClkOutMHz > 15250) {
            DrpVal  = 0xB806;
            DrpVal2 = 0xC000;
        }
        else if (PllxClkOutMHz >= 12500) {
            DrpVal  = 0x3006;
            DrpVal2 = 0x0000;
        }
        else if (PllxClkOutMHz >= 10000) {
            DrpVal  = 0x3004;
            DrpVal2 = 0x0000;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal  = 0x0104;
            DrpVal2 = 0x0000;
        }
        else if (PllxClkOutMHz >= 6500) {
            DrpVal  = 0x3002;
            DrpVal2 = 0x0054;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal  = 0x0102;
            DrpVal2 = 0x0054;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal  = 0x0100;
            DrpVal2 = 0x0054;
        }
        else {
            DrpVal  = 0x0301;
            DrpVal2 = 0x00FC;
        }
        /* Write new DRP register value for RXPI_CFG0. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x75, DrpVal);
        /* Write new DRP register value for RXPI_CFG1. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xD2, DrpVal2);

    }

    Status |= XHdmiphy1_Gtye4RxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
                ChId);

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u32 ReturnVal;
    u16 DrpVal;
    u16 WriteVal;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxVcoRateMHz;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u32 Status = XST_SUCCESS;

    ReturnVal = XHdmiphy1_Gtye4TxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
                    ChId);
    if (!XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
        return ReturnVal;
    }

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX,
                ChId);
    /* Determine which channel(s) to operate on. */
    switch (PllType) {
        case XHDMIPHY1_PLL_TYPE_QPLL:
        case XHDMIPHY1_PLL_TYPE_QPLL0:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN0;
            PllxClkOutDiv = 2;
            break;
        case XHDMIPHY1_PLL_TYPE_QPLL1:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN1;
            PllxClkOutDiv = 2;
            break;
        default:
            ChIdPll = ChId;
            PllxClkOutDiv = 1;
            break;
    }

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {

        ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];

        /* Set TX_PROGDIV_CFG to 20 */
        if ((PllType == XHDMIPHY1_PLL_TYPE_QPLL) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL0) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL1)) {
            if (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].TxOutDiv != 16) {
                /* TX_PROGDIV_CFG = 20 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x57, 57442);
            } else {
                /* TX_PROGDIV_CFG = 40 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x57, 57415);
            }
        }

        /* TX_INT_DATAWIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x85, &DrpVal);
        DrpVal &= ~(0x3 << 10);
        WriteVal = ((XHdmiphy1_DrpEncodeIntDataWidth(ChPtr->
                        TxIntDataWidth) & 0x3) << 10);
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x85, DrpVal);

        /* TX_DATA_WIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x7A, &DrpVal);
        DrpVal &= ~(0xF);
        WriteVal = (XHdmiphy1_DrpEncodeDataWidth(ChPtr->TxDataWidth) & 0xF);
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x7A, DrpVal);

        PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId,
                            ChIdPll, XHDMIPHY1_DIR_TX) / 1000000;
        PllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;

        /* TXPH_CFG */
        if ((ChPtr->TxIntDataWidth <= 40 &&
                ChPtr->OutDiv[XHDMIPHY1_DIR_TX] == 1) ||
            (ChPtr->TxIntDataWidth <= 20 &&
                ChPtr->OutDiv[XHDMIPHY1_DIR_TX] == 2)) {
            DrpVal = 0x0323;
        }
        else {
            DrpVal = 0x0723;
        }
        /* Write new DRP register value for TXPH_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x73, DrpVal);

        /* TXPI_CFG0 */
        if (PllxClkOutMHz >= 15250) {
            DrpVal = 0xB8B0;
        }
        else if (PllxClkOutMHz >= 10000) {
            DrpVal = 0x3000;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal = 0x0100;
        }
        else if (PllxClkOutMHz >= 6500) {
            DrpVal = 0x3100;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal = 0x0100;
        }
        else {
            DrpVal = 0x0300;
        }
        /* Write new DRP register value for TXPI_CFG0. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xA7, DrpVal);

        /* TXPI_CFG1 */
        if (PllxClkOutMHz >= 10000) {
            DrpVal = 0x0000;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal = 0x1000;
        }
        else if (PllxClkOutMHz >= 6500) {
            DrpVal = 0x0000;
        }
        else if (PllxClkOutMHz >= 4000) {
            DrpVal = 0x1000;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal = 0x1554;
        }
        else if (PllxClkOutMHz >= 2000) {
            DrpVal = 0x7555;
        }
        else {
            DrpVal = 0x7FFD;
        }
        /* Write new DRP register value for TXPI_CFG1. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xA8, DrpVal);

        /* TX_PI_BIASSET */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xFB, &DrpVal);
        DrpVal &= ~(0x0006);
        if (PllxClkOutMHz >= 12500) {
            DrpVal |= 3 << 1;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal |= 2 << 1;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal |= 1 << 1;
        }
        /* Write new DRP register value for TX_PI_BIASSET. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xFB, DrpVal);

        /* CH_HSPMUX_TX */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x116, &DrpVal);
        DrpVal &= ~(0xFF00);
        if (PllxClkOutMHz >= 14000) {
            DrpVal |= 0xD000;
        }
        else if (PllxClkOutMHz >= 12500) {
            DrpVal |= 0x9000;
        }
        else if (PllxClkOutMHz >= 8187) {
            DrpVal |= 0x6000;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal |= 0x4000;
        }
        else {
            DrpVal |= 0x2000;
        }
        /* Write new DRP register value for CH_HSPMUX_TX. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x116, DrpVal);
    }
    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX CLKDIV1 settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u32 TxRefClkHz;
    u32 Status = XST_SUCCESS;
    XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)];

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
        TxRefClkHz = InstancePtr->HdmiTxRefClkHz;
    }
    else {
        TxRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr, QuadId,
                                PllPtr->PllRefClkSel);
    }

    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_DRP_TXCLK25,
                    &DrpVal);
    DrpVal &= ~(0xF800);
    DrpVal |= XHdmiphy1_DrpEncodeClk25(TxRefClkHz) << 11;
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_DRP_TXCLK25,
                DrpVal);

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX CLKDIV1 settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u32 RxRefClkHz;
    u32 Status = XST_SUCCESS;
    XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)];

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        RxRefClkHz = InstancePtr->HdmiRxRefClkHz;
    }
    else {
        RxRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr, QuadId,
                                PllPtr->PllRefClkSel);
    }

    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_DRP_RXCLK25,
                    &DrpVal);
    DrpVal &= ~(0x00F8);
    DrpVal |= XHdmiphy1_DrpEncodeClk25(RxRefClkHz) << 3;
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_DRP_RXCLK25,
                    DrpVal);

    return Status;
}

/*****************************************************************************/
/**
* This function will translate the configured M value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return   The DRP encoding for M.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_MToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u8 MRefClkDiv;
    u8 DrpEncode;

    if ((ChId >= XHDMIPHY1_CHANNEL_ID_CH1) &&
        (ChId <= XHDMIPHY1_CHANNEL_ID_CH4)) {
        MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)]
                .PllParams.MRefClkDiv;
    }
    else if ((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) ||
            (ChId == XHDMIPHY1_CHANNEL_ID_CMN1)) {
        MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)]
                .PllParams.MRefClkDiv;
    }
    else {
        MRefClkDiv = 0;
    }

    DrpEncode = XHdmiphy1_DrpEncodeQpllMCpllMN2(MRefClkDiv);

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured D value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    Dir is an indicator for RX or TX.
*
* @return   The DRP encoding for D.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
    u8 OutDiv;
    u8 DrpEncode;

    OutDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
            OutDiv[Dir];

    DrpEncode = XHdmiphy1_DrpEncodeCpllTxRxD(OutDiv);

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured N1/N2 value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    NId specified to operate on N1 (if == 1) or N2 (if == 2).
*
* @return   The DRP encoding for N1/N2.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_NToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 NId)
{
    u8 NFbDiv;
    u16 DrpEncode;

    if ((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) ||
            (ChId == XHDMIPHY1_CHANNEL_ID_CMN1)) {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.NFbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeQpllN(NFbDiv);
    }
    else if (NId == 1) {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.N1FbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeCpllN1(NFbDiv);
    }
    else {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.N2FbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeQpllMCpllMN2(NFbDiv);
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured QPLL's M or CPLL's M or N2
* values to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the QPLL's M or CPLL's M or N2 values.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeQpllMCpllMN2(u8 AttrEncode)
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
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CPLL's N1 value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeCpllN1(u8 AttrEncode)
{
    u8 DrpEncode;

    DrpEncode = (AttrEncode - 4) & 0x1;

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CPLL's D values to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CPLL's D value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeCpllTxRxD(u8 AttrEncode)
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
    case 32:
        DrpEncode = 5;
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
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the QPLL's N value.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_DrpEncodeQpllN(u8 AttrEncode)
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
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the RXDATAWIDTH value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeDataWidth(u8 AttrEncode)
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
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the RXINTDATAWIDTH value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeIntDataWidth(u8 AttrEncode)
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
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CLK25 value.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_DrpEncodeClk25(u32 RefClkFreqHz)
{
    u16 DrpEncode;
    u32 RefClkFreqMHz = RefClkFreqHz / 1000000;

    DrpEncode = ((RefClkFreqMHz / 25) +
                    (((RefClkFreqMHz % 25) > 0) ? 1 : 0)) - 1;

    return (DrpEncode & 0x1F);
}

/*****************************************************************************/
/**
* This function configures the CPLL Calibration period and the count tolerance
* registers.
*
* CpllCalPeriod    = ((fPLLClkin * N1 * N2) / (20 * M)) /
*                       (16000 / (4 * fFreeRunClk))
* CpllCalTolerance = CpllCalPeriod * 0.10
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    Dir is an indicator for TX or RX.
* @param    FreeRunClkFreq is the freerunning clock freq in Hz
*            driving the GT Wiz instance
*
* @return   XST_SUCCESS / XST_FAILURE
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_CfgCpllCalPeriodandTol(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u32 FreeRunClkFreq)
{
    u64 CpllCalPeriod;
    u64 CpllCalTolerance;
    u64 PllVcoFreqHz;
    u32 RegVal;

    /* Check if ChID is not a GT Channel */
    if (!XHDMIPHY1_ISCH(ChId)) {
        return XST_FAILURE;
    }

    PllVcoFreqHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChId, Dir);
    CpllCalPeriod = PllVcoFreqHz * 200 / (u64)FreeRunClkFreq;
    if (CpllCalPeriod % 10) {
        CpllCalTolerance = (CpllCalPeriod / 10) + 1;
    }
    else {
        CpllCalTolerance = CpllCalPeriod / 10;
    }

    /* Read CPLL Calibration Period Value */
    RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
              XHDMIPHY1_CPLL_CAL_PERIOD_REG) & ~XHDMIPHY1_CPLL_CAL_PERIOD_MASK;
    RegVal |= CpllCalPeriod & XHDMIPHY1_CPLL_CAL_PERIOD_MASK;
    /* Write new CPLL Calibration Period Value */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
                    XHDMIPHY1_CPLL_CAL_PERIOD_REG, RegVal);

    /* Read CPLL Calibration Tolerance Value */
    RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_CPLL_CAL_TOL_REG) & ~XHDMIPHY1_CPLL_CAL_TOL_MASK;
    RegVal |= CpllCalTolerance & XHDMIPHY1_CPLL_CAL_TOL_MASK;
    /* Write new CPLL Calibration Tolerance Value */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
                    XHDMIPHY1_CPLL_CAL_TOL_REG, RegVal);

    return XST_SUCCESS;
}
#endif

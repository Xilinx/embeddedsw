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
 * @file xhdmiphy1_gthe4.c
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
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
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

u32 XHdmiphy1_Gthe4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XHdmiphy1_Gthe4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_Gthe4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId);
u32 XHdmiphy1_Gthe4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
#define XHDMIPHY1_DRP_RXCDR_CFG(n)       (0x0E + n)
#define XHDMIPHY1_DRP_RXCDR_CFG_GEN3(n)  (0xA2 + n)
#define XHDMIPHY1_DRP_RXCDR_CFG_GEN4(n)  (0x119 + n)

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

const u8 Gthe4CpllDivsM[]   = {1, 2, 0};
const u8 Gthe4CpllDivsN1[]  = {4, 5, 0};
const u8 Gthe4CpllDivsN2[]  = {1, 2, 3, 4, 5, 8, 0};
const u8 Gthe4CpllDivsD[]   = {1, 2, 4, 8, 0};

const u8 Gthe4QpllDivsM[]   = {4, 3, 2, 1, 0};
const u8 Gthe4QpllDivsN1[]  = {16, 20, 32, 40, 60, 64, 66, 75, 80, 84, 90,
                   96, 100, 112, 120, 125, 150, 160, 0};
const u8 Gthe4QpllDivsN2[]  = {1, 0};
const u8 Gthe4QpllDivsD[]   = {16, 8, 4, 2, 1, 0};

const XHdmiphy1_GtConfig Gthe4Config = {
    .CfgSetCdr = XHdmiphy1_Gthe4CfgSetCdr,
    .CheckPllOpRange = XHdmiphy1_Gthe4CheckPllOpRange,
    .OutDivChReconfig = XHdmiphy1_Gthe4OutDivChReconfig,
    .ClkChReconfig = XHdmiphy1_Gthe4ClkChReconfig,
    .ClkCmnReconfig = XHdmiphy1_Gthe4ClkCmnReconfig,
    .RxChReconfig = XHdmiphy1_Gthe4RxChReconfig,
    .TxChReconfig = XHdmiphy1_Gthe4TxChReconfig,

    .CpllDivs = {
        .M = Gthe4CpllDivsM,
        .N1 = Gthe4CpllDivsN1,
        .N2 = Gthe4CpllDivsN2,
        .D = Gthe4CpllDivsD,
    },
    .QpllDivs = {
        .M = Gthe4QpllDivsM,
        .N1 = Gthe4QpllDivsN1,
        .N2 = Gthe4QpllDivsN2,
        .D = Gthe4QpllDivsD,
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
u32 XHdmiphy1_Gthe4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
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
u32 XHdmiphy1_Gthe4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
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
u32 XHdmiphy1_Gthe4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
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
u32 XHdmiphy1_Gthe4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 CpllxVcoRateMHz;
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
    /* Set CPLL_REFCLKDIV. */
    WriteVal = (XHdmiphy1_MToDrpEncoding(InstancePtr, QuadId, ChId) & 0x1F);
    DrpVal |= (WriteVal << 11);
    /* Write new DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x2A, DrpVal);

    CpllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChId,
                        XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
                            XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX) / 1000000;

    /* CPLL_CFG0 */
    if (CpllxVcoRateMHz <= 3000) {
        DrpVal = 0x01FA;
    }
    else if (CpllxVcoRateMHz <= 4250) {
        DrpVal = 0x0FFA;
    }
    else {
        DrpVal = 0x03FE;
    }
    /* Write new DRP register value for CPLL_CFG0. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xCB, DrpVal);

    /* CPLL_CFG1 */
    if (CpllxVcoRateMHz <= 3000) {
        DrpVal = 0x0023;
    }
    else {
        DrpVal = 0x0021;
    }
    /* Write new DRP register value for CPLL_CFG1. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xCC, DrpVal);

    /* CPLL_CFG2 */
    if (CpllxVcoRateMHz <= 3000) {
        DrpVal = 0x0002;
    }
    else if (CpllxVcoRateMHz <= 4250) {
        DrpVal = 0x0202;
    }
    else {
        DrpVal = 0x0203;
    }
    /* Write new DRP register value for CPLL_CFG2. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xBC, DrpVal);

    /* Configure CPLL Calibration Registers */
    XHdmiphy1_CfgCpllCalPeriodandTol(InstancePtr, QuadId, ChId,
            (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
                XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX),
            InstancePtr->Config.DrpClkFreq);

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
u32 XHdmiphy1_Gthe4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 QpllxVcoRateMHz;
    u32 QpllxClkOutMHz;
    u32 Status = XST_SUCCESS;

    /* Obtain current DRP register value for QPLLx_FBDIV. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x14 : 0x94, &DrpVal);
    /* Mask out QPLLx_FBDIV. */
    DrpVal &= ~(0xFF);
    /* Set QPLLx_FBDIV. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, CmnId, 0) &
                    0xFF);
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
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x0D : 0x8D,
                    &DrpVal);

        DrpVal &= ~(0x0FC0);
        /* PPF_MUX_CRNT_CTRL0 */
        if (QpllxVcoRateMHz >= 15000) {
            DrpVal |= 0x0E00;
        }
        else if (QpllxVcoRateMHz >= 11000) {
            DrpVal |= 0x0800;
        }
        else if (QpllxVcoRateMHz >= 7000) {
            DrpVal |= 0x0600;
        }
        else {
            DrpVal |= 0x0400;
        }
        /* PPF_MUX_TERM_CTRL0 */
        if (QpllxVcoRateMHz >= 13000) {
            DrpVal |= 0x0100;
        }
        else {
            DrpVal |= 0x0000;
        }
        /* Write new DRP register value for PPFx_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x0D : 0x8D,
                    DrpVal);

        /* QPLLx_CP */
        if (InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.NFbDiv <= 40) {
            DrpVal = 0x007F;
        }
        else {
            DrpVal = 0x03FF;
        }
        /* Write new DRP register value for QPLLx_CP. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x16 : 0x96,
                    DrpVal);

        /* QPLLx_LPF */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x19 : 0x99,
                    &DrpVal);
        DrpVal &= ~(0x0003);
        if (InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.NFbDiv <= 40) {
            DrpVal |= 0x3;
        }
        else {
            DrpVal |= 0x1;
        }
        /* Write new DRP register value for QPLLx_LPF. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x19 : 0x99,
                    DrpVal);

        /* QPLLx_CFG4 */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? 0x30 : 0xB0,
                    &DrpVal);
        DrpVal &= ~(0x00E7);
        /* Q_TERM_CLK */
        if (QpllxClkOutMHz >= 7500) {
            DrpVal |= 0x2 << 5;
        }
        else if (QpllxClkOutMHz >= 3500) {
            DrpVal |= 0x0 << 5;
        }
        else {
            DrpVal |= 0x6 << 5;
        }
        /* Q_DCRNT_CLK */
        if (QpllxClkOutMHz >= 7500) {
            DrpVal |= 0x5;
        }
        else if (QpllxClkOutMHz >= 5500) {
            DrpVal |= 0x4;
        }
        else {
            DrpVal |= 0x3;
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
u32 XHdmiphy1_Gthe4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u16 DrpVal;
    u16 WriteVal;
    u8 CfgIndex;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxVcoRateMHz;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u32 Status = XST_SUCCESS;

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
            Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId,
                    XHDMIPHY1_DRP_RXCDR_CFG_GEN4(CfgIndex), DrpVal);

        }
    }

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        /* RX_INT_DATAWIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x66, &DrpVal);
        DrpVal &= ~(0x3);
        WriteVal = (XHdmiphy1_DrpEncodeIntDataWidth(ChPtr->RxIntDataWidth) &
                        0x3);
        DrpVal |= WriteVal;
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
        if (PllxClkOutMHz >= 7500) {
            DrpVal |= 0x68;
        }
        else if (PllxClkOutMHz >= 5500) {
            DrpVal |= 0x44;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal |= 0x24;
        }
        else {
            DrpVal |= 0x3C;
        }
        /* Write new DRP register value for CH_HSPMUX_RX. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x116, DrpVal);

        /* PREIQ_FREQ_BST */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xFB, &DrpVal);
        DrpVal &= ~(0x0030);
        if (PllxClkOutMHz > 14110) {
            DrpVal |= 3 << 4;
        }
        else if (PllxClkOutMHz >= 14000) {
            DrpVal |= 2 << 4; /* LPM Mode */
        }
        else if (PllxClkOutMHz >= 10000) {
            DrpVal |= 2 << 4;
        }
        else if (PllxClkOutMHz >= 6000) {
            DrpVal |= 1 << 4;
        }
        /* Write new DRP register value for PREIQ_FREQ_BST. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xFB, DrpVal);

        /* RXPI_CFG0 */
        if (PllxClkOutMHz > 7000) {
            DrpVal = 0x0004;
        }
        else if (PllxClkOutMHz >= 6500) {
            DrpVal = 0x0104;
        }
        else if (PllxClkOutMHz >= 5500) {
            DrpVal = 0x2004;
        }
        else if (PllxClkOutMHz >= 5156) {
            DrpVal = 0x0002;
        }
        else if (PllxClkOutMHz >= 4500) {
            DrpVal = 0x0102;
        }
        else if (PllxClkOutMHz >= 4000) {
            DrpVal = 0x2102;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal = 0x2202;
        }
        else if (PllxClkOutMHz >= 3000) {
            DrpVal = 0x0200;
        }
        else if (PllxClkOutMHz >= 2500) {
            DrpVal = 0x1300;
        }
        else {
            DrpVal = 0x3300;
        }
        /* Write new DRP register value for RXPI_CFG0. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x9D, DrpVal);

        /* RXPI_CFG1 */
        if (PllxClkOutMHz >= 5500) {
            DrpVal = 0x0000;
        }
        else if (PllxClkOutMHz >= 4500) {
            DrpVal = 0x0015;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal = 0x0045;
        }
        else if (PllxClkOutMHz >= 2000) {
            DrpVal = 0x00FD;
        }
        else {
            DrpVal = 0x00FF;
        }
        /* Write new DRP register value for RXPI_CFG1. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x100, DrpVal);
    }

    Status |= XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
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
u32 XHdmiphy1_Gthe4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
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

    ReturnVal = XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
                    ChId);
    if (!XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
        return ReturnVal;
    }

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId,
                XHDMIPHY1_DIR_TX, ChId);
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

        /* Set TX_PROGDIV_CFG to 20/40 */
        if ((PllType == XHDMIPHY1_PLL_TYPE_QPLL) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL0) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL1)) {
            if (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].TxOutDiv != 16) {
                /* TX_PROGDIV_CFG = 20 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x3E, 57442);
            } else {
                /* TX_PROGDIV_CFG = 40 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x3E, 57415);
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

        PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr,
                            QuadId, ChIdPll,
                            XHDMIPHY1_DIR_TX) / 1000000;
        PllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;

        /* TXPI_CFG */
        if (PllxClkOutMHz >= 5500) {
            DrpVal = 0x0000;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal = 0x0054;
        }
        else {
            DrpVal = 0x03DF;
        }
        /* Write new DRP register value for TXPI_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xFF, DrpVal);

        /* TXPI_CFG3 */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x9C, &DrpVal);
        DrpVal &= ~(0x0040);
        if (PllxClkOutMHz < 7500 && PllxClkOutMHz >= 5500) {
            DrpVal |= 1 << 6;
        }
        /* Write new DRP register value for TXPI_CFG3. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0x9C, DrpVal);

        /* TX_PI_BIASSET */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0xFB, &DrpVal);
        DrpVal &= ~(0x0006);
        if (PllxClkOutMHz >= 7500) {
            DrpVal |= 3 << 1;
        }
        else if (PllxClkOutMHz >= 5500) {
            DrpVal |= 2 << 1;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal |= 1 << 1;
        }
        /* Write new DRP register value for TX_PI_BIASSET. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, 0xFB, DrpVal);

        /* CH_HSPMUX_TX */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, 0x116, &DrpVal);
        DrpVal &= ~(0xFF00);
        if (PllxClkOutMHz >= 7500) {
            DrpVal |= 0x68 << 8;
        }
        else if (PllxClkOutMHz >= 5500) {
            DrpVal |= 0x44 << 8;
        }
        else if (PllxClkOutMHz >= 3500) {
            DrpVal |= 0x24 << 8;
        }
        else {
            DrpVal |= 0x3C << 8;
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
u32 XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
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
u32 XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
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
               XHDMIPHY1_CPLL_CAL_PERIOD_REG) &
               ~XHDMIPHY1_CPLL_CAL_PERIOD_MASK;
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

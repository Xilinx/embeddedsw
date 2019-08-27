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
 * @file xhdmiphy1_hdmi.h
 *
 * The Xilinx HDMI PHY (HDMIPHY) driver. This driver supports the
 * Xilinx HDMI PHY IP core.
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
 * @addtogroup xhdmiphy1_v1_0
 * @{
*******************************************************************************/
#include "xparameters.h"

#ifndef XHDMIPHY1_HDMI_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_HDMI_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ******************************/

#define XHDMIPHY1_HDMI_GTYE5_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK          200000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MIN      199990000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MAX      200010000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MIN     399990000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MAX     400010000LL
#define XHDMIPHY1_HDMI_GTYE5_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTYE5_LCPLL_REFCLK_MIN    120000000LL
#define XHDMIPHY1_HDMI_GTYE5_RPLL_REFCLK_MIN     120000000LL
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MIN    2160000000U
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MAX    4320000000U
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MIN    2160000000U
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MAX    4320000000U

#define XHDMIPHY1_HDMI_GTYE4_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK          156250000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MIN      156240000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MAX      156260000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MIN     399990000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MAX     400010000LL
#define XHDMIPHY1_HDMI_GTYE4_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTYE4_QPLL0_REFCLK_MIN    61250000LL
#define XHDMIPHY1_HDMI_GTYE4_QPLL1_REFCLK_MIN    50000000LL
#define XHDMIPHY1_HDMI_GTYE4_CPLL_REFCLK_MIN     50000000LL
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MAX    1600000000U
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MAX    1600000000U

#define XHDMIPHY1_HDMI_GTHE4_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK          156250000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MIN      156240000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MAX      156260000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MIN     399980000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MAX     400020000LL
#define XHDMIPHY1_HDMI_GTHE4_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTHE4_QPLL0_REFCLK_MIN    61250000LL
#define XHDMIPHY1_HDMI_GTHE4_QPLL1_REFCLK_MIN    50000000LL
#define XHDMIPHY1_HDMI_GTHE4_CPLL_REFCLK_MIN     50000000LL
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MAX    1600000000U
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MAX    1600000000U

#define XHDMIPHY1_HDMI_GTHE3_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK          156250000LL
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK_MIN      156240000LL
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK_MAX      156260000LL
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK2_MIN     399990000LL
#define XHDMIPHY1_HDMI_GTHE3_DRU_REFCLK2_MAX     400010000LL
#define XHDMIPHY1_HDMI_GTHE3_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTHE3_QPLL0_REFCLK_MIN    61250000LL
#define XHDMIPHY1_HDMI_GTHE3_QPLL1_REFCLK_MIN    50000000LL
#define XHDMIPHY1_HDMI_GTHE3_CPLL_REFCLK_MIN     50000000LL
#define XHDMIPHY1_HDMI_GTHE3_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE3_TX_MMCM_FVCO_MIN    600000000U
#define XHDMIPHY1_HDMI_GTHE3_TX_MMCM_FVCO_MAX    1200000000U
#define XHDMIPHY1_HDMI_GTHE3_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE3_RX_MMCM_FVCO_MIN    600000000U
#define XHDMIPHY1_HDMI_GTHE3_RX_MMCM_FVCO_MAX    1200000000U

#define XHDMIPHY1_HDMI21_FRL_REFCLK              400000000U

/**************************** Function Prototypes *****************************/

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
u32 XHdmiphy1_HdmiQpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_HdmiCpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
void XHdmiphy1_TxAlignReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset);
void XHdmiphy1_TxAlignStart(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Start);
#else
u32 XHdmiphy1_HdmiTxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_HdmiRxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
#endif
void XHdmiphy1_ClkDetEnable(XHdmiphy1 *InstancePtr, u8 Enable);
void XHdmiphy1_ClkDetTimerClear(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_ClkDetSetFreqLockThreshold(XHdmiphy1 *InstancePtr,
        u16 ThresholdVal);
u8 XHdmiphy1_ClkDetCheckFreqZero(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_ClkDetSetFreqTimeout(XHdmiphy1 *InstancePtr, u32 TimeoutVal);
void XHdmiphy1_ClkDetTimerLoad(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u32 TimeoutVal);
void XHdmiphy1_DruReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset);
void XHdmiphy1_DruEnable(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Enable);
u16 XHdmiphy1_DruGetVersion(XHdmiphy1 *InstancePtr);
void XHdmiphy1_DruSetCenterFreqHz(XHdmiphy1 *InstancePtr,
        XHdmiphy1_ChannelId ChId, u64 CenterFreqHz);
u64 XHdmiphy1_DruCalcCenterFreqHz(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
void XHdmiphy1_HdmiGtDruModeEnable(XHdmiphy1 *InstancePtr, u8 Enable);
void XHdmiphy1_PatgenSetRatio(XHdmiphy1 *InstancePtr,
		u8 QuadId, u64 TxLineRate);
void XHdmiphy1_PatgenEnable(XHdmiphy1 *InstancePtr, u8 QuadId, u8 Enable);
void XHdmiphy1_HdmiIntrHandlerCallbackInit(XHdmiphy1 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_HDMI_H_ */
/** @} */

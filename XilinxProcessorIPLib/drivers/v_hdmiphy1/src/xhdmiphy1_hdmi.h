/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
 * 1.1   ku   24/07/20 Removed GTHE3 parameters
 *                     Added MMCM parameters to support MAX Rate
 * </pre>
 *
 * @addtogroup xhdmiphy1_v2_1
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
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1         125000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MIN     124990000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MAX     125010000LL
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

/*
 * Following are the MMCM Parameter values for each rate.
 * Based on the MAX rate config in PHY the MMCM
 * should be programmed to generate the vid clk in
 * FRL mode
 */

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
/* 12 G -> (400 * 3/1) / 3 -> 400Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT		3
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK		1
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV	3
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV	3
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV	3

/* 10 G -> 375Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10 15
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10 4
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10 4
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10 4
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10 4

/* 8 G -> 300Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8 3
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8 1
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8 4
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8 4
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8 4

/* 6x4 G -> 225Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6 54
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6 16
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6 6
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6 6
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6 6

/* 6x3 G -> 175Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3 49
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3 16
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3 7
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3 7
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3 7

/* 3x3 G -> 150Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3 3
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3 1
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3 8

#else
/* 12 G -> (400 * 3/1) / 3 -> 400Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT		7
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK		1
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV	7
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV	7
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV	7

/* 10 G -> 375Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10 15
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10 2
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10 8

/* 8 G -> 300Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8 15
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8 2
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8 10
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8 10
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8 10

/* 6x4 G -> 225Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6 54
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6 12
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6 12
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6 12

/* 6x3 G -> 175Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3 49
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3 8
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3 14
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3 14
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3 14

/* 3x3 G -> 150Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3 15
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3 2
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3 20
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3 20
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3 20

#endif

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

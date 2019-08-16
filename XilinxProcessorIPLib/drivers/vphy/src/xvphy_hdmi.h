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
 * @file xvphy_hdmi.h
 *
 * The Xilinx Video PHY (VPHY) driver. This driver supports the Xilinx Video PHY
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/19/15 Initial release.
 * 1.1   gm   02/01/16 Added GTPE2 and GTHE4 constants
 * 1.2   gm            Added XVphy_HdmiMmcmStart function
 * 1.4   gm   29/11/16 Added preprocessor directives for sw footprint reduction
 *                     Removed XVphy_HdmiMmcmStart API
 *                     Corrected GTPE2 DRU REFCLK range
 * 1.6   gm   03/07/17 Added XVPHY_HDMI_GTXE2_DRU_LRATE_Q/CPLL definitions
 *                     Corrected FVCO range for MMCME4
 * 1.7   gm   13/09/17 Removed XVphy_DruSetGain API
 * </pre>
 *
 * @addtogroup xvphy_v1_9
 * @{
*******************************************************************************/
#include "xparameters.h"
#if defined (XPAR_XV_HDMITX_0_DEVICE_ID) || defined (XPAR_XV_HDMIRX_0_DEVICE_ID)

#ifndef XVPHY_HDMI_H_
/* Prevent circular inclusions by using protection macros. */
#define XVPHY_HDMI_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ******************************/

#define XVPHY_HDMI_GTYE4_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTYE4_DRU_REFCLK			156250000LL
#define XVPHY_HDMI_GTYE4_DRU_REFCLK_MIN		156240000LL
#define XVPHY_HDMI_GTYE4_DRU_REFCLK_MAX		156260000LL
#define XVPHY_HDMI_GTYE4_PLL_SCALE			1000
#define XVPHY_HDMI_GTYE4_QPLL0_REFCLK_MIN	61250000LL
#define XVPHY_HDMI_GTYE4_QPLL1_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTYE4_CPLL_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTYE4_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTYE4_TX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTYE4_TX_MMCM_FVCO_MAX	1600000000U
#define XVPHY_HDMI_GTYE4_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTYE4_RX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTYE4_RX_MMCM_FVCO_MAX	1600000000U

#define XVPHY_HDMI_GTHE4_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTHE4_DRU_REFCLK			156250000LL
#define XVPHY_HDMI_GTHE4_DRU_REFCLK_MIN		156240000LL
#define XVPHY_HDMI_GTHE4_DRU_REFCLK_MAX		156260000LL
#define XVPHY_HDMI_GTHE4_PLL_SCALE			1000
#define XVPHY_HDMI_GTHE4_QPLL0_REFCLK_MIN	61250000LL
#define XVPHY_HDMI_GTHE4_QPLL1_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTHE4_CPLL_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTHE4_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE4_TX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTHE4_TX_MMCM_FVCO_MAX	1600000000U
#define XVPHY_HDMI_GTHE4_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE4_RX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTHE4_RX_MMCM_FVCO_MAX	1600000000U

#define XVPHY_HDMI_GTHE3_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTHE3_DRU_REFCLK			156250000LL
#define XVPHY_HDMI_GTHE3_DRU_REFCLK_MIN		156240000LL
#define XVPHY_HDMI_GTHE3_DRU_REFCLK_MAX		156260000LL
#define XVPHY_HDMI_GTHE3_PLL_SCALE			1000
#define XVPHY_HDMI_GTHE3_QPLL0_REFCLK_MIN	61250000LL
#define XVPHY_HDMI_GTHE3_QPLL1_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTHE3_CPLL_REFCLK_MIN	50000000LL
#define XVPHY_HDMI_GTHE3_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE3_TX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTHE3_TX_MMCM_FVCO_MAX	1200000000U
#define XVPHY_HDMI_GTHE3_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE3_RX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTHE3_RX_MMCM_FVCO_MAX	1200000000U

#define XVPHY_HDMI_GTHE2_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTHE2_DRU_REFCLK			125000000LL
#define XVPHY_HDMI_GTHE2_DRU_REFCLK_MIN		124990000LL
#define XVPHY_HDMI_GTHE2_DRU_REFCLK_MAX		125010000LL
#define XVPHY_HDMI_GTHE2_PLL_SCALE			1000
#define XVPHY_HDMI_GTHE2_QPLL_REFCLK_MIN	61250000LL
#define XVPHY_HDMI_GTHE2_CPLL_REFCLK_MIN	80000000LL
#define XVPHY_HDMI_GTHE2_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE2_TX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTHE2_TX_MMCM_FVCO_MAX	1200000000U
#define XVPHY_HDMI_GTHE2_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTHE2_RX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTHE2_RX_MMCM_FVCO_MAX	1200000000U

#define XVPHY_HDMI_GTXE2_DRU_LRATE_QPLL		2000000000U
#define XVPHY_HDMI_GTXE2_DRU_LRATE_CPLL		2500000000U
#define XVPHY_HDMI_GTXE2_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTXE2_DRU_REFCLK			125000000LL
#define XVPHY_HDMI_GTXE2_DRU_REFCLK_MIN		124990000LL
#define XVPHY_HDMI_GTXE2_DRU_REFCLK_MAX		125010000LL
#define XVPHY_HDMI_GTXE2_PLL_SCALE			1000
#define XVPHY_HDMI_GTXE2_QPLL_REFCLK_MIN	74125000LL
#define XVPHY_HDMI_GTXE2_CPLL_REFCLK_MIN	80000000LL
#define XVPHY_HDMI_GTXE2_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTXE2_TX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTXE2_TX_MMCM_FVCO_MAX	1866000000U
#define XVPHY_HDMI_GTXE2_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTXE2_RX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTXE2_RX_MMCM_FVCO_MAX	1200000000U

#define XVPHY_HDMI_GTPE2_DRU_LRATE			2500000000U
#define XVPHY_HDMI_GTPE2_DRU_REFCLK			100000000LL
#define XVPHY_HDMI_GTPE2_DRU_REFCLK_MIN	 	99990000LL
#define XVPHY_HDMI_GTPE2_DRU_REFCLK_MAX		100010000LL
#define XVPHY_HDMI_GTPE2_PLL_SCALE			1000
#define XVPHY_HDMI_GTPE2_QPLL_REFCLK_MIN	80000000LL
#define XVPHY_HDMI_GTPE2_CPLL_REFCLK_MIN	80000000LL
#define XVPHY_HDMI_GTPE2_TX_MMCM_SCALE		1
#define XVPHY_HDMI_GTPE2_TX_MMCM_FVCO_MIN	800000000U
#define XVPHY_HDMI_GTPE2_TX_MMCM_FVCO_MAX	1866000000U
#define XVPHY_HDMI_GTPE2_RX_MMCM_SCALE		1
#define XVPHY_HDMI_GTPE2_RX_MMCM_FVCO_MIN	600000000U
#define XVPHY_HDMI_GTPE2_RX_MMCM_FVCO_MAX	1200000000U

/**************************** Function Prototypes *****************************/

u32 XVphy_HdmiQpllParam(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir);
u32 XVphy_HdmiCpllParam(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir);
void XVphy_TxAlignReset(XVphy *InstancePtr, XVphy_ChannelId ChId, u8 Reset);
void XVphy_TxAlignStart(XVphy *InstancePtr, XVphy_ChannelId ChId, u8 Start);
void XVphy_ClkDetEnable(XVphy *InstancePtr, u8 Enable);
void XVphy_ClkDetTimerClear(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir);
void XVphy_ClkDetSetFreqLockThreshold(XVphy *InstancePtr, u16 ThresholdVal);
u8 XVphy_ClkDetCheckFreqZero(XVphy *InstancePtr, XVphy_DirectionType Dir);
void XVphy_ClkDetSetFreqTimeout(XVphy *InstancePtr, u32 TimeoutVal);
void XVphy_ClkDetTimerLoad(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, u32 TimeoutVal);
void XVphy_DruReset(XVphy *InstancePtr, XVphy_ChannelId ChId, u8 Reset);
void XVphy_DruEnable(XVphy *InstancePtr, XVphy_ChannelId ChId, u8 Enable);
u16 XVphy_DruGetVersion(XVphy *InstancePtr);
void XVphy_DruSetCenterFreqHz(XVphy *InstancePtr, XVphy_ChannelId ChId,
		u64 CenterFreqHz);
u64 XVphy_DruCalcCenterFreqHz(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId);
void XVphy_HdmiGtDruModeEnable(XVphy *InstancePtr, u8 Enable);
void XVphy_PatgenSetRatio(XVphy *InstancePtr, u8 QuadId, u64 TxLineRate);
void XVphy_HdmiIntrHandlerCallbackInit(XVphy *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XVPHY_HDMI_H_ */
#endif
/** @} */

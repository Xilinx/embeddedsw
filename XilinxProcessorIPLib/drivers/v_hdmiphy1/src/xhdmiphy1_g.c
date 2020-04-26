
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xhdmiphy1.h"

/*
* The configuration table for devices
*/

XHdmiphy1_Config XHdmiphy1_ConfigTable[XPAR_XHDMIPHY1_NUM_INSTANCES] =
{
	{
		XPAR_V_HDMI_PHY_DEVICE_ID,
		XPAR_V_HDMI_PHY_BASEADDR,
		(XHdmiphy1_GtType)XPAR_V_HDMI_PHY_TRANSCEIVER,
		XPAR_V_HDMI_PHY_TX_NO_OF_CHANNELS,
		XPAR_V_HDMI_PHY_RX_NO_OF_CHANNELS,
		(XHdmiphy1_ProtocolType)XPAR_V_HDMI_PHY_TX_PROTOCOL,
		(XHdmiphy1_ProtocolType)XPAR_V_HDMI_PHY_RX_PROTOCOL,
		(XHdmiphy1_PllRefClkSelType)XPAR_V_HDMI_PHY_TX_REFCLK_SEL,
		(XHdmiphy1_PllRefClkSelType)XPAR_V_HDMI_PHY_RX_REFCLK_SEL,
		(XHdmiphy1_PllRefClkSelType)XPAR_V_HDMI_PHY_TX_FRL_REFCLK_SEL,
		(XHdmiphy1_PllRefClkSelType)XPAR_V_HDMI_PHY_RX_FRL_REFCLK_SEL,
		(XHdmiphy1_SysClkDataSelType)XPAR_V_HDMI_PHY_TX_PLL_SELECTION,
		(XHdmiphy1_SysClkDataSelType)XPAR_V_HDMI_PHY_RX_PLL_SELECTION,
		XPAR_V_HDMI_PHY_NIDRU,
		(XHdmiphy1_PllRefClkSelType)XPAR_V_HDMI_PHY_NIDRU_REFCLK_SEL,
		(XVidC_PixelsPerClock)XPAR_V_HDMI_PHY_INPUT_PIXELS_PER_CLOCK,
		XPAR_V_HDMI_PHY_TX_BUFFER_BYPASS,
		XPAR_V_HDMI_PHY_HDMI_FAST_SWITCH,
		XPAR_V_HDMI_PHY_TRANSCEIVER_WIDTH,
		XPAR_V_HDMI_PHY_ERR_IRQ_EN,
		XPAR_V_HDMI_PHY_AXI_LITE_FREQ_HZ,
		XPAR_V_HDMI_PHY_DRPCLK_FREQ,
		XPAR_V_HDMI_PHY_USE_GT_CH4_HDMI
	}
};

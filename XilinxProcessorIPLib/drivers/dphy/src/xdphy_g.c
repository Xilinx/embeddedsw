/*******************************************************************
 * Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
 * Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef SDT
#include "xparameters.h"
#else
#include "xdphy.h"
#endif

#ifndef SDT
/*
* The configuration table for devices
*/

XDphy_Config XDphy_ConfigTable[] =
{
	{
		XPAR_MIPI_DPHY_0_DEVICE_ID,
		XPAR_MIPI_DPHY_0_BASEADDR,
		XPAR_MIPI_DPHY_0_DPHY_MODE,
		XPAR_MIPI_DPHY_0_EN_REG_IF,
		XPAR_MIPI_DPHY_0_DPHY_LANES,
		XPAR_MIPI_DPHY_0_ESC_CLK_PERIOD,
		XPAR_MIPI_DPHY_0_ESC_TIMEOUT,
		XPAR_MIPI_DPHY_0_HS_LINE_RATE,
		XPAR_MIPI_DPHY_0_HS_TIMEOUT,
		XPAR_MIPI_DPHY_0_LPX_PERIOD,
		XPAR_MIPI_DPHY_0_STABLE_CLK_PERIOD,
		XPAR_MIPI_DPHY_0_TXPLL_CLKIN_PERIOD,
		XPAR_MIPI_DPHY_0_WAKEUP,
		XPAR_MIPI_DPHY_0_EN_TIMEOUT_REGS,
		XPAR_MIPI_DPHY_0_HS_SETTLE_NS
	},
	{
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DEVICE_ID,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_BASEADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DPHY_MODE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_EN_REG_IF,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DPHY_LANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_ESC_CLK_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_ESC_TIMEOUT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_LINE_RATE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_TIMEOUT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_LPX_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_STABLE_CLK_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_TXPLL_CLKIN_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_WAKEUP,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_EN_TIMEOUT_REGS,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_SETTLE_NS
	}
};

#else
XDphy_Config XDphy_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dphy-4.3", /* compatible */
		0x1000, /* reg */
		0x0, /* xlnx,dphy-mode */
		0x1, /* xlnx,dphy-en-reg-if */
		0x4, /* xlnx,dphy-dphy-lanes */
		0x50, /* xlnx,dphy-esc-clk-period */
		0x25600, /* xlnx,dphy-esc-timeout */
		0x1440, /* xlnx,hs-line-rate */
		0x65541, /* xlnx,dphy-hs-timeout */
		0x50, /* xlnx,lpx-period */
		0x5, /* xlnx,dphy-stable-clk-period */
		0x6, /* xlnx,dphy-txpll-clkin-period */
		0x1000000, /* xlnx,dphy-wakeup */
		0x0, /* xlnx,dphy-en-timeout-regs */
		0x141 /* xlnx,dphy-hs-settle-ns */
	},
	 {
		 NULL
	}
};

#endif

/******************************************************************************
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xmipi_rx_phy.h"

/*
* The configuration table for devices
*/

XMipi_Rx_Phy_Config XMipi_Rx_Phy_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-rx-phy-1.0", /* compatible */
		0xb0030000, /* reg */
		0x1, /* xlnx,phy-mode */
		0x1, /* xlnx,reg-on */
		0x1, /* xlnx,phy-lanes */
		0x32, /* xlnx,esc-clk-period */
		0x6400, /* xlnx,esc-timeout */
		0x5dc, /* xlnx,hs-line-rate */
		0x3801, /* xlnx,hs-timeout */
		0xc8, /* xlnx,stable-clk-period */
		0x30d40, /* xlnx,wakeup */
		0x0, /* xlnx,en-timeout-regs */
		0xc, /* xlnx,hs-settle */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};

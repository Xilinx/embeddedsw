/******************************************************************************
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xmipi_tx_phy.h"

/*
* The configuration table for devices
*/

XMipi_Tx_Phy_Config XMipi_Tx_Phy_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-tx-phy-1.0", /* compatible */
		0xb0060000, /* reg */
		0x1, /* xlnx,phy-mode */
		0x1, /* xlnx,reg-on */
		0x1, /* xlnx,phy-lanes */
		0x32, /* xlnx,esc-clk-period */
		0x6400, /* xlnx,esc-timeout */
		0x5dc, /* xlnx,line-rate */
		0x10005, /* xlnx,hs-timeout */
		0x5, /* xlnx,lpx-time */
		0xc8, /* xlnx,stable-clk-period */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};

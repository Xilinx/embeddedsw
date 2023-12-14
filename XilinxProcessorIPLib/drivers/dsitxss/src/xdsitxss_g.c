/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#else
#include "xdsitxss.h"
#endif

#ifndef SDT
/*
* List of Sub-cores included in the subsystem
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_PRESENT	 1


/*
* List of Sub-cores excluded from the subsystem
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core baseaddr is set to 0
*/

#define XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_PRESENT 0
#define XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_DEVICE_ID 255
#define XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_BASEADDR 0




XDsiTxSs_Config XDsiTxSs_ConfigTable[] =
{
	{
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DEVICE_ID,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_BASEADDR,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_HIGHADDR,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DSI_LANES,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DSI_DATATYPE,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DSI_BYTE_FIFO,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DSI_CRC_GEN,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DSI_PIXELS,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DPHY_LINERATE,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_DPHY_EN_REG_IF,

		{
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_PRESENT,
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_DEVICE_ID,
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DPHY_0_BASEADDR
		},
		{
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_PRESENT,
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DEVICE_ID,
			XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_BASEADDR
		},
	}
};

#else
XDsiTxSs_Config XDsiTxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi-tx-subsystem-2.3", /* compatible */
		0x80020000, /* reg */
		0x8003ffff, /* xlnx,highaddr */
		0x4, /* xlnx,dsi-lanes */
		0x3e, /* xlnx,dsi-datatype */
		0x0, /* xlnx,dsi-byte-fifo */
		0x1, /* xlnx,dsi-crc-gen */
		0x2, /* xlnx,dsi-pixels */
		0x320, /* xlnx,dphy-linerate */
		0x1, /* xlnx,dphy-en-reg-if */
		0x1, /* xlnx,dphy-present */
		0x10000, /* xlnx,dphy-connected */
		0x1, /* xlnx,dsi-tx-ctrl-present */
		0x0, /* xlnx,dsi-tx-ctrl-connected */
		0x406c, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	 {
		 NULL
	}
};
#endif

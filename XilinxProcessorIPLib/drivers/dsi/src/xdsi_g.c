/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#else
#include "xdsi.h"
#endif

#ifndef SDT
/*
* The configuration table for devices
*/

XDsi_Config XDsi_ConfigTable[] =
{
	{
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DEVICE_ID,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_BASEADDR,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DSI_LANES,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DSI_DATATYPE,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DSI_BYTE_FIFO,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DSI_CRC_GEN,
		XPAR_MIPI_DSI_TX_SUBSYSTEM_0_MIPI_DSI_TX_CTRL_0_DSI_PIXELS
	}
};

#else
XDsi_Config XDsi_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi-tx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x4, /* xlnx,dsi-ctrl-lanes */
		0x3e, /* xlnx,dsi-ctrl-datatype */
		0x800, /* xlnx,dsi-ctrl-byte-fifo */
		0x1, /* xlnx,dsi-ctrl-crc-gen */
		0x2 /* xlnx,dsi-ctrl-pixels */
	},
	 {
		 NULL
	}
};
#endif

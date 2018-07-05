/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xdsitxss.h"

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

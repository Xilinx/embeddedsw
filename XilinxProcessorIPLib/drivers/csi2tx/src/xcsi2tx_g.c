/*******************************************************************
 *
 * Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xcsi2tx.h"

/*
* The configuration table for devices
*/

XCsi2Tx_Config XCsi2Tx_ConfigTable[] =
{
	{
		XPAR_MIPI_CSI2_TX_SUBSYSTEM_0_MIPI_CSI2_TX_CTRL_0_DEVICE_ID,
		XPAR_MIPI_CSI2_TX_SUBSYSTEM_0_MIPI_CSI2_TX_CTRL_0_BASEADDR,
		XPAR_MIPI_CSI2_TX_SUBSYSTEM_0_MIPI_CSI2_TX_CTRL_0_CSI_LANES,
		XPAR_MIPI_CSI2_TX_SUBSYSTEM_0_MIPI_CSI2_TX_CTRL_0_CSI_EN_ACTIVELANES,
		XPAR_MIPI_CSI2_TX_SUBSYSTEM_0_MIPI_CSI2_TX_CTRL_0_EN_REG_BASED_FE_GEN
	}
};

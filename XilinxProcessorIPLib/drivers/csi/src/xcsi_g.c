/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xcsi.h"

/*
* The configuration table for devices
*/

XCsi_Config XCsi_ConfigTable[] =
{
	{
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_DEVICE_ID,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_BASEADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_LANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_OFFLOAD_NONIMAGE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_EN_VC_SUPPORT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_FIXED_VC,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_OPT3_FIXEDLANES
	}
};

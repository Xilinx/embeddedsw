/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xpciepsu.h"

/*
* The configuration table for devices
*/

XPciePsu_Config XPciePsu_ConfigTable[] =
{
	{
		XPAR_PSU_PCIE_DEVICE_ID,
		XPAR_PSU_PCIE_BASEADDR,
		XPAR_PSU_PCIE_ATTRIB_0_BASEADDR,
		XPAR_PSU_PCIE_HIGH2_BASEADDR,
		XPAR_PSU_PCIE_LOW_BASEADDR,
		XPAR_PSU_PCIE_HIGH1_BASEADDR,
		XPAR_PSU_PCIE_LOW_HIGHADDR,
		XPAR_PSU_PCIE_HIGH1_HIGHADDR
	}
};

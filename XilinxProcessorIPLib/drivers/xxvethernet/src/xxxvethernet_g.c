/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xxxvethernet_g.c
* @addtogroup xxvethernet Overview
* @{
*
* @brief Provides the XXV Ethernet configuration table for the system.
* Provide a template for user to define their own hardware settings.
*
* If using XPS, XPS will automatically generate this file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   6/16/17  First release
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "xxxvethernet.h"

/** Configuration table for each XXV Ethernet instance in the system. */
XXxvEthernet_Config XXxvEthernet_ConfigTable[] = {
	{
		XPAR_XXVETHERNET_0_DEVICE_ID,
		XPAR_XXVETHERNET_0_BASEADDR,
		XPAR_XXVETHERNET_0_STATS,
		XPAR_XXVETHERNET_0_CONNECTED_TYPE,
		XPAR_XXVETHERNET_0_CONNECTED_BASEADDR,
		XPAR_XXVETHERNET_0_CONNECTED_FIFO_INTR,
		0xFF,
		0xFF
	}
};
/** @} */

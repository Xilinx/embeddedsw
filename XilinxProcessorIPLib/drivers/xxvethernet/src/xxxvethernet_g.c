/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xxxvethernet.h"

/*
* The configuration table for devices
*/

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

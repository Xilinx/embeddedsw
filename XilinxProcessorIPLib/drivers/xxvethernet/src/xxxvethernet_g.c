/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xxxvethernet.h"

/*
*@file xxxvethernet_g.c
*@addtogroup xxvethernet Overview
*
*
*
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

/*******************************************************************
 * Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT*
*******************************************************************/

#include "xparameters.h"
#include "xcanfd.h"

/*
* The configuration table for devices
*/

XCanFd_Config XCanFd_ConfigTable[] =
{
	{
		XPAR_CANFD_0_DEVICE_ID,
		XPAR_CANFD_0_BASEADDR,
		XPAR_CANFD_0_RX_MODE,
		XPAR_CANFD_0_NUM_OF_RX_MB_BUF,
		XPAR_CANFD_0_NUM_OF_TX_BUF,
		XPAR_CANFD_0_IS_PL
	}
};

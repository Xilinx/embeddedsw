/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xaxiethernet.h"

/*
* The configuration table for devices
*/

XAxiEthernet_Config XAxiEthernet_ConfigTable[] = {
	{
	 XPAR_AXIETHERNET_0_DEVICE_ID,
	 XPAR_AXIETHERNET_0_BASEADDR,
	 XPAR_AXIETHERNET_0_TEMAC_TYPE,
	 XPAR_AXIETHERNET_0_TXCSUM,
	 XPAR_AXIETHERNET_0_RXCSUM,
	 XPAR_AXIETHERNET_0_PHY_TYPE,
	 XPAR_AXIETHERNET_0_TXVLAN_TRAN,
	 XPAR_AXIETHERNET_0_RXVLAN_TRAN,
	 XPAR_AXIETHERNET_0_TXVLAN_TAG,
	 XPAR_AXIETHERNET_0_RXVLAN_TAG,
	 XPAR_AXIETHERNET_0_TXVLAN_STRP,
	 XPAR_AXIETHERNET_0_RXVLAN_STRP,
	 XPAR_AXIETHERNET_0_MCAST_EXTEND,
	 XPAR_AXIETHERNET_0_STATS,
	 XPAR_AXIETHERNET_0_AVB,
	 XPAR_AXIETHERNET_0_ENABLE_SGMII_OVER_LVDS,
	 XPAR_AXIETHERNET_0_ENABLE_1588,
	 XPAR_AXIETHERNET_0_SPEED,
	 XPAR_AXIETHERNET_0_NUM_TABLE_ENTRIES,
	 XPAR_AXIETHERNET_0_INTR,
	 XPAR_AXIETHERNET_0_CONNECTED_TYPE,
	 XPAR_AXIETHERNET_0_CONNECTED_BASEADDR,
	 0xFF,
	 0xFF,
	 0xFF,
	 0x00,
	 {
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF
	 },
	 {
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF,
		 0xFF
	 }
	 }
};

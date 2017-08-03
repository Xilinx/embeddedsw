/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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

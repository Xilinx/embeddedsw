/*******************************************************************
*
* Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.*
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
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

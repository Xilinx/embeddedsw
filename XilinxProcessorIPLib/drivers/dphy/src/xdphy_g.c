/*******************************************************************
 *Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.*
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
*******************************************************************************/

#include "xparameters.h"
#include "xdphy.h"

/*
* The configuration table for devices
*/

XDphy_Config XDphy_ConfigTable[] =
{
	{
		XPAR_MIPI_DPHY_0_DEVICE_ID,
		XPAR_MIPI_DPHY_0_BASEADDR,
		XPAR_MIPI_DPHY_0_DPHY_MODE,
		XPAR_MIPI_DPHY_0_EN_REG_IF,
		XPAR_MIPI_DPHY_0_DPHY_LANES,
		XPAR_MIPI_DPHY_0_ESC_CLK_PERIOD,
		XPAR_MIPI_DPHY_0_ESC_TIMEOUT,
		XPAR_MIPI_DPHY_0_HS_LINE_RATE,
		XPAR_MIPI_DPHY_0_HS_TIMEOUT,
		XPAR_MIPI_DPHY_0_LPX_PERIOD,
		XPAR_MIPI_DPHY_0_STABLE_CLK_PERIOD,
		XPAR_MIPI_DPHY_0_TXPLL_CLKIN_PERIOD,
		XPAR_MIPI_DPHY_0_WAKEUP,
		XPAR_MIPI_DPHY_0_EN_TIMEOUT_REGS,
		XPAR_MIPI_DPHY_0_HS_SETTLE_NS
	},
	{
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DEVICE_ID,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_BASEADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DPHY_MODE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_EN_REG_IF,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DPHY_LANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_ESC_CLK_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_ESC_TIMEOUT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_LINE_RATE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_TIMEOUT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_LPX_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_STABLE_CLK_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_TXPLL_CLKIN_PERIOD,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_WAKEUP,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_EN_TIMEOUT_REGS,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_HS_SETTLE_NS
	}
};

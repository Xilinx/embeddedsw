/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM

/**
 * Implementation for the reset of individual node.
 */
#ifdef ENABLE_NODE_IDLING
#include <sleep.h>
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_reset.h"
#include "pm_node_reset.h"
#include "pm_node_idle.h"

#define MAX_RST_ACTION	5

typedef struct PmNodeResetInfo {
	/*
	 * NodeId for the this data
	 */
	u32 NodeId;

	/*
	 * List of Reset lines and corresponding action.
	 * The list also ends early if next element's
	 * ResetId is 0
	 */
	struct {
		u32 ResetId;	/* Reset Line */
		u32 ResetAction; /* Action Pulse / Assert */
		u32 ResetPulseWait; /* wait time (in microseconds) between Assert and de-assert*/
	} RstActionList[MAX_RST_ACTION];

	/*
	 * Individual IP soft reset function.
	 */
	void (*SoftRst)(u32);
	u32 SoftRstArgs;

	/*
	 * hook function for custom idling before soft reset.
	 */
	void (*IdleHook)(u32);
	u32 IdleHookArgs;
} PmNodeResetInfo;


/* Static resource allocation for Reset Info Data for all
 * possible nodes
 */
static const PmNodeResetInfo NodeRstData[] = {
	{
		.NodeId = NODE_USB_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_USB0_CORERESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{	.ResetId = PM_RESET_USB0_APB,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{	.ResetId = PM_RESET_USB0_HIBERRESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_USB_XHCI_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XPAR_PSU_USB_XHCI_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_USB_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_USB1_CORERESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{	.ResetId = PM_RESET_USB1_APB,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{	.ResetId = PM_RESET_USB1_HIBERRESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_USB_XHCI_1_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XPAR_PSU_USB_XHCI_1_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_TTC_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_TTC0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_TTC_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPAR_PSU_TTC_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_TTC_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_TTC1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_TTC_1_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPAR_PSU_TTC_1_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_TTC_2,
		.RstActionList= {
			{	.ResetId = PM_RESET_TTC2,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_TTC_2_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPAR_PSU_TTC_2_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#ifndef ENABLE_RECOVERY
	{
		.NodeId = NODE_TTC_3,
		.RstActionList= {
			{	.ResetId = PM_RESET_TTC3,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_TTC_3_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPAR_PSU_TTC_3_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#endif
	{
		.NodeId = NODE_SATA,
		.RstActionList= {
			{	.ResetId = PM_RESET_SATA,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},

#ifdef XPAR_PSU_SATA_S_AXI_BASEADDR
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeSataIdle,
		.IdleHookArgs = XPAR_PSU_SATA_S_AXI_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_ETH_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_GEM0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_ETHERNET_0_DEVICE_ID
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_ETHERNET_0_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSU_ETHERNET_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_ETH_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_GEM1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_ETHERNET_1_DEVICE_ID
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_ETHERNET_1_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSU_ETHERNET_1_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_ETH_2,
		.RstActionList= {
			{	.ResetId = PM_RESET_GEM2,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_ETHERNET_2_DEVICE_ID
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_ETHERNET_2_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSU_ETHERNET_2_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_ETH_3,
		.RstActionList= {
			{	.ResetId = PM_RESET_GEM3,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_ETHERNET_3_DEVICE_ID
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_ETHERNET_3_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPAR_PSU_ETHERNET_3_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#if !((STDOUT_BASEADDRESS == XPAR_PSU_UART_0_BASEADDR) && defined(DEBUG_MODE))
	{
		.NodeId = NODE_UART_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_UART0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_UART_0_DEVICE_ID
		.SoftRst = XUartPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_UART_0_BASEADDR,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#endif
#if !((STDOUT_BASEADDRESS == XPAR_PSU_UART_1_BASEADDR) && defined(DEBUG_MODE))
	{
		.NodeId = NODE_UART_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_UART1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_UART_1_DEVICE_ID
		.SoftRst = XUartPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_UART_1_BASEADDR,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#endif
	{
		.NodeId = NODE_SPI_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_SPI0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_SPI_0_DEVICE_ID
		.SoftRst = XSpiPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_SPI_0_BASEADDR,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_SPI_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_SPI1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_SPI_1_DEVICE_ID
		.SoftRst = XSpiPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_SPI_1_BASEADDR,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_I2C_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_I2C0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_I2C_0_DEVICE_ID
		.SoftRst = XIicPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_I2C_0_BASEADDR,
		.IdleHook = NodeI2cIdle,
		.IdleHookArgs = XPAR_PSU_I2C_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_I2C_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_I2C1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_I2C_1_DEVICE_ID
		.SoftRst = XIicPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_I2C_1_BASEADDR,
		.IdleHook = NodeI2cIdle,
		.IdleHookArgs = XPAR_PSU_I2C_1_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_SD_0,
		.RstActionList= {
			{	.ResetId = PM_RESET_SDIO0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_SD_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPAR_PSU_SD_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_SD_1,
		.RstActionList= {
			{	.ResetId = PM_RESET_SDIO1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_SD_1_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPAR_PSU_SD_1_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_QSPI,
		.RstActionList= {
			{	.ResetId = PM_RESET_QSPI,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_QSPI_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeQspiIdle,
		.IdleHookArgs = XQSPIPSU_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#ifndef REMOVE_GPIO_FROM_NODE_RESET_INFO
	{
		.NodeId = NODE_GPIO,
		.RstActionList= {
			{	.ResetId = PM_RESET_GPIO,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_PSU_GPIO_0_DEVICE_ID
		.SoftRst = XGpioPs_ResetHw,
		.SoftRstArgs = XPAR_PSU_GPIO_0_BASEADDR,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#endif
	{
		.NodeId = NODE_PCIE,
		.RstActionList= {
			{	.ResetId = PM_RESET_PCIE_CTRL,
				.ResetAction = PM_RESET_ACTION_PULSE,
				.ResetPulseWait = 0,
			},
			{0,0,0}
		},
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
	},
	{
		.NodeId = NODE_DP,
		.RstActionList= {
			{	.ResetId = PM_RESET_DP,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0,0,0}
		},
#ifdef XPAR_XDPPSU_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeDpIdle,
		.IdleHookArgs = XPAR_XDPPSU_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
};

/**
 * PmNodeResetInfo() - Get the reset info data for the given node
 * @NodeId      ID of Node
 *
 * @return      Pointer to PmNodeResetInfo structure for the given
 * 				node (or NULL if not found)
 */

static const PmNodeResetInfo *GetNodeResetInfo(const u32 NodeId)
{
	u32 Index;
	const PmNodeResetInfo *RstInfo = NULL;

	for (Index = 0; Index < ARRAY_SIZE(NodeRstData); Index++) {
		if (NodeId == NodeRstData[Index].NodeId) {
			RstInfo = &NodeRstData[Index];
			break;
		}
	}
	return RstInfo;
}

/**
 * PmNodeReset() - Resets the given node after idling
 * @Master         Initiator of the request
 * @NodeId         ID of Node to be reset
 * @IdleReq        flag to indicate whether node should be idle and soft reset
 */
void PmNodeReset(const PmMaster *const Master, const u32 NodeId, const u32 IdleReq)
{
	u32 Index;
	const PmNodeResetInfo *RstInfo = GetNodeResetInfo(NodeId);

	if (!RstInfo) {
		/*
		 * No reset node data available for this node.
		 */
		return;
	}

	/*
	 * If idling is requested, call the idling
	 * function along with the soft reset function
	 */
	if (NODE_IDLE_REQ == IdleReq) {

		if (RstInfo->IdleHook) {
			RstInfo->IdleHook(RstInfo->IdleHookArgs);
		}

		if (RstInfo->SoftRst) {
			RstInfo->SoftRst(RstInfo->SoftRstArgs);
		}

	}

	/*
	 * Perform the Node reset using Reset Line for the
	 * node and its corresponding actions
	 */
	for (Index = 0; Index < MAX_RST_ACTION; Index++) {

		if (RstInfo->RstActionList[Index].ResetId == 0) {
			/*
			 * No more reset action required.
			 */
			break;
		}

		PmResetAssert(Master, RstInfo->RstActionList[Index].ResetId,
								RstInfo->RstActionList[Index].ResetAction);

		if (RstInfo->RstActionList[Index].ResetAction != PM_RESET_ACTION_PULSE) {
			/*
			 * Release the reset after given interval
			 */
			usleep(RstInfo->RstActionList[Index].ResetPulseWait);
			PmResetAssert(Master, RstInfo->RstActionList[Index].ResetId,
													PM_RESET_ACTION_RELEASE);
		}
	}
}

#endif
#endif /* ENABLE_NODE_IDLING */

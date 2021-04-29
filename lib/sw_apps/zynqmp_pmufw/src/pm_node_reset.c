/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#include "pm_core.h"
#include "pm_reset.h"
#include "pm_node_reset.h"
#include "pm_node_idle.h"
#include "xpfw_default.h"

#define MAX_RST_ACTION	3

typedef struct PmNodeResetInfo {
	/*
	 * NodeId for the this data
	 */
	u8 NodeId;

	/*
	 * List of Reset lines and corresponding action.
	 * The list also ends early if next element's
	 * ResetId is 0
	 */
	struct {
		u16 ResetId;	/* Reset Line */
		u8 ResetAction; /* Action Pulse / Assert */
		u8 ResetPulseWait; /* wait time (in microseconds) between Assert and de-assert*/
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
			{
				.ResetId = PM_RESET_USB0_CORERESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{
				.ResetId = PM_RESET_USB0_APB,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{
				.ResetId = PM_RESET_USB0_HIBERRESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
		},
#ifdef XPMU_USB_0
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XPMU_USB_0_BASEADDR
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
			{
				.ResetId = PM_RESET_USB1_CORERESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{
				.ResetId = PM_RESET_USB1_APB,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{
				.ResetId = PM_RESET_USB1_HIBERRESET,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
		},
#ifdef XPMU_USB_1
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeUsbIdle,
		.IdleHookArgs = XPMU_USB_1_BASEADDR
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
			{
				.ResetId = PM_RESET_TTC0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_XTTCPS_0
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPMU_XTTCPS_0_BASEADDR
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
			{
				.ResetId = PM_RESET_TTC1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_XTTCPS_3
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPMU_XTTCPS_3_BASEADDR
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
			{
				.ResetId = PM_RESET_TTC2,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_XTTCPS_6
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPMU_XTTCPS_6_BASEADDR
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
			{
				.ResetId = PM_RESET_TTC3,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_XTTCPS_9
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeTtcIdle,
		.IdleHookArgs = XPMU_XTTCPS_9_BASEADDR
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
			{
				.ResetId = PM_RESET_SATA,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
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
			{
				.ResetId = PM_RESET_GEM0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ETHERNET_0
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPMU_ETHERNET_0_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPMU_ETHERNET_0_BASEADDR
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
			{
				.ResetId = PM_RESET_GEM1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ETHERNET_1
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPMU_ETHERNET_1_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPMU_ETHERNET_1_BASEADDR
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
			{
				.ResetId = PM_RESET_GEM2,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ETHERNET_2
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPMU_ETHERNET_2_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPMU_ETHERNET_2_BASEADDR
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
			{
				.ResetId = PM_RESET_GEM3,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ETHERNET_3
		.SoftRst = XEmacPs_ResetHw,
		.SoftRstArgs = XPMU_ETHERNET_3_BASEADDR,
		.IdleHook = NodeGemIdle,
		.IdleHookArgs = XPMU_ETHERNET_3_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
#if !((STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR) && defined(DEBUG_MODE))
	{
		.NodeId = NODE_UART_0,
		.RstActionList= {
			{
				.ResetId = PM_RESET_UART0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_UART_0
		.SoftRst = XUartPs_ResetHw,
		.SoftRstArgs = XPMU_UART_0_BASEADDR,
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
#if !((STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR) && defined(DEBUG_MODE))
	{
		.NodeId = NODE_UART_1,
		.RstActionList= {
			{
				.ResetId = PM_RESET_UART1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_UART_1
		.SoftRst = XUartPs_ResetHw,
		.SoftRstArgs = XPMU_UART_1_BASEADDR,
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
			{
				.ResetId = PM_RESET_SPI0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_SPI_0
		.SoftRst = XSpiPs_ResetHw,
		.SoftRstArgs = XPMU_SPI_0_BASEADDR,
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
			{
				.ResetId = PM_RESET_SPI1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_SPI_1
		.SoftRst = XSpiPs_ResetHw,
		.SoftRstArgs = XPMU_SPI_1_BASEADDR,
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
			{
				.ResetId = PM_RESET_I2C0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_I2C_0
		.SoftRst = XIicPs_ResetHw,
		.SoftRstArgs = XPMU_I2C_0_BASEADDR,
		.IdleHook = NodeI2cIdle,
		.IdleHookArgs = XPMU_I2C_0_BASEADDR
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
			{
				.ResetId = PM_RESET_I2C1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_I2C_1
		.SoftRst = XIicPs_ResetHw,
		.SoftRstArgs = XPMU_I2C_1_BASEADDR,
		.IdleHook = NodeI2cIdle,
		.IdleHookArgs = XPMU_I2C_1_BASEADDR
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
			{
				.ResetId = PM_RESET_SDIO0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_SD_0
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPMU_SD_0_BASEADDR
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
			{
				.ResetId = PM_RESET_SDIO1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_SD_1
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeSdioIdle,
		.IdleHookArgs = XPMU_SD_1_BASEADDR
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
			{
				.ResetId = PM_RESET_QSPI,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPAR_XQSPIPSU_0_DEVICE_ID
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
			{
				.ResetId = PM_RESET_GPIO,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPAR_XGPIOPS_0_DEVICE_ID
		.SoftRst = XGpioPs_ResetHw,
		.SoftRstArgs = XPAR_XGPIOPS_0_BASEADDR,
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
			{
				.ResetId = PM_RESET_PCIE_CTRL,
				.ResetAction = PM_RESET_ACTION_PULSE,
				.ResetPulseWait = 0U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
	},
	{
		.NodeId = NODE_DP,
		.RstActionList= {
			{
				.ResetId = PM_RESET_DP,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
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
	{
		.NodeId = NODE_GDMA,
		.RstActionList= {
			{
				.ResetId = PM_RESET_GDMA,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ZDMA_8
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeZdmaIdle,
		.IdleHookArgs = XPMU_ZDMA_8_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_ADMA,
		.RstActionList= {
			{
				.ResetId = PM_RESET_ADMA,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_ZDMA_0
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeZdmaIdle,
		.IdleHookArgs = XPMU_ZDMA_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_CAN_0,
		.RstActionList= {
			{
				.ResetId = PM_RESET_CAN0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_CAN_0
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeCanIdle,
		.IdleHookArgs = XPMU_CAN_0_BASEADDR
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_CAN_1,
		.RstActionList= {
			{
				.ResetId = PM_RESET_CAN1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPMU_CAN_1
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeCanIdle,
		.IdleHookArgs = XPMU_CAN_1_BASEADDR,
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_NAND,
		.RstActionList= {
			{
				.ResetId = PM_RESET_NAND,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
#ifdef XPAR_XNANDPSU_0_DEVICE_ID
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NodeNandIdle,
		.IdleHookArgs = XPAR_XNANDPSU_0_BASEADDR,
#else
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
		.IdleHook = NULL,
		.IdleHookArgs = 0U
#endif
	},
	{
		.NodeId = NODE_GPU,
		.RstActionList = {
			{
				.ResetId = PM_RESET_GPU,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
#ifdef XPAR_PSU_GPU_S_AXI_BASEADDR
		.IdleHook = NodeGpuIdle,
		.IdleHookArgs = XPAR_PSU_GPU_S_AXI_BASEADDR,
#else
		.IdleHook = NULL,
		.IdleHookArgs = 0U,
#endif
	},
	{
		.NodeId = NODE_GPU_PP_0,
		.RstActionList = {
			{
				.ResetId = PM_RESET_GPU_PP0,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
#ifdef XPAR_PSU_GPU_S_AXI_BASEADDR
		.IdleHook = NodeGpuPPIdle,
		.IdleHookArgs = XPAR_PSU_GPU_S_AXI_BASEADDR +
				GPU_PP_0_OFFSET,
#else
		.IdleHook = NULL,
		.IdleHookArgs = 0U,
#endif
	},
	{
		.NodeId = NODE_GPU_PP_1,
		.RstActionList = {
			{
				.ResetId = PM_RESET_GPU_PP1,
				.ResetAction = PM_RESET_ACTION_ASSERT,
				.ResetPulseWait = 10U,
			},
			{0U,0U,0U},
			{0U,0U,0U},
		},
		.SoftRst = NULL,
		.SoftRstArgs = 0U,
#ifdef XPAR_PSU_GPU_S_AXI_BASEADDR
		.IdleHook = NodeGpuPPIdle,
		.IdleHookArgs = XPAR_PSU_GPU_S_AXI_BASEADDR +
				GPU_PP_1_OFFSET,
#else
		.IdleHook = NULL,
		.IdleHookArgs = 0U,
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

	for (Index = 0U; Index < ARRAY_SIZE(NodeRstData); Index++) {
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

	if (NULL == RstInfo) {
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

		if (RstInfo->IdleHook != NULL) {
			RstInfo->IdleHook(RstInfo->IdleHookArgs);
		}

		if (RstInfo->SoftRst != NULL) {
			RstInfo->SoftRst(RstInfo->SoftRstArgs);
		}

	}

	/*
	 * Perform the Node reset using Reset Line for the
	 * node and its corresponding actions
	 */
	for (Index = 0U; Index < MAX_RST_ACTION; Index++) {

		if (RstInfo->RstActionList[Index].ResetId == 0U) {
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

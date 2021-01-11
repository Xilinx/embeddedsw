/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Implementation of individual node idle function and inclusion of
 * driver header depending on the availability of the IP in the design
 *********************************************************************/

#ifndef PM_NODE_IDLE_H_
#define PM_NODE_IDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xpfw_default.h"

#if defined(XPMU_XTTCPS_0) || \
	defined(XPMU_XTTCPS_3) || \
	defined(XPMU_XTTCPS_6) || \
	defined(XPMU_XTTCPS_9)
	#include <xttcps_hw.h>
void NodeTtcIdle(u32 BaseAddress);
#endif

#if defined(XPMU_ETHERNET_0) || \
	defined(XPMU_ETHERNET_1) || \
	defined(XPMU_ETHERNET_2) || \
	defined(XPMU_ETHERNET_3)
	#include <xemacps_hw.h>
void NodeGemIdle(u32 BaseAddress);
#endif

#if defined(XPMU_UART_0) || \
	defined(XPMU_UART_1)
	#include <xuartps_hw.h>
#endif

#if defined(XPMU_SPI_0) || \
	defined(XPMU_SPI_1)
	#include <xspips_hw.h>
#endif

#if defined(XPMU_I2C_0) || \
	defined(XPMU_I2C_1)
	#include <xiicps_hw.h>
void NodeI2cIdle(u32 BaseAddress);
#endif

#if defined(XPMU_SD_0) || \
	defined(XPMU_SD_1)
	#include <xsdps_hw.h>
void NodeSdioIdle(u32 BaseAddress);
#endif

#ifdef XPAR_XQSPIPSU_0_DEVICE_ID
	#include <xqspipsu_hw.h>
void NodeQspiIdle(u32 BaseAddress);
#endif

#ifdef XPAR_XGPIOPS_0_DEVICE_ID
	#include <xgpiops_hw.h>
#endif

#if defined(XPMU_USB_0) || \
	defined(XPMU_USB_1)
#include "xusbpsu.h"
#include "xusbpsu_endpoint.h"
void NodeUsbIdle(u32 BaseAddress);
#endif

#ifdef XPAR_XDPPSU_0_DEVICE_ID
#ifdef XPAR_XDPDMA_0_DEVICE_ID
#include "xdpdma_hw.h"
#endif
#include "xdppsu_hw.h"
void NodeDpIdle(u32 BaseAddress);
#endif

#ifdef XPAR_PSU_SATA_S_AXI_BASEADDR
void NodeSataIdle(u32 BaseAddress);
#endif

#if defined(XPMU_ZDMA_8) || \
	defined(XPMU_ZDMA_0)
#include "xzdma_hw.h"
void NodeZdmaIdle(u32 BaseAddress);
/* Total number of channels and offset per DMA */

#define XZDMA_CH_OFFSET		0X10000
#define XZDMA_NUM_CHANNEL		8U	/* Number of channels */
#endif

#if defined(XPMU_CAN_0) || \
	defined(XPMU_CAN_1)
#include "xcanps_hw.h"
void NodeCanIdle(u32 BaseAddress);
#endif

#if defined(XPAR_XNANDPSU_0_DEVICE_ID)
#include "xnandpsu.h"
void NodeNandIdle(u32 BaseAddress);
#endif

#ifdef XPAR_PSU_GPU_S_AXI_BASEADDR
#define GPU_PP_0_OFFSET		0x8000
#define GPU_PP_1_OFFSET		0xA000
void NodeGpuIdle(u32 BaseAddress);
void NodeGpuPPIdle(u32 BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_NODE_IDLE_H_ */

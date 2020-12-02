/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
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

#if defined(XPAR_PSU_TTC_0_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_3_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_6_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_9_DEVICE_ID)
	#include <xttcps_hw.h>
void NodeTtcIdle(u32 BaseAddress);
#endif

#if defined(XPAR_PSU_ETHERNET_0_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_1_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_2_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_3_DEVICE_ID)
	#include <xemacps_hw.h>
void NodeGemIdle(u32 BaseAddress);
#endif

#if defined(XPAR_PSU_UART_0_DEVICE_ID) || \
	defined(XPAR_PSU_UART_1_DEVICE_ID)
	#include <xuartps_hw.h>
#endif

#if defined(XPAR_PSU_SPI_0_DEVICE_ID) || \
	defined(XPAR_PSU_SPI_1_DEVICE_ID)
	#include <xspips_hw.h>
#endif

#if defined(XPAR_PSU_I2C_0_DEVICE_ID) || \
	defined(XPAR_PSU_I2C_1_DEVICE_ID)
	#include <xiicps_hw.h>
void NodeI2cIdle(u32 BaseAddress);
#endif

#if defined(XPAR_PSU_SD_0_DEVICE_ID) || \
	defined(XPAR_PSU_SD_1_DEVICE_ID)
	#include <xsdps_hw.h>
void NodeSdioIdle(u32 BaseAddress);
#endif

#ifdef XPAR_PSU_QSPI_0_DEVICE_ID
	#include <xqspipsu_hw.h>
void NodeQspiIdle(u32 BaseAddress);
#endif

#ifdef XPAR_PSU_GPIO_0_DEVICE_ID
	#include <xgpiops_hw.h>
#endif

#if defined(XPAR_XUSBPSU_0_DEVICE_ID) || \
	defined(XPAR_XUSBPSU_1_DEVICE_ID)
#include "xusbpsu.h"
#include "xusbpsu_endpoint.h"
void NodeUsbIdle(u32 BaseAddress);
#endif

#ifdef XPAR_XDPPSU_0_DEVICE_ID
#ifdef XPAR_PSU_DPDMA_DEVICE_ID
#include "xdpdma_hw.h"
#endif
#include "xdppsu_hw.h"
void NodeDpIdle(u32 BaseAddress);
#endif

#ifdef XPAR_PSU_SATA_S_AXI_BASEADDR
void NodeSataIdle(u32 BaseAddress);
#endif

#if defined(XPAR_PSU_ZDMA_0_DEVICE_ID) || \
	defined(XPAR_PSU_ADMA_0_DEVICE_ID)
#include "xzdma_hw.h"
void NodeZdmaIdle(u32 BaseAddress);
/* Total number of channels and offset per DMA */

#define XZDMA_CH_OFFSET		0X10000
#define XZDMA_NUM_CHANNEL		8U	/* Number of channels */
#endif

#if defined(XPAR_PSU_CAN_0_DEVICE_ID) || \
	defined(XPAR_PSU_CAN_1_DEVICE_ID)
#include "xcanps_hw.h"
void NodeCanIdle(u32 BaseAddress);
#endif

#if defined(XPAR_PSU_NAND_0_DEVICE_ID)
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

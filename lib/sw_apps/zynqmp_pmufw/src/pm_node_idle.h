/*
 * Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
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

#ifdef __cplusplus
}
#endif

#endif /* PM_NODE_IDLE_H_ */

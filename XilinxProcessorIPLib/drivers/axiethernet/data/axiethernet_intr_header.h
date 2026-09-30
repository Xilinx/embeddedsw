/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



/**
 * @file axiethernet_intr_header.h
 * @brief Declares the AXI Ethernet SGDMA interrupt example entry point.
 */

#include "xstatus.h"
#include "xil_types.h"

#include "xparameters.h"

#include "xaxidma.h"
#include "xaxiethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc	/**< Interrupt controller type for this example. */
#else
#define INTC		XScuGic	/**< Interrupt controller type for this example. */
#endif

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Runs the AXI Ethernet SGDMA interrupt example.
 *
 * @param[in] IntcInstancePtr is a parameter of the example entry point.
 * @param[in] AxiEthernetInstancePtr is a parameter of the example entry point.
 * @param[in] DmaInstancePtr is a parameter of the example entry point.
 * @param[in] AxiEthernetDeviceId is a parameter of the example entry point.
 * @param[in] AxiDmaDeviceId is a parameter of the example entry point.
 * @param[in] AxiEthernetIntrId is a parameter of the example entry point.
 * @param[in] DmaRxIntrId is a parameter of the example entry point.
 * @param[in] DmaTxIntrId is a parameter of the example entry point.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int AxiEthernetSgDmaIntrExample(INTC * IntcInstancePtr,
                          XAxiEthernet * AxiEthernetInstancePtr,
                          XAxiDma * DmaInstancePtr,
                          u16 AxiEthernetDeviceId,
                          u16 AxiDmaDeviceId, u16 AxiEthernetIntrId,
                          u16 DmaRxIntrId, u16 DmaTxIntrId);
#else
/*****************************************************************************/
/**
 * @brief Runs the AXI Ethernet SGDMA interrupt example.
 *
 * @param[in] AxiEthernetInstancePtr is a parameter of the example entry point.
 * @param[in] DmaInstancePtr is a parameter of the example entry point.
 * @param[in] AxiEthernetBaseAddress is a parameter of the example entry point.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int AxiEthernetSgDmaIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				UINTPTR AxiEthernetBaseAddress);
#endif

#ifdef __cplusplus
}
#endif

/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file csudma_header.h
 * @brief Declares the CSU DMA example entry points.
 */
#ifndef CSUDMA_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the CSU DMA self-test example header. */
#define CSUDMA_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the CSU DMA self-test example.
 *
 * @if SDT
 * @param[in] BaseAddress is the base address of the CSU DMA instance.
 * @else
 * @param[in] DeviceId is the unique device ID of the CSU DMA.
 * @endif
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifdef SDT
int XCsuDma_SelfTestExample(UINTPTR BaseAddress);
int XCsuDma_IntrExample(XCsuDma *CsuDmaInstance, UINTPTR BaseAddress);
#else
int XCsuDma_SelfTestExample(u16 DeviceId);
#ifdef XPAR_SCUGIC_0_DEVICE_ID
int XCsuDma_IntrExample(XScuGic *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
int XCsuDma_IntrExample(XIntc *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId);
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif

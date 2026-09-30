#ifndef AXIDMA_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the AXI DMA self-test example header. */
#define AXIDMA_HEADER_H		/* by using protection macros */

/**
 * @file axidma_header.h
 * @brief Declares the AXI DMA self-test example entry point.
 */
/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the AXI DMA self-test example.
 *
 * @if SDT
 * @param[in] BaseAddress is the base address of the AXI DMA instance.
 * @else
 * @param[in] DeviceId is the unique device ID of the AXI DMA.
 * @endif
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifndef SDT
int AxiDMASelfTestExample(u16 DeviceId);
#else
int AxiDMASelfTestExample(u32 BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif

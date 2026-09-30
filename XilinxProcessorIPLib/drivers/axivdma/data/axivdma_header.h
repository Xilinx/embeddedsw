/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file axivdma_header.h
 * @brief Declares the AXI VDMA self-test example entry point.
 */
#ifndef AXIVDMA_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the AXI VDMA self-test example header. */
#define AXIVDMA_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the AXI VDMA self-test example.
 *
 * @if SDT
 * @param[in] BaseAddress is the base address of the AXI VDMA instance.
 * @else
 * @param[in] DeviceId is the unique device ID of the AXI VDMA.
 * @endif
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifndef SDT
int AxiVDMASelfTestExample(u16 DeviceId);
#else
int AxiVDMASelfTestExample(UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif

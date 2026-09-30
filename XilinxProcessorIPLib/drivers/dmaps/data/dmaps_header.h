/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file dmaps_header.h
 * @brief Declares the DMAPS example entry points.
 */
#ifndef DMAPS_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the DMAPS example header. */
#define DMAPS_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the DMAPS interrupt example.
 *
 * @if SDT
 * @param[in] DmapsInstPtr is a pointer to the XDmaPs instance.
 * @param[in] BaseAddress is the base address of the DMAPS device.
 * @else
 * @param[in] GicPtr is a pointer to the interrupt controller instance.
 * @param[in] DeviceId is the unique device ID of the DMAPS device.
 * @endif
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifndef SDT
int XDmaPs_Example_W_Intr(XScuGic *GicPtr, u16 DeviceId);
#else
int XDmaPs_Example_W_Intr(XDmaPs *DmapsInstPtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif


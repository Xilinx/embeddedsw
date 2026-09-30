/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file emacps_header.h
 * @brief Declares the EmacPs example entry points.
 */
#ifndef EMACPS_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the EmacPs example header. */
#define EMACPS_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the EmacPs DMA interrupt example.
 *
 * @if SDT
 * @param[in] EmacPsInstancePtr is a pointer to the XEmacPs instance.
 * @param[in] BaseAddress is the base address of the EmacPs device.
 * @else
 * @param[in] IntcInstancePtr is a pointer to the interrupt controller instance.
 * @param[in] EmacPsInstancePtr is a pointer to the XEmacPs instance.
 * @param[in] EmacPsDeviceId is the unique device ID of the EmacPs device.
 * @endif
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifdef SDT
int EmacPsDmaIntrExample(XEmacPs *EmacPsInstancePtr, UINTPTR BaseAddress);
#elif XPAR_INTC_0_DEVICE_ID
int EmacPsDmaIntrExample(XIntc *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId);
#else
int EmacPsDmaIntrExample(XScuGic *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId);
#endif

#ifdef __cplusplus
}
#endif

#endif

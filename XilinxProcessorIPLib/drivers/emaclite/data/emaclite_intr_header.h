/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file emaclite_intr_header.h
 * @brief Declares the EmacLite interrupt example entry point.
 */
#ifndef EMACLITE_INTR_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the EmacLite interrupt example header. */
#define EMACLITE_INTR_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"


/*****************************************************************************/
/**
 * @brief Runs the EmacLite interrupt example.
 *
 * @param[in] IntcInstancePtr is a pointer to the interrupt controller instance.
 * @param[in] EmacLiteInstPtr is a pointer to the XEmacLite instance.
 * @param[in] EmacLiteDeviceId is the unique device ID of the EmacLite device.
 * @param[in] EmacLiteIntrId is the interrupt ID of the EmacLite device.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
#ifdef XPAR_INTC_0_DEVICE_ID
int EmacLiteIntrExample(XIntc* IntcInstancePtr,
                        XEmacLite* EmacLiteInstPtr,
                        u16 EmacLiteDeviceId,
                        u16 EmacLiteIntrId);
#else
int EmacLiteIntrExample(XScuGic* IntcInstancePtr,
                        XEmacLite* EmacLiteInstPtr,
                        u16 EmacLiteDeviceId,
                        u16 EmacLiteIntrId);

#endif

#ifdef __cplusplus
}
#endif

#endif

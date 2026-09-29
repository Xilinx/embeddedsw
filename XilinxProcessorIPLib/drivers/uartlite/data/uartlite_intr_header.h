/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTLITE_INTR_HEADER_H
#define UARTLITE_INTR_HEADER_H

/*****************************************************************************/
/**
*
* @file uartlite_intr_header.h
*
* @brief Declares prototypes for the UartLite interrupt example.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
#ifdef XPAR_INTC_0_DEVICE_ID
int UartLiteIntrExample(XIntc* IntcInstancePtr, \
                            XUartLite* UartLiteInstancePtr, \
                            u16 UartLiteDeviceId, \
                            u16 UartLiteIntrId);
#else
int UartLiteIntrExample(XScuGic* IntcInstancePtr, \
                            XUartLite* UartLiteInstancePtr, \
                            u16 UartLiteDeviceId, \
                            u16 UartLiteIntrId);

#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif

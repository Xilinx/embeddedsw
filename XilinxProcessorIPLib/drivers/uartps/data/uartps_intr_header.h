/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPS_INTR_HEADER_H
#define UARTPS_INTR_HEADER_H

/*****************************************************************************/
/**
*
* @file uartps_intr_header.h
*
* @brief Declares prototypes for the UartPs interrupt example.
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
int UartPsIntrExample(XIntc *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#else
int UartPsIntrExample(XScuGic *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif

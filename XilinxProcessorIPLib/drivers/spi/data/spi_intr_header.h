/******************************************************************************
* Copyright (C) 2008 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SPI_INTR_HEADER_H
#define SPI_INTR_HEADER_H

/******************************************************************************/
/**
*
* @file spi_intr_header.h
*
* @brief Declares prototypes for the SPI driver examples.
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
int SpiIntrExample(XIntc *IntcInstancePtr, \
                       XSpi *SpiInstancePtr, \
                       u16 SpiDeviceId, \
                       u16 SpiIntrId);
#else
int SpiIntrExample(XScuGic *IntcInstancePtr, \
                       XSpi *SpiInstancePtr, \
                       u16 SpiDeviceId, \
                       u16 SpiIntrId);

#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif

/******************************************************************************
* Copyright (C) 2007 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SPI_HEADER_H
#define SPI_HEADER_H

/******************************************************************************/
/**
*
* @file spi_header.h
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
#ifndef SDT
int SpiSelfTestExample(u16 DeviceId);
#else
int SpiSelfTestExample(UINTPTR BaseAddress);
int SpiIntrExample(XSpi *SpiInstancePtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif


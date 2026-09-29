/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef GPIO_INTR_HEADER_H
#define GPIO_INTR_HEADER_H

/*****************************************************************************/
/**
*
* @file gpio_intr_header.h
*
* @brief Declares prototypes for the GPIO interrupt example.
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
int GpioIntrExample(XIntc* IntcInstancePtr,
                        XGpio* InstancePtr,
                        u16 DeviceId,
                        u16 IntrId,
                        u16 IntrMask,
                        u32 *DataRead);
#else
int GpioIntrExample(XScuGic* IntcInstancePtr,
                        XGpio* InstancePtr,
                        u16 DeviceId,
                        u16 IntrId,
                        u16 IntrMask,
                        u32 *DataRead);

#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif


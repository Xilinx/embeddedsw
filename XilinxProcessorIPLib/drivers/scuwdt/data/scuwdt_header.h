/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUWDT_HEADER_H
#define SCUWDT_HEADER_H

/*****************************************************************************/
/**
*
* @file scuwdt_header.h
*
* @brief Declares prototypes for the SCUWDT driver examples.
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
int ScuWdtIntrExample(XScuGic *IntcInstancePtr, XScuWdt *WdtInstancePtr,
		   u16 WdtDeviceId, u16 WdtIntrId);
#else
int ScuWdtIntrExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress);
int ScuWdtPolledExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif

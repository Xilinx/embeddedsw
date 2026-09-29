/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file bram_intr_header.h
*
* @brief Declares prototypes for the BRAM driver examples.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
int BramIntrExample(XIntc* IntcInstancePtr,
		    XBram* InstancePtr,
		    u16 DeviceId,
		    u16 IntrId);
/** @endcond */

#ifdef __cplusplus
}
#endif

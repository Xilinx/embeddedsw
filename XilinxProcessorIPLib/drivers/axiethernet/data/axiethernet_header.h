/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



/**
 * @file axiethernet_header.h
 * @brief Declares the AXI Ethernet polled example entry point.
 */

#include "xstatus.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/**
 * @brief Runs the AXI Ethernet polled example.
 *
 * @param[in] AxiEthernetDeviceId is a parameter of the example entry point.
 * @param[in] FifoDeviceId is a parameter of the example entry point.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int AxiEthernetPolledExample(u16 AxiEthernetDeviceId, u16 FifoDeviceId);

#ifdef __cplusplus
}
#endif

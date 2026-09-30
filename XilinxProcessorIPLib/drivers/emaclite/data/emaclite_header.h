/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file emaclite_header.h
 * @brief Declares the EmacLite polled example entry point.
 */
#ifndef EMACLITE_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the EmacLite polled example header. */
#define EMACLITE_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/*****************************************************************************/
/**
 * @brief Runs the EmacLite polled example.
 *
 * @param[in] DeviceId is the unique device ID of the EmacLite device.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int EmacLitePolledExample(u16 DeviceId);

#ifdef __cplusplus
}
#endif

#endif

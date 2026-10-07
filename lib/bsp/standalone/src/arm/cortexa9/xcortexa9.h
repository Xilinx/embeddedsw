/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexa9.h
* @brief Cortex-A9 processor register definitions and hardware configuration.
*/

#ifndef XIL_XCORTEXA9_H_ /* prevent circular inclusions */
#define XIL_XCORTEXA9_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"

/**************************** Type Definitions ******************************/
/** @brief Cortex-A9 processor configuration structure. */
typedef struct {
		u32 CpuFreq; /**< CPU frequency in Hz. */
} XCortexa9_Config;

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXA9_H_ */

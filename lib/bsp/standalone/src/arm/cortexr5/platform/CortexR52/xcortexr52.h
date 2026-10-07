/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexr52.h
* @brief Cortex-R52 processor register definitions and hardware configuration.
*/

#ifndef XIL_XCORTEXR52_H_
#define XIL_XCORTEXR52_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xil_types.h"

/**************************** Type Definitions ******************************/
/** @brief Cortex-R52 processor configuration parameters. */
typedef struct {
		u32 TimestampFreq; /**< Frequency of the ARM generic timer in Hz. */
		u32 CpuFreq; /**< CPU clock frequency in Hz. */
		u8 CpuId;	/**< CPU Number */
} XCortexr52_Config;
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXR52_H_ */

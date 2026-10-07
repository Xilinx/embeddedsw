/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexr5.h
* @brief Cortex-R5 processor register definitions and hardware configuration.
*/

#ifndef XIL_XCORTEXR5_H_
#define XIL_XCORTEXR5_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xil_types.h"

/**************************** Type Definitions ******************************/
/** @brief Cortex-R5 processor configuration parameters. */
typedef struct {
		u32 AccessVal; /**< Processor access attributes; bit 0 indicates the TrustZone setting for the RPU address space. */
		u32 CpuFreq; /**< CPU clock frequency in Hz. */
		u8 CpuId;	/**< CPU Number */
} XCortexr5_Config;
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXR5_H_ */

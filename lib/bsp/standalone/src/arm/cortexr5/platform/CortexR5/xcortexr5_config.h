/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexr5_config.h
* @brief Cortex-R5 processor configuration parameters.
*/

#ifndef XIL_XCORTEXR5_CONFIG_H_
#define XIL_XCORTEXR5_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xcortexr5.h"

/************************** Variable Definitions ****************************/
extern XCortexr5_Config XCortexr5_ConfigTable; /**< Cortex-R5 processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the Cortex-R5 CPU clock frequency in Hz. */
#define XGet_CpuFreq() XCortexr5_ConfigTable.CpuFreq
/** @brief Get the Cortex-R5 CPU instance identifier. */
#define XGet_CpuId() XCortexr5_ConfigTable.CpuId
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXR5_CONFIG_H_ */

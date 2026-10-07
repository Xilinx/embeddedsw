/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexr52_config.h
* @brief Cortex-R52 processor configuration parameters.
*/

#ifndef XIL_XCORTEXR52_CONFIG_H_
#define XIL_XCORTEXR52_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xcortexr52.h"

/************************** Variable Definitions ****************************/
extern XCortexr52_Config XCortexr52_ConfigTable; /**< Cortex-R52 processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the Cortex-R52 CPU clock frequency in Hz. */
#define XGet_CpuFreq() XCortexr52_ConfigTable.CpuFreq
/** @brief Get the Cortex-R52 CPU instance identifier. */
#define XGet_CpuId() XCortexr52_ConfigTable.CpuId
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXR52_CONFIG_H_ */

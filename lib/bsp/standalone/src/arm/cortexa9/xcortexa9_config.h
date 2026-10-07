/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xcortexa9_config.h
* @brief Cortex-A9 processor configuration parameters.
*/

#ifndef XIL_XCORTEXA9_CONFIG_H_ /* prevent circular inclusions */
/** @brief Include guard for xcortexa9_config.h. */
#define XIL_XCORTEXA9_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xcortexa9.h"

/************************** Variable Definitions ****************************/
extern XCortexa9_Config XCortexa9_ConfigTable; /**< Cortex-A9 processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the Cortex-A9 CPU clock frequency in Hz. */
#define XGet_CpuFreq() XCortexa9_ConfigTable.CpuFreq

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXA9_CONFIG_H_ */

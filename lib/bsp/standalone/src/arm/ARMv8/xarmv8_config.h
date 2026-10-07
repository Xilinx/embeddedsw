/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xarmv8_config.h
* @brief ARMv8 processor configuration parameters.
*/

#ifndef XIL_XARMV8_CONFIG_H_
#define XIL_XARMV8_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xarmv8.h"

/************************** Variable Definitions ****************************/
extern XARMv8_Config XARMv8_ConfigTable; /**< ARM v8 processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the ARM generic timer frequency in Hz. */
#define XGet_TimeStampFreq() XARMv8_ConfigTable.TimestampFreq
/** @brief Get the CPU clock frequency in Hz. */
#define XGet_CpuFreq() XARMv8_ConfigTable.CpuFreq
/** @brief Get the CPU instance identifier. */
#define XGet_CpuId() XARMv8_ConfigTable.CpuId
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XARMV8_CONFIG_H_ */

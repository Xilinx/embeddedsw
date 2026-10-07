/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xmicroblaze_config.h
* @brief MicroBlaze processor configuration parameters.
*/

#ifndef _XMICROBLAZE_CONFIG_H_
#define _XMICROBLAZE_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xmicroblaze.h"

/************************** Variable Definitions ****************************/
extern XMicroblaze_Config XMicroblaze_ConfigTable; /**< MicroBlaze processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the MicroBlaze CPU clock frequency in Hz. */
#define XGet_CpuFreq() XMicroblaze_ConfigTable.CpuFreq
/** @brief Get the MicroBlaze CPU instance identifier. */
#define XGet_CpuId() XMicroblaze_ConfigTable.CpuId
/** @brief Get the DDR start address. */
#define XGet_DdrSa() XMicroblaze_ConfigTable.DdrSa
/** @brief Get pointer to the MicroBlaze CPU configuration structure. */
#define XGet_CpuCfgPtr() &XMicroblaze_ConfigTable

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_CONFIG_H_

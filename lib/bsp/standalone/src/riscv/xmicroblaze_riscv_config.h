/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef _XMICROBLAZE_RISCV_CONFIG_H_  /**< prevent circular inclusions */
#define _XMICROBLAZE_RISCV_CONFIG_H_  /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xmicroblaze_riscv.h"

/************************** Variable Definitions ****************************/
extern XMicroblaze_RISCV_Config XMicroblaze_RISCV_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XMicroblaze_RISCV_ConfigTable.CpuFreq
#define XGet_CpuId() XMicroblaze_RISCV_ConfigTable.CpuId
#define XGet_DdrSa() XMicroblaze_RISCV_ConfigTable.DdrSa
#define XGet_CpuCfgPtr() &XMicroblaze_RISCV_ConfigTable

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_RISCV_CONFIG_H_

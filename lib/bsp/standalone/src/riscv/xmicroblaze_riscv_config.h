/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xmicroblaze_riscv_config.h
* @brief MicroBlaze RISC-V processor configuration parameters.
*/

#ifndef _XMICROBLAZE_RISCV_CONFIG_H_
#define _XMICROBLAZE_RISCV_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xmicroblaze_riscv.h"

/************************** Variable Definitions ****************************/
extern XMicroblaze_RISCV_Config XMicroblaze_RISCV_ConfigTable; /**< MicroBlaze RISC-V processor configuration table instance. */

/***************** Macros (Inline Functions) Definitions ********************/
/** @brief Get the MicroBlaze RISC-V CPU clock frequency in Hz. */
#define XGet_CpuFreq() XMicroblaze_RISCV_ConfigTable.CpuFreq
/** @brief Get the MicroBlaze RISC-V CPU instance identifier. */
#define XGet_CpuId() XMicroblaze_RISCV_ConfigTable.CpuId
/** @brief Get the DDR start address. */
#define XGet_DdrSa() XMicroblaze_RISCV_ConfigTable.DdrSa
/** @brief Get pointer to the MicroBlaze RISC-V CPU configuration structure. */
#define XGet_CpuCfgPtr() &XMicroblaze_RISCV_ConfigTable

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_RISCV_CONFIG_H_

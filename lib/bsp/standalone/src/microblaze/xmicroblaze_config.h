/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef _XMICROBLAZE_CONFIG_H_  /**< prevent circular inclusions */
#define _XMICROBLAZE_CONFIG_H_  /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xmicroblaze.h"

/************************** Variable Definitions ****************************/
extern XMicroblaze_Config XMicroblaze_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XMicroblaze_ConfigTable.CpuFreq
#define XGet_CpuId() XMicroblaze_ConfigTable.CpuId
#define XGet_DdrSa() XMicroblaze_ConfigTable.DdrSa
#define XGet_CpuCfgPtr() &XMicroblaze_ConfigTable

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_CONFIG_H_

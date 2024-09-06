/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXA9_CONFIG_H_ /* prevent circular inclusions */
#define XIL_XCORTEXA9_CONFIG_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xcortexa9.h"

/************************** Variable Definitions ****************************/
extern XCortexa9_Config XCortexa9_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XCortexa9_ConfigTable.CpuFreq

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXA9_CONFIG_H_ */

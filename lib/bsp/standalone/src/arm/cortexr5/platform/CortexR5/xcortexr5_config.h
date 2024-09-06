/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXR5_CONFIG_H_ /**< prevent circular inclusions */
#define XIL_XCORTEXR5_CONFIG_H_ /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xcortexr5.h"

/************************** Variable Definitions ****************************/
extern XCortexr5_Config XCortexr5_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XCortexr5_ConfigTable.CpuFreq
#define XGet_CpuId() XCortexr5_ConfigTable.CpuId
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXR5_CONFIG_H_ */

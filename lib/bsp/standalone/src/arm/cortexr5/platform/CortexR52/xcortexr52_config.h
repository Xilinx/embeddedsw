/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXR52_CONFIG_H_ /**< prevent circular inclusions */
#define XIL_XCORTEXR52_CONFIG_H_ /**< by using protection macros */

/***************************** Include Files ********************************/
#ifdef SDT
#include "xcortexr52.h"

/************************** Variable Definitions ****************************/
extern XCortexr52_Config XCortexr52_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XCortexr52_ConfigTable.CpuFreq
#define XGet_CpuId() XCortexr52_ConfigTable.CpuId
#endif
#endif /* XIL_XCORTEXR52_CONFIG_H_ */

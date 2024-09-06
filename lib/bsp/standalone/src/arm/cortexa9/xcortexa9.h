/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXA9_H_ /* prevent circular inclusions */
#define XIL_XCORTEXA9_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"

/**************************** Type Definitions ******************************/
typedef struct {
		u32 CpuFreq;
} XCortexa9_Config;

#ifdef __cplusplus
}
#endif

#endif /* XIL_XCORTEXA9_H_ */

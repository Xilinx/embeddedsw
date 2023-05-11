/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XARMV8_H_ /**< prevent circular inclusions */
#define XIL_XARMV8_H_ /**< by using protection macros */

/***************************** Include Files ********************************/
#ifdef SDT
#include "xil_types.h"

/**************************** Type Definitions ******************************/
typedef struct {
		u32 TimestampFreq;
		u32 CpuFreq;
		u32 CpuId;	/* CPU Number */
} XARMv8_Config;
#endif
#endif /* XIL_XARMV8_H_ */

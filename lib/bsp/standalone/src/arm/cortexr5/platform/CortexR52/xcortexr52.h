/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXR52_H_ /**< prevent circular inclusions */
#define XIL_XCORTEXR52_H_ /**< by using protection macros */

/***************************** Include Files ********************************/
#ifdef SDT
#include "xil_types.h"

/**************************** Type Definitions ******************************/
typedef struct {
		u32 TimestampFreq;
		u32 CpuFreq;
		u8 CpuId;	/* CPU Number */
} XCortexr52_Config;
#endif
#endif /* XIL_XCORTEXR52_H_ */

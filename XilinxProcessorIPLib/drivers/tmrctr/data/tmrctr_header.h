/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRCTR_HEADER_H		/* prevent circular inclusions */
#define TMRCTR_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int TmrCtrSelfTestExample(u16 DeviceId);
#else
int TmrCtrSelfTestExample(UINTPTR BaseAddr);
int TmrCtrIntrExample(XTmrCtr *InstancePtr,
		      UINTPTR BaseAddr);
#endif

#endif


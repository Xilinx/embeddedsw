/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRCTR_HEADER_H		/* prevent circular inclusions */
#define TMRCTR_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int TmrCtrSelfTestExample(u16 DeviceId, u8 TmrCtrNumber);
#else
int TmrCtrSelfTestExample(UINTPTR BaseAddr, u8 TmrCtrNumber);
int TmrCtrIntrExample(XTmrCtr *InstancePtr,
		        UINTPTR BaseAddr,
                        u8 TmrCtrNumber);
#endif

#endif


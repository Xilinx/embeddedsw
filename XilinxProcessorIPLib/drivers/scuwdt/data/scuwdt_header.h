/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUWDT_HEADER_H		/* prevent circular inclusions */
#define SCUWDT_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int ScuWdtIntrExample(XScuGic *IntcInstancePtr, XScuWdt *WdtInstancePtr,
		   u16 WdtDeviceId, u16 WdtIntrId);


#endif

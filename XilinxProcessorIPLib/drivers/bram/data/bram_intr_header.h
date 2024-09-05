/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"


int BramIntrExample(XIntc* IntcInstancePtr,
		    XBram* InstancePtr,
		    u16 DeviceId,
		    u16 IntrId);

#ifdef __cplusplus
}
#endif

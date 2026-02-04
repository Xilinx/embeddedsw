/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
XStatus IOModuleSelfTestExample(u16 DeviceId);
#else
XStatus IOModuleSelfTestExample(u32 DeviceId);
#endif

#ifdef SDT
XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr,
                            u16 DeviceId);
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
                               u16 DeviceId);
#endif

#ifdef __cplusplus
}
#endif

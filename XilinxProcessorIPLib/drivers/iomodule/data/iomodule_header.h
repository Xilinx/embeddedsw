/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

XStatus IOModuleSelfTestExample(u16 DeviceId);

#ifdef SDT
XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr,
                            u16 DeviceId);
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
                               u16 DeviceId);
#endif

#ifdef __cplusplus
}
#endif

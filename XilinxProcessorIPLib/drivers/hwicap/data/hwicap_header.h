/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef HWICAP_HEADER_H		/* prevent circular inclusions */
#define HWICAP_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include <xil_types.h>
#include <xil_assert.h>
#include "xstatus.h"

XStatus HwIcapTestAppExample(u16 DeviceId);

#ifdef __cplusplus
}
#endif
#endif

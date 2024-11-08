/******************************************************************************
* Copyright (c) 2008 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform_config.h"

void init_platform();
void cleanup_platform();

#ifdef __cplusplus
}
#endif

#endif

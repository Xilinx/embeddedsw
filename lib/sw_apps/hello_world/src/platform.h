/******************************************************************************
* Copyright (C) 2023  - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDT
#include "platform_config.h"
#endif

void init_platform();
void cleanup_platform();

#ifdef __cplusplus
}
#endif
#endif

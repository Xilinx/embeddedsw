/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef BSPCONFIG_H  /* prevent circular inclusions */
#define BSPCONFIG_H  /* by using protection macros */

#ifdef __cplusplus
extern "c" {
#endif

#define MICROBLAZE_PVR_NONE
#define EL3 1
#define EL1_NONSECURE 0
#define HYP_GUEST 0

#ifdef __cplusplus
}
#endif

#endif /*end of __BSPCONFIG_H_*/

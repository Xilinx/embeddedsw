/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_RUNTIME_ALLOC_H__
#define __XPM_RUNTIME_ALLOC_H__
#include "xpm_alloc.h"
#ifdef __cplusplus
extern "C" {
#endif

void *XPm_AllocBytesSubsys(u32 SizeInBytes);
void *XPm_AllocBytesReqm(u32 SizeInBytes);
void *XPm_AllocBytesOthers(u32 SizeInBytes);
void *XPm_AllocBytesDevOps(u32 SizeInBytes);
#ifdef __cplusplus
}
#endif
#endif /* __XPM_RUNTIME_ALLOC_H__ */

/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_API_H
#define XPM_RUNTIME_API_H
#include "xstatus.h"
#include "xpm_mem.h"
#include "xpm_requirement.h"

#define PL_MEMORY_REGION	0x1U
#define NONPL_MEMORY_REGION	0x0U

#define PL_NODE_FLAGS_SHIFT	28U
#define PL_NODE_FLAGS_MASK_BITS	0xFU
#define PL_NODE_FLAGS(ID)	(((ID) & PL_NODE_FLAGS_MASK_BITS) >> PL_NODE_FLAGS_SHIFT)

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_GetAddrRegnForSubsystem(const u32 SubsystemId, XPm_AddrRegion *AddrRegnArray,
                                        u32 AddrRegnArrayLen, u32 *NumOfRegions);
XStatus XPm_IsAddressInSubsystem(const u32 SubsystemId, u64 AddressofSubsystem,
                                        u8 *IsValidAddress);

#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_API_H_ */
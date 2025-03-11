/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_MEM_H_
#define XPM_RUNTIME_MEM_H_

#include "xstatus.h"
#include "xpm_mem.h"
#include "xpm_requirement.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PL_MEM_REGN			(0x1U)
#define PL_MEM_REGN_FLAGS_SHIFT_64	(60U)
#define PL_MEM_REGN_FLAGS_MASK_64	((u64)0xF0000000U << 32)
#define PL_MEM_REGN_FLAGS(SZ_64BIT)	((u32)(((SZ_64BIT) & PL_MEM_REGN_FLAGS_MASK_64) >> PL_MEM_REGN_FLAGS_SHIFT_64))
#define IS_PL_MEM_REGN(SZ_64BIT)	((u32)PL_MEM_REGN == PL_MEM_REGN_FLAGS((u64)(SZ_64BIT)))

XStatus XPm_GetAddrRegnForSubsystem(u32 SubsystemId, XPm_AddrRegion *AddrRegnArray,
				    u32 AddrRegnArrayLen, u32 *NumOfRegions);
XStatus XPm_IsAddressInSubsystem(u32 SubsystemId, u64 AddrToCheck, u8 *IsValid);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_MEM_H_ */

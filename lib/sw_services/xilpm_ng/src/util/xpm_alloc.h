/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_ALLOC_H__
#define __XPM_ALLOC_H__
#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_TOPO_POOL_SIZE	(42U * 1024U)

typedef struct XPm_AllocablePool_s {
	u32 Size; /** Maximum size of the pool */
	void* Pool; /** The address of memory of the pool of memory */
	void* FreeMem; /** The address of next free memory in the pool */
} XPm_AllocablePool_t;

void *XPm_AllocBytes(u32 SizeInBytes);
void *XPm_AllocBytesBoard(u32 SizeInBytes);

void *XPm_AllocPool(u32 SizeInBytes, XPm_AllocablePool_t* PoolMem);
void  XPm_DumpMemUsage_Pool(XPm_AllocablePool_t PoolMem);
void XPm_DumpTopologyMemUsage(void);
void XPm_DumpMemUsage(void);
#ifdef __cplusplus
}
#endif
#endif /*__XPM_ALLOC_H__ */

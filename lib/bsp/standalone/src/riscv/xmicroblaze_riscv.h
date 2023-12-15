/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#ifndef _XMICROBLAZE_H_
#define _XMICROBLAZE_H_

#ifdef SDT
#include "xil_types.h"

typedef struct {
	u32 TimebaseFrequency;
        u32 CpuFreq;
	u32 BaseVectorAddr;
	u32 UseMmu;
	u32 UseDcache;
	u32 UseIcache;
	u32 UseMulDiv;
	u32 UseAtomic;
	u32 UseFpu;
	u32 DcacheSize;
        u32 DcacheLineSize;
	u32 DcacheLineLen;
	UINTPTR DcacheBaseaddr;
	UINTPTR DcacheHighaddr;
	u32 IcacheSize;
        u32 IcacheLineSize;
	u32 IcacheLineLen;
	UINTPTR IcacheBaseaddr;
	UINTPTR IcacheHighaddr;
	u8 CpuId;	/* CPU Number */
} XMicroblaze_RISCV_Config;

#endif
#endif // _XMICROBLAZE_H_

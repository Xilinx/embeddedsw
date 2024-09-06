/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#ifndef _XMICROBLAZE_H_
#define _XMICROBLAZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDT
#include "xil_types.h"

typedef struct {
	u32 UnalignedExceptions;
	u32 IopbBusException;
	u32 FpuException;
	u32 FslException;
	u32 DivZeroException;
	u32 DopbBusException;
	u32 IllOpcodeException;
	u32 UseStackProtection;
	u32 ExceptionsInDelaySlots;
	u32 PreDecodeFpuException;
        u32 CpuFreq;
        u32 UseMsrInstr;
        u32 DcacheLineSize;
        u32 DcacheAlwaysUsed;
	u32 DcacheSize;
	u32 IcacheLineSize;
	u32 IcacheSize;
	u32 AddrTagBits;
	u32 AllowDcaheWr;
	u32 AllowIcacheWr;
	u32 AreaOptimized;
	u32 CacheByteSize;
	u32 DLmp;
	u32 DOpb;
	u32 DPlb;
	u32 ILmb;
	u32 IOpb;
        u32 IPlb;
	u32 DcacheAddrTags;
	u32 DcacheByteSize;
	u32 DcacheLineLen;
	u32 DcacheUseFsl;
	u32 DcacheUseWriteback;
	u32 DebugEnabled;
	u32 DynamicBusSize;
        u32 EdgeIsPositive;
	u32 TimebaseFrequency;
	u32 Endianness;
	u32 FslDataSize;
        u32 FslLinks;
	u32 IcacheAlwaysUsed;
	u32 IcacheLineLen;
        u32 IcacheUseFsl;
	u32 Interconnect;
	u32 InterruptIsEdge;
	u32 MmuDtlbSize;
	u32 MmuItlbSize;
	u32 MmuTlbAccess;
	u32 MmuZones;
	u32 NumberOfPcBrk;
	u32 NumberOfRdAddrBrk;
	u32 NumberOfWrAddrBrk;
	u32 Opcode0Illegal;
	u32 Pvr;
	u32 PvrUser1;
	u32 PvrUser2;
	u32 ResetMsr ;
	u32 Sco;
	u32 UseBarrel;
	u32 UseDcache;
	u32 UseDiv;
	u32 UseExtBrk;
	u32 UseExtNmBrk;
	u32 UseExtendedFslInstr;
	u32 UseFpu;
	u32 UseHwMul;
	u32 UseIcache;
	u32 UseInterrupt;
	u32 UseMmu;
        u32 UsePcmpInst;
	UINTPTR DcacheBaseaddr;
	UINTPTR DcacheHighaddr;
	UINTPTR IcacheBaseaddr;
	UINTPTR IcacheHighaddr;
	u32 DdrSa;	/* Valid only for PMC Microblaze */
	u32 DdrEa;	/* Valid only for PMC microblaze */
	u8 CpuId;	/* CPU Number */
	u32 BaseVectorAddr;
} XMicroblaze_Config;

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_H_

/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/

/**
* @file xmicroblaze.h
* @brief MicroBlaze soft processor register definitions and hardware configuration.
*/

#ifndef _XMICROBLAZE_H_
#define _XMICROBLAZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDT
#include "xil_types.h"

/** @brief MicroBlaze processor configuration parameters. */
typedef struct {
	u32 UnalignedExceptions; /**< Enable unaligned memory access exception. */
	u32 IopbBusException; /**< Enable I-OPB bus exception. */
	u32 FpuException; /**< Enable floating-point unit exception. */
	u32 FslException; /**< Enable FSL (AXI) bus exception. */
	u32 DivZeroException; /**< Enable divide-by-zero exception. */
	u32 DopbBusException; /**< Enable D-OPB bus exception. */
	u32 IllOpcodeException; /**< Enable illegal opcode exception. */
	u32 UseStackProtection; /**< Enable hardware stack protection. */
	u32 ExceptionsInDelaySlots; /**< Allow exceptions in branch delay slots. */
	u32 PreDecodeFpuException; /**< Enable pre-decode FPU exception. */
        u32 CpuFreq; /**< CPU clock frequency in Hz. */
        u32 UseMsrInstr; /**< Enable MSR set/clr instructions. */
        u32 DcacheLineSize; /**< Data cache line size in bytes. */
        u32 DcacheAlwaysUsed; /**< Data cache is always enabled (no bypass). */
	u32 DcacheSize; /**< Total data cache size in bytes. */
	u32 IcacheLineSize; /**< Instruction cache line size in bytes. */
	u32 IcacheSize; /**< Total instruction cache size in bytes. */
	u32 AddrTagBits; /**< Number of address tag bits. */
	u32 AllowDcaheWr; /**< Allow data cache write-through. */
	u32 AllowIcacheWr; /**< Allow instruction cache write. */
	u32 AreaOptimized; /**< Area-optimised MicroBlaze configuration. */
	u32 CacheByteSize; /**< Cache size in bytes. */
	u32 DLmp; /**< Data local memory port enable. */
	u32 DOpb; /**< Data OPB bus enable. */
	u32 DPlb; /**< Data PLB bus enable. */
	u32 ILmb; /**< Instruction local memory bus enable. */
	u32 IOpb; /**< Instruction OPB bus enable. */
        u32 IPlb; /**< Instruction PLB bus enable. */
	u32 DcacheAddrTags; /**< Number of data cache address tag bits. */
	u32 DcacheByteSize; /**< Data cache total size in bytes. */
	u32 DcacheLineLen; /**< Data cache line length in words. */
	u32 DcacheUseFsl; /**< Use FSL interface for data cache. */
	u32 DcacheUseWriteback; /**< Use write-back policy for data cache. */
	u32 DebugEnabled; /**< Hardware debug support enabled. */
	u32 DynamicBusSize; /**< Dynamic bus sizing enabled. */
        u32 EdgeIsPositive; /**< Interrupt edge is positive (rising). */
	u32 TimebaseFrequency; /**< Timebase frequency in Hz. */
	u32 Endianness; /**< Processor endianness (0=big, 1=little). */
	u32 FslDataSize; /**< FSL data bus width in bits. */
        u32 FslLinks; /**< Number of FSL links. */
	u32 IcacheAlwaysUsed; /**< Instruction cache is always enabled. */
	u32 IcacheLineLen; /**< Instruction cache line length in words. */
        u32 IcacheUseFsl; /**< Use FSL interface for instruction cache. */
	u32 Interconnect; /**< Interconnect type (PLB, OPB, AXI). */
	u32 InterruptIsEdge; /**< Interrupt is edge-triggered. */
	u32 MmuDtlbSize; /**< Data TLB size (MMU). */
	u32 MmuItlbSize; /**< Instruction TLB size (MMU). */
	u32 MmuTlbAccess; /**< TLB access mode (MMU). */
	u32 MmuZones; /**< Number of MMU protection zones. */
	u32 NumberOfPcBrk; /**< Number of PC breakpoints. */
	u32 NumberOfRdAddrBrk; /**< Number of read address breakpoints. */
	u32 NumberOfWrAddrBrk; /**< Number of write address breakpoints. */
	u32 Opcode0Illegal; /**< Treat opcode 0 as illegal. */
	u32 Pvr; /**< Processor version register support level. */
	u32 PvrUser1; /**< User-defined PVR field 1. */
	u32 PvrUser2; /**< User-defined PVR field 2. */
	u32 ResetMsr ; /**< Machine status register reset value. */
	u32 Sco; /**< Shift carry option enabled. */
	u32 UseBarrel; /**< Barrel shifter enabled. */
	u32 UseDcache; /**< Data cache enabled. */
	u32 UseDiv; /**< Hardware divider enabled. */
	u32 UseExtBrk; /**< External break enabled. */
	u32 UseExtNmBrk; /**< External non-maskable break enabled. */
	u32 UseExtendedFslInstr; /**< Extended FSL instructions enabled. */
	u32 UseFpu; /**< Floating-point unit enabled. */
	u32 UseHwMul; /**< Hardware multiplier enabled. */
	u32 UseIcache; /**< Instruction cache enabled. */
	u32 UseInterrupt; /**< Interrupt controller enabled. */
	u32 UseMmu; /**< Memory management unit enabled. */
        u32 UsePcmpInst; /**< Pattern compare instructions enabled. */
	UINTPTR DcacheBaseaddr; /**< Data cache base address. */
	UINTPTR DcacheHighaddr; /**< Data cache high address. */
	UINTPTR IcacheBaseaddr; /**< Instruction cache base address. */
	UINTPTR IcacheHighaddr; /**< Instruction cache high address. */
	u32 DdrSa;	/**< Valid only for PMC Microblaze */
	u32 DdrEa;	/**< Valid only for PMC microblaze */
	u8 CpuId;	/**< CPU Number */
	u32 BaseVectorAddr; /**< Exception base vector address. */
} XMicroblaze_Config;

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_H_

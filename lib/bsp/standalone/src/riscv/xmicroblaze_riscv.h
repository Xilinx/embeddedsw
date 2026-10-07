/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/

/**
* @file xmicroblaze_riscv.h
* @brief MicroBlaze RISC-V processor register definitions and hardware configuration.
*/

#ifndef _XMICROBLAZE_H_
#define _XMICROBLAZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDT
#include "xil_types.h"

/** @brief MicroBlaze RISC-V processor configuration parameters. */
typedef struct {
	u32 TimebaseFrequency; /**< Timebase frequency in Hz. */
        u32 CpuFreq; /**< CPU clock frequency in Hz. */
	u32 BaseVectorAddr; /**< Exception base vector address. */
	u32 UseMmu; /**< Memory management unit enabled. */
	u32 UseDcache; /**< Data cache enabled. */
	u32 UseIcache; /**< Instruction cache enabled. */
	u32 UseMulDiv; /**< Hardware multiply/divide enabled. */
	u32 UseAtomic; /**< Atomic instructions enabled. */
	u32 UseFpu; /**< Floating-point unit enabled. */
	u32 DcacheSize; /**< Data cache total size in bytes. */
        u32 DcacheLineSize; /**< Data cache line size in bytes. */
	u32 DcacheLineLen; /**< Data cache line length in words. */
	UINTPTR DcacheBaseaddr; /**< Data cache base address. */
	UINTPTR DcacheHighaddr; /**< Data cache high address. */
	u32 IcacheSize; /**< Instruction cache total size in bytes. */
        u32 IcacheLineSize; /**< Instruction cache line size in bytes. */
	u32 IcacheLineLen; /**< Instruction cache line length in words. */
	UINTPTR IcacheBaseaddr; /**< Instruction cache base address. */
	UINTPTR IcacheHighaddr; /**< Instruction cache high address. */
	u8 CpuId;	/**< CPU Number */
} XMicroblaze_RISCV_Config;

#endif

#ifdef __cplusplus
}
#endif

#endif // _XMICROBLAZE_H_

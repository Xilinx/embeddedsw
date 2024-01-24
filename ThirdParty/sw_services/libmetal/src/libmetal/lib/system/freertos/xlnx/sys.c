/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/xlnx/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/compiler.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <stdint.h>
#include "xil_cache.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_mmu.h"

#if (defined(__aarch64__) || defined(ARMA53_32)) && !defined(SDT)

#ifdef VERSAL_NET
#include "xcpu_cortexa78.h"
#elif defined(versal)
#include "xcpu_cortexa72.h"
#else
#include "xreg_cortexa53.h"
#endif /* defined(versal) */

#elif defined(ARMR5)

#include "xil_mpu.h"
#include "xreg_cortexr5.h"

#endif /* (defined(__aarch64__) || defined(ARMA53_32)) && !defined(SDT) */

void sys_irq_restore_enable(unsigned int flags)
{
	Xil_ExceptionEnableMask(~flags);
}

unsigned int sys_irq_save_disable(void)
{
	unsigned int state = mfcpsr() & XIL_EXCEPTION_ALL;

	if (state != XIL_EXCEPTION_ALL) {
		Xil_ExceptionDisableMask(XIL_EXCEPTION_ALL);
	}
	return state;
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len)
		Xil_DCacheFlush();
	else
		Xil_DCacheFlushRange((intptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len)
		Xil_DCacheInvalidate();
	else
		Xil_DCacheInvalidateRange((intptr_t)addr, len);
}

/**
 * @brief poll function until some event happens
 */
void metal_weak metal_generic_default_poll(void)
{
	metal_asm volatile("wfi");
}

/*
 * VERSAL_NET is used since XMpu_Config structure is
 * different for r52(versal net) and r5(zynqmp), done to avoid
 * build failure
 */

#ifdef VERSAL_NET
void *metal_machine_io_mem_map_versal_net(void *va, metal_phys_addr_t pa,
				size_t size, unsigned int flags)
{
	void *__attribute__((unused)) physaddr;
	u32 req_end_addr = pa + size;
	XMpu_Config mpu_config;
	u32 req_addr = pa;
	u32 mmap_req = 1;
	u32 base_end_addr;
	u32 cnt;

	/* Get the MPU Config enties */
	Xil_GetMPUConfig(mpu_config);

	for (cnt = 0; (cnt < MAX_POSSIBLE_MPU_REGS) &&
			(mpu_config[cnt].flags & XMPU_VALID_REGION); cnt++){

		base_end_addr =  mpu_config[cnt].Size + mpu_config[cnt].BaseAddress;

		if ( mpu_config[cnt].BaseAddress <= req_addr && base_end_addr >= req_end_addr){
			/*
			 * Mapping available for requested region in MPU table
			 * If no change in Attribute for region then additional mapping in MPU table is not required
			 */
			 if (mpu_config[cnt].Attribute == flags) {
				mmap_req=0;
				break;
			}
		}
	}

	/* if mapping is required we call Xil_MemMap to get the mapping done */
	if (mmap_req == 1){
		physaddr = Xil_MemMap(pa, size, flags);
		metal_assert(physaddr == (void *)pa);
	}

	return va;
}
#endif

void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	void *__attribute__((unused)) physaddr;
#ifdef VERSAL_NET
	va = metal_machine_io_mem_map_versal_net(va, pa, size, flags);

#else
	physaddr = Xil_MemMap(pa, size, flags);
	metal_assert(physaddr == (void *)pa);
#endif
	return va;
}

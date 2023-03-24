/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
*
*
* riscv_flush_dcache_range (unsigned int cacheaddr, unsigned int len)
*
*    Flush a data cache range and flush (invalidate) the corresponding
*    instruction cache range, if any:
*    - Increment by dcache line length in bytes
*
*    Parameters:
*       'cacheaddr' - address in the cache where invalidation begins
*	'len    '   - length (in bytes) worth of cache to be flushed
*
*******************************************************************************/

#include "xpseudo_asm.h"
#include "xparameters.h"

.global	riscv_flush_dcache_range
.section .text
.align 2
.type riscv_flush_dcache_range, @function

#ifndef XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN
#define XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN     4
#endif

#define INCREMENT 4 * XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN

riscv_flush_dcache_range:
#if XPAR_MICROBLAZE_RISCV_USE_DCACHE==1
        beqz    a0, L_done            /* Skip loop if size is zero */
        add	t0, a0, a1            /* Compute end address */
        andi    t0, t0, -(INCREMENT)  /* Align end down to cache line */
        andi    a0, a0, -(INCREMENT)  /* Align start down to cache line */
L_start:
        cbo.flush (a0)                /* Flush the cache block (line) */
        addi    a0, a0, (INCREMENT)   /* Increment the address by line length in bytes */
        bltu    a0, t0, L_start       /* Are we at the end? */
#endif
L_done:
	ret
.end riscv_flush_dcache_range

/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
*
* riscv_invalidate_icache()
*
*    Invalidate entire instruction cache and corresponding part of data cache:
*    - Start from icache line index 0
*    - Stop at the icache byte size
*    - Increment by icache line length in bytes
*
*******************************************************************************/

#include "xpseudo_asm.h"
#include "xparameters.h"

.global	riscv_invalidate_icache
.section .text
.align 2
.type riscv_invalidate_icache, @function

#ifndef XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN
#define XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN     4
#endif

#define INCREMENT 4 * XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN
#define BYTE_SIZE XPAR_MICROBLAZE_RISCV_ICACHE_BYTE_SIZE & (-(INCREMENT))

riscv_invalidate_icache:
#if XPAR_MICROBLAZE_RISCV_USE_ICACHE==1
	li 	t0, 0               /* Start at icache index 0 */
	li 	t1, BYTE_SIZE       /* Stop at icache byte size */
L_start:
	.long	0x8022A00F          /* "cbo.flush (t0)" with bit 31 set to flush without matching address check */
				    /* Flush is used since the instruction also affects the dcache */
	addi	t0, t0, INCREMENT   /* Increment the address by line length in bytes */
	bltu	t0, t1, L_start     /* Are we at the end? */
#endif
L_done:
	ret                         /* Return */
.end riscv_invalidate_icache

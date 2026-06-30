/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file cpu_init.S
*
* This file contains CPU specific initialization. Invoked from main CRT
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00  pkp	02/10/14 Initial version
* 9.5   vmt     05/20/26 Unified Cortex-R52 armclang and GCC startup
*                        assembly sources to support armclang integrated
*                        assembler.
*
* </pre>
*
******************************************************************************/

.text
.eabi_attribute Tag_ABI_align_needed, 1
.eabi_attribute Tag_ABI_align_preserved, 1
.global __cpu_init
.type __cpu_init, %function
.align 2
__cpu_init:

/* Clear cp15 regs with unknown reset values */
	mov	r0, #0x0
	mcr	p15, 0, r0, c5, c0, 0	/* DFSR */
	mcr	p15, 0, r0, c5, c0, 1	/* IFSR */
	mcr	p15, 0, r0, c6, c0, 0	/* DFAR */
	mcr	p15, 0, r0, c6, c0, 2	/* IFAR */
	mcr	p15, 0, r0, c9, c13, 2	/* PMXEVCNTR */
	mcr	p15, 0, r0, c13, c0, 2	/* TPIDRURW */
	mcr	p15, 0, r0, c13, c0, 3	/* TPIDRURO */


/* Reset and start Cycle Counter */
	mov	r2, #0x80000000		/* clear overflow */
	mcr	p15, 0, r2, c9, c12, 3
	mov	r2, #0xd		/* D, C, E */
	mcr	p15, 0, r2, c9, c12, 0
	mov	r2, #0x80000000		/* enable cycle counter */
	mcr	p15, 0, r2, c9, c12, 1

	bx	lr

.end

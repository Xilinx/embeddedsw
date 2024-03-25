/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file boot.S
*
* @addtogroup riscv_boot_code RISC-V Processor Boot Code
* @{
* <h2> boot.S </h2>
* The boot code performs minimum configuration which is required for an
* application to run starting from processor's reset state. Below is a
* sequence illustrating what all configuration is performed before control
* reaches the main function.
*
* 1. Initialize mtvec CSR for unhandled exceptions and interrupts
* 2. Initialize processor registers to 0
* 3. Program stack pointer
* 4. Initialize floating-point registers to +0.0
* 5. Set the stack top to zero, to ensure that main program argc is zero
* 6. Transfer control to _start, which clears BSS sections and jumps to
*    the main application
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.0   sa   12/11/22 Initial version
* 9.1   mj   22/12/23 Disable relaxation before initializing stack pointer
* 9.1   mus  22/12/23 Trap vector base address must be 4 byte aligned, it
*                     is initialized to .Ldone label which is not 4 byte
*       	      aligned. Updated boot code to make .Ldone labe 4 byte
*                     aligned explicitly.
* 9.1  mus   25/01/24 Temporarily set mtvec with _trap_handler at start of
*                     boot code. It will be reverted once triggering of
*                     constructor is fixed.
* 9.1  mus   25/03/24 Use correct macro for checking FPU enablement.
* </pre>
*
******************************************************************************/

#include "xparameters.h"

.global _boot
.global __stack

.section .text.init,"ax"

_boot:

/* FIXME: Initialize mtvec CSR to _trap_handler */
	la	t0, _trap_handler
	csrw	mtvec, t0

/* Initialize processor registers to 0 */
	addi	x1,x0,0
	addi	x3,x0,0
	addi	x4,x0,0
	addi	x5,x0,0
	addi	x6,x0,0
	addi	x7,x0,0
	addi	x8,x0,0
	addi	x9,x0,0
	addi	x10,x0,0
	addi	x11,x0,0
	addi	x12,x0,0
	addi	x13,x0,0
	addi	x14,x0,0
	addi	x15,x0,0
	addi	x16,x0,0
	addi	x17,x0,0
	addi	x18,x0,0
	addi	x19,x0,0
	addi	x20,x0,0
	addi	x21,x0,0
	addi	x22,x0,0
	addi	x23,x0,0
	addi	x24,x0,0
	addi	x25,x0,0
	addi	x26,x0,0
	addi	x27,x0,0
	addi	x28,x0,0
	addi	x29,x0,0
	addi	x30,x0,0
	addi	x31,x0,0

/* Initialize stack pointer */
	.option push
	.option norelax
	la	sp, __stack
	.option pop

/* Clear the floating point registers */
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
	fmv.d.x	f0, x0
	fmv.d.x	f1, x0
	fmv.d.x	f2, x0
	fmv.d.x	f3, x0
	fmv.d.x	f4, x0
	fmv.d.x	f5, x0
	fmv.d.x	f6, x0
	fmv.d.x	f7, x0
	fmv.d.x	f8, x0
	fmv.d.x	f9, x0
	fmv.d.x	f10, x0
	fmv.d.x	f11, x0
	fmv.d.x	f12, x0
	fmv.d.x	f13, x0
	fmv.d.x	f14, x0
	fmv.d.x	f15, x0
	fmv.d.x	f16, x0
	fmv.d.x	f17, x0
	fmv.d.x	f18, x0
	fmv.d.x	f19, x0
	fmv.d.x	f20, x0
	fmv.d.x	f21, x0
	fmv.d.x	f22, x0
	fmv.d.x	f23, x0
	fmv.d.x	f24, x0
	fmv.d.x	f25, x0
	fmv.d.x	f26, x0
	fmv.d.x	f27, x0
	fmv.d.x	f28, x0
	fmv.d.x	f29, x0
	fmv.d.x	f30, x0
	fmv.d.x	f31, x0
#endif
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 1)
	fmv.w.x	f0, x0
	fmv.w.x	f1, x0
	fmv.w.x	f2, x0
	fmv.w.x	f3, x0
	fmv.w.x	f4, x0
	fmv.w.x	f5, x0
	fmv.w.x	f6, x0
	fmv.w.x	f7, x0
	fmv.w.x	f8, x0
	fmv.w.x	f9, x0
	fmv.w.x	f10, x0
	fmv.w.x	f11, x0
	fmv.w.x	f12, x0
	fmv.w.x	f13, x0
	fmv.w.x	f14, x0
	fmv.w.x	f15, x0
	fmv.w.x	f16, x0
	fmv.w.x	f17, x0
	fmv.w.x	f18, x0
	fmv.w.x	f19, x0
	fmv.w.x	f20, x0
	fmv.w.x	f21, x0
	fmv.w.x	f22, x0
	fmv.w.x	f23, x0
	fmv.w.x	f24, x0
	fmv.w.x	f25, x0
	fmv.w.x	f26, x0
	fmv.w.x	f27, x0
	fmv.w.x	f28, x0
	fmv.w.x	f29, x0
	fmv.w.x	f30, x0
	fmv.w.x	f31, x0
#endif
	sw	zero,0(sp)	/* Initialize argc to zero */
	j 	_start		/* jump to C startup code */

.align 2
.Ldone:	j	.Ldone		/* We should only get here for unhandled traps*/

.end
/**
* @} End of "addtogroup riscv_boot_code".
*/

/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
* @file riscv_disable_interrupts.S
*
* @addtogroup riscv_pseudo_asm_macro
* @{
* <h2> riscv_disable_interrupts.S </h2>
*  - API to disable interrupts: void riscv_disable_interrupts(void)
*
* This API disables interrupts in the RISC-V. It can be called when entering a
* critical section of code where a context switch is undesirable.
*
* <pre>
* File   : riscv_disable_interrupts.s
* Date   : 2022, November 21.
*
* Summary:
* Disable interrupts in the processor.
* </pre>
*
******************************************************************************/

#include "xparameters.h"

.global	riscv_disable_interrupts
.section .text
.align	2
.type riscv_disable_interrupts, @function
riscv_disable_interrupts:
	/* Read the mie CSR register */
	csrr	t1, mie
	/* Clear the mie interrupt enable bits MEIE (bit 11), MTIE (bit 7), and MSIE (bit 3) */
	li	t2, ~(0x888)
	and	t1, t1, t2
	/* Save the mie CSR register */
	csrw	mie, t1
	/* Clear the mstatus interrupt enable bit MIE (bit 3) */
	csrci	mstatus, 0x8
	/* Return */
	ret
.end riscv_disable_interrupts
/**
* @} End of "addtogroup riscv_pseudo_asm_macro".
*/

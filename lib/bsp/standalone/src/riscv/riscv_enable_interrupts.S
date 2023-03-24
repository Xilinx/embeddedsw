/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
* @file riscv_enable_interrupts.S
*
* @addtogroup riscv_pseudo_asm_macro
* @{
* <h2> riscv_enable_interrupts.S </h2>
*  - API to enable interrupts: void riscv_enable_interrupts(void)
*
* This API enables interrupts in the RISC-V processor. When the RISC-V processor
* starts up, interrupts are disabled. Interrupts can be explicitly turned on
* using this function.
*
* <pre>
*
* File   : riscv_enable_interrupts.s
* Date   : 2022, November 21.
*
* Summary:
* Enable interrupts in the processor.
* </pre>
*
******************************************************************************/

#include "xparameters.h"

.global	riscv_enable_interrupts
.section .text
.align	2
.type riscv_enable_interrupts, @function
riscv_enable_interrupts:
	/* Read the mie CSR register */
	csrr	t1, mie
	/* Set the mie interrupt enable bits MEIE (bit 11), MTIE (bit 7), and MSIE (bit 3) */
	li	t2, 0x888
	and	t1, t1, t2
	/* Save the mie CSR register */
	csrw	mie, t1
	/* Set the mstatus interrupt enable bit MIE (bit 3) */
	csrsi	mstatus, 0x8
	/* Return */
	ret
.end riscv_enable_interrupts
/**
* @} End of "addtogroup riscv_pseudo_asm_macro".
*/

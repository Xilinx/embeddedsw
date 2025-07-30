/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
 * RISC-V Trap Handler
 * - Non self-modifying trap handler for the following trap conditions
 *   - Misaligned memory access
 *******************************************************************************/

#include "xparameters.h"

/* 64-bit definitions */
#if __riscv_xlen == 64
#define INTPTR_DATAITEM			.quad
#define REGSIZE				8
#define DATAALIGN			3
#define SHIFT				4
#define STORE				sd
#define LOAD				ld
#else
#define INTPTR_DATAITEM			.long
#define REGSIZE				4
#define DATAALIGN			2
#define SHIFT				3
#define STORE				sw
#define LOAD				lw
#endif /* 64-bit definitions */

/* FPU definitions */
#define REGCOUNT			32
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#define FPREGSIZE			8
#define FPREGCOUNT			33
#define STOREFP				fsd
#define LOADFP				fld
#elif defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 1)
#define FPREGSIZE			4
#define FPREGCOUNT			33
#define STOREFP				fsw
#define LOADFP				flw
#else
#define FPREGSIZE			0
#define FPREGCOUNT			0
#endif

/* Stack macros */
#define HANDLER_STACK_SIZE		(REGSIZE * REGCOUNT + FPREGSIZE * FPREGCOUNT)

#define REG_OFFSET(regnum)		(REGSIZE * regnum)
#define NUM_TO_REG(num)			x ## num
#define PUSH_REG(regnum)		STORE NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)
#define POP_REG(regnum)			LOAD  NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)

#define FPREG_OFFSET(regnum)		(FPREGSIZE * regnum + REGSIZE * REGCOUNT)
#define NUM_TO_FPREG(num)		f ## num
#define PUSH_FPREG(regnum)		STOREFP NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)
#define POP_FPREG(regnum)		LOADFP  NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)

/* Misaligned handler macros */
#define T4_TO_STACK(regnum)		STORE t4, REG_OFFSET(regnum)(sp)
#define T4_FROM_STACK(regnum)		LOAD  t4, REG_OFFSET(regnum)(sp)

#define T4_TO_REG_Z(regnum)	   \
	.align 2;		   \
	nop;			   \
	j  misalign_done

#define T4_TO_REG_V(regnum)	   \
	.align 2;		   \
	T4_TO_STACK(regnum);	   \
	j  misalign_done

#define T4_TO_REG(regnum)	   \
	.align 2;		   \
	mv NUM_TO_REG(regnum), t4; \
	j  misalign_done

#define REG_TO_T4_Z(regnum)	   \
	.align 2;		   \
	li t4, 0;		   \
	j  store_tail

#define REG_TO_T4_V(regnum)	   \
	.align 2;		   \
	T4_FROM_STACK(regnum);	   \
	j  store_tail

#define REG_TO_T4(regnum)	   \
	.align 2;		   \
	mv t4, NUM_TO_REG(regnum); \
	j  store_tail

/* Extern declarations */
.extern XNullHandler


#ifndef RISCV_TRAPS_DISABLED				/* If traps are disabled in the processor */

/*
 * trap_handler - Handler for interrupts and exceptions
 *
 * Trap handler notes:
 * - Does not handle traps other than misaligned memory access
 */

.global _trap_handler
.section .text
.align 2
.type _trap_handler, @function
_trap_handler:
	addi		sp, sp, -(HANDLER_STACK_SIZE)	/* Create stack frame */
	PUSH_REG(6)					/* Save t1 = x6 */
	PUSH_REG(7)					/* Save t2 = x7 */
	PUSH_REG(28)					/* Save t3 = x28 */
	PUSH_REG(29)					/* Save t4 = x29 */
	PUSH_REG(30)					/* Save t5 = x30 */
	csrr		t1, mcause
	li		t2, 6				/* Store address misaligned */
	beq		t1, t2, handle_store_misalign
	li		t2, 4				/* Load address misaligned */
	beq		t1, t2, handle_load_misalign
	PUSH_REG(1)
	PUSH_REG(3)
	PUSH_REG(4)
	PUSH_REG(5)
	PUSH_REG(8)
	PUSH_REG(9)
	PUSH_REG(10)
	PUSH_REG(11)
	PUSH_REG(12)
	PUSH_REG(13)
	PUSH_REG(14)
	PUSH_REG(15)
	PUSH_REG(16)
	PUSH_REG(17)
	PUSH_REG(18)
	PUSH_REG(19)
	PUSH_REG(20)
	PUSH_REG(21)
	PUSH_REG(22)
	PUSH_REG(23)
	PUSH_REG(24)
	PUSH_REG(25)
	PUSH_REG(26)
	PUSH_REG(27)
	PUSH_REG(31)
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	PUSH_FPREG(0)					/* Save f0 to f31 */
	PUSH_FPREG(1)
	PUSH_FPREG(2)
	PUSH_FPREG(3)
	PUSH_FPREG(4)
	PUSH_FPREG(5)
	PUSH_FPREG(6)
	PUSH_FPREG(7)
	PUSH_FPREG(8)
	PUSH_FPREG(9)
	PUSH_FPREG(10)
	PUSH_FPREG(11)
	PUSH_FPREG(12)
	PUSH_FPREG(13)
	PUSH_FPREG(14)
	PUSH_FPREG(15)
	PUSH_FPREG(16)
	PUSH_FPREG(17)
	PUSH_FPREG(18)
	PUSH_FPREG(19)
	PUSH_FPREG(20)
	PUSH_FPREG(21)
	PUSH_FPREG(22)
	PUSH_FPREG(23)
	PUSH_FPREG(24)
	PUSH_FPREG(25)
	PUSH_FPREG(26)
	PUSH_FPREG(27)
	PUSH_FPREG(28)
	PUSH_FPREG(29)
	PUSH_FPREG(30)
	PUSH_FPREG(31)
	csrr		t2, fcsr			/* Save fcsr */
	sw		t2, FPREG_OFFSET(32)(sp)
#endif
	la		t2, RISCV_InterruptVectorTable	/* Load interrupt vector table base address */
	bltz		t1, is_interrupt
	la		t2, RISCV_ExceptionVectorTable	/* Load exception vector table base address */
is_interrupt:
        slli		t1, t1, SHIFT			/* Calculate vector offset = t1 * SHIFT */
	add		t2, t2, t1			/* Get pointer to vector */
	LOAD		a0, REGSIZE(t2)			/* Load argument to handler from vector */
	LOAD		t2, 0(t2)			/* Load vector itself from vector */
	jalr		t2				/* Jump to handler: ra = x1 = link address */
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	lw		t2, FPREG_OFFSET(32)(sp)	/* Restore fcsr */
	csrw		fcsr, t2
	POP_FPREG(31)					/* Restore f0 to f31 */
	POP_FPREG(30)
	POP_FPREG(29)
	POP_FPREG(28)
	POP_FPREG(27)
	POP_FPREG(26)
	POP_FPREG(25)
	POP_FPREG(24)
	POP_FPREG(23)
	POP_FPREG(22)
	POP_FPREG(21)
	POP_FPREG(20)
	POP_FPREG(19)
	POP_FPREG(18)
	POP_FPREG(17)
	POP_FPREG(16)
	POP_FPREG(15)
	POP_FPREG(14)
	POP_FPREG(13)
	POP_FPREG(12)
	POP_FPREG(11)
	POP_FPREG(10)
	POP_FPREG(9)
	POP_FPREG(8)
	POP_FPREG(7)
	POP_FPREG(6)
	POP_FPREG(5)
	POP_FPREG(4)
	POP_FPREG(3)
	POP_FPREG(2)
	POP_FPREG(1)
	POP_FPREG(0)
#endif
	POP_REG(31)
	POP_REG(27)
	POP_REG(26)
	POP_REG(25)
	POP_REG(24)
	POP_REG(23)
	POP_REG(22)
	POP_REG(21)
	POP_REG(20)
	POP_REG(19)
	POP_REG(18)
	POP_REG(17)
	POP_REG(16)
	POP_REG(15)
	POP_REG(14)
	POP_REG(13)
	POP_REG(12)
	POP_REG(11)
	POP_REG(10)
	POP_REG(9)
	POP_REG(8)
	POP_REG(5)
	POP_REG(4)
	POP_REG(3)
	POP_REG(1)
misalign_done:
	POP_REG(30)					/* Restore t5 = x30 */
	POP_REG(29)					/* Restore t4 = x29 */
	POP_REG(28)					/* Restore t3 = x28 */
	POP_REG(7)					/* Restore t2 = x7 */
	POP_REG(6)					/* Restore t1 = x6 */
	addi		sp, sp, (HANDLER_STACK_SIZE)	/* Restore stack frame */
	mret

/*
 * Load address misalign exception handler
 *   t1 = faulting virtual address
 *   t2 = transfer size
 *   t3 = destination register * 8 (without C extension) or 4 (with C extension)
 *   t4 = register data value
 *   t5 = data store address
 */

handle_load_misalign:
	csrr	t1, mtval				/* Get faulting virtual address */
	csrr	t3, mepc				/* Get faulting instruction address */
#if __riscv_compressed
	lhu	t5, 0(t3)				/* Fetch 16 low bits of faulting instruction */
	andi	t2, t5, 3				/* Extract op */
	slli	t2, t2, SHIFT-1
	la	t4, load_op_table			/* Compute address to load op label */
	add	t4, t4, t2
	LOAD	t4, 0(t4)				/* Load label itself from address */
	jr	t4					/* Jump to load op label */
load_00: /* Compressed instruction C.LW, C.LD, C.FLW, C.FLD with rd' (bit 4:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

	addi	t3, t5, 8*4				/* Extract rd destination register * 4 */
	j	load_continue
load_01: /* Non-load compressed instruction */
	j	misalign_done				/* Should never be misaligned so just return */
load_10: /* Compressed instruction C.LWSP, C.LDSP, C.FLWSP, C.FLDSP with rd (bit 11:7) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

	srli	t3, t5, 5				/* Extract rd destination register * 4 */
	j	load_continue
load_11: /* Uncompressed instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	srli	t3, t5, 5				/* Extract rd destination register * 4 */
load_continue:
	andi	t3, t3, 0x7c
#else
	lw	t5, (t3)				/* Fetch faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	srli	t3, t5, 4				/* Extract rd destination register * 8 */
	andi	t3, t3, 0xf8
#endif /* __riscv_compressed */
	li	t4, 1
	beq	t2, t4, load_half_size
	li	t4, 5
	beq	t2, t4, load_half_size_unsigned
#if __riscv_xlen == 64
	li	t4, 2
	beq	t2, t4, load_word_size
	li	t4, 6
	beq	t2, t4, load_word_size_unsigned
load_double_size:
	lb	t4, 0(t1)				/* Load double byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lb	t4, 4(t1)
	sb	t4, data_4, t5
	lb	t4, 5(t1)
	sb	t4, data_5, t5
	lb	t4, 6(t1)
	sb	t4, data_6, t5
	lb	t4, 7(t1)
	sb	t4, data_7, t5
	ld	t4, data_0				/* Load double */
	j	load_tail
load_word_size_unsigned:
	lb	t4, 0(t1)				/* Load word byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lwu	t4, data_0				/* Load unsigned word */
	j	load_tail
#endif
load_word_size:
	lb	t4, 0(t1)				/* Load word byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lw	t4, data_0				/* Load word */
	j	load_tail
load_half_size:
	lb	t4, 0(t1)				/* Load halfword byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lh	t4, data_0				/* Load halfword */
	j	load_tail
load_half_size_unsigned:
	lb	t4, 0(t1)				/* Load halfword byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lhu	t4, data_0				/* Load unsigned halfword */
load_tail:
	la	t2, load_table				/* Form load table offset (load_table + register * 8 or 4) */
	add	t2, t2, t3
	jr	t2
	.align 2
load_table:
	T4_TO_REG_Z (0)
	T4_TO_REG   (1)
	T4_TO_REG   (2)
	T4_TO_REG   (3)
	T4_TO_REG   (4)
	T4_TO_REG   (5)
	T4_TO_REG_V (6)
	T4_TO_REG_V (7)
	T4_TO_REG   (8)
	T4_TO_REG   (9)
	T4_TO_REG   (10)
	T4_TO_REG   (11)
	T4_TO_REG   (12)
	T4_TO_REG   (13)
	T4_TO_REG   (14)
	T4_TO_REG   (15)
	T4_TO_REG   (16)
	T4_TO_REG   (17)
	T4_TO_REG   (18)
	T4_TO_REG   (19)
	T4_TO_REG   (20)
	T4_TO_REG   (21)
	T4_TO_REG   (22)
	T4_TO_REG   (23)
	T4_TO_REG   (24)
	T4_TO_REG   (25)
	T4_TO_REG   (26)
	T4_TO_REG   (27)
	T4_TO_REG_V (28)
	T4_TO_REG_V (29)
	T4_TO_REG_V (30)
	T4_TO_REG   (31)

/*
 * Store address misalign exception handler
 *   t1 = faulting virtual address
 *   t2 = transfer size
 *   t3 = source register * 8 (without C extension) or 4 (with C extension)
 *   t4 = register data value
 *   t5 = scratch
 */

handle_store_misalign:
	csrr	t1, mtval				/* Get faulting virtual address */
	csrr	t3, mepc				/* Get faulting instruction address */
#if __riscv_compressed
	lhu	t5, 0(t3)				/* Fetch 16 low bits of faulting instruction */
	andi	t2, t5, 3				/* Extract op */
	slli	t2, t2, SHIFT-1
	la	t4, store_op_table			/* Compute address to store op label */
	add	t4, t4, t2
	LOAD	t4, 0(t4)				/* Load label itself from address */
	jr	t4					/* Jump to store op label */
store_00: /* Compressed instruction C.SW, C.SD, C.FSW, C.FSD with rs2' (bit 4:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

	addi	t3, t5, 8*4				/* Extract rs2 source register * 4 */
	j	store_continue
store_01: /* Non-store compressed instruction */
	j	misalign_done				/* Should never be misaligned so just return */
store_10: /* Compressed instruction C.SWSP, C.SDSP, C.FSWSP, C.FSDSP with rs2 (bit 6:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

	mv	t3, t5					/* Extract rs2 source register * 4 */
	j	store_continue
store_11: /* Uncompressed instruction with rs2 (bit 24:20) */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	lhu	t5, 2(t3)				/* Fetch 16 high bits of faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
	srli	t3, t5, 2				/* Extract rs2 source register * 4 */
store_continue:
	andi	t3, t3, 0x7c
#else
	lw	t2, (t3)				/* Fetch faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
	srli	t3, t2, 17				/* Extract rs2 source register * 8 */
	andi	t3, t3, 0xf8
	srli	t2, t2, 12				/* Extract transfer size */
	andi	t2, t2, 7
#endif /* __riscv_compressed */
	la	t4, store_table				/* Form store table offset (store_table + register * 8 or 4) */
	add	t4, t4, t3
	jr	t4
store_tail:
	li	t3, 1
	beq	t2, t3, store_half_size
#if __riscv_xlen == 64
	li	t3, 2
	beq	t2, t3, store_word_size
store_double_size:
	sd	t4, data_0, t3
	lbu	t4, data_0				/* Store double byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	lbu	t4, data_2
	sb	t4, 2(t1)
	lbu	t4, data_3
	sb	t4, 3(t1)
	lbu	t4, data_4
	sb	t4, 4(t1)
	lbu	t4, data_5
	sb	t4, 5(t1)
	lbu	t4, data_6
	sb	t4, 6(t1)
	lbu	t4, data_7
	sb	t4, 7(t1)
	j	misalign_done
#endif
store_word_size:
	sw	t4, data_0, t3
	lbu	t4, data_0				/* Store word byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	lbu	t4, data_2
	sb	t4, 2(t1)
	lbu	t4, data_3
	sb	t4, 3(t1)
	j	misalign_done
store_half_size:
	sw	t4, data_0, t3
	lbu	t4, data_0				/* Store halfword byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	j	misalign_done
	.align 2
store_table:
	REG_TO_T4_Z (0)
	REG_TO_T4   (1)
	REG_TO_T4   (2)
	REG_TO_T4   (3)
	REG_TO_T4   (4)
	REG_TO_T4   (5)
	REG_TO_T4_V (6)
	REG_TO_T4_V (7)
	REG_TO_T4   (8)
	REG_TO_T4   (9)
	REG_TO_T4   (10)
	REG_TO_T4   (11)
	REG_TO_T4   (12)
	REG_TO_T4   (13)
	REG_TO_T4   (14)
	REG_TO_T4   (15)
	REG_TO_T4   (16)
	REG_TO_T4   (17)
	REG_TO_T4   (18)
	REG_TO_T4   (19)
	REG_TO_T4   (20)
	REG_TO_T4   (21)
	REG_TO_T4   (22)
	REG_TO_T4   (23)
	REG_TO_T4   (24)
	REG_TO_T4   (25)
	REG_TO_T4   (26)
	REG_TO_T4   (27)
	REG_TO_T4_V (28)
	REG_TO_T4_V (29)
	REG_TO_T4_V (30)
	REG_TO_T4   (31)


.section .data
.align DATAALIGN

/*
 * Misaligned data value
 */

data_0:
	.byte 0
data_1:
	.byte 0
data_2:
	.byte 0
data_3:
	.byte 0
#if __riscv_xlen == 64
data_4:
	.byte 0
data_5:
	.byte 0
data_6:
	.byte 0
data_7:
	.byte 0
#endif

/*
 * Compressed transfer size and op tables
 */
#if __riscv_compressed
transfer_size_table:
	.byte 0
	.byte 3						/* 01: C.FLD, C.FSD, C.FLDSP, C.FSDSP  */
	.byte 2						/* 10: C.LW,  C.SW,  C.LWSP,  C.SWSP   */
#if __riscv_xlen == 64
	.byte 3						/* 11: C.LD,  C.SD,  C.LDSP,  C.SDSP   */
#else
	.byte 2						/* 11: C.FLW, C.FSW, C.FLWSP, C.FSWPSP */
#endif
load_op_table:
	INTPTR_DATAITEM load_00				/* OP0 compressed instruction */
	INTPTR_DATAITEM load_01			 	/* OP1 compressed instruction */
	INTPTR_DATAITEM load_10				/* OP2 compressed instruction */
	INTPTR_DATAITEM load_11				/* Uncompressed instruction   */
store_op_table:
	INTPTR_DATAITEM store_00			/* OP0 compressed instruction */
	INTPTR_DATAITEM store_01			/* OP1 compressed instruction */
	INTPTR_DATAITEM store_10			/* OP2 compressed instruction */
	INTPTR_DATAITEM store_11			/* Uncompressed instruction   */
#endif /* __riscv_compressed */

/*
 * Exception and interrupt vector tables
 */
.align DATAALIGN
.global RISCV_ExceptionVectorTable
RISCV_ExceptionVectorTable:
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 0					/* Instruction access misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 1					/* Instruction access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 2					/* Illegal instruction */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 3					/* Breakpoint */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 4					/* Load address misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 5					/* Load access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 6					/* Store/AMO address misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 7					/* Store/AMO access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 8					/* Environment call from U-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 9					/* Environment call from S-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 10					/* Reserved */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 11					/* Environment call from M-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 12                                  /* Instruction page fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 13                                  /* Load page fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 14                                  /* Reserved */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 15                                  /* Store/AMO page fault */

.global RISCV_InterruptVectorTable
RISCV_InterruptVectorTable:
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 0					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 1					/* Supervisor software interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 2					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 3					/* Machine software interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 4					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 5					/* Supervisor timer interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 6					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 7					/* Machine timer interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 8					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 9					/* Supervisor external interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 10					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 11					/* Machine external interrupt */
.end _trap_handler

#else

/*
 * Dummy trap handler, in case traps are not present in the processor
 */

.global _trap_handler
.section .text
.align 2
_trap_handler:
        j     0
.end _trap_handler

#endif  /* RISCV_TRAPS_DISABLED */

/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpseudo_asm_rvct.c
*
* This header file contains functions for using assembler code. It is
* written specifically for RVCT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sdm  11/18/09 First Release
* 7.2   asa  04/04/20 Renamed the str macro to strw.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/

#include "xpseudo_asm_rvct.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/** @brief Enable IRQ exceptions (CPSR I-bit clear). */
__asm void cpsiei(void)
{
	cpsie	i
	bx	lr
}

/** @brief Disable IRQ exceptions (CPSR I-bit set). */
__asm void cpsidi(void)
{
	cpsid	i
	bx	lr
}

/** @brief Enable FIQ exceptions (CPSR F-bit clear). */
__asm void cpsief(void)
{
	cpsie	f
	bx	lr
}

/** @brief Disable FIQ exceptions (CPSR F-bit set). */
__asm void cpsidf(void)
{
	cpsid	f
	bx	lr
}

/** @brief Instruction Synchronization Barrier. */
__asm void isb(void)
{
	isb
	bx lr
}

/** @brief Data Synchronization Barrier. */
__asm void dsb(void)
{
	dsb
	bx lr
}

/** @brief Data Memory Barrier. */
__asm void dmb(void)
{
	dmb
	bx lr
}

/**
 * @brief Load a 32-bit word from the given address.
 *
 * @param[in] adr Address to load from.
 * @return 32-bit value read from address.
 */
__asm u32 ldr(u32 adr)
{
	ldr	r0, [r0]
	bx	lr
}

/**
 * @brief Load a byte from the given address.
 *
 * @param[in] adr Address to load from.
 * @return Byte value read from address.
 */
__asm u32 ldrb(u32 adr)
{
	ldrb	r0, [r0]
	bx	lr
}

/**
 * @brief Store a 32-bit word to the given address.
 *
 * @param[in] adr Address to store to.
 * @param[in] val Value to store.
 */
__asm void strw(u32 adr, u32 val)
{
	str	r1, [r0]
	bx	lr
}

/**
 * @brief Store a byte to the given address.
 *
 * @param[in] adr Address to store to.
 * @param[in] val Byte value to store.
 */
__asm void strb(u32 adr, u32 val)
{
	strb	r1, [r0]
	bx	lr
}

/**
 * @brief Count leading zeroes.
 *
 * @param[in] arg Input value.
 * @return Number of leading zero bits in arg.
 */
__asm u32 clz(u32 arg)
{
	clz	r0, r0
	bx	lr
}

/**
 * @brief Read the Current Program Status Register (CPSR).
 *
 * @return Current value of the CPSR register.
 */
__asm u32 mfcpsr(void)
{
	mrs	r0, cpsr
	bx lr
}

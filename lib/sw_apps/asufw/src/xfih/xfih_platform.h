/**************************************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/******************************************************************************/
/**
*
* @file xfih_platform.h
* @{
* @details This file defines RISC-V specific macros/function required for
*          fault injection hardening code. This file shall be updated when
*          fault injection hardening code is ported to other processor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   mmd  09/04/23 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/

#ifndef XFIH_PLATFORM_H_
#define XFIH_PLATFORM_H_

/* RISC V specific macros */
#ifdef __riscv

/**
 * @brief Processor specific macro for planting illegal instruction in code
 */
#define XFIH_PLAT_ILLEGAL_INSTRUCTION_TRAP \
	__asm__ volatile ("unimp")


/**
 * @brief Processor specific macro for uncondition branch (goto)
 */
#define XFIH_PLAT_UNCONDITIONAL_BRANCH(Label) \
	__asm__ volatile goto ("j %0"::::Label)

/**
 * @brief Processor specific macro for variable assignment without getting
 *        optimized.
 */
#define XFIH_PLAT_VOLATILE_ASSIGNMENT(ReturnVal, Val) \
	__asm__ volatile("mv %0, %1" \
			 : "=r" (ReturnVal) \
			 : "r" (Val))

/* Non supported processors */
#else
#error "FIH library do not support the processor"
#endif

#endif /* XFIH_PLATFORM_H_ */

/** @} */
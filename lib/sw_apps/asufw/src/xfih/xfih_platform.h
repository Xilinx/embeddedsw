/******************************************************************************
*
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
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

#ifndef XFIH_PLATFORM_H
#define XFIH_PLATFORM_H

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

#endif /* XFIH_PLATFORM_H */

/** @} */
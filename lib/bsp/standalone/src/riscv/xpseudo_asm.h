/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpseudo_asm.h
*
* @addtogroup RISC-V Processor Specific Include Files
*
* The xpseudo_asm.h includes xreg_riscv.h and xpseudo_asm_gcc.h.
*
* The xreg_riscv.h file contains definitions for inline assembler code.
* It provides inline definitions for RISC-V GPRs, CSRs, and Debug registers.
*
* The xpseudo_asm_gcc.h contains the definitions for the most often used
* inline assembler instructions, available as macros. These can be very
* useful for tasks such as setting or getting special purpose registers,
* or synchronization. These inline assembler instructions can be used from
* drivers and user applications written in C.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 9.0   sa   07/20/20 Initial version
* </pre>
*
******************************************************************************/
#ifndef XPSEUDO_ASM_H /* prevent circular inclusions */
#define XPSEUDO_ASM_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xreg_riscv.h"
#if defined (__GNUC__)
#include "xpseudo_asm_gcc.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPSEUDO_ASM_H */
/**
* @} End of "addtogroup RISC-V specific".
*/

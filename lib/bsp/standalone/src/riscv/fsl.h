/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef FSL_H
#define FSL_H
/*****************************************************************************/
/**
*
* @file fsl.h
*
* @addtogroup microblaze_riscv_fsl_macro MicroBlaze RISC-V Processor FSL Macros
*
* Microblaze RISC-V BSP includes macros for custom instructions to provide
* convenient access to accelerators connected to AXI4-Stream Interfaces.To use
* these functions, include the header file fsl.h in your source code
*
* @{
*
******************************************************************************/

#include "riscv_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *@cond nocomments
 */
/* Extended macros. */
#define FSL_DEFAULT                              0x00000000
#define FSL_NONBLOCKING                          0x40000000
#define FSL_EXCEPTION                            0x08000000
#define FSL_CONTROL                              0x80000000
#define FSL_ATOMIC                               0x10000000

#define FSL_NONBLOCKING_EXCEPTION                0x48000000
#define FSL_NONBLOCKING_CONTROL                  0xC0000000
#define FSL_NONBLOCKING_ATOMIC                   0x50000000
#define FSL_EXCEPTION_CONTROL                    0x88000000
#define FSL_EXCEPTION_ATOMIC                     0x18000000
#define FSL_CONTROL_ATOMIC                       0x90000000

#define FSL_NONBLOCKING_EXCEPTION_CONTROL        0xC8000000
#define FSL_NONBLOCKING_EXCEPTION_ATOMIC         0x58000000
#define FSL_NONBLOCKING_CONTROL_ATOMIC           0xD0000000
#define FSL_EXCEPTION_CONTROL_ATOMIC             0x98000000

#define FSL_NONBLOCKING_EXCEPTION_CONTROL_ATOMIC 0xD8000000

/**
 *@endcond
 */

/**
Performs a get function on an input stream of the MicroBlaze RISC-V processor
@param val    variable to sink data from get function
@param id     literal in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define getfslx(val, id, flags) \
	{ register unsigned int _item __asm__("t0"); \
		__asm__ __volatile__ (".word (" stringify(flags) ") | (" stringify(id) " << 15) | 0x22AB # rd = %0" : "=r" (_item)); \
		val = _item; }

/**
Performs a put function on an input stream of the MicroBlaze RISC-V processor
@param val    variable to source data to put function
@param id     literal in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define putfslx(val, id, flags) \
	{ register unsigned int _item __asm__("t0") = val; \
		__asm__ __volatile__ (".word (" stringify(flags) ") | 0x2B02B | (" stringify(id) " << 7) # rs1 = %0" :: "r" (_item)); }

/**
Performs a test get function on an input stream of the MicroBlaze RISC-V processor
@param val    variable to sink data from get function
@param id     literal in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define tgetfslx(val, id, flags) \
	{ register unsigned int _item __asm__("t0"); \
		__asm__ __volatile__ (".word 0x20000000 | (" stringify(flags) ") | (" stringify(id) " << 15) | 0x22AB # rd = %0" : "=r" (_item)); \
		val = _item; }

/**
Performs a put function on an input stream of the MicroBlaze RISC-V processor
@param id     FSL identifier
@param flags  valid FSL macro flags
*/
#define tputfslx(id, flags) \
	__asm__ __volatile__ (".word 0x20000000 | (" stringify(flags) ") | 0x302B | (" stringify(id) " << 7")

/**
Performs a get function on a dynamic input stream of the MicroBlaze RISC-V processor
@param val    variable to sink data from get function
@param var    variable in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define getdfslx(val, var, flags) \
	{ register unsigned int _item __asm__("t0"); \
		register unsigned int _link __asm__("t1") = var; \
		__asm__ __volatile__ (".word (" stringify(flags) ") | 0x6022AB # rd = %0, rs2 = %1" : "=r" (_item) : "r" (_link)); \
		val = _item; }

/**
Performs a put function on a dynamic input stream of the MicroBlaze RISC-V processor
@param val    variable to source data to put function
@param var    variable in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define putdfslx(val, var, flags) \
	{ register unsigned int _item __asm__("t0") = val; \
		register unsigned int _link __asm__("t1") = var; \
		__asm__ __volatile__ (".word (" stringify(flags) ") | 0x62B02B # rs1 = %0, rs2 = %1" :: "r" (_item), "r" (_link)); }

/**
Performs a test get function on a dynamic input stream of the MicroBlaze RISC-V processor;
@param val    variable to sink data from get function
@param var    variable in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define tgetdfslx(val, var, flags) \
	{ register unsigned int _item __asm__("t0"); \
		register unsigned int _link __asm__("t1") = var; \
		__asm__ __volatile__ (".word 0x20000000 | (" stringify(flags) ") | 0x6022AB # rd = %0, rs2 = %1" : "=r" (_item) : "r" (_link)); \
		val = _item; }

/**
Performs a test put function on a dynamic input stream of the MicroBlaze RISC-V processor
@param var    variable in the range of 0 to 15
@param flags  valid FSL macro flags
*/
#define tputdfslx(var, flags) \
	{ register unsigned int _link __asm__("t1") = var; \
		__asm__ __volatile__ (".word 0x20000000 | (" stringify(flags) ") | 0x60302B # rs2 = %0" :: "r" (_link)); }

#ifdef __cplusplus
}
#endif
#endif /* FSL_H */
/**
* @} End of "addtogroup microblaze_riscv_fsl_macro".
*/

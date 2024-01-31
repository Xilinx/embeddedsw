/******************************************************************************
*
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file riscv_interface.h
*
* @addtogroup riscv_pseudo_asm_macro RISC-V Pseudo-asm Macros and Interrupt Handling APIs
*
* RISC-V BSP includes macros to provide convenient access to various registers
* in the RISC-V processor. Some of these macros are very useful within
* exception handlers for retrieving information about the exception. Also,
* the interrupt handling functions help manage interrupt handling on RISC-V
* processor devices.To use these functions, include the header file
* riscv_interface.h in your source code
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 9.0   sa   05/16/22 Initial release
* 9.0   sa   06/21/23 Fix sleep related macros based on HW configuration.
*                     It fixes CR#1162997.
* 9.1   ml   11/16/23 Fix compilation errors reported with -std=c2x compiler flag
* </pre>
*
******************************************************************************/
#ifndef _RISCV_INTERFACE_H_
#define _RISCV_INTERFACE_H_

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_exception.h"
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void riscv_enable_interrupts(void);          /* Enable Interrupts */
extern void riscv_disable_interrupts(void);         /* Disable Interrupts */
extern void riscv_register_handler(                 /* Register external interrupt handler */
	XInterruptHandler Handler,
	void *DataPtr);

/**
 *@cond nocomments
 */
extern void riscv_invalidate_icache(void);                              /* Invalidate entire instruction cache */
extern void riscv_flush_icache(void);                                   /* Flush entire instruction cache */
extern void riscv_invalidate_icache_range(UINTPTR cacheaddr, u32 len);  /* Invalidate a part of instruction cache */
extern void riscv_flush_icache_range(UINTPTR cacheaddr, u32 len);       /* Flush a part of instruction cache */
extern void riscv_invalidate_dcache(void);                              /* Invalidate entire data cache */
extern void riscv_flush_dcache(void);                                   /* Flush entire data cache */
extern void riscv_invalidate_dcache_range(UINTPTR cacheaddr, u32 len);  /* Invalidate a part of data cache */
extern void riscv_flush_dcache_range(UINTPTR cacheaddr, u32 len);       /* Flush a part of data cache */
extern void riscv_scrub(void);                                          /* Scrub LMB and internal BRAM */

/* necessary for pre-processor */
#define stringify(s)    tostring(s)
#define tostring(s)     #s


/* FSL Access Macros */

/* Blocking Data Read and Write to FSL no. id */
#define getfsl(val, id)         { register unsigned int _item asm("t0"); \
		__asm volatile (".word (" stringify(id) " << 15) | 0x000022AB # rd = %0" : "=r" (_item)); \
		val = _item; }
#define putfsl(val, id)         { register unsigned int _item asm("t0") = val; \
		__asm volatile (".word 0x0002B02B | (" stringify(id) " << 7) # rs1 = %0" :: "r" (_item)); }

/* Non-blocking Data Read and Write to FSL no. id */
#define ngetfsl(val, id)        { register unsigned int _item asm("t0"); \
		__asm volatile (".word (" stringify(id) " << 15) | 0x400022AB # rd = %0" : "=r" (_item)); \
		val = _item; }
#define nputfsl(val, id)        { register unsigned int _item asm("t0") = val; \
		__asm volatile (".word 0x4002B02B | (" stringify(id) " << 7) # rs1 = %0" :: "r" (_item)); }

/* Blocking Control Read and Write to FSL no. id */
#define cgetfsl(val, id)        { register unsigned int _item asm("t0"); \
		__asm volatile (".word (" stringify(id) " << 15) | 0x800022AB # rd = %0" : "=r" (_item)); \
		val = _item; }
#define cputfsl(val, id)        { register unsigned int _item asm("t0") = val; \
		__asm volatile (".word 0x8002B02B | (" stringify(id) " << 7) # rs1 = %0" :: "r" (_item)); }

/* Non-blocking Control Read and Write to FSL no. id */
#define ncgetfsl(val, id)       { register unsigned int _item asm("t0"); \
		__asm volatile (".word (" stringify(id) " << 15) | 0xC00022AB # rd = %0" : "=r" (_item)); \
		val = _item; }
#define ncputfsl(val, id)       { register unsigned int _item asm("t0") = val; \
		__asm volatile (".word 0xC002B02B | (" stringify(id) " << 7) # rs1 = %0" :: "r" (_item)); }

/* Polling versions of FSL access macros. This makes the FSL access interruptible */
#define getfsl_interruptible(val, id)       { register unsigned int _item asm("t0");       \
		__asm volatile ("\n1:\n\t.word (" stringify(id) " << 15) | 0x400022AB # rd = %0\n\t" \
				      "csrr\tt1,0x7C0\n\t"           \
				      "andi\tt1,t1,0x1\n\t"          \
				      "bnez\tt1,1b\n"                \
				      : "=r" (_item) :: "t1");       \
		val = _item; }

#define putfsl_interruptible(val, id)       { register unsigned int _item asm("t0") = val; \
		__asm volatile ("\n1:\n\t.word 0x4002B02B | (" stringify(id) " << 7) # rs1 = %0\n\t" \
				      "csrr\tt1,0x7C0\n\t"             \
				      "andi\tt1,t1,0x1\n\t"            \
				      "bnez\tt1,1b\n"                  \
				      :: "r" (_item) : "t1"); }

#define cgetfsl_interruptible(val, id)      { register unsigned int _item asm("t0");       \
		__asm volatile ("\n1:\n\t.word (" stringify(id) " << 15) | 0xC00022AB # rd = %0\n\t" \
				      "csrr\tt1,0x7C0\n\t"           \
				      "andi\tt1,t1,0x1\n\t"          \
				      "bnez\tt1,1b\n"                \
				      : "=r" (_item) :: "t1");       \
		val = _item; }

#define cputfsl_interruptible(val, id)      { register unsigned int _item asm("t0") = val; \
		__asm volatile ("\n1:\n\t.word 0xC002B02B | (" stringify(id) " << 7) # rs1 = %0\n\t" \
				      "csrr\tt1,0x7C0\n\t"           \
				      "andi\tt1,t1,0x1\n\t"          \
				      "bnez\tt1,1b\n"                \
				      :: "r" (_item) : "t1"); }

/* FSL valid and error check macros. */
#define fsl_isinvalid(result)               __asm volatile ("csrr\t%0,0x7C0\n\t"             \
	"andi\t%0,%0,0x1\n\t"            \
	: "=r" (result))
#define fsl_iserror(error)                  __asm volatile ("csrr\t%0,0x7C0\n\t"             \
	"andi\t%0,%0,0x2"                \
	: "=r" (error))
/* Pseudo assembler instructions */
#ifndef XPAR_MICROBLAZE_RISCV_USE_SLEEP
#define XPAR_MICROBLAZE_RISCV_USE_SLEEP 1
#endif

#if XPAR_MICROBLAZE_RISCV_USE_SLEEP==1
#define mb_sleep()     	({ __asm__ __volatile__ ("wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==2
#define mb_hibernate()	({ __asm__ __volatile__ ("wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==4
#define mb_suspend()   	({ __asm__ __volatile__ ("wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==3
#define mb_sleep()     	({ __asm__ __volatile__ ("csrwi 0x7c4, 1 ; wfi\t"); })
#define mb_hibernate() 	({ __asm__ __volatile__ ("csrwi 0x7c4, 2 ; wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==5
#define mb_sleep()     	({ __asm__ __volatile__ ("csrwi 0x7c4, 1 ; wfi\t"); })
#define mb_suspend()   	({ __asm__ __volatile__ ("csrwi 0x7c4, 4 ; wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==6
#define mb_hibernate() 	({ __asm__ __volatile__ ("csrwi 0x7c4, 2 ; wfi\t"); })
#define mb_suspend()   	({ __asm__ __volatile__ ("csrwi 0x7c4, 4 ; wfi\t"); })

#elif XPAR_MICROBLAZE_RISCV_USE_SLEEP==7
#define mb_sleep()     	({ __asm__ __volatile__ ("csrwi 0x7c4, 1 ; wfi\t"); })
#define mb_hibernate() 	({ __asm__ __volatile__ ("csrwi 0x7c4, 2 ; wfi\t"); })
#define mb_suspend()   	({ __asm__ __volatile__ ("csrwi 0x7c4, 4 ; wfi\t"); })
#endif

#ifdef __cplusplus
}
#endif
#endif // _RISCV_INTERFACE_H_
/**
* @} End of "addtogroup riscv_pseudo_asm_macro".
*/

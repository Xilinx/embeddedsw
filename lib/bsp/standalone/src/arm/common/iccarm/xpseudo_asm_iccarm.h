/******************************************************************************
*
* Copyright (C) 2009 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpseudo_asm_iccarm.h
*
* This header file contains macros for using inline assembler code. It is
* written specifically for the IAR C/C++ compiler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 1.00a ecm/sdm  10/28/09 First release
* 3.12a asa		 11/02/13  Removed the macro mfcpsr to make it a function.
* 5.3	pkp		 10/09/15 Modified dsb, dmb and isb definitions
* 5.4	pkp		 03/02/16 Included header file instrinsic.h for assembly
*						  instructions definitions
* 6.2   kvn      03/03/17 Added support thumb mode
* </pre>
*
******************************************************************************/

#ifndef XPSEUDO_ASM_ICCARM_H  /* prevent circular inclusions */
#define XPSEUDO_ASM_ICCARM_H  /* by using protection macros */

/***************************** Include Files ********************************/

#include "xil_types.h"
#include <intrinsics.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/* necessary for pre-processor */
#define stringify(s)	tostring(s)
#define tostring(s)	#s

/* pseudo assembler instructions */

#define mtcpsr(v)	__asm volatile(\
			  "msr	cpsr_cf,%0\n"\
			  : : "r" (v)\
			)

#define cpsiei()	__asm volatile("cpsie	i\n")
#define cpsidi()	__asm volatile("cpsid	i\n")

#define cpsief()	__asm volatile("cpsie	f\n")
#define cpsidf()	__asm volatile("cpsid	f\n")



#define mtgpr(rn, v)	__asm volatile(\
			  "mov r" stringify(rn) ", %0 \n"\
			  : : "r" (v)\
			)

#define mfgpr(rn)	({u32 rval; \
			  __asm volatile(\
			    "mov %0,r" stringify(rn) "\n"\
			    : "=r" (rval)\
			  );\
			  rval;\
			 })

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
#define isb() __ISB();

/* Data Synchronization Barrier */
#define dsb() __DSB();

/* Data Memory Barrier */
#define dmb() __DMB();


/* Memory Operations */
#define ldr(adr)	({u32 rval; \
			  __asm volatile(\
			    "ldr	%0,[%1]"\
			    : "=r" (rval) : "r" (adr)\
			  );\
			  rval;\
			 })

#define ldrb(adr)	({u8 rval; \
			  __asm volatile(\
			    "ldrb	%0,[%1]"\
			    : "=r" (rval) : "r" (adr)\
			  );\
			  rval;\
			 })

#define str(adr, val)	__asm volatile(\
			  "str	%0,[%1]\n"\
			  : : "r" (val), "r" (adr)\
			)

#define strb(adr, val)	__asm volatile(\
			  "strb	%0,[%1]\n"\
			  : : "r" (val), "r" (adr)\
			)

/* Count leading zeroes (clz) */
#define clz(arg)	({u8 rval; \
			  __asm volatile(\
			    "clz	%0,%1"\
			    : "=r" (rval) : "r" (arg)\
			  );\
			  rval;\
			 })

/* CP15 operations */
#define mtcp(rn, v)	__asm volatile(\
			 "mcr " rn "\n"\
			 : : "r" (v)\
			);

/*#define mfcp(rn)	({u32 rval; \
			 __asm volatile(\
			   "mrc " rn "\n"\
			   : "=r" (rval)\
			 );\
			 rval;\
			 }) */

#define mfcp(rn, v)	__asm volatile ("mrc " rn : "=r" (v));

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
int mfcpsr (void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XPSEUDO_ASM_ICCARM_H */

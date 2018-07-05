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
* @file xpseudo_asm_rvct.h
*
* This header file contains macros for using __inline assembler code. It is
* written specifically for RVCT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sdm  11/18/09 First Release
* </pre>
*
******************************************************************************/

#ifndef XPSEUDO_ASM_RVCT_H  /* prevent circular inclusions */
#define XPSEUDO_ASM_RVCT_H  /* by using protection macros */

/***************************** Include Files ********************************/
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/* necessary for pre-processor */
#define stringify(s)	tostring(s)
#define tostring(s)	#s


#define mtcpsr(v)	{ volatile register u32 Reg __asm("cpsr");\
			  Reg = v; }

/* general purpose register read/write */
/*#define mfgpr(rn) 	({ unsigned int val; \
			  register unsigned int Reg __asm("r" stringify(rn));\
			  val = Reg; \
			  val;})*/

#define mtgpr(rn, v)	{ volatile register u32 Reg __asm("r" stringify(rn));\
			  Reg = v; }

/* CP15 operations */
/*#define mfcp(rn)	({ unsigned int val; \
			  val = register unsigned int Reg __asm(rn); \
			  val;})*/

#define mtcp(rn, v)	{ volatile register u32 Reg __asm(rn); \
			  Reg = v; }

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

__asm void cpsiei(void);

__asm void cpsidi(void);

__asm void cpsief(void);

__asm void cpsidf(void);

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
__asm void isb(void);

/* Data Synchronization Barrier */
__asm void dsb(void);

/* Data Memory Barrier */
__asm void dmb(void);

/* Memory Operations */
__asm u32 ldr(u32 adr);

__asm u32 ldrb(u32 adr);

__asm void str(u32 adr, u32 val);

__asm void strb(u32 adr, u32 val);

/* Count leading zeroes (clz) */
__asm u32 clz(u32 arg);
__asm u32 mfcpsr(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XPSEUDO_ASM_RVCT_H */

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

/* embedded assembler instructions */
__asm void cpsiei(void)
{
	cpsie	i
	bx	lr
}
__asm void cpsidi(void)
{
	cpsid	i
	bx	lr
}

__asm void cpsief(void)
{
	cpsie	f
	bx	lr
}

__asm void cpsidf(void)
{
	cpsid	f
	bx	lr
}

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
__asm void isb(void)
{
	isb
	bx lr
}

/* Data Synchronization Barrier */
__asm void dsb(void)
{
	dsb
	bx lr
}

/* Data Memory Barrier */
__asm void dmb(void)
{
	dmb
	bx lr
}

/* Memory Operations */
__asm u32 ldr(u32 adr)
{
	ldr	r0, [r0]
	bx	lr
}

__asm u32 ldrb(u32 adr)
{
	ldrb	r0, [r0]
	bx	lr
}

__asm void str(u32 adr, u32 val)
{
	str	r1, [r0]
	bx	lr
}

__asm void strb(u32 adr, u32 val)
{
	strb	r1, [r0]
	bx	lr
}

/* Count leading zeroes (clz) */
__asm u32 clz(u32 arg)
{
	clz	r0, r0
	bx	lr
}

__asm u32 mfcpsr(void)
{
	mrs	r0, cpsr
	bx lr
}

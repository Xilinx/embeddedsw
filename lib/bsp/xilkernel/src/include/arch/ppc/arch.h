/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
//----------------------------------------------------------------------------------------------------//
//! @file arch.h
//! PPC405 specific definitions, constants, declarations
//----------------------------------------------------------------------------------------------------//
#ifndef _PPC_ARCH_H
#define _PPC_ARCH_H

#define STACK_ALIGN                8

#ifndef __ASM__
#define SSTACK_PTR_ADJUST ((int)-8)

//! Process Management Data Structures
//! Context data structure

//////////////////////////////////////////////////////////////////////////////////////
//
// Context map:
//
//    +------------+  + 0
//    |     MSR    | 
//    +------------+  + 4
//    |     PC     | 
//    +------------+  + 8
//    |     LR     | 
//    +------------+  + 12
//    |     CTR    | 
//    +------------+  + 16
//    |     XER    | 
//    +------------+  + 20
//    |     CR     | 
//    +------------+  + 24
//    |     r0     | 
//    |      .     |
//    |      .     |
//    |      .     |
//    |     r31    | 
//    +------------+  + 152
//
//////////////////////////////////////////////////////////////////////////////////////
struct _process_context { 
    unsigned int regs[38]; 	// Process context information store
    char isrflag;
} __attribute__ ((packed, aligned(STACK_ALIGN)));

#define PCONTEXT_SIZ  (sizeof(struct _process_context))

#else

#define SSTACK_PTR_ADJUST      (-8)

#endif

#define CTX_INDEX_MSR           0
#define CTX_INDEX_PC            1
#define CTX_INDEX_LR            2
#define CTX_INDEX_CTR           3
#define CTX_INDEX_XER           4
#define CTX_INDEX_CR            5
#define CTX_INDEX_GPR(r)        (6 + r)

// Various offsets within the context structure
#define CTX_OFFSET              (0)                        // Offset of context structure within process structure
#define CTX_MSR_FIELD           (0)
#define CTX_PC_FIELD            (CTX_MSR_FIELD + 4)
#define CTX_LR_FIELD            (CTX_PC_FIELD  + 4)
#define CTX_CTR_FIELD           (CTX_LR_FIELD  + 4)
#define CTX_XER_FIELD           (CTX_CTR_FIELD + 4)
#define CTX_CR_FIELD            (CTX_XER_FIELD + 4)
#define CTX_GPR_FIELD           (CTX_CR_FIELD  + 4)
#define CTX_GPR_REG_FIELD(reg)  (CTX_GPR_FIELD + (reg * 4))
#define CTX_SIZE                (38 * 4)
#define ISRFLAG_OFFSET          (CTX_OFFSET + CTX_SIZE)

#define ISRFLAG_SYSTEM_CALL     0
#define ISRFLAG_NON_CRITICAL    1
#define ISRFLAG_CRITICAL        2
#define ISRFLAG_NEW_PROC        3

// Various stack sizes used in the kernel
#define PROCESS_STARTUP_STACKSZ    400

#endif /* _PPC_ARCH_H */

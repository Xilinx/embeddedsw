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
//! Microblaze specific definitions, constants, declarations
//----------------------------------------------------------------------------------------------------//
#ifndef _MB_ARCH_H
#define _MB_ARCH_H

#define STACK_ALIGN             4

#ifndef __ASM__

#define SSTACK_PTR_ADJUST ((int)0)
//! Process Management Data Structures
//! Context data structure
struct _process_context {
    unsigned int regs[35];
    char         isrflag;

} __attribute__ ((packed, aligned(STACK_ALIGN)));

#define PCONTEXT_SIZ  (sizeof(struct _process_context))

#else

#define SSTACK_PTR_ADJUST      (0)

#endif  /* __ASM__ */

// Various offsets within the context structure
#define CTX_OFFSET              (0)                        // Offset of context structure within process structure
#define CTX_REG_OFFSET(regnum)  (CTX_OFFSET + 4*(regnum))  // Offset of a register in the context save structure
#define CTX_SIZE                (35 * 4)
#define ISRFLAG_OFFSET          (CTX_OFFSET + CTX_SIZE)
#define ISRFLAG_SYSTEM_CALL     0
#define ISRFLAG_INTERRUPT       1
#define ISRFLAG_NEW_PROC        2


#define PROCESS_STARTUP_STACKSZ         400

#endif /* _MB_ARCH_H */

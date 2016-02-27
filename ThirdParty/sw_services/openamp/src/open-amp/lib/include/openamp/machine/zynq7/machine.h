/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_H
#define _MACHINE_H

#include "openamp/machine/machine_common.h"

/* Define bit values for the architecture's status register / machine state register /
 etc that are used to enable and disable interrupts for the given architecture. */
#define         ARM_AR_INTERRUPTS_DISABLE_BITS         0x000000C0
#define         ARM_AR_INTERRUPTS_ENABLE_BITS          0x00000000

/* This macro reads the current program status register (CPSR - all fields) */
#define         ARM_AR_CPSR_CXSF_READ(cpsr_cxsf_ptr)                                   \
                {                                                                      \
                    asm volatile("    MRS     %0, CPSR"                                \
                                     : "=r" (*(cpsr_cxsf_ptr))                         \
                                     : /* No inputs */ );                              \
                }

/* This macro writes the current program status register (CPSR - all fields) */
#define         ARM_AR_CPSR_CXSF_WRITE(cpsr_cxsf_value)                                \
                {                                                                      \
                    asm volatile("    MSR     CPSR_cxsf, %0"                           \
                                     : /* No outputs */                                \
                                     : "r" (cpsr_cxsf_value) );                        \
                }

/* This macro sets the interrupt related bits in the status register / control
 register to the specified value. */
#define         ARM_AR_INT_BITS_SET(set_bits)                                 	 \
                {                                                              	 \
                    int     tmp_val;                                           	 \
                                                                                 \
                    ARM_AR_CPSR_CXSF_READ(&tmp_val);                       		 \
                    tmp_val &= ~ARM_AR_INTERRUPTS_DISABLE_BITS;                  \
                    tmp_val |= set_bits;                                         \
                    ARM_AR_CPSR_CXSF_WRITE(tmp_val);                             \
                }

/* This macro gets the interrupt related bits from the status register / control
 register. */
#define         ARM_AR_INT_BITS_GET(get_bits_ptr)                              \
                {                                                               \
                    int     tmp_val;                                            \
                                                                                \
                    ARM_AR_CPSR_CXSF_READ(&tmp_val);                       \
                    tmp_val &= ARM_AR_INTERRUPTS_DISABLE_BITS;                 \
                    *get_bits_ptr = tmp_val;                                    \
                }

/* Macro used to make a 32-bit value with the specified bit set */
#define             ESAL_GE_MEM_32BIT_SET(bit_num)      (1UL<<(bit_num))

/* Macro used to make a 32-bit value with the specified bit clear */
#define             ESAL_GE_MEM_32BIT_CLEAR(bit_num)    ~(1UL<<(bit_num))

/* Translation table is 16K in size */
#define     ARM_AR_MEM_TTB_SIZE                    16*1024

/* Each TTB descriptor covers a 1MB region */
#define     ARM_AR_MEM_TTB_SECT_SIZE               1024*1024

/* Mask off lower bits of addr */
#define     ARM_AR_MEM_TTB_SECT_SIZE_MASK          (~(ARM_AR_MEM_TTB_SECT_SIZE-1UL))

/* Define shift to convert memory address to index of translation table entry (descriptor).
 Shift 20 bits (for a 1MB section) - 2 bits (for a 4 byte TTB descriptor) */
#define     ARM_AR_MEM_TTB_SECT_TO_DESC_SHIFT      (20-2)

/* Define domain access values */
#define     ARM_AR_MEM_DOMAIN_D0_MANAGER_ACCESS    0x3

#define     ARM_AR_MEM_TTB_DESC_BACKWARDS          ESAL_GE_MEM_32BIT_SET(4)
#define     ARM_AR_MEM_TTB_DESC_AP_MANAGER        (ESAL_GE_MEM_32BIT_SET(10)        |          \
                                                    ESAL_GE_MEM_32BIT_SET(11))
#define     ARM_AR_MEM_TTB_DESC_SECT               ESAL_GE_MEM_32BIT_SET(1)

/* Define translation table descriptor bits */
#define     ARM_AR_MEM_TTB_DESC_B                  ESAL_GE_MEM_32BIT_SET(2)
#define     ARM_AR_MEM_TTB_DESC_C                  ESAL_GE_MEM_32BIT_SET(3)
#define     ARM_AR_MEM_TTB_DESC_TEX                ESAL_GE_MEM_32BIT_SET(12)
#define     ARM_AR_MEM_TTB_DESC_S                  ESAL_GE_MEM_32BIT_SET(16)

/* Define all access  (manager access permission / not cachable / not bufferd)  */
#define     ARM_AR_MEM_TTB_DESC_ALL_ACCESS         (ARM_AR_MEM_TTB_DESC_AP_MANAGER |          \
                                                     ARM_AR_MEM_TTB_DESC_SECT)

typedef enum {
    NOCACHE,
    WRITEBACK,
    WRITETHROUGH
} CACHE_TYPE;

#endif				/* _MACHINE_H */

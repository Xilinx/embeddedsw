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

#ifndef _BAREMETAL_H
#define _BAREMETAL_H

#define         MEM_READ8(addr)         *(volatile unsigned char *)(addr)
#define         MEM_READ16(addr)        *(volatile unsigned short *)(addr)
#define         MEM_READ32(addr)        *(volatile unsigned long *)(addr)
#define         MEM_WRITE8(addr,data)   *(volatile unsigned char *)(addr) = (unsigned char)(data)
#define         MEM_WRITE16(addr,data)  *(volatile unsigned short *)(addr) = (unsigned short)(data)
#define         MEM_WRITE32(addr,data)  *(volatile unsigned long *)(addr) = (unsigned long)(data)

/* Define bit values for the architecture's status register / machine state register /
 etc that are used to enable and disable interrupts for the given architecture. */
#define         ARM_AR_INTERRUPTS_DISABLE_BITS         0x000000C0
#define         ARM_AR_INTERRUPTS_ENABLE_BITS          0x00000000

#define SWITCH_TO_SYS_MODE()    ARM_AR_CPSR_C_WRITE(ARM_AR_INT_CPSR_SYS_DISABLED); \
				ARM_AR_SP_WRITE(ARM_GE_STK_ALIGN(&ARM_AR_ISR_SYS_Stack[ARM_AR_ISR_STACK_SIZE-1]))

/* This define is used to add quotes to anything passed in */
#define         ARM_AR_QUOTES(x)           #x

/* This macro writes to a coprocessor register */
#define         ARM_AR_CP_WRITE(cp, op1, cp_value, crn, crm, op2)              \
                {                                                              \
                    asm volatile("    MCR    " ARM_AR_QUOTES(cp) ","           \
                                             #op1                              \
                                             ", %0, "                          \
                                             ARM_AR_QUOTES(crn) ","            \
                                             ARM_AR_QUOTES(crm) ","            \
                                             #op2                              \
                                    : /* No outputs */                         \
                                    : "r" (cp_value));                         \
                }

/* This macro reads from a coprocessor register */
#define         ARM_AR_CP_READ(cp, op1, cp_value_ptr, crn, crm, op2)           \
                {                                                              \
                    asm volatile("    MRC    " ARM_AR_QUOTES(cp) ","           \
                                             #op1                              \
                                             ", %0, "                          \
                                             ARM_AR_QUOTES(crn) ","            \
                                             ARM_AR_QUOTES(crm) ","            \
                                             #op2                                   \
                                        : "=r" (*(unsigned long *)(cp_value_ptr))   \
                                        : /* No inputs */ );                        \
                }

/* This macro executes a NOP instruction */
#define         ARM_AR_NOP_EXECUTE()                                           \
                {                                                              \
                    asm volatile("    NOP");                                   \
                }

/* This macro writes the c (control) bits of the current program status register (CPSR) */
#define         ARM_AR_CPSR_C_WRITE(c_bits)                                    \
                {                                                              \
                    asm volatile("    MSR     CPSR_c, %0"                      \
                                     : /* No outputs */                        \
                                     : "I"  (c_bits) );                        \
                }

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

/* This macro writes the stack pointer. */
#define         ARM_AR_SP_WRITE(stack_ptr)                                     \
                {                                                                   \
                    /* Set hardware stack pointer to passed in address */           \
                    asm volatile(" MOV     sp, %0"                                  \
                                 : : "r" (stack_ptr) );                             \
                }

/* This macro writes the stack pointer. */
#define         ARM_AR_SP_WRITE(stack_ptr)                                     \
                {                                                                   \
                    /* Set hardware stack pointer to passed in address */           \
                    asm volatile(" MOV     sp, %0"                                  \
                                 : : "r" (stack_ptr) );                             \
                }

/* This macro executes a ISB instruction */
#define         ARM_AR_ISB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    ISB");                                        \
                }

/* This macro executes a DSB instruction */
#define         ARM_AR_DSB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    DSB");                                        \
                }

#define 		MIDR_ARCH_MASK              		   0x000F0000	/* Main ID register's architecture mask */
#define 		MIDR_ARCH_ARMV7             		   0xF	/* ARMv7 */
#define 		MIDR_PART_NO_CORTEX_A      		   0xC00	/* Primary part number of Cortex-A series. */
#define 		MIDR_PART_NO_MASK           		   0x0000FF00	/* Primary part number mask  */
#define         ARM_AR_INTERRUPTS_DISABLE_BITS         0x000000C0
#define         ARM_AR_INTERRUPTS_ENABLE_BITS          0x00000000

/* Macro used to make a 32-bit value with the specified bit set */
#define             ESAL_GE_MEM_32BIT_SET(bit_num)      (1UL<<(bit_num))

/* Macro used to make a 32-bit value with the specified bit clear */
#define             ESAL_GE_MEM_32BIT_CLEAR(bit_num)    ~(1UL<<(bit_num))

/* CPSR bit defines / masks */
#define         ARM_AR_INT_CPSR_THUMB                  MEM_32BIT_SET(5)
#define         ARM_AR_INT_CPSR_MODE_MASK              0x0000001F
#define         ARM_AR_INT_CPSR_SYS_MODE               0x0000001F
#define         ARM_AR_INT_CPSR_IRQ_MODE               0x00000012
#define         ARM_AR_INT_CPSR_FIQ_MODE               0x00000011
#define         ARM_AR_INT_CPSR_SUP_MODE               0x00000013
#define         ARM_AR_INT_CPSR_E_BIT                  0x00000200
#define         ARM_AR_INT_CPSR_IRQ_BIT                MEM_32BIT_SET(7)
#define         ARM_AR_INT_CPSR_FIQ_BIT                MEM_32BIT_SET(6)
#define         ARM_AR_INT_CPSR_SYS_DISABLED           (ARM_AR_INT_CPSR_SYS_MODE |     \
                                                       ARM_AR_INTERRUPTS_DISABLE_BITS)

#define         ARM_AR_STK_ALIGNMENT               8
/* Define a generic alignment mask used to obtain a specified toolset required alignment */
#define             ARM_GE_STK_ALIGN_MASK              (~(ARM_AR_STK_ALIGNMENT - 1))

/* Generic macro to align stack end address when stack grows down */
#define             ARM_GE_STK_ALIGN(end_stk_addr)                                 \
                                (void *)((unsigned int)end_stk_addr & (unsigned int)ARM_GE_STK_ALIGN_MASK)

#define         ARM_AR_PERIPH_BASE             	0xF8F00000
#define         INT_GIC_CPU_BASE                (ARM_AR_PERIPH_BASE + 0x00000100)
#define         INT_GIC_DIST_BASE               (ARM_AR_PERIPH_BASE + 0x00001000)

/* CPU Interface Register Offsets */
#define         INT_GIC_CPU_CTRL                0x00
#define         INT_GIC_CPU_PRIORITY            0x04
#define         INT_GIC_CPU_POINT               0x08
#define         INT_GIC_CPU_ACK                 0x0c
#define         INT_GIC_CPU_ENDINT              0x10
#define         INT_GIC_CPU_RUNNING             0x14
#define         INT_IC_CPU_HIGHEST_PENDING      0x18
#define         INT_IC_CPU_NON_SECURE_POINT     0x1C
#define         INT_IC_CPU_IMPLEMENTOR          0xFC

/* Distribution Register Offsets */
#define         INT_GIC_DIST_CTRL               0x000
#define         INT_GIC_DIST_CTR                0x004
#define         INT_GIC_DIST_ISR                0x080
#define         INT_GIC_DIST_ENABLE_SET         0x100
#define         INT_GIC_DIST_ENABLE_CLEAR       0x180
#define         INT_GIC_DIST_PENDING_SET        0x200
#define         INT_GIC_DIST_PENDING_CLEAR      0x280
#define         INT_GIC_DIST_ACTIVE_BIT         0x300
#define         INT_GIC_DIST_PRI                0x400
#define         INT_GIC_DIST_TARGET             0x800
#define         INT_GIC_DIST_CONFIG             0xC00
#define         INT_GIC_DIST_PPI_STATUS         0xD00
#define         INT_GIC_DIST_SPI_STATUS         0xD04
#define         INT_GIC_DIST_SOFTINT            0xF00

/* Define value to disable all interrupts */
#define         INT_IRQ_DISABLE_ALL             0x00000000

/* Define value to enable interrupts on cpu */
#define         INT_CPU_ENABLE                  0x00000001
#define         INT_DIST_ENABLE                 0x00000001

/* Define Interrupt Ack Mask */
#define         INT_ACK_MASK                    0x000003FF

/* Define Spurious Int value */
#define         INT_SPURIOUS_INT               	1023

#define         ESAL_PR_ISR_GIC_NUM_PRI_REG    	16

/* Define number of GIC target registers */
#define         ESAL_PR_ISR_GIC_NUM_TARGET_REG	16

/* Define value to disable all interrupts */
#define         INT_DISABLE                     0x00000000

/* Define value to clear interrupt registers */
#define         INT_CLEAR                       0xFFFFFFFF

#define         GIC_SFI_TRIG_CPU_MASK           0x00FF0000
#define         GIC_SFI_TRIG_SATT_MASK          0x00008000
#define         GIC_SFI_TRIG_INTID_MASK         0x0000000F
#define         GIC_CPU_ID_BASE                (1 << 4)

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

/*********************************************
 * Common definitions
 *********************************************/
/* Define CP15 Register 1: control register bits */
#define     ARM_AR_MEM_CP15_CTRL_V                ESAL_GE_MEM_32BIT_SET(13)
#define     ARM_AR_MEM_CP15_CTRL_I                ESAL_GE_MEM_32BIT_SET(12)
#define     ARM_AR_MEM_CP15_CTRL_Z                ESAL_GE_MEM_32BIT_SET(11)
#define     ARM_AR_MEM_CP15_CTRL_W                ESAL_GE_MEM_32BIT_SET(3)
#define     ARM_AR_MEM_CP15_CTRL_C                ESAL_GE_MEM_32BIT_SET(2)
#define     ARM_AR_MEM_CP15_CTRL_A                ESAL_GE_MEM_32BIT_SET(1)
#define     ARM_AR_MEM_CP15_CTRL_M                ESAL_GE_MEM_32BIT_SET(0)

/* MVA Format SBZ mask */
#define     ARM_AR_MEM_MVA_SBZ_MASK             ~(ARM_AR_MEM_CACHE_LINE_SIZE - 1UL)

/* Defines related to Cache Level ID Register */
#define     ARM_AR_MEM_DCACHE_SIZE_SHIFT           16
#define     ARM_AR_MEM_CACHE_SIZE_BIT              4
#define     ARM_AR_MEM_CACHE_SIZE_MASK             0xF

/* Define all access  (manager access permission / not cachable / not bufferd)  */
#define     ARM_AR_MEM_TTB_DESC_ALL_ACCESS         (ARM_AR_MEM_TTB_DESC_AP_MANAGER |          \
                                                     ARM_AR_MEM_TTB_DESC_SECT)

/* Macro used to check if a value is aligned to the required boundary.
 Returns NU_TRUE if aligned; NU_FALSE if not aligned. The required alignment must be a power of 2 (2, 4, 8, 16, 32, etc) */
#define             MEM_ALIGNED_CHECK(value, req_align)                             \
                        (((unsigned int)(value) & ((unsigned int)(req_align) - (unsigned int)1)) == (unsigned int)0)

/* Macro used to align a data pointer to next address that meets the specified
 required alignment. The required alignment must be a power of 2 (2, 4, 8, 16, 32, etc) */
#define             MEM_PTR_ALIGN(ptr_addr, req_align)                              \
                        ((MEM_ALIGNED_CHECK(ptr_addr, req_align)) ? (void *)ptr_addr : \
                         (void *)(((unsigned int)(ptr_addr) & (unsigned int)(~((req_align) - 1))) + (unsigned int)(req_align)))

/* Coprocessor registers */
#define         ARM_AR_CP0                 p0
#define         ARM_AR_CP1                 p1
#define         ARM_AR_CP2                 p2
#define         ARM_AR_CP3                 p3
#define         ARM_AR_CP4                 p4
#define         ARM_AR_CP5                 p5
#define         ARM_AR_CP6                 p6
#define         ARM_AR_CP7                 p7
#define         ARM_AR_CP8                 p8
#define         ARM_AR_CP9                 p9
#define         ARM_AR_CP10                p10
#define         ARM_AR_CP11                p11
#define         ARM_AR_CP12                p12
#define         ARM_AR_CP13                p13
#define         ARM_AR_CP14                p14
#define         ARM_AR_CP15                p15

/* CRn and CRm register values */
#define         ARM_AR_C0                  c0
#define         ARM_AR_C1                  c1
#define         ARM_AR_C2                  c2
#define         ARM_AR_C3                  c3
#define         ARM_AR_C4                  c4
#define         ARM_AR_C5                  c5
#define         ARM_AR_C6                  c6
#define         ARM_AR_C7                  c7
#define         ARM_AR_C8                  c8
#define         ARM_AR_C9                  c9
#define         ARM_AR_C10                 c10
#define         ARM_AR_C11                 c11
#define         ARM_AR_C12                 c12
#define         ARM_AR_C13                 c13
#define         ARM_AR_C14                 c14
#define         ARM_AR_C15                 c15

#define ARM_AR_ISR_STACK_SIZE              2 * 1024

extern unsigned char ARM_AR_ISR_IRQ_Data[ARM_AR_ISR_STACK_SIZE];
extern unsigned char ARM_AR_ISR_FIQ_Data[ARM_AR_ISR_STACK_SIZE];
extern unsigned char ARM_AR_ISR_SUP_Stack[ARM_AR_ISR_STACK_SIZE];
extern unsigned char ARM_AR_ISR_SYS_Stack[ARM_AR_ISR_STACK_SIZE];


#define TLB_SIZE        2*1024*1024	/* TLB memory size */

#define PERIPH_BASE     0xE0000000	/* Peripheral registers start */
#define PERIPH_SIZE     3 *1024 *1024	/* size */

#define SLCR_BASE       0xF8000000	/* SLCR registers start */
#define SLCR_SIZE       3 * 1024	/* size */

#define CPU_BASE        0xF8F00000	/* CPU registers start */
#define CPU_SIZE        12 *1024	/* size */

typedef enum {
	TRIG_NOT_SUPPORTED,
	TRIG_RISING_EDGE,
	TRIG_FALLING_EDGE,
	TRIG_LEVEL_LOW,
	TRIG_LEVEL_HIGH,
	TRIG_RISING_FALLING_EDGES,
	TRIG_HIGH_LOW_RISING_FALLING_EDGES
} INT_TRIG_TYPE;

typedef enum {
	NOCACHE,
	WRITEBACK,
	WRITETHROUGH
} CACHE_TYPE;

/* This macro executes a ISB instruction */
#define         ARM_AR_ISB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    ISB");                                        \
                }

/* This macro executes a DSB instruction */
#define         ARM_AR_DSB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    DSB");                                        \
                }

/* CLIDR and CCSIDR mask values */
#define     ARM_AR_MEM_CLIDR_LOC_MASK              0x7000000
#define     ARM_AR_MEM_CCSIDR_LINESIZE_MASK        0x7
#define     ARM_AR_MEM_CCSIDR_ASSOC_MASK           0x3FF
#define     ARM_AR_MEM_CCSIDR_NUMSET_MASK          0x7FFF

/* CLIDR and CCSIDR shift values */
#define     ARM_AR_MEM_CLIDR_LOC_RSHT_OFFSET       24
#define     ARM_AR_MEM_CCSIDR_ASSOC_RSHT_OFFSET    3
#define     ARM_AR_MEM_CCSIDR_NUMSET_RSHT_OFFSET   13

/* Extract 'encoded' line length of the cache */
#define     ARM_AR_MEM_CCSIDR_LINESIZE_GET(ccsidr_reg) (ccsidr_reg &                           \
                                                         ARM_AR_MEM_CCSIDR_LINESIZE_MASK)

/* Extract 'encoded' way size of the cache */
#define     ARM_AR_MEM_CCSIDR_ASSOC_GET(ccsidr_reg)    (ARM_AR_MEM_CCSIDR_ASSOC_MASK &        \
                                                        (ccsidr_reg >>                          \
                                                         ARM_AR_MEM_CCSIDR_ASSOC_RSHT_OFFSET))

/* Extract 'encoded' maximum number of index size */
#define     ARM_AR_MEM_CCSIDR_NUMSET_GET(ccsidr_reg)   (ARM_AR_MEM_CCSIDR_NUMSET_MASK &       \
                                                        (ccsidr_reg >>                          \
                                                         ARM_AR_MEM_CCSIDR_NUMSET_RSHT_OFFSET))

/* Refer to chapter B3.12.31 c7, Cache and branch predictor maintenance functions in the
 ARM Architecture Reference Manual ARMv7-A and ARMv7-R Edition 1360*/
/* Calculate # of bits to be shifted for set size and way size */

/* log2(line size in bytes) = ccsidr_linesize + 2 + logbase2(4) */
#define     ARM_AR_MEM_L_CALCULATE(linesize)           (linesize + 2 + 2)

/* log2(nsets) = 32 - way_size_bit_pos */

/* Find the bit position of way size increment */
#define     ARM_AR_MEM_A_CALCULATE(assoc, a_offset_ref)                                        \
            {                                                                                   \
                unsigned int  temp_pos = 0x80000000;                                                  \
                                                                                                \
                *a_offset_ref = 0;                                                              \
                                                                                                \
                /* Logic to count the number of leading zeros before the first 1 */             \
                while(!((assoc & temp_pos) == temp_pos))                                        \
                {                                                                               \
                    (*a_offset_ref)++;                                                          \
                    temp_pos = temp_pos >> 1;                                                   \
                }                                                                               \
            }

/* Factor way, cache number, index number */
#define     ARM_AR_MEM_DCCISW_SET(dccisw_ref, level, numsets, assoc, l_offset, a_offset)       \
            {                                                                                   \
                *dccisw_ref = (level | (numsets << l_offset) | (assoc << a_offset));            \
            }

/* This macro extracts line size, assoc and set size from CCSIDR */
#define     ARM_AR_MEM_CCSIDR_VALS_GET(linesize_ref, assoc_ref, numsets_ref,                   \
                                        l_offset_ref, a_offset_ref)                             \
            {                                                                                   \
                unsigned int  ccsidr_val;                                                             \
                                                                                                \
                /* Read the selected cache's CCSIDR */                                          \
                ARM_AR_CP_READ(ARM_AR_CP15, 1, &ccsidr_val,                           \
                                    ARM_AR_C0, ARM_AR_C0, 0);                         \
                                                                                                \
                /* Extract 'encoded' line length of the cache */                                \
                *linesize_ref = ARM_AR_MEM_CCSIDR_LINESIZE_GET(ccsidr_val);                    \
                                                                                                \
                /* Extract 'encoded' way size of the cache */                                   \
                *assoc_ref = ARM_AR_MEM_CCSIDR_ASSOC_GET(ccsidr_val);                          \
                                                                                                \
                /* Extract 'encoded' maximum number of index size */                            \
                *numsets_ref = ARM_AR_MEM_CCSIDR_NUMSET_GET(ccsidr_val);                       \
                                                                                                \
                /* Calculate # of bits to be shifted for set size and way size */               \
                                                                                                \
                /* log2(line size in bytes) = ccsidr_linesize + 2 + log2(4) */                  \
                *l_offset_ref = ARM_AR_MEM_L_CALCULATE(*linesize_ref);                         \
                                                                                                \
                /* log2(nsets) = 32 - way_size_bit_pos */                                       \
                ARM_AR_MEM_A_CALCULATE(*assoc_ref, a_offset_ref);                              \
            }

/* This macro invalidates all of the instruction cache at the core level. */
#define     ARM_AR_MEM_ICACHE_ALL_INVALIDATE()                         \
            {                                                           \
                ARM_AR_CP_WRITE(ARM_AR_CP15, 0,               \
                                     0, ARM_AR_C7,                 \
                                     ARM_AR_C5, 0);                \
            }

/* This macro invalidates all of the cache at the core level. */
#define     ARM_AR_MEM_CACHE_ALL_INVALIDATE()                                                  \
            {                                                                                   \
                ARM_AR_MEM_ICACHE_ALL_INVALIDATE();                                            \
                ARM_AR_MEM_DCACHE_ALL_INVALIDATE();                                             \
            }

/* This macro invalidates and flushes all of the cache at the core level. */
#define     ARM_AR_MEM_CACHE_ALL_FLUSH_INVALIDATE()                                                  \
            {                                                                                   \
                ARM_AR_MEM_DCACHE_ALL_FLUSH_INVALIDATE();                                      \
                ARM_AR_MEM_ICACHE_ALL_INVALIDATE();                                            \
            }

/* This macro invalidates all of the data cache at the core level. */
#define     ARM_AR_MEM_DCACHE_ALL_OP(type)                                                     \
            {                                                                                   \
                unsigned int  clidr_val = 0;                                                          \
                unsigned int  clidr_loc = 0;                                                          \
                unsigned int  cache_number = 0;                                                       \
                unsigned int  cache_type = 0;                                                         \
                unsigned int  ccsidr_linesize = 0;                                                    \
                unsigned int  ccsidr_assoc = 0;                                                       \
                int   ccsidr_numsets = 0;                                                     \
                int   way_size_copy = 0;                                                      \
                unsigned int  set_size_bit_pos = 0;                                                   \
                unsigned int  cache_number_pos = 0;                                                   \
                unsigned int  way_size_bit_pos = 0;                                                   \
                unsigned int  set_way_value = 0;                                                      \
                                                                                                \
                                                                                                \
                /* Read CLIDR to extract level of coherence (LOC) */                            \
                ARM_AR_CP_READ(ARM_AR_CP15, 1, &clidr_val,                            \
                                    ARM_AR_C0, ARM_AR_C0, 1);                         \
                                                                                                \
                /* Extract LOC from CLIDR and align it at bit 1 */                              \
                clidr_loc = (clidr_val & ARM_AR_MEM_CLIDR_LOC_MASK) >>                         \
                            ARM_AR_MEM_CLIDR_LOC_RSHT_OFFSET;                                  \
                                                                                                \
                /* Proceed only iff LOC is non-zero */                                          \
                if (clidr_loc != 0)                                                             \
                {                                                                               \
                    do                                                                          \
                    {                                                                           \
                        /* Extract cache type from CLIDR */                                     \
                        cache_number_pos = cache_number + (cache_number >> 1);                  \
                        cache_type = (clidr_val >> cache_number_pos) & 0x7;                     \
                                                                                                \
                        /* Continue only iff data cache */                                      \
                        if (cache_type >= 2)                                                    \
                        {                                                                       \
                            /* Select desired cache level in CSSELR */                          \
                            ARM_AR_CP_WRITE(ARM_AR_CP15, 2, cache_number,             \
                                                 ARM_AR_C0, ARM_AR_C0, 0);            \
                                                                                                \
                            ARM_AR_ISB_EXECUTE();                                          \
                                                                                                \
                            /* Get data like linesize, assoc and set size */                    \
                            ARM_AR_MEM_CCSIDR_VALS_GET(&ccsidr_linesize,                       \
                                                        &ccsidr_assoc,                          \
                                                        &ccsidr_numsets,                        \
                                                        &set_size_bit_pos,                      \
                                                        &way_size_bit_pos);                     \
                                                                                                \
                            do                                                                  \
                            {                                                                   \
                                way_size_copy = ccsidr_assoc;                                   \
                                                                                                \
                                do                                                              \
                                {                                                               \
                                    /* Factor way, cache number, index number */                \
                                    ARM_AR_MEM_DCCISW_SET(&set_way_value, cache_number,        \
                                                           ccsidr_numsets, way_size_copy,       \
                                                           set_size_bit_pos,                    \
                                                           way_size_bit_pos);                   \
                                                                                                \
                                    /* Execute invalidate if type = 0 */                        \
                                    if (type == 0)                                              \
                                    {                                                           \
                                        ARM_AR_CP_WRITE(ARM_AR_CP15, 0,               \
                                                             set_way_value,                     \
                                                             ARM_AR_C7,                    \
                                                             ARM_AR_C6, 2);                \
                                    }                                                           \
                                    else                                                        \
                                    {                                                           \
                                        ARM_AR_CP_WRITE(ARM_AR_CP15, 0,               \
                                                             set_way_value,                     \
                                                             ARM_AR_C7,                    \
                                                             ARM_AR_C14, 2);               \
                                    }                                                           \
                                                                                                \
                                /* decrement the way */                                         \
                                } while((--way_size_copy) >= 0);                                \
                                                                                                \
                            /* decrement the set */                                             \
                            } while((--ccsidr_numsets) >= 0);                                   \
                                                                                                \
                        } /* end if */                                                          \
                                                                                                \
                        /* Increment cache number */                                            \
                        cache_number += 2;                                                      \
                                                                                                \
                    /* end do-while */                                                          \
                    } while(clidr_loc >= cache_number);                                         \
                                                                                                \
                }                                                                               \
                                                                                                \
                /* Switch back to cache level 0 in CSSELR */                                    \
                ARM_AR_CP_WRITE(ARM_AR_CP15, 2, 0,                                    \
                                     ARM_AR_C0, ARM_AR_C0, 0);                        \
                                                                                                \
                /* Sync */                                                                      \
                ARM_AR_DSB_EXECUTE();                                                      \
                ARM_AR_ISB_EXECUTE();                                                      \
            }

/* This macro invalidates all of the data cache at the core level. */
#define     ARM_AR_MEM_DCACHE_ALL_INVALIDATE()         ARM_AR_MEM_DCACHE_ALL_OP(0)

/* This macro invalidates all of the data cache at the core level. */
#define     ARM_AR_MEM_DCACHE_ALL_FLUSH_INVALIDATE()   ARM_AR_MEM_DCACHE_ALL_OP(1)

#define         ARM_AR_MEM_CACHE_DISABLE()                                                     \
                {                                                                               \
                    unsigned int  cp15_ctrl_val;                                                      \
                                                                                                \
                    /* Read current CP15 control register value */                              \
                    ARM_AR_CP_READ(ARM_AR_CP15, 0, &cp15_ctrl_val, ARM_AR_C1, ARM_AR_C0, 0); \
                                                                                                \
                    /* Clear instruction cache enable and data cache enable bits */             \
                    cp15_ctrl_val &= ~(ARM_AR_MEM_CP15_CTRL_I  | ARM_AR_MEM_CP15_CTRL_C);      \
                                                                                                \
                    /* Write updated CP15 control register value */                             \
                    ARM_AR_CP_WRITE(ARM_AR_CP15, 0, cp15_ctrl_val, ARM_AR_C1, ARM_AR_C0, 0); \
                    ARM_AR_NOP_EXECUTE();                                                  \
                    ARM_AR_NOP_EXECUTE();                                                  \
                    ARM_AR_NOP_EXECUTE();                                                 \
                }

void arm_ar_map_mem_region(unsigned int vrt_addr, unsigned int phy_addr,
			   unsigned int size, int is_mem_mapped,
			   CACHE_TYPE cache_type);

#endif				/* _BAREMETAL_H */

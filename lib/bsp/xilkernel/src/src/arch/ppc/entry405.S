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

/*
//-----------------------------------------------------------------------------------------//
//! @file
//! entry.S - Represents all the entry and exit points into the kernel
//!           i.e - System calls, Interrupts and Traps  
//-----------------------------------------------------------------------------------------//
*/
#include <sys/entry.h>
#include <sys/arch.h>
#include <xreg405.h>

.extern entry_mode
.extern resched
.extern ptable
.extern current_process, ctx_save_process                                                       /* Pointers to corresponding process control blocks */
.extern restore_kernel_context
.extern timer_int_handler
.extern pit_reset, pit_disable
.extern kernel_irq_stack_ptr
.extern proc_restore_state
.extern syscall_table
       
#define NUM_TO_REG(num)                 r ## num
                
#define STACK_SAVE(reg, offset)                                                 \
        stw     reg, offset(1); 

#define STACK_RESTORE(reg, offset)                                              \
        lwz     reg, offset(1); 

/* Uses r11 */                                       
#define DISABLE_INTERRUPTS                                                      \
        mfmsr   11;                                                             \
        rlwinm  11, 11, 0, 17, 15;              /* Turn OFF EE bit */           \
        mtmsr   11; 
       
/* Uses r11 */                               
#define ENABLE_INTERRUPTS                                                                       \
        mfmsr   11;                                                                             \
        ori     11, 11, XREG_MSR_NON_CRITICAL_INTERRUPT_ENABLE;         /* Turn ON EE bit */    \
        mtmsr   11; 


#define GET_CURRENT_PROC(reg)                                                                           \
        lis     reg, current_process@ha;        /* Require 2 instructions to load 32-bit value */       \
        lwz     reg, current_process@l(reg);
        
#define GET_CTX_SAVE_PROC(reg)                                                                          \
        lis     reg, ctx_save_process@ha;       /* Require 2 instructions to load 32-bit value */       \
        lwz     reg, ctx_save_process@l(reg); 
                                
/*--------------------------------------------------------------------------------------*/
/* System Call Handling                                                                 */
/*--------------------------------------------------------------------------------------*/

/* Syscall Macros */

#define SYSCALL_STACK_FRAME_SIZ         (32)                    

#define SC_NEXT_LR_FIELD        (4)        
#define SC_LR_FIELD             (8)
#define SC_PADDING_BEG          (SC_LR_FIELD + 4)                                                         /* 4 bytes of padding */
#define SC_PADDING_END          (SC_PADDING_BEG + 4)    
#define SC_R3_FIELD             (SC_PADDING_END)
#define SC_R4_FIELD             (SC_R3_FIELD + 4)                
#define SC_R11_FIELD            (SC_R4_FIELD + 4)
#define SC_R12_FIELD            (SC_R11_FIELD + 4)
        
#define SC_OFFSET(reg)          (SC_R ## reg ## _FIELD)
                        
#define SYSCALL_SAVE_TMP                                                        \
        STACK_SAVE(11, SC_OFFSET(11));                                          \
        STACK_SAVE(12, SC_OFFSET(12));        

#define SYSCALL_RESTORE_TMP                                                     \
        STACK_RESTORE(11, SC_OFFSET(11));                                       \
        STACK_RESTORE(12, SC_OFFSET(12));                

#define SYSCALL_SAVE_LR                                                         \
        mflr    11;                                                             \
        STACK_SAVE(11, SC_LR_FIELD);
        
#define SYSCALL_RESTORE_LR                                                      \
        STACK_RESTORE(11, SC_LR_FIELD);                                         \
        mtlr    11;                                                              

#define SYSCALL_SAVE_RET                                                        \
        STACK_SAVE(3, SC_OFFSET(3));                                            \
        STACK_SAVE(4, SC_OFFSET(4));                

#define SYSCALL_RESTORE_RET                                                     \
        STACK_RESTORE(3, SC_OFFSET(3));                                         \
        STACK_RESTORE(4, SC_OFFSET(4));                
                                                          
/*
 * SYSTEM CALL HANDLER
 * -------------------       
 *      - Disable interrupts
 *      - Save volatiles and a few other important registers. Do not save non-volatiles, they are callee-saved
 *      - Look up the address for the system call and vector there (system call number in r8)
 *      - After handling system call, (and assuming we were not rescheduled in between), check to see if rescheduling is
 *        required. If so, then call the scheduler and if context switch is required, save context and restore new context.
 *
 * FIXME 
 * -----
 *      - Need to save and restore SDA structures to support separate executable mode
 *        
 * STACK FRAME STRUCTURE (stack grows upwards in the figure below)
 * ---------------------
 *              
 *      - Sticks to standard EABI frame conventions
 *      - Will work from a debugger
 *      
 *      +-------------+         + 0
 *      | Back Chain  |
 *      +-------------+         + 4
 *      |   Next LR   |
 *      +-------------+         + 8
 *      |     LR      |     
 *      +-------------+         + 12        
 *      |   Padding   |                Padding adjusts size to multiple of 8-bytes (4 bytes here)
 *      +-------------+         + 16
 *      |     r3      |
 *      +-------------+         + 20
 *      |     r4      |
 *      +-------------+         + 24                       
 *      |     r11     |
 *      +-------------+         + 28       
 *      |     r12     |            
 *      +-------------+         + 32
 *      |      .      |
 *      |      .      |
 *
 *                
 */
        .global system_call_handler
        .section .text
        .align 2
        .type   system_call_handler@function
system_call_handler:
        stwu    1, -SYSCALL_STACK_FRAME_SIZ(1);         /* Make space on the stack and save backchain */
        SYSCALL_SAVE_TMP;
        SYSCALL_SAVE_LR;       
        lis     11, entry_mode@ha;
        lbz     11, entry_mode@l(11);
        cmpwi   11, 1;
        beq     handle_syscall;                         /* Do not disable interrupts if entry mode is ENTRY_MODE_KERNEL */ 
        DISABLE_INTERRUPTS;
handle_syscall:
        /* SYSCALL_SAVE_SDA_REGS;  */
        /* RESTORE_KERNEL_CONTEXT; */
        add     8, 8, 8;                                /* Load syscall addresss from syscall table     */
        add     8, 8, 8;                                /* (4 * syscall number)                         */
        addis   8, 8, syscall_table@ha;
        lwz     8, syscall_table@l(8);
        mtlr    8; 
        blrl;                                           /* Make the system call here                    */
        lis     11,  entry_mode@ha;
        lbz     11,  entry_mode@l(11);
        cmpwi   11, 1;
        beq     out_syscall;                            /* Entered system call in kernel mode. Quit early */
        SYSCALL_SAVE_RET;                               /* Save return value of the system call to avoid stomping them in calls below */        
        lis     11,  resched@ha;
        lbz     11,  resched@l(11);
        cmpwi   11, 1;
        bne     ret_syscall;                            /* No rescheduling. Lets get out of the system call */
        bl      scheduler;
        cmpwi   3, 1;
        beq     ret_syscall;                            /* Scheduler returns 1 => No rescheduling */ 
        GET_CTX_SAVE_PROC (3);
        bl      save_context;                           /* Call save_context with the pointer to the context structure in r5 */
        li      12, 0; 
        lis     11, ctx_save_process@ha;
        stw     12, ctx_save_process@l(11);
        cmpwi   3, 0;
        beq     restore_context;                        /* When "I" (who was saved in save_context above) am restored, I will have 1 in r3 */
                                                        /* Otherwise, a new process is to execute here. So restore the new context  */
ret_syscall:
        bl      proc_restore_state ;                    /* Call C routine to restore application level state. Machine level state restored below */
        /* SYSCALL_RESTORE_SDA_REGS; */
        SYSCALL_RESTORE_RET;                            /* Restore the return value of the system call */
        lis     11,  entry_mode@ha;
        lbz     11,  entry_mode@l(11);
        cmpwi   11, 1;
        beq     out_syscall;                            /* Entered system call in kernel mode. Don't enable interrupts */                
        ENABLE_INTERRUPTS;
out_syscall:
        SYSCALL_RESTORE_LR;
        SYSCALL_RESTORE_TMP;       
        addi   1, 1, SYSCALL_STACK_FRAME_SIZ;
        blr; 
        

/*--------------------------------------------------------------------------------------*/
/* Interrupt Handling                                                                   */
/*--------------------------------------------------------------------------------------*/
                
/* IRQ Macros */
        
#define IRQ_STACK_FRAME_SIZ             (16)

#define IRQ_NEXT_LR_FIELD       (4)        
#define IRQ_PADDING_END         (IRQ_NEXT_LR_FIELD)                                     /* 0 bytes of padding */        
#define IRQ_R3_FIELD            (IRQ_PADDING_END + 4)
#define IRQ_R11_FIELD           (IRQ_R3_FIELD + 4)      
        
#define IRQ_OFFSET(reg) (IRQ_R ## reg ## _FIELD)

#define BR              3                                                               /* Context save base register */
        
#define IRQ_SAVE_TMP                                                            \
        STACK_SAVE(3, IRQ_OFFSET(3));                                           \
        STACK_SAVE(11, IRQ_OFFSET(11));                

#define IRQ_RESTORE_TMP                                                         \
        STACK_RESTORE(3, IRQ_OFFSET(3));                                        \
        STACK_RESTORE(11, IRQ_OFFSET(11));                

#define CTX_SAVE_MSR_IN_SYSCALL(tr)                                             \
        mfmsr   tr;                                                             \
        stw     tr, CTX_MSR_FIELD(BR) ;                                         

#define CTX_RESTORE_MSR_IN_SYSCALL(tr)                                          \
        lwz     tr, CTX_MSR_FIELD(BR) ;                                         \
        mtmsr   tr;                                                             
    
        
#define CTX_SAVE_MSR_PC_CRITICAL(tr)                                            \
        mfsrr3  tr;                                                             \
        stw     tr, CTX_MSR_FIELD(BR);                                          \
        mfsrr2  tr;                                                             \
        stw     tr, CTX_PC_FIELD(BR);                                                

#define CTX_SAVE_MSR_PC_NON_CRITICAL(tr)                                        \
        mfsrr1  tr;                                                             \
        stw     tr, CTX_MSR_FIELD(BR);                                          \
        mfsrr0  tr;                                                             \
        stw     tr, CTX_PC_FIELD(BR);                                                
        
#define CTX_RESTORE_MSR_PC_CRITICAL(tr)                                         \
        lwz     tr, CTX_MSR_FIELD(BR);                                          \
        mtsrr3  tr;                                                             \
        lwz     tr, CTX_PC_FIELD(BR);                                           \
        mtsrr2  tr; 

#define CTX_RESTORE_MSR_PC_NON_CRITICAL(tr)                                     \
        lwz     tr, CTX_MSR_FIELD(BR);                                          \
        mtsrr1  tr;                                                             \
        lwz     tr, CTX_PC_FIELD(BR);                                           \
        mtsrr0  tr; 

#define CTX_SAVE_MSR_PC(type, tr)                                               \
        CTX_SAVE_MSR_PC_ ## type (tr);

#define CTX_RESTORE_MSR_PC(type, tr)                                            \
        CTX_RESTORE_MSR_PC_ ## type (tr);        
        
#define CTX_SAVE_STATE_REGS(tr)                                                 \
        mflr    tr;                                                             \
        stw     tr, CTX_LR_FIELD(BR);                                           \
        mfctr   tr;                                                             \
        stw     tr, CTX_CTR_FIELD(BR);                                          \
        mfxer   tr;                                                             \
        stw     tr, CTX_XER_FIELD(BR);                                          \
        mfcr    tr;                                                             \
        stw     tr, CTX_CR_FIELD(BR);                                           
       

#define CTX_RESTORE_STATE_REGS(tr)                                              \
        lwz     tr, CTX_LR_FIELD(BR);                                           \
        mtlr    tr;                                                             \
        lwz     tr, CTX_CTR_FIELD(BR);                                          \
        mtctr   tr;                                                             \
        lwz     tr, CTX_XER_FIELD(BR);                                          \
        mtxer   tr;                                                             \
        lwz     tr, CTX_CR_FIELD(BR);                                           \
        mtcr    tr;                                                             

#define CTX_SAVE_GPREGS(sr)                                                     \
        stmw    sr, CTX_GPR_REG_FIELD(sr)(BR);

#define CTX_RESTORE_GPREGS(sr)                                                  \
        lmw     sr, CTX_GPR_REG_FIELD(sr)(BR);                                                   

#define CTX_SAVE_REG(reg)                                                       \
        stw     reg, CTX_GPR_REG_FIELD(reg)(BR);

#define CTX_RESTORE_REG(reg)                                                    \
        lwz     reg, CTX_GPR_REG_FIELD(reg)(BR);        
        
#define IRQ_RETURN_CRITICAL                                                     \
        rfci; 
        
#define IRQ_RETURN_NON_CRITICAL                                                 \
        rfi; 
             
#define EXCEPTION_ENTRY(type)                                                                                            \
        stwu    1, -IRQ_STACK_FRAME_SIZ(1);                     /* Make space on the stack and save backchain */         \
        IRQ_SAVE_TMP;                                           /* Free up some temporaries for our use here  */         \
        GET_CURRENT_PROC (BR);                                  /* Get the ctx ptr into ctx save base register*/         \
        CTX_SAVE_MSR_PC (type, 11);                             /* Save MSR and PC                            */         \
        CTX_SAVE_STATE_REGS (11);                               /* Save state regs; Use 12 as temporary       */         \
        CTX_SAVE_REG (0);                                       /* Save all gp registers (0 - 31) in context structure */\
        CTX_SAVE_REG (1);                                                                                                \
        CTX_SAVE_REG (2);                                                                                                \
        CTX_SAVE_GPREGS (4);                                                                                             \
        li      11, ISRFLAG_ ## type;                                                                                    \
        stb     11, ISRFLAG_OFFSET(BR);                                                                                 
                
#define EXCEPTION_EXIT(type)                                                                                             \
        GET_CURRENT_PROC (BR);                                  /* Get the ctx ptr into ctx save base register*/         \
        li      11, 0;                                          /* Reset ISR flag */                                     \
        stb     11, ISRFLAG_OFFSET(BR);                                                                                  \
        CTX_RESTORE_MSR_PC (type, 11);                          /* Restore MSR and PC        */                          \
        CTX_RESTORE_STATE_REGS (11);                            /* Restore state regs; Use 12 as temporary    */         \
        CTX_RESTORE_REG (0);                                    /* Restore all GP regs                        */         \
        CTX_RESTORE_REG (1);                                                                                             \
        CTX_RESTORE_REG (2);                                                                                             \
        CTX_RESTORE_GPREGS (4);                                                                                          \
        IRQ_RESTORE_TMP;                                        /* Restore temporaries here   */                         \
        addi    1, 1, IRQ_STACK_FRAME_SIZ;                      /* Free up stack space */                                \
        IRQ_RETURN_ ## type;                                                                                             \

#define IRQ_ENTRY(type)                                                                                                  \
        stwu    1, -IRQ_STACK_FRAME_SIZ(1);                     /* Make space on the stack and save backchain */         \
        IRQ_SAVE_TMP;                                           /* Free up some temporaries for our use here  */         \
        GET_CURRENT_PROC (BR);                                  /* Get the ctx ptr into ctx save base register*/         \
        CTX_SAVE_MSR_PC (type, 11);                             /* Save MSR and PC                            */         \
        CTX_SAVE_STATE_REGS (11);                               /* Save state regs; Use 12 as temporary       */         \
        CTX_SAVE_REG (0);                                       /* Save all gp registers (0 - 31) in context structure */\
        CTX_SAVE_REG (1);                                                                                                \
        CTX_SAVE_REG (2);                                                                                                \
        CTX_SAVE_GPREGS (4);                                                                                             \
        li      11, ISRFLAG_ ## type;                           /* Mark entry as from ISR */                             \
        stb     11, ISRFLAG_OFFSET(BR);                                                                                  \
        li      11, 1;                                                                                                   \
        lis     12, entry_mode@ha;                              /* Mark kernel entry mode */                             \
        stb     11, entry_mode@l(12);                                                                                    \
        lis     10, kernel_irq_stack_ptr@ha;                    /* Switch stacks */                                      \
        lwz     1, kernel_irq_stack_ptr@l(10);                                                                                
                
#define IRQ_EXIT(type)                                                                                                   \
        CTX_RESTORE_MSR_PC (NON_CRITICAL, 11);                  /* Restore MSR and PC        */                          \
        CTX_RESTORE_STATE_REGS (11);                            /* Restore state regs; Use 12 as temporary    */         \
        CTX_RESTORE_REG (0);                                    /* Restore all GP regs                        */         \
        CTX_RESTORE_REG (1);                                                                                             \
        CTX_RESTORE_REG (2);                                                                                             \
        CTX_RESTORE_GPREGS (4);                                                                                          \
        IRQ_RESTORE_TMP;                                        /* Restore temporaries here   */                         \
        addi    1, 1, IRQ_STACK_FRAME_SIZ;                      /* Free up stack space */                                \
        IRQ_RETURN_ ## type;
        

/*
 * Exception handler 
 * - For CRITICAL and NON-CRITICAL exceptions that the kernel is unaware of.
 *   - i.e Rescheduling within such exceptions will NOT be handled               
 * - Save the complete context of the current task (in the current processes' context structure)
 * - Invoke the main exception handler
 * - Restore context and return
 *
 * STACK FRAME STRUCTURE (stack grows upwards in the figure below)
 * ---------------------
 *              
 *      - Sticks to standard EABI frame conventions
 *      - Will work from a debugger
 *      
 *      +-------------+         + 0
 *      | Back Chain  |
 *      +-------------+         + 4
 *      |   Next LR   |                    
 *      +-------------+         + 8        
 *      |   Padding   |                Padding adjusts size to multiple of 8-bytes (0 bytes here)
 *      +-------------+         + 8                        
 *      |     r3      |
 *      +-------------+         + 12       
 *      |     r11     |            
 *      +-------------+         + 16
 *      |      .      |
 *      |      .      |
 *
 */

#define EXCEPTION_HANDLER(type, int_num, base)                                                                          \
        VECTOR_LABEL(base);                                                                                             \
        EXCEPTION_ENTRY (type);                                                                                         \
        li      3, int_num;                                     /* Load the interrupt ordinal in r3 */                  \
        lis     9, XExc_VectorTable@ha;                         /* Load the base address of the vector table */         \
        la      9, XExc_VectorTable@l(9);                                                                               \
        slwi    0, 3, 4;                                                                                                \
        add     9, 9, 0;                                                                                                \
        lwz     11, 0(9);                                                                                               \
        lwz     3, 4(9);                                                                                                \
        mtlr    11;                                                                                                     \
        blrl;                                                    /* Handle the exception here */                        \
        EXCEPTION_EXIT (type);                                   /* Return from the exception here */                   

/*
 * IRQ handler 
 * - ONLY for EXTERNAL EXCEPTIONS (EE), critical input interrupt and PIT exceptions. 
 * - The kernel is aware of these exceptions.
 * - Save the complete context of the current task
 * - Switch to kernel interrupt stack
 * - Mark our current entry mode as ENTRY_KERNEL
 * - Invoke the main IRQ handler
 * - If rescheduling occurred within the kernel, invoke the scheduler       
 * - Call context restore routine to restore the currently chosen context.
 *
 * STACK FRAME STRUCTURE (stack grows upwards in the figure below)
 * ---------------------
 *              
 *      - Sticks to standard EABI frame conventions
 *      - Will work from a debugger
 *
 *      +-------------+         + 0
 *      | Back Chain  |
 *      +-------------+         + 4
 *      |   Next LR   |                    
 *      +-------------+         + 8        
 *      |   Padding   |                Padding adjusts size to multiple of 8-bytes (0 bytes here)
 *      +-------------+         + 8                        
 *      |     r3      |
 *      +-------------+         + 12       
 *      |     r11     |            
 *      +-------------+         + 16
 *      |      .      |
 *      |      .      |
 *
 */
                        
#define IRQ_HANDLER(type, int_num, base)                                                                                \
        VECTOR_LABEL(base);                                                                                             \
        IRQ_ENTRY (type);                                                                                               \
       /* bl      pit_disable;    */                                                                                    \
       /* GET_CURRENT_PROC (BR);  */                                                                                    \
        li      3, int_num;                                     /* Load the interrupt ordinal in r3 */                  \
        lis     9, XExc_VectorTable@ha;                         /* Load the base address of the vector table */         \
        la      9, XExc_VectorTable@l(9);                                                                               \
        slwi    0, 3, 4;                                                                                                \
        add     9, 9, 0;                                                                                                \
        lwz     11, 0(9);                                                                                               \
        lwz     3, 4(9);                                                                                                \
        mtlr    11;                                                                                                     \
        blrl;                                                    /* Handle the exception here */                        \
        li      10, 0;                                           /* Reset entry mode flag */                            \
        lis     12, entry_mode@ha;                                                                                      \
        stb     10, entry_mode@l(12);                                                                                   \
        lis     12, resched@ha;                                  /* Check the reschedule flag */                        \
        lbz     12, resched@l(12);                                                                                      \
        cmpwi   12, 1;                                                                                                  \
        bne     out_irq_ ## base;                                                                                       \
        bl      scheduler;                                                                                              \
        li      10, 0;                                                                                                  \
        lis     12, ctx_save_process@ha;                                                                                \
        stw     10, ctx_save_process@l(12);                                                                             \
out_irq_ ## base ##:                                                                                                    \
        b       restore_context;                                                                                        
                      
        
/*--------------------------------------------------------------------------------------*/
/* Context Save and Restore                                                             */
/*--------------------------------------------------------------------------------------*/        

/*
 * Restore Context
 * - Doesn't care about any current state. Throws it all away
 * - Picks up the context from the context pointed to by current_process
 * - Refreshes PIT budget always
 * - Restores volatiles only if restoring from ISR context.                     
 */
        .global restore_context
        .section .text  
        .align 2      
        .type   restore_context@function
restore_context:
        bl      proc_restore_state; 
        bl      pit_reset;
        GET_CURRENT_PROC (BR);                                  /* Get the ctx ptr into ctx save base register */                 
        lbz     11, ISRFLAG_OFFSET(BR);                         
        li      12, 0;                                                                                               
        stb     12, ISRFLAG_OFFSET(BR);                         /* Clear ISR Flag */
        cmpwi   11, ISRFLAG_SYSTEM_CALL;                        /* This is a system call entry */
        beq     syscall_restore;
        cmpwi   11, ISRFLAG_NEW_PROC;                           /* This is a new process entry */
        beq     new_proc_restore;                                                                                              
        cmpwi   11, ISRFLAG_CRITICAL;                           /* This is an entry from a CRITICAL ISR */
        beq     crit_isr_restore;                         
non_crit_isr_restore:                                           /* Else this is an entry from a non-critical ISR */
        CTX_RESTORE_MSR_PC (NON_CRITICAL, 11);                  /* Restore MSR and PC        */                          
        CTX_RESTORE_STATE_REGS (11);                            /* Restore state regs; Use 12 as temporary    */         
        CTX_RESTORE_REG (0);                                    /* Restore all GP regs                        */         
        CTX_RESTORE_REG (1);                                                                                             
        CTX_RESTORE_REG (2);                                                                                             
        CTX_RESTORE_GPREGS (4);                                                                                          
        IRQ_RESTORE_TMP;                                        /* Restore temporaries here   */                         
        addi    1, 1, IRQ_STACK_FRAME_SIZ;                      /* Free up stack space */                                
        IRQ_RETURN_NON_CRITICAL;                                                                                             
crit_isr_restore:               
        CTX_RESTORE_MSR_PC (CRITICAL, 11);                      /* Restore MSR and PC        */                          
        CTX_RESTORE_STATE_REGS (11);                            /* Restore state regs; Use 12 as temporary    */         
        CTX_RESTORE_REG (0);                                    /* Restore all GP regs                        */         
        CTX_RESTORE_REG (1);                                                                                             
        CTX_RESTORE_REG (2);                                                                                             
        CTX_RESTORE_GPREGS (4);                                                                                          
        IRQ_RESTORE_TMP;                                        /* Restore temporaries here   */                         
        addi    1, 1, IRQ_STACK_FRAME_SIZ;                      /* Free up stack space */                                
        IRQ_RETURN_CRITICAL;
new_proc_restore:                                               /* Else this is an exit out from a newly created process */
        CTX_RESTORE_MSR_PC (NON_CRITICAL, 11);                  /* Restore MSR and PC        */                          
        CTX_RESTORE_STATE_REGS (11);                            /* Restore state regs; Use 12 as temporary    */         
        CTX_RESTORE_REG (0);                                    /* Restore all GP regs                        */         
        CTX_RESTORE_REG (1);                                                                                             
        CTX_RESTORE_REG (2);                                                                                             
        CTX_RESTORE_GPREGS (4);                                                                                          
        IRQ_RETURN_NON_CRITICAL;                                                                                             
syscall_restore:
        CTX_RESTORE_MSR_IN_SYSCALL(11) ; 
        CTX_RESTORE_STATE_REGS (11);
        CTX_RESTORE_REG (1);
        CTX_RESTORE_REG (2);
        CTX_RESTORE_GPREGS (13); 
        li      3, 1;                                           /* Return 1 indicating return from restore context */   
        blr; 

/*
 * Save context 
 * - Saves only kernel context
 * - Invoked only from "suspend" (ISR saves its own context)
 * - Indicate ISRFLAG 0
 * - Pointer to process structure in r3
 * - Needs to save lesser context than an ISR. Only Dedicated and non-volatile registers need to be saved.
 * - The current processes stack will be continued to use for a while till a restore is done.
 */

        .global save_context
        .section .text  
        .align 2      
        .type   save_context@function
save_context:
        CTX_SAVE_MSR_IN_SYSCALL(12) ;     
        CTX_SAVE_STATE_REGS (12);               /* Save state registers using 12 as temporary */
        CTX_SAVE_REG (1);        
        CTX_SAVE_REG (2);
        CTX_SAVE_GPREGS (13); 
        li     12, 0; 
        stb    12, ISRFLAG_OFFSET(BR);          /* Save context always invoked from a system call */
out_save_context:
        li      3, 0;                           /* Save context returns zero */
        blr; 
        
        
#define VECTOR_LABEL(base)                                                      \
        .org _vectorbase + 0x ## base;                                          \
        .global _vector ## base;                                                \
_vector ## base ##:                ;
        


        /*
         *---------------------------------------------------------------------
         *
         * Vector table.
         *
         * Define the vector table for all exceptions. The code for each
         * exception is assembled inline at each vector entry point. Most
         * PPC exceptions are 0x100 bytes apart, so there is 256 bytes of
         * code space to handle an exception. The current exception code
         * spans about 204 bytes, this code size fits.
         *
         * NOTE:
         *
         * Vectors 0x1000 (programmable interval timer) and 0x1010 
         * (fixed interval timer) and vectors 0x1010 (fixed interval timer) and 
         * 0x1020 (watchdog timer) are only 16 bytes apart and don't have enough 
         * room for the exception handling code to fit between them. Fortunately, 
         * there is a hole in the exception location sequence. Vectors 0x1300 
         * through 0x1F00 are not used in the 405. So as a work-around, relocate 
         * the exception code for vectors 0x1010 and 0x1020 (where there isn't space) 
         * to the unused locations 0x1300 and 0x1400 (where there is room).
         * Locations 0x1010 and 0x1020 merely contain a branch to the proper
         * relocated exception code; which fits within the 16 bytes of space
         * of these "special" vector locations.
         *
         * If in the future this code is used for a different PPC processor,
         * this vector table may need to be changed.
         *
         *---------------------------------------------------------------------
         */
        
        .section ".vectors","ax" 
        .global _vectorbase     
_vectorbase:

        EXCEPTION_HANDLER (CRITICAL, 0, 0000) ;                 /* Vector 0x0000, Jump to zero.                         */
        EXCEPTION_HANDLER (CRITICAL, 1, 0100) ;                 /* Vector 0x0100, Critical interrupt.                   */
        EXCEPTION_HANDLER (CRITICAL, 2, 0200) ;                 /* Vector 0x0200, Machine Check interrupt.              */
        EXCEPTION_HANDLER (NON_CRITICAL, 3, 0300) ;             /* Vector 0x0300, Data Storage interrupt.               */
        EXCEPTION_HANDLER (NON_CRITICAL, 4, 0400) ;             /* Vector 0x0400, Instruction Storage interrupt.        */
        IRQ_HANDLER       (NON_CRITICAL, 5, 0500) ;             /* Vector 0x0500, External interrupt.                   */
        EXCEPTION_HANDLER (NON_CRITICAL, 6, 0600) ;             /* Vector 0x0600, Alignment interrupt.                  */
        EXCEPTION_HANDLER (NON_CRITICAL, 7, 0700) ;             /* Vector 0x0700, Program Interrupt.                    */
        EXCEPTION_HANDLER (NON_CRITICAL, 8, 0800) ;             /* Vector 0x0800, FPU Unavailable interrupt.            */
        .org _vectorbase + 0x0C00
        b system_call_handler;

        EXCEPTION_HANDLER (NON_CRITICAL, 10, 0F20) ;            /* Vector 0x0F20, APU Available interrupt.              */
        
        .org _vectorbase + 0x1000                               /* Vector 0x1000, PIT interrupt.                        */
        VECTOR_LABEL(1000);                                     /* This vector need the workaround described above      */
        b _vector1300;                                          

        .org _vectorbase + 0x1010                               /* Vector 0x1010, FIT interrupt.                        */
        VECTOR_LABEL(1010);                                     /* This vector need the workaround described above      */
        b _vector1400;                                          
              
        EXCEPTION_HANDLER (CRITICAL, 13, 1020) ;                /* Vector 0x1020, Watchdog Timer interrupt.             */
        EXCEPTION_HANDLER (NON_CRITICAL, 14, 1100) ;            /* Vector 0x1100, Data TLB Miss interrupt.              */
        EXCEPTION_HANDLER (NON_CRITICAL, 15,1200);              /* Vector 0x1200, Instruction TLB Miss interrupt.       */
        IRQ_HANDLER       (NON_CRITICAL, 11, 1300) ;            /* Real vector code 0x1000, PIT interrupt exception. Worksaround space span problem */
        EXCEPTION_HANDLER (NON_CRITICAL, 12, 1400) ;            /* Real vector code 0x1010, FIT interrupt exception. Worksaround space span problem */
        EXCEPTION_HANDLER (CRITICAL, 16, 2000) ;                /* Vector 0x2000, Debug interrupt.                      */

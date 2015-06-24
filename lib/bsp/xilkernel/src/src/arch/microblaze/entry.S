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
#include <os_config.h>
#include <sys/entry.h>
#include <sys/arch.h>
#include <xparameters.h>
#include <microblaze_exceptions_g.h>

.extern current_pid
.extern entry_mode
.extern resched
.extern ptable
.extern current_process, ctx_save_process                                                       /* Pointers to corresponding process control blocks */
.extern XIntc_DeviceInterruptHandler
.extern restore_kernel_context
.extern timer_int_handler
.extern pit_reset, pit_disable
.extern kernel_irq_stack_ptr
.extern kernel_irq_stack_ptr_end
.extern proc_restore_state
.extern syscall_table
.extern microblaze_report_exception
                                
#define NUM_TO_REG(num)                 r ## num
#define REG_OFFSET(regnum)              (4 * (regnum))
        
#define GET_CURRENT_PROC(reg)                                                   \
        lwi     reg, r0, current_process; 
        
#define GET_CTX_SAVE_PROC(reg)                                                  \
        lwi     reg, r0, ctx_save_process; 

/* Microblaze specific register's index in the context structure */        
#define MSR                             32
#define SHR_REG				33
#define SLR_REG				34
   
#define PUSH_REG(regnum)                                                        \
        swi     NUM_TO_REG(regnum), r1, REG_OFFSET(regnum)

#define POP_REG(regnum)                                                         \
        lwi     NUM_TO_REG(regnum), r1, REG_OFFSET(regnum)      

/* Uses r11 */        
#define CTX_SAVE_REG(regnum)                                                    \
        swi     NUM_TO_REG(regnum), r11, CTX_REG_OFFSET(regnum)

/* Uses r11 */                
#define CTX_RESTORE_REG(regnum)                                                 \
        lwi     NUM_TO_REG(regnum), r11, CTX_REG_OFFSET(regnum)

#define CTX_SAVE_SP                                                             \
        CTX_SAVE_REG(1);        

#define CTX_RESTORE_SP                                                          \
        CTX_RESTORE_REG(1); 

#define CTX_SAVE_LR                                                             \
        CTX_SAVE_REG(15); 

#if (XPAR_MICROBLAZE_USE_STACK_PROTECTION == 1)

#define CTX_SAVE_STACK_HIGH_REG                                                 \
        mfs     r12, rshr;							\
	swi     r12, r11, CTX_REG_OFFSET(SHR_REG);

#define CTX_SAVE_STACK_LOW_REG                                                 \
        mfs     r12, rslr;						       \
	swi     r12, r11, CTX_REG_OFFSET(SLR_REG);

#endif /*XPAR_MICROBLAZE_USE_STACK_PROTECTION */
#define CTX_RESTORE_LR                                                          \
        CTX_RESTORE_REG(15); 

/* Uses r11, stomps r12 */   
#define CTX_SAVE_MSR                                                            \
        mfs     r12, rmsr;                                                      \
        swi     r12, r11, CTX_REG_OFFSET(MSR);

/* Uses r11, stomps r12 */   
#define CTX_RESTORE_MSR                                                         \
        lwi     r12, r11, CTX_REG_OFFSET(MSR);                                  \
        mts     rmsr, r12;                                                      \
        bri     4; 
        
#if (XPAR_MICROBLAZE_USE_STACK_PROTECTION == 1)
#define CTX_RESTORE_STACK_HIGH_REG						\
	lwi     r12, r11, CTX_REG_OFFSET(SHR_REG);				\
	mts     rshr, r12;   							\
	bri     4; 

#define CTX_RESTORE_STACK_LOW_REG						\
	lwi     r12, r11, CTX_REG_OFFSET(SLR_REG);				\
	mts     rslr, r12;   							\
	bri     4; 
#endif /*XPAR_MICROBLAZE_USE_STACK_PROTECTION*/

#if (XPAR_MICROBLAZE_USE_STACK_PROTECTION == 1)             

#define CTX_SAVE_STATE_REGS                                                     \
        CTX_SAVE_MSR;                                                           \
        CTX_SAVE_LR;                                                            \
        CTX_SAVE_SP;								\
        CTX_SAVE_STACK_HIGH_REG;						\
        CTX_SAVE_STACK_LOW_REG;							\
        CTX_SAVE_REG(14);                                                       \
        CTX_SAVE_REG(16);                                                       \
        CTX_SAVE_REG(17);                                                       \
        CTX_SAVE_REG(18);
        

#define CTX_RESTORE_STATE_REGS                                                  \
        CTX_RESTORE_MSR;                                                        \
        CTX_RESTORE_LR;                                                         \
        CTX_RESTORE_SP;                                                         \
        CTX_RESTORE_STACK_HIGH_REG;						\
        CTX_RESTORE_STACK_LOW_REG;						\
        CTX_RESTORE_REG(14);                                                    \
        CTX_RESTORE_REG(16);                                                    \
        CTX_RESTORE_REG(17);                                                    \
        CTX_RESTORE_REG(18);                                                                       
#else

#define CTX_SAVE_STATE_REGS                                                     \
        CTX_SAVE_MSR;                                                           \
        CTX_SAVE_LR;                                                            \
        CTX_SAVE_SP;								\
        CTX_SAVE_REG(14);                                                       \
        CTX_SAVE_REG(16);                                                       \
        CTX_SAVE_REG(17);                                                       \
        CTX_SAVE_REG(18);
        

#define CTX_RESTORE_STATE_REGS                                                  \
        CTX_RESTORE_MSR;                                                        \
        CTX_RESTORE_LR;                                                         \
        CTX_RESTORE_SP;                                                         \
        CTX_RESTORE_REG(14);                                                    \
        CTX_RESTORE_REG(16);                                                    \
        CTX_RESTORE_REG(17);                                                    \
        CTX_RESTORE_REG(18);                                                      
#endif


#define DISABLE_INTERRUPTS                                                      \
        mfs r11, rmsr;                                                          \
        andi r11, r11, ~2;                                                      \
        mts rmsr, r11;                                                          \
        bri 4; 
        
#define ENABLE_INTERRUPTS                                                       \
        mfs r11, rmsr;                                                          \
        ori r11, r11, 2;                                                        \
        mts rmsr, r11;                                                          \
        bri 4; 


#define RESTORE_KERNEL_CONTEXT                                                  \
        lwi     r2,  r0, kernelr2;                                              \
        lwi     r13, r0, kernelr13;         
              
/*--------------------------------------------------------------------------------------*/
/* System Call Handling                                                                 */
/*--------------------------------------------------------------------------------------*/        

/* Syscall Macros */

#define SYSCALL_STACK_FRAME_SIZ         (4*32)
                
#define SYSCALL_SAVE_TMP                                                        \
        PUSH_REG(11);                                                           \
/*      PUSH_REG(12);                                                           */

#define SYSCALL_RESTORE_TMP                                                     \
        POP_REG(11);                                                            \
/*      POP_REG(12);                                                            */

#define SYSCALL_SAVE_SDA_REGS                                                   \
        PUSH_REG(2);                                                            \
        PUSH_REG(13);
        
#define SYSCALL_RESTORE_SDA_REGS                                                \
        POP_REG(2);                                                             \
        POP_REG(13);                                                                    

#define SYSCALL_SAVE_LR                                                         \
        PUSH_REG(15);                                                           \

#define SYSCALL_RESTORE_LR                                                      \
        POP_REG(15);                                                            \

#define SYSCALL_SAVE_RET                                                        \
        PUSH_REG (3);                                                           \
        PUSH_REG (4);                                                           \
                
#define SYSCALL_RESTORE_RET                                                     \
        POP_REG (3);                                                            \
        POP_REG (4);                                                            \

#define MSR_VM_MASK             0x00002000
#define MSR_VMS_MASK            0x00004000
#define MSR_EIP_MASK            0x00000200    
#define MSR_EE_MASK             0x00000100
#define MSR_INTR_MASK           0x00000002


#if (XPAR_MICROBLAZE_USE_MMU >= 2) && !defined (XILKERNEL_MB_MPU_DISABLE)
#define SYSCALL_MSR_SET_VM                                                      \
        mfs r11, rmsr;                                                          \
        ori r11, r11, (MSR_VM_MASK | MSR_VMS_MASK | MSR_EE_MASK);               \
        mts rmsr, r11;                                                          \
        bri 4;

#define IRQ_MSR_SET_VM                                                          \
        mfs r11, rmsr;                                                          \
        ori r11, r11, (MSR_VM_MASK | MSR_VMS_MASK | MSR_EE_MASK);               \
        mts rmsr, r11;                                                          \
        bri 4;

#define EXCEPTION_MSR_SET_VM                                                    \
        mfs r11, rmsr;                                                          \
        ori r11, r11, (MSR_VM_MASK | MSR_VMS_MASK | MSR_EE_MASK);               \
        mts rmsr, r11;                                                          \
        bri 4;         
#endif

/*
 * SYSTEM CALL HANDLER
 * -------------------
 *      - Disable interrupts
 *      - Save volatiles and a few other important registers. Do not save non-volatiles, they are callee-saved
 *      - Look up the address for the system call and vector there (system call number in r10)
 *      - After handling system call, (and assuming we were not rescheduled in between), check to see if rescheduling is
 *        required. If so, then call the scheduler and if context switch is required, save context and restore new context.
 *
 * FIXME
 * -----
 *      - Need to save and restore SDA structures to support separate executable mode
 *      - Stack frame does not stick to standard EABI frame conventions
 *
 * STACK FRAME STRUCTURE
 * ---------------------
 *
 *      +-------------+         + 0
 *      |     r0      |
 *      +-------------+                
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      +-------------+         + 124                
 *      |     r31     |
 *      +-------------+         + 128
 *      |      .      |
 *      |      .      |        
 *
 */
                        
        .global _exception_handler
        .section .text
        .align 2        
        .ent system_call_handler
_exception_handler:     
system_call_handler:
        addik   r1, r1, -SYSCALL_STACK_FRAME_SIZ;                               
        SYSCALL_SAVE_TMP;
#if (XPAR_MICROBLAZE_USE_MMU >= 2) && !defined (XILKERNEL_MB_MPU_DISABLE)
        /* MicroBlaze sets VM bit to 0 and sets VMS = 1 when executing syscall instruction (bralid r15, 0x8).
           We want VM mode enabled always. So we set it here again */
        SYSCALL_MSR_SET_VM; 
#endif            
        lbui    r11, r0, entry_mode;            /* Do not disable interrupts if entry mode is ENTRY_MODE_KERNEL */
        bnei    r11, handle_syscall;                                            
        DISABLE_INTERRUPTS;                                             
handle_syscall:
        SYSCALL_SAVE_LR;
        /* SYSCALL_SAVE_SDA_REGS;       */
        /* RESTORE_KERNEL_CONTEXT;      */
        add     r10, r10, r10;                  /* Load syscall addresss from syscall table     */
        add     r10, r10, r10;                  /* (4 * syscall number)                         */                                              
        lwi     r10, r10, syscall_table;                                                
        brald   r15, r10;                       /* Make the system call here                    */              
        nop;
        lbui    r11, r0, entry_mode;                                            
        bnei    r11, out_syscall;
        SYSCALL_SAVE_RET;                       /* Save return value of the system call to avoid stomping them in calls below */          
        lbui    r11, r0, resched;
        beqi    r11, ret_syscall;               /* No rescheduling. Lets get out of the system call */
        brlid   r15, scheduler;                 
        nop;                            
        bnei    r3, ret_syscall;                /* Scheduler returns 1 => No rescheduling */
        GET_CTX_SAVE_PROC (r5);
        brlid   r15, save_context;              /* Call save_context with the pointer to the context structure in r5 */
        nop;
        swi     r0, r0, ctx_save_process;       /* Set the ctx_save_process identifier to NULL */
        beqi    r3, restore_context;            /* When I (who was saved in save_context above) am restored, I will have 1 in r3 */
                                                /* Otherwise a new process is to execute. So restore the new context  */
ret_syscall:
        brlid   r15, proc_restore_state;        /* Call C routine to restore application level state. Machine level state restored below */
        nop;        
        SYSCALL_RESTORE_RET;                    /* Restore the return value of the system call */
        /* SYSCALL_RESTORE_SDA_REGS;    */              
        lbui    r11, r0, entry_mode;                                            
        bnei    r11, out_syscall;                                                       
        ENABLE_INTERRUPTS;                                                      
out_syscall:                                                                    
        SYSCALL_RESTORE_TMP;
        SYSCALL_RESTORE_LR;                                                     
        rtsd    r15, 8;
        addik   r1, r1, SYSCALL_STACK_FRAME_SIZ;                                        
        .end system_call_handler        

/*--------------------------------------------------------------------------------------*/
/* Interrupt Handling                                                                   */
/*--------------------------------------------------------------------------------------*/

/* IRQ Macros */

#define IRQ_STACK_FRAME_SIZ             (4*13)
                
#define IRQ_SAVE_TMP                                                            \
        PUSH_REG (11);                                                          \
        PUSH_REG (12);                                                          

#define IRQ_RESTORE_TMP                                                         \
        POP_REG (11);                                                           \
        POP_REG (12);                                                           

#define IRQ_SAVE_LR                                                             \
        PUSH_REG (15);                                                          \

#define IRQ_RESTORE_LR                                                          \
        POP_REG (15);                                                           \
        
#define IRQ_SAVE_RET                                                            \
        PUSH_REG (3);                                                           \
        PUSH_REG (4);                                                           \
                
#define IRQ_SAVE_PARAMS                                                         \
        PUSH_REG (5);                                                           \
        PUSH_REG (6);                                                           \
        PUSH_REG (7);                                                           \
        PUSH_REG (8);                                                           \
        PUSH_REG (9);                                                           \
        PUSH_REG (10);
        
#define IRQ_RESTORE_RET                                                         \
        POP_REG (3);                                                            \
        POP_REG (4);                                                            \
        
#define IRQ_RESTORE_PARAMS                                                      \
        POP_REG (5);                                                            \
        POP_REG (6);                                                            \
        POP_REG (7);                                                            \
        POP_REG (8);                                                            \
        POP_REG (9);                                                            \
        POP_REG (10);                                                           
                
/*
 * IRQ handler
 * - Save the complete context of the current task
 * - Switch to kernel interrupt stack
 * - Mark our current entry mode as ENTRY_KERNEL
 * - Invoke the main IRQ handler
 * - Restore context (and user stack) 
 */     
        .global irq_entry
        .global _interrupt_handler
        .section .text  
        .align 2        
        .ent irq_entry
_interrupt_handler:             
irq_entry:
        addik   r1, r1, -IRQ_STACK_FRAME_SIZ;                           /* Interrupts are turned off here       */
        IRQ_SAVE_TMP;
#if (XPAR_MICROBLAZE_USE_MMU >= 2) && !defined (XILKERNEL_MB_MPU_DISABLE)
        /* MicroBlaze sets VM bit to 0 and sets VMS = 1 when interrupted.
           We want VM mode enabled always. So we set it here again */    
        IRQ_MSR_SET_VM;
#endif
        GET_CURRENT_PROC (r11);                                         /* Get the context pointer into r11     */
        CTX_SAVE_STATE_REGS;                                            /* Save MSR, SP, LR and other state regs (r14,r16,r17,r18)      */
        CTX_SAVE_REG (2);                                               /* Save complete context in process context structure */
        CTX_SAVE_REG (3); 
        CTX_SAVE_REG (4); 
        CTX_SAVE_REG (5); 
        CTX_SAVE_REG (6); 
        CTX_SAVE_REG (7); 
        CTX_SAVE_REG (8); 
        CTX_SAVE_REG (9); 
        CTX_SAVE_REG (10); 
        CTX_SAVE_REG (13);
        CTX_SAVE_REG (19);                                                                                                      
        CTX_SAVE_REG (20);                                                                                                      
        CTX_SAVE_REG (21);
        CTX_SAVE_REG (22);                                                                                                      
        CTX_SAVE_REG (23);                                                                                                      
        CTX_SAVE_REG (24);                                                                                                      
        CTX_SAVE_REG (25);                                                                                                      
        CTX_SAVE_REG (26);                                                                                                      
        CTX_SAVE_REG (27);                                                                                                      
        CTX_SAVE_REG (28);
        CTX_SAVE_REG (29); 
        CTX_SAVE_REG (30);
        CTX_SAVE_REG (31);
        /* RESTORE_KERNEL_CONTEXT; */
        brlid   r15, pit_disable;                                       /* Stop the running PIT. CR 225388 -- For some reason, the scheme where */
        nop;                                                            /* the PIT is running free does not seem to work. i.e have to disable here */
        GET_CURRENT_PROC (r11);                                         /* Get the context pointer into r11 */        
        ori     r5, r0, ISRFLAG_INTERRUPT;
        sbi     r5, r11, ISRFLAG_OFFSET;                                /* Mark entry as from ISR               */
        sbi     r5, r0, entry_mode;                                     /* Entry mode kernel                    */
#if (XPAR_MICROBLAZE_USE_STACK_PROTECTION == 1)        
        lwi	r12, r0, kernel_irq_stack_ptr;
        mts	rshr, r12;
        lwi     r12, r0, kernel_irq_stack_ptr_end;
        mts	rslr, r12;
#endif
        lwi     r1, r0, kernel_irq_stack_ptr;                           /* Switch to kernel IRQ stack           */
#ifdef CONFIG_INTC
        brlid   r15, XIntc_DeviceInterruptHandler;                      /* Handle the IRQ                       */
        ori     r5, r0, SYSINTC_DEVICE_ID;                              /* Parameter to the interrupt handler   */
#else
   
        brlid   r15, timer_int_handler;
        nop;                                                            /* delay slot */
#endif  
        sbi     r0, r0, entry_mode;                                     /* Reset entry_mode flag */
        lbui    r11, r0, resched;                                               
        beqi    r11, out_irq;
        brlid   r15, scheduler;         
        nop;
        swi     r0, r0, ctx_save_process                                /* Reset the context save process identifier */
out_irq:
        braid   restore_context;                                        /* End of IRQ handler. */
        nop;                                                            /* The context switch routine will take us where we need to go */
        .end irq_entry


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
        .ent restore_context
restore_context:
        brlid   r15, proc_restore_state;        /* Call C routine to restore program level state. Machine level state restored below */
        nop;
        GET_CURRENT_PROC (r11); 
        CTX_RESTORE_REG (2);                    /* Restore non-volatile registers */
        CTX_RESTORE_REG (13);
        CTX_RESTORE_REG (19);
        CTX_RESTORE_REG (20); 
        CTX_RESTORE_REG (21);
        CTX_RESTORE_REG (22);
        CTX_RESTORE_REG (23);
        CTX_RESTORE_REG (24);
        CTX_RESTORE_REG (25);
        CTX_RESTORE_REG (26); 
        CTX_RESTORE_REG (27);
        CTX_RESTORE_REG (28); 
        CTX_RESTORE_REG (29);
        CTX_RESTORE_REG (30); 
        CTX_RESTORE_REG (31);
        brlid   r15, pit_reset;                 /* We do this as late as possible to give as much of the budget as possible to the new task */
        nop;
        GET_CURRENT_PROC (r11);                 /* Someone might have stomped it during the previous function call */
        CTX_RESTORE_STATE_REGS;                 /* Restore MSR, SP, LR and other state regs (r14,r16,r17,r18)      */
        lbui    r5, r11, ISRFLAG_OFFSET;        /* If not entered the kernel through an ISR, restore only callee-saved and few other regs */
        xori    r6, r5, ISRFLAG_SYSTEM_CALL;
        beqi    r6, no_isr;
    
        sbi     r0, r11, ISRFLAG_OFFSET;        /* Reset isrflag */
        xori    r6, r5, ISRFLAG_NEW_PROC;       /* Indicates interrupt */
        beqi    r6, new_proc_restore;

        /* Fall through to ISR */
isr:    
        CTX_RESTORE_REG (3);                    /* ISR restore: Restore volatile regs too */
        CTX_RESTORE_REG (4);
        CTX_RESTORE_REG (5);    
        CTX_RESTORE_REG (6);
        CTX_RESTORE_REG (7);    
        CTX_RESTORE_REG (8);
        CTX_RESTORE_REG (9);                            
        CTX_RESTORE_REG (10);
        IRQ_RESTORE_TMP;
        rtid    r14, 0;
        addik   r1, r1, IRQ_STACK_FRAME_SIZ;    /* Restore the stack pointer here */
new_proc_restore:
        rtid    r14, 0;
        nop;
no_isr:
        rtsd    r15, 8;                         /* No need to restore r11. It is caller-saved */
        ori     r3, r0, 1;                      /* Return 1 indicating return from restore context */
        .end restore_context


/*
 * Save context
 * - Saves only kernel context        
 * - Invoked only from "suspend". (ISR saves its own context)
 * - Indicate ISRFLAG 0
 * - Pointer to process structure in r5
 * - Needs to save lesser context than an ISR. Only Dedicated and non-volatile registers need to be saved.
 * - The current processes stack will be continued to use for a while till a restore is done.
 */

        .global save_context
        .section .text  
        .align 2        
        .ent save_context
save_context:
        or      r11, r0, r5;                    /* Move the process structure pointer to our work-horse r11    */
        CTX_SAVE_STATE_REGS;                    /* Save MSR, SP, LR and other state registers (r14,r16,r17,r18)*/
        CTX_SAVE_REG (2);
        CTX_SAVE_REG (13);
        CTX_SAVE_REG (19); 
        CTX_SAVE_REG (20); 
        CTX_SAVE_REG (21);
        CTX_SAVE_REG (22); 
        CTX_SAVE_REG (23); 
        CTX_SAVE_REG (24); 
        CTX_SAVE_REG (25); 
        CTX_SAVE_REG (26); 
        CTX_SAVE_REG (27); 
        CTX_SAVE_REG (28); 
        CTX_SAVE_REG (29); 
        CTX_SAVE_REG (30); 
        CTX_SAVE_REG (31);
        sbi     r0, r11, ISRFLAG_OFFSET;        /* We will always be called from a system call */
out_save_context:               
        rtsd    r15, 8;
        or      r3, r0, r0;                     /* Return 0 */
        .end save_context
        

#ifdef MICROBLAZE_EXCEPTIONS_ENABLED
        .global xilkernel_process_exception
        .global xilkernel_process_mmu_exception    
        .section .text  
        .align 2        
        .ent xilkernel_process_exception    
xilkernel_process_exception:
xilkernel_process_mmu_exception:    
         lwi     r1, r0, kernel_irq_stack_ptr;                           /* Switch to kernel IRQ stack           */
         DISABLE_INTERRUPTS;

         mfs     r6, rear
         addik   r7, r17, 0
         addik   r17, r0, rted_loc
rted_loc:   
         rted    r17, 8
         nop
    
         brlid  r15, microblaze_report_exception;                        /* Report the current exception back, if there is a way */
         nop;                                                            /* r5 contains ESR, r6 contains PC */

         lbui    r5, r0, current_pid                                     /* Kill the current process */
         brlid  r15, sys_kill
         nop

         /* Control does not reach here. A context switch happens at the end of sys_kill */
        .end xilkernel_process_exception
#endif                            

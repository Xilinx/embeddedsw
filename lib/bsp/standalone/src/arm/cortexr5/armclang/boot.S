;/******************************************************************************
;* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
;* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
;* SPDX-License-Identifier: MIT
;******************************************************************************/
;/*****************************************************************************/
;/**
;* @file boot.S
;*
;* @addtogroup r5_boot_code Cortex R5 and Cortex R52 Processor Boot Code
;* @{
;* <h2> boot.S </h2>
;* The boot code performs minimum configuration which is required for an
;* application to run starting from processor's reset state. Below is a
;* sequence illustrating what all configuration is performed before control
;* reaches to main function.
;*
;* 1. Program vector table base for exception handling
;* 2. Program stack pointer for various modes (IRQ, FIQ, supervisor, undefine,
;*    abort, system)
;* 3. Disable instruction cache, data cache and MPU
;* 4. Invalidate instruction and data cache
;* 5. Configure MPU with short descriptor translation table format and program
;*    base address of translation table
;* 6. Enable data cache, instruction cache and MPU
;* 7. Enable Floating point unit
;* 8. Transfer control to _start which clears BSS sections and jumping to main
;*    application
;*
;*
;* <pre>
;* MODIFICATION HISTORY:
;*
;* Ver   Who  Date     Changes
;* ----- ---- -------- ---------------------------------------------------
;* 7.3  dp  06/25/20   Initial version for armclang
;* 9.1  mus 10/10/23   Fix stack initialization for IRQ, FIQ, supervisory,
;*                     undefine, and abort mode. It is incorrectly pointing
;*                     to start of stack address space, it should point to
;*                     end of stack address space, since CortexR5 stack
;*                     works in descending order. It fixes CR#1177971.
;* 9.5	vmt 28/11/25   Added Cortex-R52 processor support for armclang
;* </pre>
;*
;******************************************************************************/
#ifndef SDT
#include "xparameters.h"
#else
#include "bspconfig.h"
#endif

	EXPORT _prestart
	EXPORT _boot
	IMPORT |Image$$ARM_LIB_STACK$$ZI$$Base|
	IMPORT |Image$$ARM_IRQ_STACK$$ZI$$Limit|
	IMPORT |Image$$ARM_SV_STACK$$ZI$$Limit|
	IMPORT |Image$$ARM_ABORT_STACK$$ZI$$Limit|
	IMPORT |Image$$ARM_FIQ_STACK$$ZI$$Limit|
	IMPORT |Image$$ARM_UNDEF_STACK$$ZI$$Limit|
	IMPORT Init_MPU
	IMPORT _startup
	IMPORT _vector_table

#if defined (ARMR52)
	IMPORT _TCM_A_REGION
	IMPORT _TCM_B_REGION
	IMPORT _TCM_C_REGION
#ifndef SDT
counterfreq EQU XPAR_CPU_CORTEXR52_0_TIMESTAMP_CLK_FREQ
#else
	IMPORT XCortexr52_ConfigTable
#endif
#endif

RPU_GLBL_CNTL EQU 0xFF9A0000
RPU_ERR_INJ EQU 0xFF9A0020
RPU_0_CFG EQU 0xFF9A0100
RPU_1_CFG EQU 0xFF9A0200
#if defined(versal)
RST_LPD_DBG EQU 0xFF5E0338
BOOT_MODE_USER EQU 0xF1260200
#else
RST_LPD_DBG EQU 0xFF5E0240
BOOT_MODE_USER EQU 0xFF5E0200
#endif
fault_log_enable EQU 0x101

;0th bit of PROCESSOR_ACCESS_VALUE macro signifies trustzone
;setting for RPU address space

#define RPU_TZ_MASK	0x1

	AREA |.boot|, CODE
	REQUIRE8 {TRUE}
	PRESERVE8 {TRUE}
	ENTRY
;this initializes the various processor modes

_prestart
_boot

OKToRun
#if defined (ARMR52)
    mrs r0, cpsr
    bfi r1, r0, #0, #5
    cmp r1, #0x1A         ; Check if it is HYP mode
    bne InitEL1

    ldr r0, =_vector_table
    mcr p15, 4, r0, c12, c0, 0      ; Write HVBAR
#endif

;Initialize processor registers to 0
	mov	r0,#0
	mov	r1,#0
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0
	mov	r5,#0
	mov	r6,#0
	mov	r7,#0
	mov	r8,#0
	mov	r9,#0
	mov	r10,#0
	mov	r11,#0
	mov	r12,#0

#if defined (ARMR52)
	; Set counter frequency, CNTFRQ: RW from EL2 RO from EL1
#ifndef SDT
	ldr     r0,=counterfreq
#else
	ldr	r1,=XCortexr52_ConfigTable
	ldr     r0,[r1]
#endif
	mcr     p15,0,r0,c14,c0,0        ; Write CNTFRQ

	; Change EL1 exception base address
	ldr r0, =_vector_table
	mcr p15, 0, r0, c12, c0, 0      ; Write VBAR

	mrc     p15, 4, r1, c1, c0, 1   ; Read HACTLR
	orr     r1, r1, #(0x1 << 8)     ; Enable access to IMP_PERIPHPREGIONR at EL1
	mcr     p15, 4, r1, c1, c0, 1   ; Write HACTLR

	; Switch to EL1 SVC mode
	mrs r0, cpsr
	mov r1, #0x13                   ; Mode SVC
	bfi r0, r1, #0, #5
	msr spsr_cxsf, r0
	ldr r0, =InitEL1
	msr elr_hyp, r0
	dsb
	isb
	eret
InitEL1
#endif

;Initialize stack pointer and banked registers for various mode
	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the irq stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x12			;IRQ mode
	msr	cpsr_cxsf, r2
	ldr	r13,=|Image$$ARM_IRQ_STACK$$ZI$$Limit|	;IRQ stack pointer
	mov 	r14,#0

	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the supervisor stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x13			;supervisor mode
	msr	cpsr_cxsf, r2
	ldr	r13,=|Image$$ARM_SV_STACK$$ZI$$Limit|	;Supervisor stack pointer
	mov 	r14,#0

	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the Abort stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x17			;Abort mode
	msr	cpsr_cxsf, r2
	ldr	r13,=|Image$$ARM_ABORT_STACK$$ZI$$Limit|	;Abort stack pointer
	mov 	r14,#0

	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the FIQ stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x11			;FIQ mode
	msr	cpsr_cxsf, r2
	mov 	r8, #0
	mov 	r9, #0
	mov 	r10, #0
	mov 	r11, #0
	mov 	r12, #0
	ldr	r13,=|Image$$ARM_FIQ_STACK$$ZI$$Limit|	;FIQ stack pointer
	mov 	r14,#0

	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the Undefine stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x1b			;Undefine mode
	msr	cpsr_cxsf, r2
	ldr	r13,=|Image$$ARM_UNDEF_STACK$$ZI$$Limit|	;Undefine stack pointer
	mov 	r14,#0

	mrs	r0, cpsr			;get the current PSR
	mvn	r1, #0x1f			;set up the system stack pointer
	and	r2, r1, r0
	orr	r2, r2, #0x1F			;SYS mode
	msr	cpsr_cxsf, r2
	ldr	r13,=|Image$$ARM_LIB_STACK$$ZI$$Base|	;SYS stack pointer
	mov 	r14,#0

;Enable access to VFP by enabling access to Coprocessors 10 and 11.
;Enables Full Access i.e. in both privileged and non privileged modes
	mrc     p15, 0, r0, c1, c0, 2	;Read Coprocessor Access Control Register (CPACR)
	orr     r0, r0, #(0xF << 20) 	;Enable access to CP 10 & 11
	mcr     p15, 0, r0, c1, c0, 2	;Write Coprocessor Access Control Register (CPACR)
	isb

;enable fpu access
	vmrs	r3, FPEXC
	orr	r1, r3, #(1<<30)
	vmsr	FPEXC, r1

;clear the floating point register
	mov	r1,#0
	vmov	d0,r1,r1
	vmov	d1,r1,r1
	vmov	d2,r1,r1
	vmov	d3,r1,r1
	vmov	d4,r1,r1
	vmov	d5,r1,r1
	vmov	d6,r1,r1
	vmov	d7,r1,r1
	vmov	d8,r1,r1
	vmov	d9,r1,r1
	vmov	d10,r1,r1
	vmov	d11,r1,r1
	vmov	d12,r1,r1
	vmov	d13,r1,r1
	vmov	d14,r1,r1
	vmov	d15,r1,r1
#if defined (ARMR52)
        vmov    d16, r1, r1
        vmov    d17, r1, r1
        vmov    d18, r1, r1
        vmov    d19, r1, r1
        vmov    d20, r1, r1
        vmov    d21, r1, r1
        vmov    d22, r1, r1
        vmov    d23, r1, r1
        vmov    d24, r1, r1
        vmov    d25, r1, r1
        vmov    d26, r1, r1
        vmov    d27, r1, r1
        vmov    d28, r1, r1
        vmov    d29, r1, r1
        vmov    d30, r1, r1
        vmov    d31, r1, r1
#endif

#ifdef __SOFTFP__
;Disable the FPU if SOFTFP is defined
	vmsr	FPEXC,r3
#endif

;Disable MPU and caches
	mrc     p15, 0, r0, c1, c0, 0	;Read CP15 Control Register
	bic     r0, r0, #0x05        	;Disable MPU (M bit) and data cache (C bit)
	bic     r0, r0, #0x1000      	;Disable instruction cache (I bit)
	dsb                          	;Ensure all previous loads/stores have completed
	mcr     p15, 0, r0, c1, c0, 0	;Write CP15 Control Register
	isb                          	;Ensure subsequent insts execute wrt new MPU settings
#if defined (ARMR52)
	; Enable ECC checks for R52
	mrc p15, 1, r0, c9, c1, 2			; Read IMP_MEMPROTCTLR
	bic r0, r0, #(0x1 << 0)				; disable TCM and L1 cache ECC initially
	mcr p15, 1, r0, c9, c1, 2			; Write IMP_MEMPROTCTLR

	; Enable TCM regions for R52
	ldr r0, =_TCM_A_REGION
	ldr r1, [r0]
	tst r1, #0x1
	beq InitTCMB
	mcr p15, 0, r1, c9, c1, 0  ; Write r1 to IMP_ATCMREGIONR

InitTCMB
	ldr r0, =_TCM_B_REGION
	ldr r1, [r0]
	tst r1, #0x1
	beq InitTCMC
	mcr p15, 0, r1, c9, c1, 1  ; Write r1 to IMP_BTCMREGIONR

InitTCMC
	ldr r0, =_TCM_C_REGION
	ldr r1, [r0]
	tst r1, #0x1
	beq InitTCMDone
	mcr p15, 0, r1, c9, c1, 2  ; Write r1 to IMP_CTCMREGIONR

InitTCMDone
	mrc     p15, 0, r1, c15, c0, 0   ; Read IMP_PERIPHPREGIONR
	orr     r1, r1, #(0x3)           ; Enable peripheral port at EL2 and EL1 and EL0
	mcr     p15, 0, r1, c15, c0, 0   ; Write IMP_PERIPHPREGIONR

	; Disable branch prediction initially for R52
	mrc p15, 1, r0, c9, c1, 1			; Read IMP_BPCTLR
	orr r0, r0, #(0x1 << 1)				; Disable branch prediction
	mcr p15, 1, r0, c9, c1, 1 			; Write IMP_BPCTLR

	; Set attributes index for normal and device memory in MAIR0 for R52
	MRC p15, 0, r0, c10, c2, 0	; Read MAIR0
	LDR r1, =0xBB                   ; Attribute index0: Normal inner/outer RW cacheable, write-through
	BFI r0, r1, #0, #8              ; Update attribute index0
	LDR r1, =0x04                   ; Attribute index1: Device nGnRE
	BFI r0, r1, #8, #8              ; Update Attribute index1
	LDR r1, =0x44                   ; Attribute index2: Normal non cacheable
	BFI r0, r1, #16, #8             ; Update Attribute index2
	LDR r1, =0x00                   ; Attribute index3: Strongly ordered nGnRnE
	BFI r0, r1, #24, #8             ; Update Attribute index3
	MCR p15,0,r0,c10,c2,0           ; Write MAIR0

#else
;Disable Branch prediction, TCM ECC checks
	mrc     p15, 0, r0, c1, c0, 1	;Read ACTLR
	orr     r0, r0, #(0x1 << 17)	;Enable RSDIS bit 17 to disable the return stack
	orr     r0, r0, #(0x1 << 16)	;Clear BP bit 15 and set BP bit 16:
	bic     r0, r0, #(0x1 << 15)	;Branch always not taken and history table updates disabled
	orr     r0, r0, #(0x1 << 27)	;Enable B1TCM ECC check
	orr     r0, r0, #(0x1 << 26)	;Enable B0TCM ECC check
	orr     r0, r0, #(0x1 << 25)	;Enable ATCM ECC check
	bic	r0, r0, #(0x1 << 5)	;Generate abort on parity errors, with [5:3]=b 000
	bic 	r0, r0, #(0x1 << 4)
	bic	r0, r0, #(0x1 << 3)
	mcr     p15, 0, r0, c1, c0, 1	;Write ACTLR
	dsb	;Complete all outstanding explicit memory operations

;Invalidate caches
	mov	r0,#0			;r0 = 0
	dsb
	mcr	p15, 0, r0, c7, c5, 0	;invalidate icache
	mcr 	p15, 0, r0, c15, c5, 0 	;Invalidate entire data cache
	isb
#if LOCKSTEP_MODE_DEBUG == 0 && (PROCESSOR_ACCESS_VALUE & RPU_TZ_MASK)
;enable fault log for lock step
	ldr	r0,=RPU_GLBL_CNTL
	ldr	r1, [r0]
	ands	r1, r1, #0x8
;branch to initialization if split mode
	bne 	init
;check for boot mode if in lock step, branch to init if JTAG boot mode
	ldr	r0,=BOOT_MODE_USER
	ldr 	r1, [r0]
	ands	r1, r1, #0xF
	beq 	init
;reset the debug logic
	ldr	r0,=RST_LPD_DBG
	ldr	r1, [r0]
	orr	r1, r1, #(0x1 << 4)
	orr	r1, r1, #(0x1 << 5)
	str	r1, [r0]
;enable fault log
	ldr	r0,=RPU_ERR_INJ
	ldr	r1,=fault_log_enable
	ldr	r2, [r0]
	orr	r2, r2, r1
	str	r2, [r0]
	nop
	nop
#endif
#endif

init
	bl 	Init_MPU		;Initialize MPU

#if defined (ARMR52)
;Enable Branch prediction for R52
	mrc p15, 1, r0, c9, c1, 1       ; Read IMP_BPCTLR
	bic r0, r0, #(0x1 << 1)         ; Enable branch prediction
	mcr p15, 1, r0, c9, c1, 1       ; Write IMP_BPCTLR

	mrc p15, 0, r1, c1, c0, 0	; Read System Control Register
	ldr     r0, =0x1005             ; Set M bit to enable MPU, C & I bit for data and instruction caches
	orr r1,r1,r0
	dsb
	mcr     p15, 0, r0, c1, c0, 0   ; Write System Control Register
	isb
#else
;Enable Branch prediction
	mrc     p15, 0, r0, c1, c0, 1	;Read ACTLR
	bic     r0, r0, #(0x1 << 17)	;Clear RSDIS bit 17 to enable return stack
	bic     r0, r0, #(0x1 << 16)	;Clear BP bit 15 and BP bit 16:
	bic     r0, r0, #(0x1 << 15)	;Normal operation, BP is taken from the global history table.
	orr	r0, r0, #(0x1 << 14)	;Disable DBWR for errata 780125
	mcr     p15, 0, r0, c1, c0, 1	;Write ACTLR

;Enable icahce and dcache
	mrc 	p15,0,r1,c1,c0,0
	ldr	r0, =0x1005
	orr 	r1,r1,r0
	dsb
	mcr	p15,0,r1,c1,c0,0	;Enable cache
	isb				;isb flush prefetch buffer

;Set vector table in TCM/LOVEC
#ifndef VEC_TABLE_IN_OCM
	mrc	p15, 0, r0, c1, c0, 0
	mvn	r1, #0x2000
	and	r0, r0, r1
	mcr	p15, 0, r0, c1, c0, 0

;Check if processor is having access to RPU address space
#if (PROCESSOR_ACCESS_VALUE & RPU_TZ_MASK)
;Clear VINITHI to enable LOVEC on reset
#if XPAR_CPU_ID == 0
	ldr r0, =RPU_0_CFG
#else
	ldr r0, =RPU_1_CFG
#endif
	ldr r1, [r0]
	bic r1, r1, #(0x1 << 2)
	str r1, [r0]
#endif
#endif
#endif
;enable asynchronous abort exception
	mrs	r0, cpsr
	bic	r0, r0, #0x100
	msr	cpsr_xsf, r0

	bl _startup

Ldone
	b Ldone	;Paranoia: we should never get here

	END
;@} End of "addtogroup r5_boot_code".

/******************************************************************************
*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xfsbl_exit.s
*
* This is the main file which contains exit code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   11/13/13 Initial release
*       mus  02/26/19 Added support for armclang compiler
*       dp   06/25/20 Added armclang support for Cortex-R5
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#if ! defined (__clang__)
.globl XFsbl_Exit

/************************** Variable Definitions *****************************/

XFsbl_Exit:

#ifndef ARMA53_64
	mov	 lr, r0	/* move the destination address into link register */

	mcr	 15,0,r0,cr7,cr5,0	/* Invalidate Instruction cache */
	mcr	 15,0,r0,cr7,cr5,6	/* Invalidate branch predictor array */

	dsb
	isb				/* make sure it completes */

	mrc     p15,0,r4,c1,c0,0	/* Read SCTLR */
	bic     r4, r4, #0x04 		/* disable L1 I Cache */
	bic     r4, r4, #0x1000		/* Disable L1 D Cache */
	mcr	 15,0,r4,cr1,cr0,0	/* disable the DCache, ICache */
	isb				/* make sure it completes */

	/* set exception vector to HIVEC */
	/* this is done because, in LOVEC we can not disable the MPU as  */
	/* OCM region is not present in default MPU regions when in LOVEC*/
	mrc     p15, 0, r0, c1, c0, 0   /* Read SCTLR */
	orr	r0, r0, #0x2000
	mcr	15, 0, r0, cr1, cr0, 0
	isb

	/* disable the MPU */
	mrc     p15,0,r4,c1,c0,0 	/* Read SCTLR */
	bic     r4, r4, #0x01
	mcr	 15,0,r4,cr1,cr0,0
	isb

	cmp r1, #0  		/* exit to wfe    */
	beq XFsbl_Loop

	cmp r1, #2   /* x1 is 2 - exit in aarch32  */
	beq XFsbl_StartApp

	/* x1 is 1 - exit in aarch64 */
	mov r2, #3   /* request for warm reset and aarch64 */
	dsb
	isb
	mcr	p15, 0, r2, c12, c0, 2 /* write to reset management register */
	isb

#else
	mov x30, x0 /* move the destination address into x30 register */

	tlbi ALLE3   /* invalidate All E3 translation tables */
	ic IALLU   /* invalidate I Cache All to PoU, Inner Shareable */

	dsb sy
	isb 			 /* make sure it completes */

	mrs x5, SCTLR_EL3	 /* Read control register */
	mov x6, #0x1005  	 /* D, I , M bits disable */
        bic     x5, x5, x6 	 /* Disable MMU, L1 and L2 I/D cache */
        msr     SCTLR_EL3, x5    /*  */

	isb

	cmp x1, #0  		/* exit to wfe    */
	beq XFsbl_Loop

	cmp x1, #1   /* x1 is 1 - exit in aarch64  */
	beq XFsbl_StartApp

		     /* x1 is 2 - exit in aarch32 */

	mov x2, #2   /* request for warm reset and aarch32 */
	dsb sy
	isb
	msr RMR_EL3,x2 /* write to reset management register */
	isb
#endif

XFsbl_Loop:
	wfe				/* wait for event */
	b XFsbl_Loop

.Ldone:
	b		.Ldone		/* Paranoia: we should never get here */

XFsbl_StartApp:
#ifdef ARMA53_64
	br 	x30        /* branch to */
#else
	bx	lr;
#endif
.end
#else

	EXPORT XFsbl_Exit
	AREA |.exit|, CODE
XFsbl_Exit

#ifdef ARMR5
	mov	 lr, r0 ;move the destination address into link register

	mcr	 p15,0,r0,c7,c5,0 ;Invalidate Instruction cache
	mcr	 p15,0,r0,c7,c5,6 ;Invalidate branch predictor array

	dsb
	isb ;make sure it completes

	mrc     p15,0,r4,c1,c0,0 ;Read SCTLR
	bic     r4, r4, #0x04  ;disable L1 I Cache
	bic     r4, r4, #0x1000 ;Disable L1 D Cache
	mcr	p15,0,r4,c1,c0,0 ;disable the DCache, ICache
	isb ;make sure it completes

	;set exception vector to HIVEC
	;this is done because, in LOVEC we can not disable the MPU as
	;OCM region is not present in default MPU regions when in LOVEC
	mrc     p15, 0, r0, c1, c0, 0  ;Read SCTLR
	orr	r0, r0, #0x2000
	mcr	p15, 0, r0, c1, c0, 0
	isb

	;disable the MPU
	mrc     p15,0,r4,c1,c0,0 ;Read SCTLR
	bic     r4, r4, #0x01
	mcr	p15,0,r4,c1,c0,0
	isb

	cmp r1, #0  ;exit to wfe
	beq XFsbl_Loop

	cmp r1, #2   ;x1 is 2 - exit in aarch32
	beq XFsbl_StartApp

	;x1 is 1 - exit in aarch64
	mov r2, #3   ;request for warm reset and aarch64
	dsb
	isb
	mcr	p15, 0, r2, c12, c0, 2 ;write to reset management register
	isb
#else
	mov x30, x0 ; move the destination address into x30 register

	tlbi ALLE3   ; invalidate All E3 translation tables
	ic IALLU   ; invalidate I Cache All to PoU, Inner Shareable

	dsb sy
	isb 			; make sure it completes

	mrs x5, SCTLR_EL3	; Read control register
	mov x6, #0x1005		; D, I , M bits disable
	bic     x5, x5, x6 	; Disable MMU, L1 and L2 I/D cache
	msr     SCTLR_EL3, x5

	isb

	cmp x1, #0  		; exit to wfe
	beq XFsbl_Loop

	cmp x1, #1   ; x1 is 1 - exit in aarch64
	beq XFsbl_StartApp

	;x1 is 2 - exit in aarch32

	mov x2, #2   ; request for warm reset and aarch32
	dsb sy
	isb
	msr RMR_EL3,x2 ; write to reset management register
	isb
#endif

XFsbl_Loop
	wfe				;wait for event
	b XFsbl_Loop


XFsbl_StartApp
#ifdef ARMR5
	bx	lr;
#else
	br 	x30        ; branch to
#endif
	END
#endif

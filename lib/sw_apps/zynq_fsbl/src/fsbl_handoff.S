#ifdef __GNUC__
/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file handoff.S
*
* Contains the code that does the handoff to the loaded application. This
* code lives high in the ROM. 
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date.word	Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a ecm	03/01/10 Initial release
* 7.00a kc	10/23/13 Added support for armcc compiler
* </pre>
*
* @note
* Assumes that the starting address of the FSBL is provided by the calling routine
* in R0.
*
******************************************************************************/

.globl FsblHandoffJtagExit

.globl FsblHandoffExit

.section .handoff,"axS"

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

FsblHandoffJtagExit:
		mcr	 15,0,r0,cr7,cr5,0		/* Invalidate Instruction cache */
		mcr	 15,0,r0,cr7,cr5,6		/* Invalidate branch predictor array */

		dsb
		isb					/* make sure it completes */

	ldr	r4, =0
		mcr	 15,0,r4,cr1,cr0,0		/* disable the ICache and MMU */

		isb					/* make sure it completes */
Loop:
	wfe
	b Loop

FsblHandoffExit:
		mov	 lr, r0	/* move the destination address into link register */

		mcr	 15,0,r0,cr7,cr5,0		/* Invalidate Instruction cache */
		mcr	 15,0,r0,cr7,cr5,6		/* Invalidate branch predictor array */

		dsb
		isb					/* make sure it completes */

	ldr	r4, =0
		mcr	 15,0,r4,cr1,cr0,0		/* disable the ICache and MMU */

		isb					/* make sure it completes */


		bx		lr	/* force the switch, destination should have been in r0 */

.Ldone: b		.Ldone					/* Paranoia: we should never get here */
.end

#elif defined (__ICCARM__)
	
	PUBLIC FsblHandoffJtagExit
	
	PUBLIC FsblHandoffExit
	
	SECTION .handoff:CODE:NOROOT(2)

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

FsblHandoffJtagExit
		mcr	 p15,0,r0,c7,c5,0		;/* Invalidate Instruction cache */
		mcr	 p15,0,r0,c7,c5,6		;/* Invalidate branch predictor array */

		dsb
		isb							;/* make sure it completes */

	ldr	r4, =0
		mcr	 p15,0,r4,c1,c0,0		;/* disable the ICache and MMU */

		isb							;/* make sure it completes */
Loop
	wfe
	b Loop

FsblHandoffExit
		mov	 lr, r0					;/* move the destination address into link register */

		mcr	 p15,0,r0,c7,c5,0		;/* Invalidate Instruction cache */
		mcr	 p15,0,r0,c7,c5,6		;/* Invalidate branch predictor array */

		dsb
		isb							;/* make sure it completes */

	ldr	r4, =0
		mcr	 p15,0,r4,c1,c0,0		;/* disable the ICache and MMU */

		isb							;/* make sure it completes */


		bx		lr					;/* force the switch, destination should have been in r0 */

.Ldone 
	b		.Ldone				;/* Paranoia: we should never get here */

	END


#else
	EXPORT FsblHandoffJtagExit

	EXPORT FsblHandoffExit

	AREA |.handoff|,CODE

;/***************************** Include Files *********************************/

;/************************** Constant Definitions *****************************/

;/**************************** Type Definitions *******************************/

;/***************** Macros (Inline Functions) Definitions *********************/

;/************************** Function Prototypes ******************************/

;/************************** Variable Definitions *****************************/


FsblHandoffJtagExit
		mcr	 p15,0,r0,c7,c5,0		;/* Invalidate Instruction cache */
		mcr	 p15,0,r0,c7,c5,6		;/* Invalidate branch predictor array */

		dsb
		isb							;/* make sure it completes */

	ldr	r4, =0
		mcr	 p15,0,r4,c1,c0,0		;/* disable the ICache and MMU */

		isb							;/* make sure it completes */
Loop
	wfe
	b Loop

FsblHandoffExit
		mov	 lr, r0	;/* move the destination address into link register */

		mcr	 p15,0,r0,c7,c5,0		;/* Invalidate Instruction cache */
		mcr	 p15,0,r0,c7,c5,6		;/* Invalidate branch predictor array */

		dsb
		isb							;/* make sure it completes */

	ldr	r4, =0
		mcr	 p15,0,r4,c1,c0,0		;/* disable the ICache and MMU */

		isb							;/* make sure it completes */


		bx		lr	;/* force the switch, destination should have been in r0 */

Ldone b		Ldone					;/* Paranoia: we should never get here */
	END
#endif

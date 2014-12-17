/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xreg_cortexa53.h
*
* This header file contains definitions for using inline assembler code. It is
* written specifically for the GNU compiler.
*
* All of the ARM Cortex A53 GPRs, SPRs, and Debug Registers are defined along
* with the positions of the bits within the registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.00 	pkp  05/29/14 First release
* </pre>
*
******************************************************************************/
#ifndef XREG_CORTEXA53_H
#define XREG_CORTEXA53_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* GPRs */
#define XREG_GPR0				r0
#define XREG_GPR1				r1
#define XREG_GPR2				r2
#define XREG_GPR3				r3
#define XREG_GPR4				r4
#define XREG_GPR5				r5
#define XREG_GPR6				r6
#define XREG_GPR7				r7
#define XREG_GPR8				r8
#define XREG_GPR9				r9
#define XREG_GPR10				r10
#define XREG_GPR11				r11
#define XREG_GPR12				r12
#define XREG_GPR13				r13
#define XREG_GPR14				r14
#define XREG_GPR15				r15
#define XREG_CPSR				cpsr

/* Coprocessor number defines */
#define XREG_CP0				0
#define XREG_CP1				1
#define XREG_CP2				2
#define XREG_CP3				3
#define XREG_CP4				4
#define XREG_CP5				5
#define XREG_CP6				6
#define XREG_CP7				7
#define XREG_CP8				8
#define XREG_CP9				9
#define XREG_CP10				10
#define XREG_CP11				11
#define XREG_CP12				12
#define XREG_CP13				13
#define XREG_CP14				14
#define XREG_CP15				15

/* Coprocessor control register defines */
#define XREG_CR0				cr0
#define XREG_CR1				cr1
#define XREG_CR2				cr2
#define XREG_CR3				cr3
#define XREG_CR4				cr4
#define XREG_CR5				cr5
#define XREG_CR6				cr6
#define XREG_CR7				cr7
#define XREG_CR8				cr8
#define XREG_CR9				cr9
#define XREG_CR10				cr10
#define XREG_CR11				cr11
#define XREG_CR12				cr12
#define XREG_CR13				cr13
#define XREG_CR14				cr14
#define XREG_CR15				cr15

/* Current Processor Status Register (CPSR) Bits */
#define XREG_CPSR_THUMB_MODE			0x20
#define XREG_CPSR_MODE_BITS			0x1F
#define XREG_CPSR_SYSTEM_MODE			0x1F
#define XREG_CPSR_UNDEFINED_MODE		0x1B
#define XREG_CPSR_DATA_ABORT_MODE		0x17
#define XREG_CPSR_SVC_MODE			0x13
#define XREG_CPSR_IRQ_MODE			0x12
#define XREG_CPSR_FIQ_MODE			0x11
#define XREG_CPSR_USER_MODE			0x10

#define XREG_CPSR_IRQ_ENABLE			0x80
#define XREG_CPSR_FIQ_ENABLE			0x40

#define XREG_CPSR_N_BIT				0x80000000U
#define XREG_CPSR_Z_BIT				0x40000000U
#define XREG_CPSR_C_BIT				0x20000000U
#define XREG_CPSR_V_BIT				0x10000000U



/* MPE register definitions */
#define XREG_FPSID				c0
#define XREG_FPSCR				c1
#define XREG_MVFR1				c6
#define XREG_MVFR0				c7
#define XREG_FPEXC				c8
#define XREG_FPINST				c9
#define XREG_FPINST2				c10

/* FPSID bits */
#define XREG_FPSID_IMPLEMENTER_BIT	(24U)
#define XREG_FPSID_IMPLEMENTER_MASK	(0x000000FFU << FPSID_IMPLEMENTER_BIT)
#define XREG_FPSID_SOFTWARE		(0X00000001U<<23U)
#define XREG_FPSID_ARCH_BIT		(16U)
#define XREG_FPSID_ARCH_MASK		(0x0000000FU  << FPSID_ARCH_BIT)
#define XREG_FPSID_PART_BIT		(8U)
#define XREG_FPSID_PART_MASK		(0x000000FFU << FPSID_PART_BIT)
#define XREG_FPSID_VARIANT_BIT		(4U)
#define XREG_FPSID_VARIANT_MASK		(0x0000000FU  << FPSID_VARIANT_BIT)
#define XREG_FPSID_REV_BIT		(0U)
#define XREG_FPSID_REV_MASK		(0x0000000FU  << FPSID_REV_BIT)

/* FPSCR bits */
#define XREG_FPSCR_N_BIT		(0X00000001U << 31U)
#define XREG_FPSCR_Z_BIT		(0X00000001U << 30U)
#define XREG_FPSCR_C_BIT		(0X00000001U << 29U)
#define XREG_FPSCR_V_BIT		(0X00000001U << 28U)
#define XREG_FPSCR_QC			(0X00000001U << 27U)
#define XREG_FPSCR_AHP			(0X00000001U << 26U)
#define XREG_FPSCR_DEFAULT_NAN		(0X00000001U << 25U)
#define XREG_FPSCR_FLUSHTOZERO		(0X00000001U << 24U)
#define XREG_FPSCR_ROUND_NEAREST	(0X00000000U << 22U)
#define XREG_FPSCR_ROUND_PLUSINF	(0X00000001U << 22U)
#define XREG_FPSCR_ROUND_MINUSINF	(0X00000002U << 22U)
#define XREG_FPSCR_ROUND_TOZERO		(0X00000003U << 22U)
#define XREG_FPSCR_RMODE_BIT		(22U)
#define XREG_FPSCR_RMODE_MASK		(0X00000003U << FPSCR_RMODE_BIT)
#define XREG_FPSCR_STRIDE_BIT		(20U)
#define XREG_FPSCR_STRIDE_MASK		(0X00000003U << FPSCR_STRIDE_BIT)
#define XREG_FPSCR_LENGTH_BIT		(16U)
#define XREG_FPSCR_LENGTH_MASK		(0X00000007U << FPSCR_LENGTH_BIT)
#define XREG_FPSCR_IDC			(0X00000001U << 7U)
#define XREG_FPSCR_IXC			(0X00000001U << 4U)
#define XREG_FPSCR_UFC			(0X00000001U << 3U)
#define XREG_FPSCR_OFC			(0X00000001U << 2U)
#define XREG_FPSCR_DZC			(0X00000001U << 1U)
#define XREG_FPSCR_IOC			(0X00000001U << 0U)

/* MVFR0 bits */
#define XREG_MVFR0_RMODE_BIT		(28U)
#define XREG_MVFR0_RMODE_MASK		(0x0000000FU << XREG_MVFR0_RMODE_BIT)
#define XREG_MVFR0_SHORT_VEC_BIT	(24U)
#define XREG_MVFR0_SHORT_VEC_MASK	(0x0000000FU << XREG_MVFR0_SHORT_VEC_BIT)
#define XREG_MVFR0_SQRT_BIT		(20U)
#define XREG_MVFR0_SQRT_MASK		(0x0000000FU << XREG_MVFR0_SQRT_BIT)
#define XREG_MVFR0_DIVIDE_BIT		(16U)
#define XREG_MVFR0_DIVIDE_MASK		(0x0000000FU << XREG_MVFR0_DIVIDE_BIT)
#define XREG_MVFR0_EXEC_TRAP_BIT	(0X00000012U)
#define XREG_MVFR0_EXEC_TRAP_MASK	(0X0000000FU << XREG_MVFR0_EXEC_TRAP_BIT)
#define XREG_MVFR0_DP_BIT		(8U)
#define XREG_MVFR0_DP_MASK		(0x0000000FU << XREG_MVFR0_DP_BIT)
#define XREG_MVFR0_SP_BIT		(4U)
#define XREG_MVFR0_SP_MASK		(0x0000000FU << XREG_MVFR0_SP_BIT)
#define XREG_MVFR0_A_SIMD_BIT		(0U)
#define XREG_MVFR0_A_SIMD_MASK		(0x0000000FU << MVFR0_A_SIMD_BIT)

/* FPEXC bits */
#define XREG_FPEXC_EX			(0X00000001U << 31U)
#define XREG_FPEXC_EN			(0X00000001U << 30U)
#define XREG_FPEXC_DEX			(0X00000001U << 29U)


#define XREG_CONTROL_DCACHE_BIT	(0X00000001U<<2U)
#define XREG_CONTROL_ICACHE_BIT	(0X00000001U<<12U)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XREG_CORTEXA53_H */

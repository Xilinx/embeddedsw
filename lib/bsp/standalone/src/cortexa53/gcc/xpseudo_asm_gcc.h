/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpseudo_asm_gcc.h
*
* This header file contains macros for using inline assembler code. It is
* written specifically for the GNU compiler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.00 	pkp		 05/21/14 First release
* </pre>
*
******************************************************************************/

#ifndef XPSEUDO_ASM_GCC_H  /* prevent circular inclusions */
#define XPSEUDO_ASM_GCC_H  /* by using protection macros */

/***************************** Include Files ********************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/* necessary for pre-processor */
#define stringify(s)	tostring(s)
#define tostring(s)	#s

/* pseudo assembler instructions */
#define mfcpsr()	({u32 rval; \
			   asm volatile("mrs %0,  DAIF" : "=r" (rval));\
			  rval;\
			 })

#define mtcpsr(v) asm ("msr DAIF, %0" : : "r" (v))

#define cpsiei()	//__asm__ __volatile__("cpsie	i\n")
#define cpsidi()	//__asm__ __volatile__("cpsid	i\n")

#define cpsief()	//__asm__ __volatile__("cpsie	f\n")
#define cpsidf()	//__asm__ __volatile__("cpsid	f\n")



#define mtgpr(rn, v)	/*__asm__ __volatile__(\
			  "mov r" stringify(rn) ", %0 \n"\
			  : : "r" (v)\
			)*/

#define mfgpr(rn)	/*({u32 rval; \
			  __asm__ __volatile__(\
			    "mov %0,r" stringify(rn) "\n"\
			    : "=r" (rval)\
			  );\
			  rval;\
			 })*/

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
#define isb() asm ("isb sy")

/* Data Synchronization Barrier */
#define dsb() asm("dsb sy")

/* Data Memory Barrier */
#define dmb() asm("dmb sy")


/* Memory Operations */
#define ldr(adr)	({u32 rval; \
			  __asm__ __volatile__(\
			    "ldr	%0,[%1]"\
			    : "=r" (rval) : "r" (adr)\
			  );\
			  rval;\
			 })

#define ldrb(adr)	({u8 rval; \
			  __asm__ __volatile__(\
			    "ldrb	%0,[%1]"\
			    : "=r" (rval) : "r" (adr)\
			  );\
			  rval;\
			 })

#define str(adr, val)	__asm__ __volatile__(\
			  "str	%0,[%1]\n"\
			  : : "r" (val), "r" (adr)\
			)

#define strb(adr, val)	__asm__ __volatile__(\
			  "strb	%0,[%1]\n"\
			  : : "r" (val), "r" (adr)\
			)

/* Count leading zeroes (clz) */
#define clz(arg)	({u8 rval; \
			  __asm__ __volatile__(\
			    "clz	%0,%1"\
			    : "=r" (rval) : "r" (arg)\
			  );\
			  rval;\
			 })
#define mtcpdc(reg,val)	asm("dc " #reg ",%0"  : : "r" (val))
#define mtcpic(reg,val)	asm("ic " #reg ",%0"  : : "r" (val))

#define mtcpicall(reg)	asm("ic " #reg)
#define mtcptlbi(reg)	asm("tlbi " #reg)
#define mtcpat(reg,val)	asm("at " #reg ",%0"  : : "r" (val))
/* CP15 operations */
#define mfcp(reg)	({u32 rval;\
			asm("mrs	%0, " #reg : "=r" (rval));\
			rval;\
			})

#define mtcp(reg,val)	asm("msr " #reg ",%0"  : : "r" (val))

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XPSEUDO_ASM_GCC_H */

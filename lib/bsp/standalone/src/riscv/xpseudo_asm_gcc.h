/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
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
* 9.00  sa       10/24/22 First release
* 9.1   mus      10/31/23 Added macro for rdtime pseudo instruction.
* 9.1   ml       11/16/23 Fix compilation errors reported with -std=c2x compiler flag
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#ifndef XPSEUDO_ASM_GCC_H
#define XPSEUDO_ASM_GCC_H

/***************************** Include Files ********************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/* necessary for pre-processor */
#define stringify(s)	tostring(s) /**< Convert token to string after macro expansion. */
#define tostring(s)	#s /**< Convert token to string literal. */

/** @brief Write value to a CSR (Control and Status Register). */
#define csrw(csr, v)	__asm__ __volatile__(	\
		"csrw " stringify(csr) ", %0\n" \
		: : "r" (v)			\
	)

/** @brief Write immediate value to a CSR. */
#define csrwi(csr, v)	__asm__ __volatile__(		       \
		"csrwi " stringify(csr) ", " stringify(v) "\n" \
	)

/** @brief Read a CSR value. */
#define csrr(csr)	({unsigned long rval = 0U;			       \
		__asm__ __volatile__("csrr %0," stringify(csr) : "=r" (rval)); \
		rval;							       \
	})

/** @brief Set bits in a CSR using immediate mask. */
#define csrsi(csr, v)	__asm__ __volatile__(		       \
		"csrsi " stringify(csr) ", " stringify(v) "\n" \
	)

/** @brief Clear bits in a CSR using immediate mask. */
#define csrci(csr, v)	__asm__ __volatile__(	       \
	"csrci " stringify(csr) ", " stringify(v) "\n" \
					  )

/** @brief Move value to a general-purpose register. */
#define mtgpr(rn, v)	__asm__ __volatile__( \
		"mv x" stringify(rn) ", %0\n" \
		: : "r" (v)		      \
	)

/** @brief Move value from a general-purpose register. */
#define mfgpr(rn)	({unsigned long rval;		      \
		__asm__ __volatile__(			      \
				 "mv %0,x" stringify(rn) "\n" \
				 : "=r" (rval)		      \
				    );			      \
		rval;					      \
	})

#define fence()		__asm__ __volatile__("fence\n") /**< Memory fence instruction. */
#define fence_i()	__asm__ __volatile__("fence.i\n") /**< Instruction fence instruction. */

#define cbo_clean()	__asm__ __volatile__("cbo.clean\n") /**< Cache block clean operation. */
#define cbo_flush()	__asm__ __volatile__("cbo.flush\n") /**< Cache block flush operation. */
#define cbo_inval()	__asm__ __volatile__("cbo.inval\n") /**< Cache block invalidate operation. */

#define ecall()		__asm__ __volatile__("ecall\n") /**< Environment call instruction. */

/** @brief Read the time CSR (cycle counter). */
#define rdtime()	({					   \
		register unsigned long rval;			   \
		__asm__ __volatile__ ( "rdtime %0" : "=r" (rval)); \
		rval;						   \
	})

/************** Atomic Macros (Inline Functions) Definitions ****************/

/** @brief Generic atomic word (32-bit) operation using AMO instruction. */
#define atomic_w(instr, addr, data) ({register unsigned int rval;	   \
				  __asm__ __volatile__(			   \
				    stringify(instr) " %0, %1, (%2)\n"	   \
				    : "=r" (rval) : "r" (data), "r" (addr) \
				  );					   \
				  rval;					   \
				 })

#define amoswap_w(addr, data) atomic_w(amoswap.w, addr, data) /**< Atomic swap 32-bit word. */
#define amoadd_w(addr, data)  atomic_w(amoadd.w,  addr, data) /**< Atomic add 32-bit word. */
#define amoand_w(addr, data)  atomic_w(amoand.w,  addr, data) /**< Atomic AND 32-bit word. */
#define amoor_w(addr, data)   atomic_w(amoor.w,   addr, data) /**< Atomic OR 32-bit word. */
#define amoxor_w(addr, data)  atomic_w(amoxor.w,  addr, data) /**< Atomic XOR 32-bit word. */
#define amomax_w(addr, data)  atomic_w(amomax.w,  addr, data) /**< Atomic signed max 32-bit word. */
#define amomaxu_w(addr, data) atomic_w(amomaxu.w, addr, data) /**< Atomic unsigned max 32-bit word. */
#define amomin_w(addr, data)  atomic_w(amomin.w,  addr, data) /**< Atomic signed min 32-bit word. */
#define amominu_w(addr, data) atomic_w(amominu.w, addr, data) /**< Atomic unsigned min 32-bit word. */

/** @brief Load-reserved 32-bit word (for LR/SC sequences). */
#define lr_w(addr) ({unsigned int rval;				     \
		__asm__ __volatile__ (				     \
			"lr.w\t%0, (%1)\n" : "=r"(rval) : "r" (addr) \
			     );					     \
		rval;						     \
	})

/** @brief Store-conditional 32-bit word (for LR/SC sequences). */
#define sc_w(addr, data) ({unsigned int rval;		       \
		__asm__ __volatile__ (			       \
			"sc.w\t%0, %1, (%2)\n"		       \
			: "=r"(rval) : "r" (data), "r" (addr)  \
				     );			       \
		rval;					       \
	})

#if __riscv_xlen == 64
#define atomic_d(instr, addr, data) ({register unsigned long rval;	   \
				  __asm__ __volatile__(			   \
				    stringify(instr) " %0, %1, (%2)\n"	   \
				    : "=r" (rval) : "r" (data), "r" (addr) \
				  );					   \
				  rval;					   \
				 })

#define amoswap_d(addr, data) atomic_d(amoswap.d, addr, data)
#define amoadd_d(addr, data)  atomic_d(amoadd.d,  addr, data)
#define amoand_d(addr, data)  atomic_d(amoand.d,  addr, data)
#define amoor_d(addr, data)   atomic_d(amoor.d,   addr, data)
#define amoxor_d(addr, data)  atomic_d(amoxor.d,  addr, data)
#define amomax_d(addr, data)  atomic_d(amomax.d,  addr, data)
#define amomaxu_d(addr, data) atomic_d(amomaxu.d, addr, data)
#define amomin_d(addr, data)  atomic_d(amomin.d,  addr, data)
#define amominu_d(addr, data) atomic_d(amominu.d, addr, data)

#define lr_d(addr) ({unsigned long rval;			     \
		__asm__ __volatile__ (				     \
			"lr.d\t%0, (%1)\n" : "=r"(rval) : "r" (addr) \
			     );					     \
		rval;						     \
	})

#define sc_d(addr, data) ({unsigned long rval;		      \
		__asm__ __volatile__ (			      \
			"sc.d\t%0, %1, (%2)\n"		      \
			: "=r"(rval) : "r" (data), "r" (addr) \
			     );				      \
		rval;					      \
	})
#endif /* __riscv_xlen == 64 */

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XPSEUDO_ASM_GCC_H */

/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
*
* riscv_scrub ()
*
*    Scrub LMB memory and all internal BRAMs (data cache, instruction cache,
*    and branch target cache) in MicroBlaze RISC-V to reduce the possibility
*    of an uncorrectable error when fault tolerance support is enabled.
*
*    Call this routine regularly from a timer interrupt.
*
*    Parameters:
*       None
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.0   sa   12/11/22 Initial version
* 9.2   ss   21/04/26 Use t0 instead of t3 for RV32E compatibility.
*                     t3 (x28) does not exist in RV32E (only x0-x15).
*
*******************************************************************************/

#include "xparameters.h"

/* Define if fault tolerance is used */
#ifdef XPAR_MICROBLAZE_RISCV_FAULT_TOLERANT
  #if XPAR_MICROBLAZE_RISCV_FAULT_TOLERANT > 0
    #define FAULT_TOLERANT
  #endif
#endif

/* Define if LMB is used and can be scrubbed */
#if defined(XPAR_MICROBLAZE_RISCV_D_LMB) &&            \
	    defined(XPAR_DLMB_CNTLR_BASEADDR) && \
	    defined(XPAR_DLMB_CNTLR_HIGHADDR)
  #if XPAR_MICROBLAZE_RISCV_D_LMB == 1
    #define HAS_SCRUBBABLE_LMB
    #define DLMB_MASK (XPAR_DLMB_CNTLR_HIGHADDR - XPAR_DLMB_CNTLR_BASEADDR)
  #endif
#endif

/* Set default cache line lengths */
#ifndef XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN
  #define XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN   4
#endif

#ifndef XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN
  #define XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN   4
#endif

/* Define if internal Data Cache BRAMs are used */
#if defined(XPAR_MICROBLAZE_RISCV_USE_DCACHE) && defined(XPAR_MICROBLAZE_RISCV_DCACHE_BYTE_SIZE)
  #if XPAR_MICROBLAZE_RISCV_USE_DCACHE == 1 && XPAR_MICROBLAZE_RISCV_DCACHE_BYTE_SIZE > 1024
    #define HAS_BRAM_DCACHE
    #define DCACHE_INCREMENT (XPAR_MICROBLAZE_RISCV_DCACHE_LINE_LEN * 4)
    #define DCACHE_MASK (XPAR_MICROBLAZE_RISCV_DCACHE_BYTE_SIZE - 1)
  #endif
#endif

/* Define if internal Instruction Cache BRAMs are used */
#if defined(XPAR_MICROBLAZE_RISCV_USE_ICACHE) && defined(XPAR_MICROBLAZE_RISCV_CACHE_BYTE_SIZE)
  #if XPAR_MICROBLAZE_RISCV_USE_ICACHE == 1 && XPAR_MICROBLAZE_RISCV_CACHE_BYTE_SIZE > 1024
    #define HAS_BRAM_ICACHE
    #define ICACHE_INCREMENT (XPAR_MICROBLAZE_RISCV_ICACHE_LINE_LEN * 4)
    #define ICACHE_MASK (XPAR_MICROBLAZE_RISCV_CACHE_BYTE_SIZE - 1)
  #endif
#endif

/* Define if internal BTC BRAM is used, and match BTC clear to a complete cache scrub */
#if defined(XPAR_MICROBLAZE_RISCV_USE_BRANCH_TARGET_CACHE) && \
    defined(XPAR_MICROBLAZE_RISCV_BRANCH_TARGET_CACHE_SIZE)
  #if XPAR_MICROBLAZE_RISCV_USE_BRANCH_TARGET_CACHE == 1
    #if XPAR_MICROBLAZE_RISCV_BRANCH_TARGET_CACHE_SIZE == 0 || \
	XPAR_MICROBLAZE_RISCV_BRANCH_TARGET_CACHE_SIZE >  4
      #define HAS_BRAM_BRANCH_TARGET_CACHE
      #ifdef HAS_BRAM_DCACHE
	#define BTC_MASK_D (XPAR_MICROBLAZE_RISCV_DCACHE_BYTE_SIZE/DCACHE_INCREMENT-1)
      #else
	#define BTC_MASK_D 256
      #endif
      #ifdef HAS_BRAM_ICACHE
	#define BTC_MASK_I (XPAR_MICROBLAZE_RISCV_CACHE_BYTE_SIZE/ICACHE_INCREMENT-1)
      #else
	#define BTC_MASK_I 256
      #endif
      #if BTC_MASK_D > BTC_MASK_I
	#define BTC_MASK BTC_MASK_D
      #else
	#define BTC_MASK BTC_MASK_I
      #endif
    #endif
  #endif
#endif

/* Define index offsets to persistent data used by this routine */
#define DLMB_INDEX_OFFSET     0
#define DCACHE_INDEX_OFFSET   4
#define ICACHE_INDEX_OFFSET   8
#define BTC_CALL_COUNT_OFFSET 12

	.text
	.globl	riscv_scrub
	.align	2

/*
 * Register usage:
 *   t0 (x5)  - scratch/temporary for load-store in LMB scrub
 *   t1 (x6)  - pointer to persistent data
 *   t2 (x7)  - index values
 *
 * Note: t0 is used instead of t3 for RV32E compatibility.
 *       RV32E only has registers x0-x15 (t0-t2 are x5-x7).
 *       t3-t6 (x28-x31) do not exist in RV32E.
 */
riscv_scrub:
#ifdef FAULT_TOLERANT
	la	t1, L_persistent_data			/* Get pointer to data */

#ifdef HAS_SCRUBBABLE_LMB
L_dlmb:
	lw	t2, DLMB_INDEX_OFFSET(t1)		/* Get dlmb index */
	lw	t0, 0(t2)				/* Load from memory (use t0 for RV32E compat) */
	sw	t0, 0(t2)				/* Store back to scrub ECC */
	addi	t2, t2, 4				/* Increment and save dlmb index */
	andi	t2, t2, DLMB_MASK
	sw	t2, DLMB_INDEX_OFFSET(t1)
#endif /* HAS_SCRUBBABLE_LMB */

#ifdef HAS_BRAM_DCACHE
L_dcache:
	lw	t2, DCACHE_INDEX_OFFSET(t1)		/* Get dcache line index */
	/* Invalidate data cache line */
	addi	t2, t2, DCACHE_INCREMENT		/* Increment and save entry index */
	andi	t2, t2, DCACHE_MASK
	sw	t2, DCACHE_INDEX_OFFSET(t1)
#endif /* HAS_BRAM_DCACHE */

#ifdef HAS_BRAM_ICACHE
L_icache:
	lw	t2, ICACHE_INDEX_OFFSET(t1)		/* Get icache line index */
	/* Invalidate data cache line */
	addi	t2, t2, ICACHE_INCREMENT		/* Increment and save entry index */
	andi	t2, t2, ICACHE_MASK
	sw	t2, ICACHE_INDEX_OFFSET(t1)
#endif /* HAS_BRAM_ICACHE */

#ifdef HAS_BRAM_BRANCH_TARGET_CACHE
L_btc:
	lw	t2, BTC_CALL_COUNT_OFFSET(t1)		/* Get BTC call count offset */
	addi	t2, t2, 1				/* Increment and save call count */
	andi	t2, t2, BTC_MASK
	sw	t2, BTC_CALL_COUNT_OFFSET(t1)
	bnez	t2, L_skip_btc_scrub			/* Skip scrub unless count wrap */
	nop
	fence.i						/* Clear branch target cache */
L_skip_btc_scrub:
#endif /* HAS_BRAM_BRANCH_TARGET_CACHE */

#endif /* FAULT_TOLERANT */
L_done:
	ret						/* Return */
	.end	riscv_scrub

	/* Persistent data used by this routine */
	.data
	.align	2
L_persistent_data:
	.long	0					/* dlmb index      */
	.long	0					/* dcache index    */
	.long	0					/* icache index    */
	.long	0					/* btc call count  */

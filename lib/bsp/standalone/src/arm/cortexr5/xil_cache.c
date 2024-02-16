/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.c
*
* Contains required functions for the ARM cache functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who Date     Changes
* ----- ---- -------- -----------------------------------------------
* 5.00 	pkp  02/20/14 First release
* 6.2   mus  01/27/17 Updated to support IAR compiler
* 7.3   dp   06/25/20 Updated to support armclang compiler
* 7.7	sk   01/10/22 Update IRQ_FIQ_MASK macro from signed to unsigned
* 		      to fix misra_c_2012_rule_10_4 violation.
* 7.7	sk   01/10/22 Typecast to fix wider essential type misra_c_2012_rule_10_7
* 		      violation.
* 8.0   mus  02/21/22 Updated cache API's to support Cortex-R52
* 8.1    asa 02/13/23 The existing Xil_DCacheInvalidateRange has a bug where
*                     the last cache line will not get invalidated under certain
*                     scenarios. Changes are made to fix the same.
* 9.0    ml  03/03/23 Added description to fix doxygen warnings.
* 9.0    ml  07/12/23 fixed compilation warnings.
* 9.1    asa 12/01/24 Fix issues in Xil_DCacheInvalidateRange.
* 9.1    asa 27/01/24 The fix for the above change (on 12/01/24) has
*                     created issues for Cortex-R52. Because of the
*                     bug previously present (before the last fix) R52
*                     somehow worked. With the bux fixed through the
*                     patch in 12/01/24, it exposed issues in the
*                     DCacheInvalidate API for R52 that was there
*                     from the beginning.
*                     Also in an unlikely scenario where the start address
*                     passed is 0x0 and length is less than 0x20 (cache line),
*                     the XilDCacheInvalidateRange API will result in
*                     a probable crash as it will try to invalidate the
*                     complete 4 GB address range.
*                     The changes are made to fix the same.
* 9.1   asa  31/01/24 Fix overflow issues under corner cases for various
*                     cache maintenance APIs.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xil_io.h"
#include "xpseudo_asm.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xreg_cortexr5.h"
#include "xil_exception.h"


/************************** Variable Definitions *****************************/

#define IRQ_FIQ_MASK 0xC0U	/**< Mask IRQ and FIQ interrupts in cpsr */
#define MAX_ADDR 				0xFFFFFFFFU
#define LAST_CACHELINE_START	0xFFFFFFE0U

#if defined (__clang__)
extern s32  Image$$ARM_LIB_STACK$$Limit;
extern s32  Image$$ARM_UNDEF_STACK$$Base;
#elif defined (__GNUC__)
extern s32  _stack_end;
extern s32  __undef_stack;
#endif

/****************************************************************************/
/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
* @brief    Enable the Data cache.
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheEnable(void)
{
	register u32 CtrlReg;

	/* enable caches only if they are disabled */
#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif
	if ((CtrlReg & XREG_CP15_CONTROL_C_BIT) == 0x00000000U) {
		/* invalidate the Data cache */
		Xil_DCacheInvalidate();

		/* enable the Data cache */
		CtrlReg |= (XREG_CP15_CONTROL_C_BIT);

		mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	}
}

/****************************************************************************/
/**
* @brief    Disable the Data cache.
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheDisable(void)
{
	register u32 CtrlReg;

	/* clean and invalidate the Data cache */
	Xil_DCacheFlush();

	/* disable the Data cache */
#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif

	CtrlReg &= ~(XREG_CP15_CONTROL_C_BIT);

	mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
}

/****************************************************************************/
/**
* @brief    Invalidate the entire Data cache.
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheInvalidate(void)
{
	u32 currmask;
	u32 stack_start, stack_end, stack_size;
#if defined (ARMR52)
	register u32 CsidReg, C7Reg;
	u32 LineSize, NumWays;
	u32 Way, WayIndex, Set, SetIndex, NumSet;
#endif

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

#if defined (__clang__)
	stack_end = (u32 )&Image$$ARM_LIB_STACK$$Limit;
	stack_start = (u32 )&Image$$ARM_UNDEF_STACK$$Base;
#elif defined (__GNUC__)
	stack_end = (u32 )&_stack_end;
	stack_start = (u32 )&__undef_stack;
#endif

#if defined(__GNUC__) || defined(__clang__)
	stack_size = stack_start - stack_end;

	/* Flush stack memory to save return address */
	Xil_DCacheFlushRange(stack_end, stack_size);
#endif

#if !defined (ARMR52)
	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);

	/*invalidate all D cache*/
	mtcp(XREG_CP15_INVAL_DC_ALL, 0);
#else
	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);

#if defined (__GNUC__)
	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_CACHE_SIZE_ID, CsidReg);
#endif
	/* Number of sets */
	NumSet = (CsidReg >> 13U) & 0x000001FFU;
	NumSet += 0x00000001U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x000003ffU) >> 3U;
	NumWays += 0x00000001U;


	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;


	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set;
			/* Flush by Set/Way */
			mtcp(XREG_CP15_INVAL_DC_LINE_SW, C7Reg);

			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += 0x40000000U;
	}

	/* Wait for flush to complete */
	dsb();
#endif
	mtcpsr(currmask);
}

/****************************************************************************/
/**
* @brief    Invalidate a Data cache line. If the byte specified by the
*           address (adr) is cached by the data cache, the cacheline
*           containing that byte is invalidated.If the cacheline is modified
* 	        (dirty), the modified contents are lost and are NOT written
*           to system memory before the line is invalidated.
*
*
* @param	adr: 32bit address of the data to be flushed.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_DCacheInvalidateLine(INTPTR adr)
{
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);
	mtcp(XREG_CP15_INVAL_DC_LINE_MVA_POC, (adr & (~0x1F)));

	/* Wait for invalidate to complete */
	dsb();

	mtcpsr(currmask);
}

/****************************************************************************/
/**
* @brief    Invalidate the Data cache for the given address range.
*           If the bytes specified by the address (adr) are cached by the
*           Data cache,the cacheline containing that byte is invalidated.
*           If the cacheline is modified (dirty), the modified contents are
*           lost and are NOT written to system memory before the line is
*           invalidated.
*
* @param	adr: 32bit start address of the range to be invalidated.
* @param	len: Length of range to be invalidated in bytes.
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheInvalidateRange(INTPTR adr, u32 len)
{
#if !defined(ARMR52)
	const u32 cacheline = 32U;
	u32 end;
	u32 tempadr;
	u32 tempend;
	u32 currmask;
	u32 unalignedstart = 0x0;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	if (len != 0U) {
		tempadr = adr & (~(cacheline - 1U));
		((MAX_ADDR - (u32)adr) < len) ? (end = MAX_ADDR) : (end = adr + len);
		tempend = end & (~(cacheline - 1U));

		/* Select L1 Data cache in CSSR */
		mtcp(XREG_CP15_CACHE_SIZE_SEL, 0U);

		if (tempadr != (u32)adr) {
			unalignedstart = 1;
			Xil_DCacheFlushLine(tempadr);
			tempadr >= LAST_CACHELINE_START ? (adr = end) : (adr = tempadr + cacheline);
		}
		if ((tempend != end) && ((tempend != tempadr) || (unalignedstart == 0x0U))) {
			Xil_DCacheFlushLine(tempend);
			end >= cacheline ? (end -= cacheline) : (end = 0);
		}

		while (adr < (INTPTR)end) {
			/* Invalidate Data cache line */
			asm_inval_dc_line_mva_poc(adr);
			((MAX_ADDR - (u32)adr) < cacheline) ? (adr = MAX_ADDR) : (adr += cacheline);
		}
	}

	dsb();
	mtcpsr(currmask);
#else
	const u32 cacheline = 32U;
	u32 currmask;
	u32 tempadr = adr & (~(cacheline - 1U));
	u32 end;

	((MAX_ADDR - adr) < len) ? (end = MAX_ADDR) : (end = adr + len);

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	if (len != 0U) {
		while (tempadr < end) {
			/* Invalidate Data cache line */
			asm_inval_dc_line_mva_poc(tempadr);
			((MAX_ADDR - tempadr) < cacheline) ? (tempadr = MAX_ADDR) : (tempadr += cacheline);
		}
	}

	dsb();
	mtcpsr(currmask);

#endif
}

/****************************************************************************/
/**
* @brief    Flush the entire Data cache.
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheFlush(void)
{
#if !defined(ARMR52)
	register u32 CsidReg, C7Reg;
	u32 CacheSize, LineSize, NumWays;
	u32 Way, WayIndex, Set, SetIndex, NumSet;
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);

#if defined (__GNUC__)
	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_CACHE_SIZE_ID, CsidReg);
#endif
	/* Determine Cache Size */

	CacheSize = (CsidReg >> 13U) & 0x000001FFU;
	CacheSize += 0x00000001U;
	CacheSize *= (u32)128;    /* to get number of bytes */

	/* Number of Ways */
	NumWays = (CsidReg & 0x000003ffU) >> 3U;
	NumWays += 0x00000001U;

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	NumSet = CacheSize / NumWays;
	NumSet /= (((u32)0x00000001U) << LineSize);

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set;
			/* Flush by Set/Way */
			asm_clean_inval_dc_line_sw(C7Reg);

			Set += (((u32)0x00000001U) << LineSize);
		}
		Set = 0U;
		Way += 0x40000000U;
	}

	/* Wait for flush to complete */
	dsb();
	mtcpsr(currmask);

	mtcpsr(currmask);
#endif
}

/****************************************************************************/
/**
* @brief   Flush a Data cache line. If the byte specified by the address (adr)
*          is cached by the Data cache, the cacheline containing that byte is
*          invalidated.	If the cacheline is modified (dirty), the entire
*          contents of the cacheline are written to system memory before the
*          line is invalidated.
*
* @param   adr: 32bit address of the data to be flushed.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_DCacheFlushLine(INTPTR adr)
{
#if defined(ARMR52)
	(void)adr;
#else
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);

	mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC, (adr & (~0x1F)));

	/* Wait for flush to complete */
	dsb();
	mtcpsr(currmask);
#endif
}

/****************************************************************************/
/**
* @brief    Flush the Data cache for the given address range.
*           If the bytes specified by the address (adr) are cached by the
*           Data cache, the cacheline containing those bytes is invalidated.If
*           the cacheline is modified (dirty), the written to system memory
*           before the lines are invalidated.
*
* @param	adr: 32bit start address of the range to be flushed.
* @param	len: Length of the range to be flushed in bytes
*
* @return	None.
*
****************************************************************************/
void Xil_DCacheFlushRange(INTPTR adr, u32 len)
{
#if defined(ARMR52)
	(void)adr;
	(void)len;
#else
	u32 LocalAddr = adr;
	const u32 cacheline = 32U;
	u32 end;
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	if (len != 0x00000000U) {
		/* Back the starting address up to the start of a cache line
		 * perform cache operations until adr+len
		 */
		((MAX_ADDR - LocalAddr) < len) ? (end = MAX_ADDR) : (end = LocalAddr + len);
		LocalAddr &= ~(cacheline - 1U);

		while (LocalAddr < end) {
			/* Flush Data cache line */
			asm_clean_inval_dc_line_mva_poc(LocalAddr);
			((MAX_ADDR - LocalAddr) < cacheline) ? (LocalAddr = MAX_ADDR) : (LocalAddr += cacheline) ;
		}
	}
	dsb();
	mtcpsr(currmask);
#endif
}
/****************************************************************************/
/**
* @brief    Store a Data cache line. If the byte specified by the address
*           (adr) is cached by the Data cache and the cacheline is modified
*           (dirty), the entire contents of the cacheline are written to
*           system memory.After the store completes, the cacheline is marked
*           as unmodified (not dirty).
*
* @param	adr: 32bit address of the data to be stored
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_DCacheStoreLine(INTPTR adr)
{
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);
	mtcp(XREG_CP15_CLEAN_DC_LINE_MVA_POC, (adr & (~0x1F)));

	/* Wait for store to complete */
	dsb();
	isb();

	mtcpsr(currmask);
}

/****************************************************************************/
/**
* @brief    Enable the instruction cache.
*
* @return	None.
*
****************************************************************************/
void Xil_ICacheEnable(void)
{
	register u32 CtrlReg;

	/* enable caches only if they are disabled */
#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif
	if ((CtrlReg & XREG_CP15_CONTROL_I_BIT) == 0x00000000U) {
		/* invalidate the instruction cache */
		mtcp(XREG_CP15_INVAL_IC_POU, 0);

		/* enable the instruction cache */
		CtrlReg |= (XREG_CP15_CONTROL_I_BIT);

		mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	}
}

/****************************************************************************/
/**
* @brief    Disable the instruction cache.
*
* @return	None.
*
****************************************************************************/
void Xil_ICacheDisable(void)
{
	register u32 CtrlReg;

	dsb();

	/* invalidate the instruction cache */
	mtcp(XREG_CP15_INVAL_IC_POU, 0);

	/* disable the instruction cache */
#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif

	CtrlReg &= ~(XREG_CP15_CONTROL_I_BIT);

	mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
}

/****************************************************************************/
/**
* @brief    Invalidate the entire instruction cache.
*
* @return	None.
*
****************************************************************************/
void Xil_ICacheInvalidate(void)
{
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL, 1);

	/* invalidate the instruction cache */
	mtcp(XREG_CP15_INVAL_IC_POU, 0);

	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************/
/**
* @brief    Invalidate an instruction cache line.If the instruction specified
*           by the address is cached by the instruction cache, the
*           cacheline containing that instruction is invalidated.
*
* @param	adr: 32bit address of the instruction to be invalidated.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_ICacheInvalidateLine(INTPTR adr)
{
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL, 1);
	mtcp(XREG_CP15_INVAL_IC_LINE_MVA_POU, (adr & (~0x1F)));

	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************/
/**
* @brief    Invalidate the instruction cache for the given address range.
*           If the bytes specified by the address (adr) are cached by the
*           Data cache, the cacheline containing that byte is invalidated.
*           If the cachelineis modified (dirty), the modified contents are
*           lost  and are NOT written to system memory before the line is
*           invalidated.
*
* @param	adr: 32bit start address of the range to be invalidated.
* @param	len: Length of the range to be invalidated in bytes.
*
* @return	None.
*
****************************************************************************/
void Xil_ICacheInvalidateRange(INTPTR adr, u32 len)
{
	u32 LocalAddr = adr;
	const u32 cacheline = 32U;
	u32 end;
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);
	if (len != 0x00000000U) {
		/* Back the starting address up to the start of a cache line
		 * perform cache operations until adr+len
		 */
		((MAX_ADDR - LocalAddr) < len) ? (end = MAX_ADDR) : (end = LocalAddr + len);
		LocalAddr = LocalAddr & ~(cacheline - 1U);

		/* Select cache L0 I-cache in CSSR */
		mtcp(XREG_CP15_CACHE_SIZE_SEL, 1U);

		while (LocalAddr < end) {

			/* Invalidate L1 I-cache line */
			asm_inval_ic_line_mva_pou(LocalAddr);
			((MAX_ADDR - LocalAddr) < cacheline) ? (LocalAddr = MAX_ADDR) : (LocalAddr += cacheline) ;
		}
	}

	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

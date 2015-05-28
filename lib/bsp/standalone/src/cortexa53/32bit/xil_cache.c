/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* 5.2	pkp  	28/05/15 First release
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xil_cache_l.h"
#include "xil_io.h"
#include "xpseudo_asm.h"
#include "xparameters.h"
#include "xreg_cortexa53.h"
#include "xil_exception.h"

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#define IRQ_FIQ_MASK 0xC0U	/* Mask IRQ and FIQ interrupts in cpsr */

extern s32  _stack_end;
extern s32  __undef_stack;

/****************************************************************************
*
* Enable the Data cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheEnable(void)
{
	u32 CtrlReg;
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	/* enable caches only if they are disabled */
	if((CtrlReg & XREG_CONTROL_DCACHE_BIT) == 0X00000000U){

		/* invalidate the Data cache */
		Xil_DCacheInvalidate();

		CtrlReg |= XREG_CONTROL_DCACHE_BIT;

		/* enable the Data cache */
		mtcp(XREG_CP15_SYS_CONTROL,CtrlReg);
	}
}

/****************************************************************************
*
* Disable the Data cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheDisable(void)
{
	u32 CtrlReg;
	/* clean and invalidate the Data cache */
	Xil_DCacheFlush();
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);

	CtrlReg &= ~(XREG_CONTROL_DCACHE_BIT);
	/* disable the Data cache */
	mtcp(XREG_CP15_SYS_CONTROL,CtrlReg);
}

/****************************************************************************
*
* Invalidate the entire Data cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheInvalidate(void)
{
	register u32 CsidReg, C7Reg;
	u32 LineSize, NumWays;
	u32 Way, WayIndex,WayAdjust, Set, SetIndex, NumSet, NumCacheLevel, CacheLevel,CacheLevelIndex;
	u32 currmask;

	u32 stack_start,stack_end,stack_size;


	stack_end = (u32)&_stack_end;
	stack_start = (u32)&__undef_stack;
	stack_size=stack_start-stack_end;

	/*Flush stack memory to save return address*/
	Xil_DCacheFlushRange(stack_end, stack_size);

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);


	/* Number of level of cache*/
	NumCacheLevel = (mfcp(XREG_CP15_CACHE_LEVEL_ID)>>24U) & 0x00000007U;

	CacheLevel=0U;
	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,CacheLevel);
	isb();

	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0X00000001U;

	/*Number of Set*/
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0X00000001U;

	WayAdjust = 0x1E;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex =0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			mtcp(XREG_CP15_INVAL_DC_LINE_SW,C7Reg);
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}

	/* Wait for invalidate to complete */
	dsb();

	/* Select cache level 1 and D cache in CSSR */
	CacheLevel += (0x00000001U<<1U) ;
	mtcp(XREG_CP15_CACHE_SIZE_SEL,CacheLevel);
	isb();

	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0x00000001U;

	/* Number of Sets */
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0x00000001U;

	WayAdjust = 0x1C;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			mtcp(XREG_CP15_INVAL_DC_LINE_SW,C7Reg);
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}
	/* Wait for invalidate to complete */
	dsb();

	mtcpsr(currmask);
}

/****************************************************************************
*
* Invalidate a Data cache line. If the byte specified by the address (adr)
* is cached by the Data cache, the cacheline containing that byte is
* invalidated.	If the cacheline is modified (dirty), the modified contents
* are lost and are NOT written to system memory before the line is
* invalidated.
*
* @param	Address to be flushed.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_DCacheInvalidateLine(u32 adr)
{

	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x0);
	isb();
	mtcp(XREG_CP15_INVAL_DC_LINE_MVA_POC,(adr & (~0x3F)));
	/* Wait for invalidate to complete */
	dsb();
	/* Select cache level 1 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x2);
	isb();
	mtcp(XREG_CP15_INVAL_DC_LINE_MVA_POC,(adr & (~0x3F)));
	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************
*
* Invalidate the Data cache for the given address range.
* If the bytes specified by the address (adr) are cached by the Data cache,
* the cacheline containing that byte is invalidated.	If the cacheline
* is modified (dirty), the modified contents are lost and are NOT
* written to system memory before the line is invalidated.
*
* In this function, if start address or end address is not aligned to cache-line,
* particular cache-line containing unaligned start or end address is flush first
* and then invalidated the others as invalidating the same unaligned cache line
* may result into loss of data. This issue raises few possibilities.
*
*
* If the address to be invalidated is not cache-line aligned, the
* following choices are available:
* 1) Invalidate the cache line when required and do not bother much for the
* side effects. Though it sounds good, it can result in hard-to-debug issues.
* The problem is, if some other variable are allocated in the
* same cache line and had been recently updated (in cache), the invalidation
* would result in loss of data.
*
* 2) Flush the cache line first. This will ensure that if any other variable
* present in the same cache line and updated recently are flushed out to memory.
* Then it can safely be invalidated. Again it sounds good, but this can result
* in issues. For example, when the invalidation happens
* in a typical ISR (after a DMA transfer has updated the memory), then flushing
* the cache line means, loosing data that were updated recently before the ISR
* got invoked.
*
* Linux prefers the second one. To have uniform implementation (across standalone
* and Linux), the second option is implemented.
* This being the case, follwoing needs to be taken care of:
* 1) Whenever possible, the addresses must be cache line aligned. Please nore that,
* not just start address, even the end address must be cache line aligned. If that
* is taken care of, this will always work.
* 2) Avoid situations where invalidation has to be done after the data is updated by
* peripheral/DMA directly into the memory. It is not tough to achieve (may be a bit
* risky). The common use case to do invalidation is when a DMA happens. Generally
* for such use cases, buffers can be allocated first and then start the DMA. The
* practice that needs to be followed here is, immediately after buffer allocation
* and before starting the DMA, do the invalidation. With this approach, invalidation
* need not to be done after the DMA transfer is over.
*
* This is going to always work if done carefully.
* However, the concern is, there is no guarantee that invalidate has not needed to be
* done after DMA is complete. For example, because of some reasons if the first cache
* line or last cache line (assuming the buffer in question comprises of multiple cache
* lines) are brought into cache (between the time it is invalidated and DMA completes)
* because of some speculative prefetching or reading data for a variable present
* in the same cache line, then we will have to invalidate the cache after DMA is complete.
*
*
* @param	Start address of range to be invalidated.
* @param	Length of range to be invalidated in bytes.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheInvalidateRange(INTPTR adr, u32 len)
{
	const u32 cacheline = 64U;
	u32 end;
	u32 tempadr = adr;
	u32 tempend;
	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);
	if (len != 0U) {
		end = tempadr + len;
		tempend = end;

		if ((tempadr & (cacheline-1U)) != 0U) {
			tempadr &= (~(cacheline - 1U));
			Xil_DCacheFlushLine(tempadr);
			tempadr += cacheline;
		}
		if ((tempend & (cacheline-1U)) != 0U) {
			tempend &= (~(cacheline - 1U));
			Xil_DCacheFlushLine(tempend);
		}

		while (tempadr < tempend) {
			/* Select cache level 0 and D cache in CSSR */
			mtcp(XREG_CP15_CACHE_SIZE_SEL,0x0);
			/* Invalidate Data cache line */
			mtcp(XREG_CP15_INVAL_DC_LINE_MVA_POC,(tempadr & (~0x3F)));
			/* Wait for invalidate to complete */
			dsb();
			/* Select cache level 0 and D cache in CSSR */
			mtcp(XREG_CP15_CACHE_SIZE_SEL,0x2);
			/* Invalidate Data cache line */
			mtcp(XREG_CP15_INVAL_DC_LINE_MVA_POC,(tempadr & (~0x3F)));
			/* Wait for invalidate to complete */
			dsb();
			tempadr += cacheline;
		}
	}
	mtcpsr(currmask);
}

/****************************************************************************
*
* Flush the entire Data cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheFlush(void)
{
	register u32 CsidReg, C7Reg;
	u32 LineSize, NumWays;
	u32 Way, WayIndex,WayAdjust, Set, SetIndex, NumSet, NumCacheLevel, CacheLevel,CacheLevelIndex;
	u32 currmask;

	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);


	/* Number of level of cache*/
	NumCacheLevel = (mfcp(XREG_CP15_CACHE_LEVEL_ID)>>24U) & 0x00000007U;

	CacheLevel=0U;
	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,CacheLevel);
	isb();

	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0X00000001U;

	/*Number of Set*/
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0X00000001U;

	WayAdjust = 0x1E;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex =0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_SW,C7Reg);
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}

	/* Wait for invalidate to complete */
	dsb();

	/* Select cache level 1 and D cache in CSSR */
	CacheLevel += (0x00000001U<<1U) ;
	mtcp(XREG_CP15_CACHE_SIZE_SEL,CacheLevel);
	isb();

	CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0x00000001U;

	/* Number of Sets */
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0x00000001U;

	WayAdjust = 0x1C;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_SW,C7Reg);
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}
	/* Wait for invalidate to complete */
	dsb();

	mtcpsr(currmask);
}

/****************************************************************************
*
* Flush a Data cache line. If the byte specified by the address (adr)
* is cached by the Data cache, the cacheline containing that byte is
* invalidated.	If the cacheline is modified (dirty), the entire
* contents of the cacheline are written to system memory before the
* line is invalidated.
*
* @param	Address to be flushed.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_DCacheFlushLine(u32 adr)
{
	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	/* Select cache level 0 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x0);
	isb();
	mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC,(adr & (~0x3F)));
	/* Wait for invalidate to complete */
	dsb();
	/* Select cache level 1 and D cache in CSSR */
	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x2);
	isb();
	mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC,(adr & (~0x3F)));
	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************
* Flush the Data cache for the given address range.
* If the bytes specified by the address (adr) are cached by the Data cache,
* the cacheline containing that byte is invalidated.	If the cacheline
* is modified (dirty), the written to system memory first before the
* before the line is invalidated.
*
* @param	Start address of range to be flushed.
* @param	Length of range to be flushed in bytes.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_DCacheFlushRange(INTPTR adr, u32 len)
{
	const u32 cacheline = 64U;
	u32 end;
	u32 tempadr = adr;
	u32 tempend;
	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);
	if (len != 0U) {
		end = tempadr + len;
		tempend = end;

		if ((tempadr & (cacheline-1U)) != 0U) {
			tempadr &= (~(cacheline - 1U));
			Xil_DCacheFlushLine(tempadr);
			tempadr += cacheline;
		}
		if ((tempend & (cacheline-1U)) != 0U) {
			tempend &= (~(cacheline - 1U));
			Xil_DCacheFlushLine(tempend);
		}

		while (tempadr < tempend) {
			/* Select cache level 0 and D cache in CSSR */
			mtcp(XREG_CP15_CACHE_SIZE_SEL,0x0);
			/* Invalidate Data cache line */
			mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC,(tempadr & (~0x3F)));
			/* Wait for invalidate to complete */
			dsb();
			/* Select cache level 0 and D cache in CSSR */
			mtcp(XREG_CP15_CACHE_SIZE_SEL,0x2);
			/* Invalidate Data cache line */
			mtcp(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC,(tempadr & (~0x3F)));
			/* Wait for invalidate to complete */
			dsb();
			tempadr += cacheline;
		}
	}
	mtcpsr(currmask);
}

/****************************************************************************
*
* Enable the instruction cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_ICacheEnable(void)
{
	u32 CtrlReg;
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	/* enable caches only if they are disabled */
	if((CtrlReg & XREG_CONTROL_ICACHE_BIT)==0x00000000U){
	/* invalidate the instruction cache */
	Xil_ICacheInvalidate();

	CtrlReg |= XREG_CONTROL_ICACHE_BIT;
	/* enable the instruction cache */
	mtcp(XREG_CP15_SYS_CONTROL,CtrlReg);
	}
}

/****************************************************************************
*
* Disable the instruction cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_ICacheDisable(void)
{
	u32 CtrlReg;
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	/* invalidate the instruction cache */
	Xil_ICacheInvalidate();
	CtrlReg &= ~(XREG_CONTROL_ICACHE_BIT);
	/* disable the instruction cache */
	mtcp(XREG_CP15_SYS_CONTROL,CtrlReg);

}

/****************************************************************************
*
* Invalidate the entire instruction cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_ICacheInvalidate(void)
{
	unsigned int currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);
	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x1);
	dsb();
	/* invalidate the instruction cache */
	mtcp(XREG_CP15_INVAL_IC_POU,0x0);
	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************
*
* Invalidate an instruction cache line.	If the instruction specified by the
* parameter adr is cached by the instruction cache, the cacheline containing
* that instruction is invalidated.
*
* @param	None.
*
* @return	None.
*
* @note		The bottom 4 bits are set to 0, forced by architecture.
*
****************************************************************************/
void Xil_ICacheInvalidateLine(u32 adr)
{
	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	mtcp(XREG_CP15_CACHE_SIZE_SEL,0x1);
	/*Invalidate I Cache line*/
	mtcp(XREG_CP15_INVAL_IC_LINE_MVA_POU,adr & (~0x3F));
	/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

/****************************************************************************
*
* Invalidate the instruction cache for the given address range.
* If the bytes specified by the address (adr) are cached by the Data cache,
* the cacheline containing that byte is invalidated. If the cacheline
* is modified (dirty), the modified contents are lost and are NOT
* written to system memory before the line is invalidated.
*
* @param	Start address of range to be invalidated.
* @param	Length of range to be invalidated in bytes.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void Xil_ICacheInvalidateRange(INTPTR adr, u32 len)
{
	const u32 cacheline = 64U;
	u32 end;
	u32 tempadr = adr;
	u32 tempend;
	u32 currmask;
	currmask = mfcpsr();
	mtcpsr(currmask | IRQ_FIQ_MASK);

	if (len != 0x00000000U) {
		end = tempadr + len;
		tempend = end;
		tempadr &= ~(cacheline - 0x00000001U);

		/* Select cache Level 0 I-cache in CSSR */
		mtcp(XREG_CP15_CACHE_SIZE_SEL,0x1);
		while (tempadr < tempend) {
			/*Invalidate I Cache line*/
			mtcp(XREG_CP15_INVAL_IC_LINE_MVA_POU,adr & (~0x3F));

			tempadr += cacheline;
		}
	}
/* Wait for invalidate to complete */
	dsb();
	mtcpsr(currmask);
}

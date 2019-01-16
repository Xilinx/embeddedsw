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
* @file xil_mmu.c
*
* This file provides APIs for enabling/disabling MMU and setting the memory
* attributes for sections, in the MMU translation table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.2	pkp  28/05/15 First release
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xpseudo_asm.h"
#include "xil_types.h"
#include "xil_mmu.h"


/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

extern u32 MMUTable;

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
* @brief	This function sets the memory attributes for a section covering 1MB
*			of memory in the translation table.
*
* @param	Addr: 32-bit address for which the attributes need to be set.
* @param	attrib: Attributes for the specified memory region. xil_mmu.h
*			contains commonly used memory attributes definitions which can be
*			utilized for this function.
*
*
* @return	None.
*
* @note		The MMU or D-cache does not need to be disabled before changing a
*			translation table entry.
*
******************************************************************************/
void Xil_SetTlbAttributes(UINTPTR Addr, u32 attrib)
{
	u32 *ptr;
	u32 section;

	section = Addr / 0x100000U;
	ptr = &MMUTable;
	ptr += section;
	if(ptr != NULL) {
		*ptr = (Addr & 0xFFF00000U) | attrib;
	}

	Xil_DCacheFlush();

	mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
	/* Invalidate all branch predictors */
	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);

	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}

/*****************************************************************************/
/**
* @brief	Enable MMU for Cortex-A53 processor in 32bit mode. This function
*			invalidates the instruction and data caches before enabling MMU.
*
* @param	None.
* @return	None.
*
******************************************************************************/
void Xil_EnableMMU(void)
{
	u32 Reg;
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();

	Reg = mfcp(XREG_CP15_SYS_CONTROL);
	Reg |= (u32)0x05U;
	mtcp(XREG_CP15_SYS_CONTROL, Reg);

	dsb();
	isb();
}

/*****************************************************************************/
/**
* @brief	Disable MMU for Cortex A53 processors in 32bit mode. This function
*			invalidates the TLBs, Branch Predictor Array and flushed the data
*			cache before disabling the MMU.
*
* @param	None.
*
* @return	None.
*
* @note		When the MMU is disabled, all the memory accesses are treated as
*			strongly ordered.
******************************************************************************/
void Xil_DisableMMU(void)
{
	u32 Reg;

	mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);
	Xil_DCacheFlush();
	Reg = mfcp(XREG_CP15_SYS_CONTROL);
	Reg &= (u32)(~0x05U);
	mtcp(XREG_CP15_SYS_CONTROL, Reg);
}

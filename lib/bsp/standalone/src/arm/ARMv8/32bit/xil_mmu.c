/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 9.1   bl   10/11/23 Add API Xil_MemMap
* 9.2   ml   17/01/24 Modified description and code for Xil_MemMap API
*                     to fix doxygen warnings.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xpseudo_asm.h"
#include "xil_types.h"
#include "xil_mmu.h"

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef __GNUC__
#define u32overflow(a, b) ({typeof(a) s; __builtin_uadd_overflow(a, b, &s); })
#else
#define u32overflow(a, b) ((a) > ((a) + (b))) /**< u32 overflow is defined for
                                               *   readability and __GNUC__ */
#endif /* __GNUC__ */

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
	if (ptr != NULL) {
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
* @brief    Memory mapping for ARMv8 based processors. If successful, the
*	    mapped region will include all of the memory requested, but
*	    may include more. Specifically, it will be a power of 2 in
*           size, aligned on a boundary of that size.
*
* @param       PhysAddr is base physical address at which to start mapping.
*                   NULL in Physaddr masks possible mapping errors.
* @param       size of region to be mapped.
* @param       flags used to set translation table.
*
* @return      Physaddr on success, NULL on error. Ambiguous if Physaddr==NULL
*
* @cond Xil_MemMap_internal
* @note:    u32overflow() is defined for readability and (for __GNUC__) to
*           - force the type of the check to be the same as the first argument
*           - hide the otherwise unused third argument of the builtin
*           - improve safety by choosing the explicit _uadd_ version.
*           Consider __builtin_add_overflow_p() when available.
*           Use an alternative (less optimal?) for compilers w/o the builtin.
* @endcond
******************************************************************************/
void *Xil_MemMap(UINTPTR PhysAddr, size_t size, u32 flags)
{
	UINTPTR section_offset;
	UINTPTR ttb_addr;
	UINTPTR ttb_size = 1024UL * 1024UL;

	if (flags == 0U) {
		return (void *)PhysAddr;
	}
	if (u32overflow(PhysAddr, size)) {
		return NULL;
	}

	/* Ensure alignment on a section boundary */
	ttb_addr = PhysAddr & ~(ttb_size - 1UL);

	/*
	 * Loop through entire region of memory (one MMU section at a time).
	 * Each section requires a TTB entry.
	 */
	for (section_offset = 0; section_offset < size; ) {
		/* Calculate translation table entry for this memory section */
		ttb_addr += section_offset;

		/* Write translation table entry value to entry address */
		Xil_SetTlbAttributes(ttb_addr, flags);

		section_offset += ttb_size;
	}

	return PhysAddr;
}

/*****************************************************************************/
/**
* @brief	Enable MMU for Cortex-A53 processor in 32bit mode. This function
*			invalidates the instruction and data caches before enabling MMU.
*
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

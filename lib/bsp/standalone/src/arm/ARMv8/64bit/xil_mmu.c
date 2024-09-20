/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mmu.c
*
* This file provides APIs for enabling/disabling MMU and setting the memory
* attributes for sections, in the MMU translation table.
* MMU APIs are yet to be implemented. They are left blank to avoid any
* compilation error
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00 	pkp  05/29/14 First release
* 6.02  pkp  01/22/17 Added support for EL1 non-secure
* 9.00  ml   03/03 23 Add description to fix doxygen warnings.
* 9.01  bl   10/11/23 Add API Xil_MemMap
* 9.2   ml   01/17/24 Modified description and code for Xil_MemMap API
*                     to fix doxygen warnings.
* 9.2   ml   09/19/24 Corrected return type to fix compilation warning by
*                     typecasting from 'UINTPTR'(long unsigned int) to 'void *'
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
#include "bspconfig.h"
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

#define BLOCK_SIZE_2MB 0x200000U /**< block size is 2MB */
#define BLOCK_SIZE_1GB 0x40000000U /**< block size is 1GB */
#define ADDRESS_LIMIT_4GB 0x100000000UL /**< Address limit is 4GB */
#define BLOCK_SIZE_1TB 0x10000000000UL /**< block size 1TB */

#if defined(PLATFORM_ZYNQMP) || defined (VERSAL_NET)
#define BLOCK_SIZE_2MB_RANGE	4UL
#else
#define BLOCK_SIZE_2MB_RANGE	5UL
#endif

#if defined(VERSAL_NET)
#if defined (ENABLE_MINIMAL_XLAT_TBL)
#define HIGH_ADDR	(BLOCK_SIZE_1TB * 4) /* 4 TB */
#else
#define HIGH_ADDR 	(BLOCK_SIZE_1TB * 256)/* 256 TB */
#endif
#elif defined (versal)
#define HIGH_ADDR	(BLOCK_SIZE_1TB * 16)
#elif defined (PLATFORM_ZYNQMP)
#define HIGH_ADDR	BLOCK_SIZE_1TB
#endif

/************************** Variable Definitions *****************************/

extern INTPTR MMUTableL1;
extern INTPTR MMUTableL2;

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
* @brief	It sets the memory attributes for a section, in the translation
* 			table. If the address (defined by Addr) is less than 4GB, the
*			memory attribute(attrib) is set for a section of 2MB memory. If the
*			address (defined by Addr) is greater than 4GB, the memory attribute
*			(attrib) is set for a section of 1GB memory.
*
* @param	Addr: 64-bit address for which attributes are to be set.
* @param	attrib: Attribute for the specified memory region. xil_mmu.h
*			contains commonly used memory attributes definitions which can be
*			utilized for this function.
*
* @return	None.
*
* @note		The MMU and D-cache need not be disabled before changing an
*			translation table attribute.
*
******************************************************************************/
void Xil_SetTlbAttributes(UINTPTR Addr, u64 attrib)
{
	INTPTR *ptr;
	INTPTR section;
	u64 block_size;
	/* if region is less than 4GB MMUTable level 2 need to be modified */
	if (Addr < ADDRESS_LIMIT_4GB) {
		/* block size is 2MB for addressed < 4GB*/
		block_size = BLOCK_SIZE_2MB;
		section = Addr / block_size;
		ptr = &MMUTableL2 + section;
	}
	/* if region is greater than 4GB MMUTable level 1 need to be modified */
	else {
		/* block size is 1GB for addressed > 4GB */
		block_size = BLOCK_SIZE_1GB;
		section = Addr / block_size;
		ptr = &MMUTableL1 + section;
	}
	*ptr = (Addr & (~(block_size - 1))) | attrib;

	Xil_DCacheFlush();

	if (EL3 == 1) {
		mtcptlbi(ALLE3);
	} else if (EL1_NONSECURE == 1) {
		mtcptlbi(VMALLE1);
	}

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
******************************************************************************/
void *Xil_MemMap(UINTPTR PhysAddr, size_t size, u32 flags)
{
	UINTPTR section_offset;
	UINTPTR ttb_addr;
	UINTPTR ttb_size = (PhysAddr < BLOCK_SIZE_2MB_RANGE * BLOCK_SIZE_1GB)
			   ? BLOCK_SIZE_2MB : BLOCK_SIZE_1GB;

	if (PhysAddr >= HIGH_ADDR || size >= HIGH_ADDR ||
	    PhysAddr + size >= HIGH_ADDR) {
		return NULL;
	}
	if (flags == 0U) {
		return (void *)PhysAddr;
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

		/*
		 * recalculate if we started below 4GB and going above in
		 * 64bit mode
		 */
		if (ttb_addr >= BLOCK_SIZE_2MB_RANGE * BLOCK_SIZE_1GB) {
			ttb_size = BLOCK_SIZE_1GB;
		}
		section_offset += ttb_size;
	}

	return (void *)PhysAddr;
}

/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mpu.c
*
* This file provides APIs for enabling/disabling MPU and setting the memory
* attributes for sections, in the MPU translation table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00  pkp  02/10/14 Initial version
* 6.2   mus  01/27/17 Updated to support IAR compiler
* 6.4   asa  08/16/17 Added many APIs for MPU access to make MPU usage
* 					  user-friendly. The APIs added are: Xil_UpdateMPUConfig,
* 					  Xil_GetMPUConfig, Xil_GetNumOfFreeRegions,
* 					  Xil_GetNextMPURegion, Xil_DisableMPURegionByRegNum,
* 					  Xil_GetMPUFreeRegMask, Xil_SetMPURegionByRegNum, and
* 					  Xil_InitializeExistingMPURegConfig.
* 					  Added a new array of structure of type XMpuConfig to
* 					  represent the MPU configuration table.
* 6.8  aru  07/02/18 Returned the pointer instead of address
*			of that pointer in Xil_MemMap().
* 7.5  asa  03/01/21  Ensure that Mpu_Config does not stay in .boot/.vector
*                     sections which generally should be executable code
*                     which can be allocated and not written.
*                     Mpu_Config array is populated during boot time, hence
*                     cannot be placed in .bss or .data section. Putting
*                     Mpu_Config in a new .bootdata section.
* 7.7  sk   01/10/22  Update int to u32 to fix misrac misra_c_2012_directive_4_6
* 		      violations.
* 7.7  sk   01/10/22  Typecast variables from signed to unsigned to fix
* 		      misra_c_2012_rule_10_4 violation.
* 7.7  sk   01/10/22  Add explicit parentheses for region_size and region_size[0]
* 		      to fix misra_c_2012_rule_12_1 violation.
* 7.7  sk   01/10/22  Remove unsigned sign to fix misra_c_2012_rule_10_3 violation.
* 7.7  sk   01/10/22  Modify if condition to fix misra_c_2012_rule_10_1 violation.
* 7.7  sk   01/10/22  Typecast to fix wider essential type misra_c_2012_rule_10_7
* 		      violation.
* 7.7  sk   01/10/22  Update conditional expression to fix misra_c_2012_rule_14_4
* 		      violation.
* 7.7  sk   01/10/22  Add braces for the if statement to make it a compound
* 		      statement to fix misra_c_2012_rule_15_6 violation.
* 8.0  mus  05/09/22  Updated MPU related APIs to support CortexR52 processor.
* 8.1  asa  02/13/23  Updated xdbg_printf arguments to have the correct debug
*                     type.
* 9.0  ml   03/03/23  Add description to fix doxygen warnings.
* 9.0  ml   04/14/23  Add U to numericals and correct the typecast to fix
*                     misra-c violations.
* 9.0  mus  04/20/23  Remove CortexR52 specific changes and move to file to
*                     arm/cortexr5/platform/versal/ directory. There will be
*                     separate file for CortexR52.
* 9.0  ml   04/26/23  Updated code to fix overrun coverity warnings.
* 9.1  ml   11/16/23  Fix compilation errors reported with -std=c2x compiler flag
* </pre>
*
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xpseudo_asm.h"
#include "xil_types.h"
#include "xil_mpu.h"
#include "xdebug.h"
#include "xstatus.h"
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
#define MPU_REGION_SIZE_MIN 0x20 /**< MPU Region size minimum */
/************************** Variable Definitions *****************************/

static const struct {
	u64 size;
	u32 encoding;
} region_size[] = {
	{ 0x20U, REGION_32B },
	{ 0x40U, REGION_64B },
	{ 0x80U, REGION_128B },
	{ 0x100U, REGION_256B },
	{ 0x200U, REGION_512B },
	{ 0x400U, REGION_1K },
	{ 0x800U, REGION_2K },
	{ 0x1000U, REGION_4K },
	{ 0x2000U, REGION_8K },
	{ 0x4000U, REGION_16K },
	{ 0x8000U, REGION_32K },
	{ 0x10000U, REGION_64K },
	{ 0x20000U, REGION_128K },
	{ 0x40000U, REGION_256K },
	{ 0x80000U, REGION_512K },
	{ 0x100000U, REGION_1M },
	{ 0x200000U, REGION_2M },
	{ 0x400000U, REGION_4M },
	{ 0x800000U, REGION_8M },
	{ 0x1000000U, REGION_16M },
	{ 0x2000000U, REGION_32M },
	{ 0x4000000U, REGION_64M },
	{ 0x8000000U, REGION_128M },
	{ 0x10000000U, REGION_256M },
	{ 0x20000000U, REGION_512M },
	{ 0x40000000U, REGION_1G },
	{ 0x80000000U, REGION_2G },
	{ 0x100000000U, REGION_4G },
};

#if defined (__GNUC__)
XMpu_Config Mpu_Config __attribute__((section(".bootdata")));
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ ".bootdata"
XMpu_Config Mpu_Config;
#endif

/************************** Function Prototypes ******************************/
void Xil_InitializeExistingMPURegConfig(void);
/*****************************************************************************/
/**
* @brief    This function sets the memory attributes for a section covering
*           1MB, of memory in the translation table.
*
* @param	Addr : 32-bit address for which memory attributes need to be set.
* @param	attrib : Attribute for the given memory region.
* @return	None.
*
*
******************************************************************************/
void Xil_SetTlbAttributes(INTPTR Addr, u32 attrib)
{
	INTPTR Localaddr = Addr;

	Localaddr &= (UINTPTR)(~(0xFFFFFU));
	/* Setting the MPU region with given attribute with 1MB size */
	(void) Xil_SetMPURegion(Localaddr, 0x100000, attrib);
}

/*****************************************************************************/
/**
* @brief    Set the memory attributes for a section of memory in the
*           translation table.
*
* @param	addr: 32-bit address for which memory attributes need to be set..
* @param	size: size is the size of the region.
* @param	attrib: Attribute for the given memory region.
* @return	None.
*
*
******************************************************************************/
u32 Xil_SetMPURegion(INTPTR addr, u64 size, u32 attrib)
{
	u32 Regionsize = 0;
	INTPTR Localaddr = addr;
	u32 NextAvailableMemRegion;
	u32 i;
	u32 Status;

	NextAvailableMemRegion = Xil_GetNextMPURegion();
	if (NextAvailableMemRegion == 0xFFU) {
		xdbg_printf(XDBG_DEBUG_ERROR, "No regions available\r\n");
		return XST_FAILURE;
	}

	Xil_DCacheFlush();
	Xil_ICacheInvalidate();

	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, NextAvailableMemRegion);
	isb();
	/* Lookup the size.  */
	for (i = 0; i < (sizeof (region_size) / sizeof (region_size[0])); i++) {
		if (size <= region_size[i].size) {
			Regionsize = region_size[i].encoding;
			break;
		}
	}
	if (i == (sizeof (region_size) / sizeof (region_size[0]))) {
		i -= 1U;
	}
	Localaddr &= (UINTPTR)(~(region_size[i].size - 1U));

	Regionsize <<= 1;
	Regionsize |= REGION_EN;

	dsb();
	mtcp(XREG_CP15_MPU_REG_BASEADDR, Localaddr);	/* Set base address of a region */
	mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL, attrib);	/* Set the control attribute */
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Regionsize);	/* set the region size and enable it*/
	dsb();
	isb();
	Status = Xil_UpdateMPUConfig(NextAvailableMemRegion, Localaddr, Regionsize, attrib);
	xdbg_printf(XDBG_DEBUG_GENERAL, " Xil_UpdateMPUConfig : %s", (Status == XST_SUCCESS) ? "PASS" : "FAIL");
	return Status;
}
/*****************************************************************************/
/**
* @brief    Enable MPU for Cortex R5 processor. This function invalidates I
*           cache and flush the D Caches, and then enables the MPU.
*
* @return	None.
*
******************************************************************************/
void Xil_EnableMPU(void)
{
	u32 CtrlReg, Reg;
	s32 DCacheStatus = 0, ICacheStatus = 0;
	/* enable caches only if they are disabled */
#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif
	if ((CtrlReg & XREG_CP15_CONTROL_C_BIT) != 0x00000000U) {
		DCacheStatus = 1;
	}
	if ((CtrlReg & XREG_CP15_CONTROL_I_BIT) != 0x00000000U) {
		ICacheStatus = 1;
	}

	if (DCacheStatus != 0) {
		Xil_DCacheDisable();
	}
	if (ICacheStatus != 0) {
		Xil_ICacheDisable();
	}
#if defined (__GNUC__)
	Reg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, Reg);
#endif
	Reg |= 0x00000001U;
	dsb();
	mtcp(XREG_CP15_SYS_CONTROL, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if (DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if (ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}

/*****************************************************************************/
/**
* @brief    Disable MPU for Cortex R5 processors. This function invalidates I
*           cache and flush the D Caches, and then disabes the MPU.
*
* @return	None.
*
******************************************************************************/
void Xil_DisableMPU(void)
{
	u32 CtrlReg, Reg;
	s32 DCacheStatus = 0, ICacheStatus = 0;
	/* enable caches only if they are disabled */

#if defined (__GNUC__)
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, CtrlReg);
#endif
	if ((CtrlReg & XREG_CP15_CONTROL_C_BIT) != 0x00000000U) {
		DCacheStatus = 1;
	}
	if ((CtrlReg & XREG_CP15_CONTROL_I_BIT) != 0x00000000U) {
		ICacheStatus = 1;
	}

	if (DCacheStatus != 0) {
		Xil_DCacheDisable();
	}
	if (ICacheStatus != 0) {
		Xil_ICacheDisable();
	}

	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0);
#if defined (__GNUC__)
	Reg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, Reg);
#endif
	Reg &= ~(0x00000001U);
	dsb();
	mtcp(XREG_CP15_SYS_CONTROL, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if (DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if (ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}

/*****************************************************************************/
/**
* @brief    Update the MPU configuration for the requested region number in
* 			the global MPU configuration table.
*
* @param	reg_num: The requested region number to be updated information for.
* @param	address: 32 bit address for start of the region.
* @param	size: Requested size of the region.
* @param	attrib: Attribute for the corresponding region.
* @return	XST_FAILURE: When the requested region number if 16 or more.
* 			XST_SUCCESS: When the MPU configuration table is updated.
*
*
******************************************************************************/
u32 Xil_UpdateMPUConfig(u32 reg_num, INTPTR address, u32 size, u32 attrib)
{
	u32 ReturnVal = XST_SUCCESS;
	u32 Tempsize = size;
	u32 Index;

	if (reg_num >=  MAX_POSSIBLE_MPU_REGS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid region number\r\n");
		ReturnVal = XST_FAILURE;
		goto exit;
	}

	if ((size & REGION_EN) != 0U) {
		Mpu_Config[reg_num].RegionStatus = MPU_REG_ENABLED;
		Mpu_Config[reg_num].BaseAddress = address;
		Tempsize &= (~REGION_EN);
		Tempsize >>= 1;
		/* Lookup the size.  */
		for (Index = 0; Index <
		     (sizeof (region_size) / sizeof (region_size[0])); Index++) {
			if (Tempsize <= region_size[Index].encoding) {
				Mpu_Config[reg_num].Size = region_size[Index].size;
				break;
			}
		}
		Mpu_Config[reg_num].Attribute = attrib;
	} else {
		Mpu_Config[reg_num].RegionStatus = 0U;
		Mpu_Config[reg_num].BaseAddress = 0;
		Mpu_Config[reg_num].Size = 0U;
		Mpu_Config[reg_num].Attribute = 0U;
	}

exit:
	return ReturnVal;
}

/*****************************************************************************/
/**
* @brief    The MPU configuration table is passed to the caller.
*
* @param	mpuconfig: This is of type XMpu_Config which is an array of
* 			16 entries of type structure representing the MPU config table
* @return	none
*
*
******************************************************************************/
void Xil_GetMPUConfig (XMpu_Config mpuconfig)
{
	u32 Index = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		mpuconfig[Index].RegionStatus = Mpu_Config[Index].RegionStatus;
		mpuconfig[Index].BaseAddress = Mpu_Config[Index].BaseAddress;
		mpuconfig[Index].Attribute = Mpu_Config[Index].Attribute;
		mpuconfig[Index].Size = Mpu_Config[Index].Size;
		Index++;
	}
}

/*****************************************************************************/
/**
* @brief    Returns the total number of free MPU regions available.
*
* @return	Number of free regions available to users
*
*
******************************************************************************/
u32 Xil_GetNumOfFreeRegions (void)
{
	u32 Index = 0U;
	u32 NumofFreeRegs = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (MPU_REG_DISABLED == Mpu_Config[Index].RegionStatus) {
			NumofFreeRegs++;
		}
		Index++;
	}
	return NumofFreeRegs;
}

/*****************************************************************************/
/**
* @brief    Returns the total number of free MPU regions available in the form
*           of a mask. A bit of 1 in the returned 16 bit value represents the
*           corresponding region number to be available.
*           For example, if this function returns 0xC0000, this would mean, the
*           regions 14 and 15 are available to users.
*
* @return	The free region mask as a 16 bit value
*
*
******************************************************************************/
u16 Xil_GetMPUFreeRegMask (void)
{
	u32 Index = 0U;
	u16 FreeRegMask = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (MPU_REG_DISABLED == Mpu_Config[Index].RegionStatus) {
			FreeRegMask |= ((u16)1U << Index);
		}
		Index++;
	}
	return FreeRegMask;
}

/*****************************************************************************/
/**
* @brief    Disables the corresponding region number as passed by the user.
*
* @param	reg_num: The region number to be disabled
* @return	XST_SUCCESS: If the region could be disabled successfully
* 			XST_FAILURE: If the requested region number is 16 or more.
*
*
******************************************************************************/
u32 Xil_DisableMPURegionByRegNum (u32 reg_num)
{
	u32 Temp = 0U;
	u32 ReturnVal = XST_FAILURE;
	u32 Status;

	if (reg_num >= 16U) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid region number\r\n");
		goto exit1;
	}
	Xil_DCacheFlush();
	Xil_ICacheInvalidate();

	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, reg_num);
#if defined (__GNUC__)
	Temp = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_MPU_REG_SIZE_EN, Temp);
#endif
	Temp &= (~REGION_EN);
	dsb();
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Temp);
	dsb();
	isb();
	Status = Xil_UpdateMPUConfig(reg_num, 0, 0U, 0U);
	xdbg_printf(XDBG_DEBUG_GENERAL, " Xil_UpdateMPUConfig : %s", (Status == XST_SUCCESS) ? "PASS" : "FAIL");
	ReturnVal = Status;

exit1:
	return ReturnVal;
}

/*****************************************************************************/
/**
* @brief    Enables the corresponding region number as passed by the user.
*
* @param	reg_num: The region number to be enabled
* @param	addr: 32 bit address for start of the region.
* @param	size: Requested size of the region.
* @param	attrib: Attribute for the corresponding region.
* @return	XST_SUCCESS: If the region could be created successfully
* 			XST_FAILURE: If the requested region number is 16 or more.
*
*
******************************************************************************/
u32 Xil_SetMPURegionByRegNum (u32 reg_num, INTPTR addr, u64 size, u32 attrib)
{
	u32 ReturnVal = XST_SUCCESS;
	INTPTR Localaddr = addr;
	u32 Regionsize = 0;
	u32 Index;
	u32 Status;

	if (reg_num >= 16U) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid region number\r\n");
		ReturnVal = XST_FAILURE;
		goto exit2;
	}

	if (Mpu_Config[reg_num].RegionStatus == MPU_REG_ENABLED) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Region already enabled\r\n");
		ReturnVal = XST_FAILURE;
		goto exit2;
	}

	Xil_DCacheFlush();
	Xil_ICacheInvalidate();
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, reg_num);
	isb();

	/* Lookup the size.  */
	for (Index = 0; Index <
	     (sizeof (region_size) / sizeof (region_size[0])); Index++) {
		if (size <= region_size[Index].size) {
			Regionsize = region_size[Index].encoding;
			break;
		}
	}

	if (Index == (sizeof (region_size) / sizeof (region_size[0]))) {
		Index -= 1U;
	}
	Localaddr &= (UINTPTR)(~(region_size[Index].size - 1U));
	Regionsize <<= 1;
	Regionsize |= REGION_EN;

	dsb();
	mtcp(XREG_CP15_MPU_REG_BASEADDR, Localaddr);
	mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL, attrib);
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Regionsize);
	dsb();
	isb();
	Status = Xil_UpdateMPUConfig(reg_num, Localaddr, Regionsize, attrib);
	xdbg_printf(XDBG_DEBUG_GENERAL, " Xil_UpdateMPUConfig : %s", (Status == XST_SUCCESS) ? "PASS" : "FAIL");
	ReturnVal = Status;
exit2:
	return ReturnVal;

}

/*****************************************************************************/
/**
* @brief    Initializes the MPU configuration table that are setup in the
* 			R5 boot code in the Init_Mpu function called before C main.
*
* @return	none
*
*
******************************************************************************/
void Xil_InitializeExistingMPURegConfig(void)
{
	u32 Index = 0U;
	u32 Index1 = 0U;
	u32 MPURegSize;
	UINTPTR MPURegBA;
	u32 MPURegAttrib;
	u32 Tempsize;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, Index);
#if defined (__GNUC__)
		MPURegSize = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
		MPURegBA = mfcp(XREG_CP15_MPU_REG_BASEADDR);
		MPURegAttrib = mfcp(XREG_CP15_MPU_REG_ACCESS_CTRL);
#elif defined (__ICCARM__)
		mfcp(XREG_CP15_MPU_REG_SIZE_EN, MPURegSize);
		mfcp(XREG_CP15_MPU_REG_BASEADDR, MPURegBA);
		mfcp(XREG_CP15_MPU_REG_ACCESS_CTRL, MPURegAttrib);
#endif
		if ((MPURegSize & REGION_EN) != 0U) {
			Mpu_Config[Index].RegionStatus = MPU_REG_ENABLED;
			Mpu_Config[Index].BaseAddress = MPURegBA;
			Mpu_Config[Index].Attribute = MPURegAttrib;
			Tempsize = MPURegSize & (~REGION_EN);
			Tempsize >>= 1;
			for (Index1 = 0; Index1 <
			     (sizeof (region_size) / sizeof (region_size[0])); Index1++) {
				if (Tempsize <= region_size[Index1].encoding) {
					Mpu_Config[Index].Size = region_size[Index1].size;
					break;
				}
			}
		}
		Index++;
	}
}

/*****************************************************************************/
/**
* @brief    Returns the next available free MPU region
*
* @return	The free MPU region available
*
*
******************************************************************************/
u32 Xil_GetNextMPURegion(void)
{
	u32 Index = 0U;
	u32 NextAvailableReg = 0xFF;
	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (Mpu_Config[Index].RegionStatus != MPU_REG_ENABLED) {
			NextAvailableReg = Index;
			break;
		}
		Index++;
	}
	return NextAvailableReg;
}

#ifdef __GNUC__
#define u32overflow(a, b) ({__typeof__(a) s; __builtin_uadd_overflow(a, b, &s); })
#else
#define u32overflow(a, b) ((a) > ((a) + (b))) /**< u32 overflow is defined for
                                               *   readability and __GNUC__ */
#endif /* __GNUC__ */

/*****************************************************************************/
/**
* @brief    Memory mapping for Cortex-R5F. If successful, the mapped
*           region will include all of the memory requested, but may
*           include more. Specifically, it will be a power of 2 in
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
	size_t Regionsize = MPU_REGION_SIZE_MIN;
	UINTPTR Basephysaddr = 0, end = PhysAddr + size;

	if (flags == 0U) {
		return (void *)PhysAddr;
	}
	if (u32overflow(PhysAddr, size)) {
		return NULL;
	}
	for ( ; Regionsize != 0U; Regionsize <<= 1) {
		if (Regionsize >= size) {
			Basephysaddr = PhysAddr & ~(Regionsize - 1U);
			if (u32overflow(Basephysaddr, Regionsize)) {
				break;
			}
			if ((Basephysaddr + Regionsize) >= end) {
				return ((Xil_SetMPURegion((INTPTR) Basephysaddr,
							  Regionsize, flags) == XST_SUCCESS) ?
					(void *)PhysAddr : NULL);
			}
		}
	}
	return NULL;
}

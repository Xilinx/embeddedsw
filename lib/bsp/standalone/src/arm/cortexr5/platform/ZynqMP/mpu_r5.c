/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file mpu.c
*
* This file contains initial configuration of the MPU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00 	pkp  02/20/14 First release
* 5.04	pkp  12/18/15 Updated MPU initialization as per the proper address map
* 6.00  pkp  06/27/16 moving the Init_MPU code to .boot section since it is a
*                     part of processor boot process
* 6.2   mus  01/27/17 Updated to support IAR compiler
* 7.1   mus  09/11/19 Added warning message if DDR size is not in power of 2.
*                     Fix for CR#1038577.
* 7.2   asa  04/08/20 Fix warning in the function Init_MPU.
* 7.5   mus  01/13/21 Removed redundant declaration for Mpu_Config,
*                     declaration is present in xil_mpu.h.
* 7.5   asa  03/07/21 Ensure that Update_MpuConfig_Array stays in .boot section
*                     as it is used only during bootup.
*                     Add function header to Init_MPU function.
* 7.7   mus  11/17/21 Existing Init_MPU logic assumes that DDR would be always
*                     mapped at 0x0 location. This logic would not work
*                     in typical isolation use cases where actual DDR size
*                     could be 2 GB (0x0 - 0x7FFF_FFFF), but part of DDR
*                     that is mapped to DDR could start from
*                     1 GB (0x4000_0000). Updated Init_MPU function to fix
*                     said use case. It fixes CR#1113181.
* 7.7   mus  11/26/21 Updated Init_MPU to remove xil_printf, and set up the
*                     variables required for printing warning. New function
*                     Print_DDRSize_Warning has been introduced to print warnings.
*                     It will be called from boot code after MPU enablement to
*                     ensure the correct behavior. It fixes CR#1116431.
* 7.7   mus  02/23/22 Updated Print_DDRSize_Warning function to use xdbg_printf
*                     instead of xil_printf, so that warning would be printed
*                     only when DEBUG is enabled. It fixes CR#1123028.
* 7.7   dp   03/08/22 Update Init_MPU and Print_DDRSize_Warning to use latest
*                     DDR macros that will contain lowest DDR base address and
*                     highest DDR address. It fixes CR#1118988
* 8.0   dp   03/31/22 Fix compilation warning in Print_DDRSize_Warning().
*                     Fixes CR#1124773.
* 9.0   ml   03/03/23 Add description and remove comments to fix doxygen warnings.
* 9.0   ml   09/14/23 Updated Regnum argument data type from s32 to u32 in Xil_setattribute
*                     API prototype to fix MISRA-C violations for Rule 10.3.
* 9.0   ml   09/13/23 Replaced numerical types (int) with proper typedefs(s32) to
*                     fix MISRA-C violations for Rule 4.6
* 9.0   ml   09/13/23 Assigned proper suffix to integer constants to fix MISRA-C
*                     violations for Rule 7.2 and 10.4
* 9.0   ml   09/13/23 Added parenthensis on sub-expression to fix MISRA-C
*                     violations for Rule 12.1
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_printf.h"
#include "xreg_cortexr5.h"
#include "xil_mpu.h"
#include "xpseudo_asm.h"
#include "bspconfig.h"
#include "xdebug.h"
#ifdef SDT
#include "xmem_config.h"
#else
#include "xparameters.h"
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
#if defined(SDT) && defined(XPAR_PSU_DDR_0_BASEADDRESS)
#define XPAR_PSU_R5_DDR_0_LOW_ADDR XPAR_PSU_DDR_0_BASEADDRESS
#define XPAR_PSU_R5_DDR_0_HIGH_ADDR XPAR_PSU_DDR_0_HIGHADDRESS
#endif

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

#ifdef	XPAR_PSU_R5_DDR_0_LOW_ADDR
#if defined (__GNUC__)
static u32 DDRSizeWarning  __attribute__((section(".bootdata")));
static u32 DDRSizeIndex __attribute__((section(".bootdata")));
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ ".bootdata"
static u32 DDRSizeWarning;
static u32 DDRSizeIndex;
#endif
#endif

/************************** Function Prototypes ******************************/
#if defined (__GNUC__)
void Init_MPU(void) __attribute__((__section__(".boot")));
void Print_DDRSize_Warning(void) __attribute__((__section__(".boot")));
static void Xil_SetAttribute(u32 addr, u32 reg_size, u32 reg_num, u32 attrib) __attribute__((__section__(".boot")));
static void Xil_DisableMPURegions(void) __attribute__((__section__(".boot")));
static inline void Update_MpuConfig_Array(u32 Addr, u32 RegSize, u32 RegNum,
		u32 Attrib) __attribute__((__section__(".boot")));
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ ".boot"
void Init_MPU(void);
void Print_DDRSize_Warning(void);
static void Xil_SetAttribute(u32 addr, u32 reg_size, u32 reg_num, u32 attrib);
static void Xil_DisableMPURegions(void);
#endif

/*****************************************************************************/
/**
*
* Initialize MPU for a given address map and Enabled the background Region in
* MPU with default memory attributes for rest of address range for Cortex R5
* processor.
*
* @return	None.
*
*
******************************************************************************/
static inline void Update_MpuConfig_Array(u32 Addr, u32 RegSize, u32 RegNum,
		u32 Attrib)
{
	Mpu_Config[RegNum].RegionStatus = MPU_REG_ENABLED;
	Mpu_Config[RegNum].BaseAddress = Addr;
	Mpu_Config[RegNum].Size = RegSize;
	Mpu_Config[RegNum].Attribute = Attrib;
}

/*****************************************************************************/
/**
*
* Initialize MPU for during bootup with predefined region attributes.
*
* @return	None.
*
*
******************************************************************************/
void Init_MPU(void)
{
	u32 Addr;
	u32 RegSize = 0U;
	u32 Attrib;
	u32 RegNum = 0, i, Offset = 0;
	u64 size;
	u32 CreateTCMRegion = 0;

	Xil_DisableMPURegions();

	Addr = 0x00000000U;
#ifdef	XPAR_PSU_R5_DDR_0_LOW_ADDR
	/* If the DDR is present, configure region as per DDR size */
	size = (XPAR_PSU_R5_DDR_0_HIGH_ADDR - XPAR_PSU_R5_DDR_0_LOW_ADDR) + 1;
	if (size < 0x80000000) {
		/* Lookup the size.  */
		for (i = 0; i < (sizeof (region_size) / sizeof (region_size[0])); i++) {
			if (size <= region_size[i].size) {
				RegSize = region_size[i].encoding;

				/*
				 * Check if DDR size is in power of 2
				 * and it is mapped at 0x0 in HW design
				 */
				if ( XPAR_PSU_R5_DDR_0_LOW_ADDR != 0x100000) {
					CreateTCMRegion = 1;
					Addr = XPAR_PSU_R5_DDR_0_LOW_ADDR;
					Offset = 0;
				} else {
					Offset = XPAR_PSU_R5_DDR_0_LOW_ADDR;
				}

				if (region_size[i].size > (size + Offset + 1U)) {
					/*
					 * Set variables needed to print warning on console.
					 * xil_printf would be called to print the warning after
					 * MPU enablement from boot code. As application could
					 * be running from OCM, printing after MPU enablement
					 * ensures correct behavior
					 *
					 */
					DDRSizeWarning = 1;
					DDRSizeIndex = i;
				}
				break;
			}
		}
	} else {
		/* if the DDR size is > 2GB, truncate it to 2GB */
		RegSize = REGION_2G;
	}
#else
	/* For DDRless system, configure region for TCM */
	RegSize = REGION_256K;
#endif
	if (1U == CreateTCMRegion) {
		/*
		 * DDR is not mapped at 0x0 in HW design, so
		 * creating separate MPU region, as TCM and mapped
		 * DDR are not continuous.
		 * Note: Creating MPU region for 256 KB of TCM starting
		 * from 0x0. For split mode 128 KB TCM would be
		 * available to each R5 core (64 KB at 0x0 and 64 KB
		 * at 0x20000. We are not fine tuning the MPU region
		 * as per actual TCM size to avoid consuming extra MPU
		 * regions.
		 */
		Attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW;
		Xil_SetAttribute(0x0, REGION_256K, RegNum, Attrib);
		Update_MpuConfig_Array(0x0, REGION_256K, RegNum, Attrib);
		RegNum++;
	}

	Attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/*
	 * 1G of strongly ordered memory from 0x80000000 to 0xBFFFFFFF for PL.
	 * 512 MB - LPD-PL interface
	 * 256 MB - FPD-PL (HPM0) interface
	 * 256 MB - FPD-PL (HPM1) interface
	 */
	Addr = 0x80000000;
	RegSize = REGION_1G;
	Attrib = STRONG_ORDERD_SHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 512M of device memory from 0xC0000000 to 0xDFFFFFFF for QSPI */
	Addr = 0xC0000000U;
	RegSize = REGION_512M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 256M of device memory from 0xE0000000 to 0xEFFFFFFF for PCIe Low */
	Addr = 0xE0000000U;
	RegSize = REGION_256M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xF8000000 to 0xF8FFFFFF for STM_CORESIGHT */
	Addr = 0xF8000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 1M of device memory from 0xF9000000 to 0xF90FFFFF for RPU_A53_GIC */
	Addr = 0xF9000000U;
	RegSize = REGION_1M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xFD000000 to 0xFDFFFFFF for FPS slaves */
	Addr = 0xFD000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xFE000000 to 0xFEFFFFFF for Upper LPS slaves */
	Addr = 0xFE000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/*
	 * 16M of device memory from 0xFF000000 to 0xFFFFFFFF for Lower LPS slaves,
	 * CSU, PMU, TCM, OCM
	 */
	Addr = 0xFF000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);
	RegNum++;

	/* 256K of OCM RAM from 0xFFFC0000 to 0xFFFFFFFF marked as normal memory */
	Addr = 0xFFFC0000U;
	RegSize = REGION_256K;
	Attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW  ;
	Xil_SetAttribute(Addr, RegSize, RegNum, Attrib);
	Update_MpuConfig_Array(Addr, RegSize, RegNum, Attrib);

	/* A total of 10 MPU regions are allocated with another 6 being free for users */
}

/*****************************************************************************/
/**
*
* Print warning on console if DDR size mapped to given CortexR5 core is not in
* power of 2.
*
* @return       None.
*
*
******************************************************************************/
void Print_DDRSize_Warning(void)
{
#ifdef XPAR_PSU_R5_DDR_0_LOW_ADDR
	if (1U == DDRSizeWarning) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "WARNING: DDR size mapped to Cortexr5 processor is not \
		in power of 2. As processor allocates MPU regions size \
            in power of 2, address range %llx to %x has been \
            incorrectly mapped as normal memory \n", \
			    region_size[DDRSizeIndex].size - 1, ((u32)XPAR_PSU_R5_DDR_0_HIGH_ADDR + 1));
	}
#endif

}

/*****************************************************************************/
/**
*
* Set the memory attributes for a section of memory with starting address addr
* of the region size defined by reg_size having attributes attrib of region number
* reg_num
*
* @param	addr is the address for which attributes are to be set.
* @param	attrib specifies the attributes for that memory region.
* @param	reg_size specifies the size for that memory region.
* @param	reg_num specifies the number for that memory region.
* @return	None.
*
*
******************************************************************************/
static void Xil_SetAttribute(u32 addr, u32 reg_size, u32 reg_num, u32 attrib)
{
	u32 Local_reg_size = reg_size;

	Local_reg_size = Local_reg_size << 1U;
	Local_reg_size |= REGION_EN;
	dsb();
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, reg_num);
	isb();
	mtcp(XREG_CP15_MPU_REG_BASEADDR, addr); 		/* Set base address of a region */
	mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL, attrib); 	/* Set the control attribute */
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Local_reg_size);	/* set the region size and enable it*/
	dsb();
	isb();						/* synchronize context on this processor */
}

/*****************************************************************************/
/**
*
* Disable all the MPU regions if any of them is enabled
*
* @return	None.
*
*
******************************************************************************/
static void Xil_DisableMPURegions(void)
{
	u32 Temp = 0U;
	u32 Index = 0U;
	for (Index = 0; Index <= 15U; Index++) {
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, Index);
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
	}

}

#if defined (__ICCARM__)
#pragma default_function_attributes =
#endif

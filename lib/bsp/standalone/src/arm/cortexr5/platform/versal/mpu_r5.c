/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 7.00 	mus  02/20/14 First release
* 7.00  mus  03/16/19 Updated MPU region to mark DDR regions as
*                     memory, based on the DDR size in hdf
* 7.01  nis  09/02/19 Map AIE region if AIE instance is defined
* 7.3   asa  09/25/20 Make changes to update the global array
*                     Mpu_Config for the static regions created
*                     during boot up.
* 7.5   mus  01/13/21 Removed redundant declaration for Mpu_Config,
*                     declaration is present in xil_mpu.h.
* 7.5   asa  03/07/21 Ensure that Update_MpuConfig_Array stays in .boot section
*                     as it is used only during bootup.
*                     Add function header to Init_MPU function.
* 7.7	sk   01/10/22 Update unsigned int to u32 to fix misra_c_2012_directive_4_6
* 		      violation.
* 7.7	sk   01/10/22 Update values from signed to unsigned to fix misrac
* 		      misra_c_2012_rule_10_4 violation.
* 7.7	sk   01/10/22 Add explicit parentheses for region_size and region_size[0]
* 		      to fix misra_c_2012_rule_12_1 violation.
* 7.7	sk   01/10/22 Typecast variables to unsigned to fix misra_c_2012_rule_10_3
* 		      violation.
* 7.7	sk   01/10/22 Add unsigned to hexadecimal value to fix misra_c_2012_rule_7_2
* 		      violation.
* 8.0   mus  07/06/21 Added support for VERSAL NET
* 8.0   mus  22/12/22 Updated default VERSAL NET MPU table to configure DDR and OCM
*                     as cacheable memory.
* 9.0   ml   03/03/23 added description and removed comments to fix doxygen warnings.
* 9.0   adk  17/04/23 Added support for system device-tree flow.
* 9.0   adk  26/04/23 Updated the ifdef checks for XPAR_AXI_NOC_DDR_LOW_0_BASEADDR
* 		      properly.
* 9.0   mus  04/20/23  Removed CortexR52 specific changes, separate file is
*                      created for CortexR52.
* 9.2   ml   20/09/24 Add support for AXI NOC2 DDR region in MPU initialization
* 9.2   mus  01/10/24 Existing configuration of MPU region for DDR in Init_MPU is incorrect.
*                     In certain scenarios, where DDR is mapped at address other
*                     than 0, some portion at the end of DDR is not being inclued.
*                     For example, if 1 GB of DDR is mapped at address starting from
*                     0x10_0000, said API configures MPU region with start address
*                     0x0 and end address 0x3FFF_FFFF, so 1 MB of DDR region is not
*                     included. It always start MPU region from 0x0 to take care of
*                     TCM regions. Updated Init_MPU to configure DDR region based on
*                     DDR end address, so that it would always include whole DDR
*                     region into MPU region.
* 9.3   ml   12/20/24 Fixed GCC warnings
* 9.4   ml   07/24/25 Fixed compilation warnings
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xreg_cortexr5.h"
#include "xil_mpu.h"
#include "xpseudo_asm.h"
#include "bspconfig.h"

#ifndef SDT
#include "xparameters.h"
#else
#include "xmem_config.h"
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
#if defined(XPAR_AXI_NOC_DDR_LOW_0_BASEADDR) || defined(XPAR_AXI_NOC_0_BASEADDRESS) || defined(XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR)
static const struct {
	u64 size;
	u32 encoding;
}region_size[] = {
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
#endif
/************************** Function Prototypes ******************************/
#if defined (__GNUC__)
void Init_MPU(void) __attribute__((__section__(".boot")));
static void Xil_SetAttribute(u32 addr, u32 reg_size,u32 reg_num, u32 attrib) __attribute__((__section__(".boot")));
static void Xil_DisableMPURegions(void) __attribute__((__section__(".boot")));
static inline void Update_MpuConfig_Array(u32 Addr,u32 RegSize,u32 RegNum, u32 Attrib) __attribute__((__section__(".boot")));
#elif defined (__ICCARM__)
#pragma default_function_attributes = @ ".boot"
void Init_MPU(void);
static void Xil_SetAttribute(u32 addr, u32 reg_size,u32 reg_num, u32 attrib);
static void Xil_DisableMPURegions(void);
#endif

/*****************************************************************************/
/**
*
* Initialize MPU for a given address map and Enabled the background Region in
* MPU with default memory attributes for rest of address range for Cortex R5
* processor.
*
*
* @return	None.
*
*
******************************************************************************/
static inline void Update_MpuConfig_Array(u32 Addr,u32 RegSize,u32 RegNum,
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
	u32 RegNum = 0;

	Xil_DisableMPURegions();
	Addr = 0x00000000U;
#if defined(XPAR_AXI_NOC_DDR_LOW_0_BASEADDR) || defined(XPAR_AXI_NOC_0_BASEADDRESS) || defined(XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR)
	u32 i;
	u64 size;
#ifdef XPAR_AXI_NOC_DDR_LOW_0_BASEADDR
	/* If the DDR is present, configure region as per DDR size */
	size = (UINTPTR)XPAR_AXI_NOC_DDR_LOW_0_HIGHADDR + 1U;
#elif defined(XPAR_AXI_NOC_0_BASEADDRESS)
	size = XPAR_AXI_NOC_0_HIGHADDRESS + 1U;
#elif defined(XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR)
	size = XPAR_AXI_NOC2_DDR_LOW_0_HIGHADDR + 1U;
#endif

	if (size < 0x80000000) {
		/* Lookup the size.  */
		for (i = 0; i < (sizeof (region_size) / sizeof (region_size[0])); i++) {
			if (size <= region_size[i].size) {
				RegSize = region_size[i].encoding;
				break;
			}
		}
	} else {
#ifdef XPAR_AIE_NUM_INSTANCES
		/* If AIE is mapped, DDR space is reduced to 1GB */
		RegSize = REGION_1G;
#else
		/* if the DDR size is > 2GB, truncate it to 2GB */
		RegSize = REGION_2G;
#endif
	}
#else
	/* For DDRless system, configure region for TCM */
	RegSize = REGION_256K;
#endif

	Attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/*
	 * 1G of strongly ordered memory from 0x80000000 to 0xBFFFFFFF for PL.
	 *         256 MB - LPD-AFI
	 *          64 MB - VCU controller
	 *         192 MB - FPD-AFI-0
	 *         256 MB - FPD-AFI-1
	 *
	 *
	 */
	Addr = 0x80000000U;
	RegSize = REGION_1G;
	Attrib = STRONG_ORDERD_SHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 512M of device memory from 0xC0000000 to 0xDFFFFFFF for QSPI */
	Addr = 0xC0000000U;
	RegSize = REGION_512M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 256M of device memory from 0xE0000000 to 0xEFFFFFFF for PCIe Low */
	Addr = 0xE0000000U;
	RegSize = REGION_256M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 128M of device memory from 0xF0000000 to 0xF7FFFFFF for PMC */
	Addr = 0xF0000000U;
	RegSize = REGION_128M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xF8000000 to 0xF8FFFFFF for STM_CORESIGHT */
	Addr = 0xF8000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 1M of device memory from 0xF9000000 to 0xF90FFFFF for RPU_A53_GIC */
	Addr = 0xF9000000U;
	RegSize = REGION_1M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xFD000000 to 0xFDFFFFFF for FPS slaves */
	Addr = 0xFD000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/* 16M of device memory from 0xFE000000 to 0xFEFFFFFF for Upper LPS slaves */
	Addr = 0xFE000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/*
	 * 16M of device memory from 0xFF000000 to 0xFFFFFFFF for Lower LPS slaves,
	 * CSU, PMU, TCM, OCM
	 */
	Addr = 0xFF000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	/**
	 * 1G of remapped adddress space from 0x40000000 to 0x7FFFFFFF for AIE.
	 * The number of allocated MPU regions would be 12, 4 being free for
	 * the user.
	 * TODO: The value assigned to Addr must be parsed from XSA if the
	 * remap address is part of it (currently we are not sure if that's
	 * the case).
	 */
#ifdef XPAR_AIE_NUM_INSTANCES
	Addr = 0x40000000U;
	RegSize = REGION_1G;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW  ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
	RegNum++;
#endif

	/* 256K of OCM RAM from 0xFFFC0000 to 0xFFFFFFFF marked as normal memory */
	Addr = 0xFFFC0000U;
	RegSize = REGION_256K;
	Attrib = NORM_NSHARED_WB_WA| PRIV_RW_USER_RW  ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);

	/* A total of 11 MPU regions are allocated with another 5 being free for users */

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
static void Xil_SetAttribute(u32 addr, u32 reg_size,u32 reg_num, u32 attrib)
{
	u32 Local_reg_size = reg_size;

	Local_reg_size = Local_reg_size<<1U;

	Local_reg_size |= REGION_EN;
	dsb();
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);
	isb();
	mtcp(XREG_CP15_MPU_REG_BASEADDR,addr); 		    /* Set base address of a region */
	mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL,attrib); 	/* Set the control attribute */
	mtcp(XREG_CP15_MPU_REG_SIZE_EN,Local_reg_size);	/* set the region size and enable it*/
	dsb();
	isb();						/* synchronize context on this processor */
}

/*****************************************************************************/
/**
*
* Disable all the MPU regions if any of them is enabled
*
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
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,Index);
#if defined (__GNUC__)
		Temp = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
#elif defined (__ICCARM__)
		mfcp(XREG_CP15_MPU_REG_SIZE_EN,Temp);
#endif
		Temp &= (~REGION_EN);
		dsb();
		mtcp(XREG_CP15_MPU_REG_SIZE_EN,Temp);
		dsb();
		isb();
	}

}

#if defined (__ICCARM__)
#pragma default_function_attributes =
#endif

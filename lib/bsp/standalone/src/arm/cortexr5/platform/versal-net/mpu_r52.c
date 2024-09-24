/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file mpu.c
*
* This file contains initial configuration of the CortexR52  MPU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.00 	mus  04/10/23 First release
* 9.1 	mus  10/03/23 Added MPU region for HNIC region.
* 9.1   asa  03/04/24 Made changes to ensure that InitialMpu_Config is used
*                     in Init_MPU function to initialize the MPU regions
*                     in stead of using hard coded information inside
*                     Init_MPU.
* 9.2   mus  09/23/24 Added comments for MPU regions.
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
#include "xparameters.h"

/***************** Macros (Inline Functions) Definitions *********************/
static inline void Update_MpuConfig_Array(u32 Addr, u32 RegSize, u32 RegNum,
		u32 Attrib) __attribute__((__section__(".boot")));
/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
__attribute__((weak)) u32  _TCM_A_REGION  __attribute__((__section__(".bootdata"))) = 0;
__attribute__((weak)) u32 _TCM_B_REGION  __attribute__((__section__(".bootdata"))) = 0;
__attribute__((weak)) u32 _TCM_C_REGION  __attribute__((__section__(".bootdata"))) = 0;

/************************** Function Prototypes ******************************/
extern void Xil_SetAttributeBasedOnConfig(u32 IsSortingNedded)  __attribute__((__section__(".boot")));
void Init_MPU(void) __attribute__((__section__(".boot")));

/*
 * InitialMpu_Config is an array of structures initialized with MPU regions
 * configuration data. This is defined as "weak" and is placed in the bootdata
 * section.
 * In an isolation use case, default addresses allocated to MPU regions will change
 * depending on user confugurations. For such cases Init_MPU must initialize
 * MPU regions as per user configurations instead of default configurations.
 * To have proper MPU configurations, users must override the default "weak" array
 * defined here with a non-weak array in their application.
 * As an example, if user has allocated just 1 GB of DDR, they can have an
 * array defined in their application as following:
 *
 * #include "xil_mpu.h"
 * XMpuConfig_Initial InitialMpu_Config __attribute__((section(".bootdata"))) = {
 * {
 *		0x00000000U,
 *		0x3FFFFFFF,
 *		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW
 *  },
 *  // Other MPU region definitions follow as decided bu user
 * }
 */

__attribute__((weak)) XMpuConfig_Initial InitialMpu_Config __attribute__((section(".bootdata"))) = {
	{
		/* 2 GB DDR DDRMC0_Region0_mem */
		0x00000000U,
		0x7FFFFFFF,
		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW,
	},
	{
		/* 512 MB LPD_AXI_PL_mmap LPD to PL AXI interface */
		0x80000000U,
		0x1FFFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 256 MB PCIe_Region0_mmap, 128 MB FPD_AXI_PL_mmap */
		0xA0000000U,
		0x17FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 1 MB OCM */
		0xBBF00000U,
		0xFFFFF,
		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW,
	},
	{
		/* 512 MB OSPI_mem, 16 MB Coresight */
		0xC0000000U,
		0x20FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/*
		 * 2MB RPU GIC
		 * Note: This region is mandatory for using PS interrupts.
		 */
		0xE2000000U,
		0x1FFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 16 MB CPM */
		0xE4000000U,
		0xFFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 8 MB HNIC */
		0xE6000000U,
		0x7FFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 16 MB FPD + 32 MB LPD peripherals
		 * Note:
		 * 	- Refer "4 GB Detailed Address Map" table in VersalNet TRM
		 * 	- This region is mandatory, since BSP boot code access few
		 * 	  registers from LPD_SLCR region to identify warm boot.
		 */
		0xEA000000U,
		0x2FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 128 MB PMC */
		0xF0000000U,
		0x7FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 64 MB FPD_AXI_CMN_mem */
		0xF8000000U,
		0x3FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	/* A total of 11 MPU regions are allocated with another 5 being free for users */
	{
		0U
	}
};

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
static inline void Update_MpuConfig_Array(u32 Addr, u32 RegSize, u32 RegNum,
		u32 Attrib)
{
	Mpu_Config[RegNum].RegionStatus = MPU_REG_ENABLED;
	Mpu_Config[RegNum].BaseAddress = (Addr & XMPU_PBAR_REG_BASEADDR_MASK);
	Mpu_Config[RegNum].Size = RegSize;
	Mpu_Config[RegNum].Attribute = Attrib;
	Mpu_Config[RegNum].flags = XMPU_VALID_REGION;
}

/*****************************************************************************/
/**
*
* Initialize MPU for during bootup with predefined region attributes.
* Note: If user wants to edit regions in this function, he has to ensure that
* regions are not overlapping. Internal MPU APIs in mpu.c are not dealing with
* overlapping detection and correction. Since, we would like to keep APIs in this
* simple.
*
* @return	None.
*
*
******************************************************************************/
void Init_MPU(void)
{
	u32 RegNum = 0;

	Xil_DisableMPURegions();

	while ((InitialMpu_Config[RegNum].Size != 0U) &&
									(RegNum < MAX_POSSIBLE_MPU_REGS)) {
		Update_MpuConfig_Array(InitialMpu_Config[RegNum].BaseAddress,
								InitialMpu_Config[RegNum].Size,
								RegNum,
								InitialMpu_Config[RegNum].Attribute);
		RegNum++;
	}
	Xil_SetAttributeBasedOnConfig(1U);

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
void Xil_DisableMPURegions(void)
{
	u32 Temp = 0U;
	u32 Index = 0U;
	for (Index = 0; Index <= 15U; Index++) {
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER, Index);
		Temp = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
		Temp &= (~REGION_EN);
		dsb();
		mtcp(XREG_CP15_MPU_REG_SIZE_EN, Temp);
		dsb();
		isb();
	}

}

/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
static inline void Update_MpuConfig_Array(u32 Addr,u32 RegSize,u32 RegNum, u32 Attrib) __attribute__((__section__(".boot")));
/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
extern void Xil_SetAttributeBasedOnConfig(u32 IsSortingNedded)  __attribute__((__section__(".boot")));
void Init_MPU(void) __attribute__((__section__(".boot")));

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
	u32 Addr;
	u32 RegSize = 0U;
	u32 Attrib;
	u32 RegNum = 0;
	u32 Needsorting = 1;

	Xil_DisableMPURegions();

    /* 2 GB DDR */
    Addr = 0x00000000U;
    RegSize = 0x7FFFFFFF;
    Attrib = NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 512 MB LPD to AFI fabric slave port */
    Addr = 0x80000000U;
    RegSize = 0x1FFFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 256 MB PCIE region + 128 MB PS_FPD_AFI_FS */
    Addr = 0xA0000000U;
    RegSize = 0x17FFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 1 MB OCM */
    Addr = 0xBBF00000U;
    RegSize = 0xFFFFF;
    Attrib = NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 512 MB xSPI + 16 MB Coresight */
    Addr = 0xC0000000U;
    RegSize = 0x20FFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 2MB RPU GIC */
    Addr = 0xE2000000U;
    RegSize = 0x1FFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 16 MB CPM */
    Addr = 0xE4000000U;
    RegSize = 0xFFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 16 MB FPD + 32 MB LPD */
    Addr = 0xEA000000U;
    RegSize = 0x2FFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;


    /* 128 MB PMC */
    Addr = 0xF0000000U;
    RegSize = 0x7FFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    /* 64 MB PS_FPD_CMN */
    Addr = 0xF8000000U;
    RegSize = 0x3FFFFFF;
    Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
    Update_MpuConfig_Array(Addr,RegSize,RegNum, Attrib);
    RegNum++;

    Xil_SetAttributeBasedOnConfig(Needsorting);
    /* A total of 10 MPU regions are allocated with another 6 being free for users */
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
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,Index);
		Temp = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
		Temp &= (~REGION_EN);
		dsb();
		mtcp(XREG_CP15_MPU_REG_SIZE_EN,Temp);
		dsb();
		isb();
	}

}

/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplatform_info.c
* @addtogroup common_platform_info Hardware Platform Information
* @{
* This file contains information about hardware for which the code is built
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00  pkp  12/15/14 Initial release
* 5.04  pkp  01/12/16 Added platform information support for Cortex-A53 32bit
*					  mode
* 6.00  mus  17/08/16 Removed unused variable from XGetPlatform_Info
* 6.4   ms   05/23/17 Added PSU_PMU macro to support XGetPSVersion_Info
*                     function for PMUFW.
*       ms   06/13/17 Added PSU_PMU macro to provide support of
*                     XGetPlatform_Info function for PMUFW.
*       mus  08/17/17 Add EL1 NS mode support for
*                     XGet_Zynq_UltraMp_Platform_info and XGetPSVersion_Info
*                     APIs.
* 7.0	aru 03/15/19  Check for versal before aarch64 and armr5
*		      in XGetPlatform_Info()
* 7.2   adk 08/01/20  Added versal support for the XGetPSVersion_Info function.
* 7.6   mus 08/23/21  Updated prototypes for functions which are not taking any
*                     arguments with void keyword. This has been done to fix
*                     compilation warnings with "-Wstrict-prototypes" flag.
*                     It fixes CR#1108601.
* 7.6    mus 08/30/21 Updated flag checking to fix compilation warnings
*                     reported with "-Wundef" flag. It fixes CR#1108601.

* 7.7    mus 11/02/21 Updated XGet_Zynq_UltraMp_Platform_info and
*                     XGetPSVersion_Info to fix compilation warning
*                     reported with "-Wundef" flag CR#1111453
* 8.1    mus 02/13/23 Added new API's XGetCoreId and XGetClusterId. As of now
*                     they are supported only for VERSAL_NET APU and RPU.
* 9.0    mus 03/28/23 Added new API XGetBootStatus for VERSAL_NET. It can be
*                     used to identify type of boot (cold/warm).
* 9.0    mus 07/27/23 Updated XGetCoreId API to support A9, R5 and A53 processor.
* 9.1    mus 06/28/24 Fix typo in XGetCoreId, due to this XGetCoreId
*                     always returns 0 in case of A78 processor CR#1204077.
* 9.2    mus 09/23/24 Fix XGetBootStatus for Versal 2VE and 2VM devices.
* 9.2    tnt 02/10/25 Replace all RPU_PCI_[XY]_PWRDWN for Versal 2VE and 2VM
                      devices with XPS_PSX_RPU_CLUSTER_XY_CORE_X_PWRDWN
                      registers.
* 9.4    vmt 28/10/25 Added XIOCoherencySupported() API to check cache coherency
		      support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"
#include "xplatform_info.h"
#if defined (__aarch64__)
#include "bspconfig.h"
#include "xil_smc.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* @brief    This API is used to provide information about platform
*
* @return   The information about platform defined in xplatform_info.h
*
******************************************************************************/
u32 XGetPlatform_Info(void)
{
#if defined (versal)
	return XPLAT_VERSAL;
#elif defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32) || defined (PSU_PMU)
	return XPLAT_ZYNQ_ULTRA_MP;
#elif defined (__microblaze__)
	return XPLAT_MICROBLAZE;
#else
	return XPLAT_ZYNQ;
#endif
}

/*****************************************************************************/
/**
*
* @brief    This API is used to provide information about zynq ultrascale MP platform
*
* @return   The information about zynq ultrascale MP platform defined in
*			xplatform_info.h
*
******************************************************************************/
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32)
u32 XGet_Zynq_UltraMp_Platform_info(void)
{
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	XSmc_OutVar reg;
	/*
	 * This SMC call will return,
	 *  idcode - upper 32 bits of reg.Arg0
	 *  version - lower 32 bits of reg.Arg1
	 */
	reg = Xil_Smc(GET_CHIPID_SMC_FID, 0, 0, 0, 0, 0, 0, 0);
	return (u32)((reg.Arg1 >> XPLAT_INFO_SHIFT) & XPLAT_INFO_MASK);
#else
	u32 reg;
	reg = ((Xil_In32(XPLAT_PS_VERSION_ADDRESS) >> XPLAT_INFO_SHIFT )
	       & XPLAT_INFO_MASK);
	return reg;
#endif
}
#endif

/*****************************************************************************/
/**
*
* @brief    This API is used to provide information about PS Silicon version
*
* @return   The information about PS Silicon version.
*
******************************************************************************/
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32) || defined (PSU_PMU) || defined (versal)
u32 XGetPSVersion_Info(void)
{
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/*
	 * This SMC call will return,
	 *  idcode - upper 32 bits of reg.Arg0
	 *  version - lower 32 bits of reg.Arg1
	 */
	XSmc_OutVar reg;
	reg = Xil_Smc(GET_CHIPID_SMC_FID, 0, 0, 0, 0, 0, 0, 0);
	return (u32)((reg.Arg1 &  XPS_VERSION_INFO_MASK) >>
		     XPS_VERSION_INFO_SHIFT);
#else
	u32 reg;
	reg = (Xil_In32(XPLAT_PS_VERSION_ADDRESS)
	       & XPS_VERSION_INFO_MASK);
	return (reg >> XPS_VERSION_INFO_SHIFT);
#endif
}
#endif

#if ! defined(__microblaze__) && ! defined(__riscv)
/*****************************************************************************/
/**
*
* @brief    This API is used to provide infomation about core id of the
*           CPU core from which it is executed.
*
* @return   Core id of the core on which API is executed.
*
******************************************************************************/
u8 XGetCoreId(void)
{
	UINTPTR CoreId;

#if (defined (__aarch64__) && ! defined (VERSAL_NET))
	/* CortexA53 and CortexA72 */
	CoreId = (mfcp(MPIDR_EL1) & XREG_MPIDR_MASK);
	CoreId = ((CoreId & XREG_MPIDR_AFFINITY0_MASK) >> \
		  XREG_MPIDR_AFFINITY0_SHIFT);
#elif (defined (__aarch64__) && defined (VERSAL_NET))
	/* CortexA78 */
	CoreId = (mfcp(MPIDR_EL1) & XREG_MPIDR_MASK);
	CoreId = ((CoreId & XREG_MPIDR_AFFINITY1_MASK) >> \
		  XREG_MPIDR_AFFINITY1_SHIFT);
#else
	/* CortexA9, CortexR5 and CortexR52 */
#ifdef __GNUC__
	CoreId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & XREG_MPIDR_MASK);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_MULTI_PROC_AFFINITY, CoreId);
	CoreId &= XREG_MPIDR_MASK;
#else
	{
		register u32 C15Reg __asm(XREG_CP15_MULTI_PROC_AFFINITY);
		CoreId = C15Reg;
	}
	CoreId &= XREG_MPIDR_MASK;
#endif

	CoreId = ((CoreId & XREG_MPIDR_AFFINITY0_MASK) >> \
		  XREG_MPIDR_AFFINITY0_SHIFT);
#endif

	return (u8)CoreId;
}
#endif

#if (defined (__aarch64__) && defined (VERSAL_NET)) || defined (ARMR52)
/*****************************************************************************/
/**
*
* @brief    This API is used to provide infomation about cluster id of the
*           CPU core from which it is executed.
*
* @return   Cluster id of the core on which API is executed.
*
******************************************************************************/
u8 XGetClusterId(void)
{
	u64 ClusterId = 0;

#if defined (ARMR52)
	ClusterId = (mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & XREG_MPIDR_MASK);
	ClusterId = ((ClusterId & XREG_MPIDR_AFFINITY1_MASK) >> \
		     XREG_MPIDR_AFFINITY1_SHIFT);
#else
	ClusterId = (mfcp(MPIDR_EL1) & XREG_MPIDR_MASK);
	ClusterId = ((ClusterId & XREG_MPIDR_AFFINITY2_MASK) >> \
		     XREG_MPIDR_AFFINITY2_SHIFT);
#endif

	return (u8)ClusterId;
}

/*****************************************************************************/
/**
*
* @brief    This API returns boot status of core from which it is executed.
*           0th bit of CORE_X_PWRDWN/RPU_PCIL_X_PWRDWN register indicates boot type.
*
* @return   - 0 for cold boot
* 	    - 1 for warm boot
*
******************************************************************************/
u8 XGetBootStatus(void)
{
	u32 Status;
	UINTPTR Addr;

#if (__aarch64__)
	u8 CpuNum;

	CpuNum = XGetClusterId();
	CpuNum *= XPS_NUM_OF_CORES_PER_CLUSTER;
	CpuNum += XGetCoreId();

	Addr = XPS_CORE_X_PWRDWN_BASEADDR + (CpuNum * XPS_CORE_X_PWRDWN_OFFSET);
	Status = Xil_In32(Addr);

	return (Status & XPS_CORE_X_PWRDWN_EN_MASK);
#else
#if defined (VERSAL_2VE_2VM)
	u8 ClusterId = XGetClusterId();

	/*
	 * Offset between RPU_PCIL_X_PWRDWN registers of 2
	 * consecutive clusters/cores are not consistent
	 * across all 5 clusters.
	 * - Cluster A,B: cluster offset 0x1000, core offset 0x100
	 * - Cluster C,D,E: cluster offset 0x40, core offset 0x20
	 */


	Addr = (XPS_PSX_RPU_PWRDWN_CLUSTER_OFFSET * ClusterId) + XPS_PSX_RPU_CLUSTER_A0_CORE_0_PWRDWN;
	Addr += (XGetCoreId() * XPS_PSX_RPU_PWRDWN_CORE_OFFSET);
	Status = Xil_In32(Addr);

	return (Status & XPS_PSX_RPU_CORE_X_PWRDWN_EN_MASK);
#else
	Addr = (XPS_RPU_PCIL_CLUSTER_OFFSET * XGetClusterId()) + XPS_RPU_PCIL_A0_PWRDWN;
	Addr += (XGetCoreId() * XPS_RPU_PCIL_CORE_OFFSET);
	Status = Xil_In32(Addr);

	return (Status & XPS_RPU_PCIL_X_PWRDWN_EN_MASK);
#endif

#endif

}

#endif

/*****************************************************************************/
/**
 *
 * @brief    This API checks if cache coherency is supported on the current processor.
 *           For ARM AArch64 processors, coherency through CCI/CMN is allowed only at
 *           EL1 NonSecure. For other cores such as Cortex-R5, MicroBlaze, and RISC-V,
 *           coherency is not supported.
 *
 * @return   TRUE if coherency is supported, otherwise FALSE.
 *
 ******************************************************************************/
u32 XIOCoherencySupported(void)
{
#if defined (__aarch64__)
	u64 CurrentEL;
	u32 ExceptionLevel;

	/* Read CurrentEL register */
	__asm__ __volatile__ ("mrs %0, CurrentEL" : "=r" (CurrentEL));

	ExceptionLevel = (u32)((CurrentEL >> 2U) & 0x3U);

	/* Baremetal coherency allowed at EL1 Non-Secure */
	if (ExceptionLevel == APU_EL1) {
		return TRUE;
	}
	return FALSE;

#else
	/* For ARM Cortex-R5, MicroBlaze and RISC-V */
	return FALSE;
#endif
}

/** @} */

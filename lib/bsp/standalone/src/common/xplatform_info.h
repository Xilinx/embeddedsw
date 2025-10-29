/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplatform_info.h
*
*
* @addtogroup common_platform_info APIs to Get Platform Information
*
*
* The xplatform_info.h file contains definitions for various available Xilinx&reg;
* platforms. Also, it contains prototype of APIs, which can be used to get the
* platform information.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    Changes
* ----- ---- --------- -------------------------------------------------------
* 6.4    ms   05/23/17 Added PSU_PMU macro to support XGetPSVersion_Info
*                      function for PMUFW.
* 7.2    adk  08/01/20 Added versal support for the XGetPSVersion_Info function.
* 7.6    mus  08/23/21 Updated prototypes for functions which are not taking any
*                      arguments with void keyword. This has been done to fix
*                      compilation warnings with "-Wstrict-prototypes" flag.
*                      It fixes CR#1108601.
* 7.6    mus  08/30/21 Updated flag checking to fix compilation warnings
*                      reported with "-Wundef" flag.
* 7.7	 sk   01/10/22 Update XPLAT_INFO_MASK from signed to unsigned to fix
*		       misra_c_2012_rule_10_4 violation.
* 8.1    mus  02/13/23 Added new API's XGetCoreId and XGetClusterId. As of now
*                      they are supported only for VERSAL_NET APU and RPU.
* 9.0    mus 03/28/23 Added new API XGetBootStatus for VERSAL_NET. It can be
*                     used to identify type of boot (cold/warm).
* 9.0    mus 07/27/23 Updated XGetCoreId API to support A9, R5 and A53 processor
* 9.0    ml  09/14/23 Added U to numerical to fix MISRA-C violation for Rule
*                     10.1 and 10.4
* 9.2    ng  08/20/24 Added SpartanUP device support
* 9.2    mus 09/23/24 Add definitions related to cluster/core specific offsets
*                     for RPU_PCIL_X_PWRDWN register. They are applicable for
*                     cluster C, D and E in Versal 2VE and 2VM devices.
* 9.3    mus 01/07/25 Fix address of RPU_PCIL_C0_PWRDWN register.
* 9.3    tnt 02/10/25 versal_2ve_2vm: replace all RPU_PCI_[XY]_PWRDWN
                      with XPS_PSX_RPU_CLUSTER_XY_CORE_X_PWRDWN registers
* 9.4    vmt 28/10/25 Added XIOCoherencySupported() declaration to check cache
		      coherency support.
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XPLATFORM_INFO_H		/* prevent circular inclusions */
#define XPLATFORM_INFO_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/
#if defined(SPARTANUP)
#define XPAR_PMC_TAP_BASEADDR 0x040C0000U
#define XPAR_PMC_TAP_VERSION_OFFSET 0x00000004U
#define XPLAT_PS_VERSION_ADDRESS (XPAR_PMC_TAP_BASEADDR + \
				  XPAR_PMC_TAP_VERSION_OFFSET)
#elif defined (versal)
#define XPAR_PMC_TAP_BASEADDR 0xF11A0000U
#define XPAR_PMC_TAP_VERSION_OFFSET 0x00000004U
#define XPLAT_PS_VERSION_ADDRESS (XPAR_PMC_TAP_BASEADDR + \
				  XPAR_PMC_TAP_VERSION_OFFSET)
#else
#define XPAR_CSU_BASEADDR 0xFFCA0000U
#define	XPAR_CSU_VER_OFFSET 0x00000044U
#define XPLAT_PS_VERSION_ADDRESS (XPAR_CSU_BASEADDR + \
				  XPAR_CSU_VER_OFFSET)
#endif
#define XPLAT_ZYNQ_ULTRA_MP_SILICON 0x0
#define XPLAT_ZYNQ_ULTRA_MP 0x1
#define XPLAT_ZYNQ_ULTRA_MPVEL 0x2
#define XPLAT_ZYNQ_ULTRA_MPQEMU 0x3
#define XPLAT_ZYNQ 0x4
#define XPLAT_MICROBLAZE 0x5
#define XPLAT_VERSAL 0x6U

#define XPS_VERSION_1 0x0
#define XPS_VERSION_2 0x1
#define XPLAT_INFO_MASK (0xFU)

#if defined (versal) || defined(SPARTANUP)
#define XPS_VERSION_INFO_MASK 0xFF00U
#define XPS_VERSION_INFO_SHIFT 0x8U
#define XPLAT_INFO_SHIFT 0x18U
#else
#define XPS_VERSION_INFO_MASK  0xFU
#define XPS_VERSION_INFO_SHIFT 0x0U
#define XPLAT_INFO_SHIFT 0xCU
#endif

#if defined (VERSAL_NET)
#if defined (ARMR52)
#define XPS_NUM_OF_CORES_PER_CLUSTER	2U
#define XPS_RPU_PCIL_A0_PWRDWN		0xEB4200C0U
/*
 * Offset between RPU_PCIL_X_PWRDWN registers of consecutive
 * CPU cores in given cluster
 */
#define XPS_RPU_PCIL_CORE_OFFSET	0x100U

/*
 * Offset between RPU_PCIL_A0_PWRDWN registers of 2 clusters
 */
#define XPS_RPU_PCIL_CLUSTER_OFFSET	0x1000U
#define XPS_RPU_PCIL_X_PWRDWN_EN_MASK	1U
#else
#define XPS_NUM_OF_CORES_PER_CLUSTER	4U
#define XPS_CORE_X_PWRDWN_BASEADDR	0xECB10000U
/*
 * Offset between CORE_X_PWRDWN registers of consecutive
 * CPU cores
 */
#define XPS_CORE_X_PWRDWN_OFFSET	48U
#define XPS_CORE_X_PWRDWN_EN_MASK	1U
#endif
#endif

#if defined (VERSAL_2VE_2VM)
#define XPS_PSX_RPU_CLUSTER_A0_CORE_0_PWRDWN 0xEB588200U
/*
 * Offset between XPS_PSX_RPU_CLUSTER_Xi_CORE_i registers of consecutive
 * CPU cores in given cluster
 */
#define XPS_PSX_RPU_PWRDWN_CORE_OFFSET	0x4000U
/*
 * Offset between XPS_PSX_RPU_CLUSTER_xI_CORE_I registers of 2 clusters
 */
#define XPS_PSX_RPU_PWRDWN_CLUSTER_OFFSET	0x10000U

#define XPS_PSX_RPU_CORE_X_PWRDWN_EN_MASK	1U
#endif

#define APU_EL1         1U

/**************************** Type Definitions *******************************/
/**
 *@endcond
 */
/***************** Macros (Inline Functions) Definitions *********************/


u32 XGetPlatform_Info(void);
#if ! defined(__microblaze__) && ! defined(__riscv)
u8 XGetCoreId(void);
#endif

#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32) || defined (PSU_PMU) || defined (versal)
u32 XGetPSVersion_Info(void);
#endif

#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32)
u32 XGet_Zynq_UltraMp_Platform_info(void);
#endif

#if (defined (__aarch64__) && defined (VERSAL_NET)) || defined (ARMR52)
u8 XGetClusterId(void);
u8 XGetCoreId(void);
u8 XGetBootStatus(void);
#endif

u32 XIOCoherencySupported(void);

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup common_platform_info".
*/

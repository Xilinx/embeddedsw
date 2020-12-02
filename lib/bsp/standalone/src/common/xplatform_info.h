/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplatform_info.h
*
* @addtogroup common_platform_info APIs to Get Platform Information
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
* </pre>
*
******************************************************************************/

#ifndef XPLATFORM_INFO_H		/* prevent circular inclusions */
#define XPLATFORM_INFO_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/
#if defined (versal)
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
#define XPLAT_INFO_MASK (0xF)

#if defined (versal)
#define XPS_VERSION_INFO_MASK 0xFF00U
#define XPS_VERSION_INFO_SHIFT 0x8U
#define XPLAT_INFO_SHIFT 0x18U
#else
#define XPS_VERSION_INFO_MASK (0xF)
#define XPS_VERSION_INFO_SHIFT 0x0U
#define XPLAT_INFO_SHIFT 0xCU
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


u32 XGetPlatform_Info(void);

#if defined (ARMR5) || (__aarch64__) || (ARMA53_32) || (PSU_PMU) || defined(versal)
u32 XGetPSVersion_Info();
#endif

#if defined (ARMR5) || (__aarch64__) || (ARMA53_32)
u32 XGet_Zynq_UltraMp_Platform_info();
#endif
/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup common_platform_info".
*/

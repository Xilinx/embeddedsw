/******************************************************************************
* (c) Copyright 2016 - 2020 Xilinx, Inc. All rights reserved.
* (c) Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
******************************************************************************/

/*****************************************************************************/
/**
* @file xstl_topmb.h
*  This file contains structures, global variables and Macro definitions
*  required for STL top level interface MB component
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh   03/08/2016  Initial creation
* 1.1  Naresh   04/18/2016  Updated file header copyright information
* 1.2  Naresh 	04/23/2016	Fixed Misra-C violations
* 1.3  Naresh 	05/16/2016	Addressed review comments, Misra-C violations
* 1.4  Naresh 	10/14/2016	Added new hooks
* 1.5  Naresh 	10/20/2016  Added hook for TTC event test timer
* 1.6  Rama     01/03/2017  Added hooks for MBIST, GEM STLs
* 1.7  Rama     02/16/2017  Added hooks for XPPU, WDT STLs
* 1.8  Rama		02/23/2017	Fixed Misra-C Violations
* 1.9  Rama		05/16/2017	Updated to address CR#976368
* 1.10 Rama		05/26/2017	Updated to address CR#976231
* 1.11 Rama		12/19/2018	Updated to address CR#1016540
* 1.12 Naresh   04/08/2019  Updated code to address CR#1018587
* 1.13 Rama	    04/02/2020  Adds macros for PMu Global General Storage_5
* 						    Register and RPU check start on PMU .
* 						    This change is to address CR#1052625.
* 1.14 Trinendra 04/22/2026 Added SDT Support
*
* </pre>
*
******************************************************************************/
#ifdef ENABLE_SAFETY
#ifdef __MICROBLAZE__
#ifndef XSTL_TOPMB_H		/* prevent circular inclusions */
#define XSTL_TOPMB_H		/* by using protection macros */

/***************************** Include Files *********************************/
#ifdef SDT
#include "xstl_config.h"
#endif
#include "xil_types.h"
#include "xpfw_module.h"
/***************************** Global variables *********************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XSTL_PMU_ERR_NOACT			0x0U
#define XSTL_PMU_ERR_SRST			0x1U
#define XSTL_PMU_ERR_POR			0x2U
#define XSTL_PMU_ERR_INTR			0x3U
#define XSTL_PMU_ERR_PSERR			0x4U
#define XSTL_PMU_HW_ERR_BITMASK		0x200000U
#define XSTL_NUM_STLS_ON_PMU		0x21U
#define XSTL_PMU_STL_UNSCH_FAIL		0x2000U
#define XSTL_PMU_SERV_ERR_REG		0xFFD6033CU
#define XSTL_PMU_SERV_ERR_BITMASK	0x80000000U
#define XSTL_PMU_GLBL_GEN_STORAGE_5	0xFFD80044U
#define XSTL_START_ON_PMU_MASK		0x20000000U

/**************************** Type Definitions *******************************/
typedef struct {
	void (*StlFuncPtr)(void);
	u32 *ArgListPtr;
} XStl_PmuStlList;

/************************** Function Prototypes ******************************/
void XStl_PmuStlInit(void);
void XStl_RegChkInitHook(void);
void XStl_RegCheckTopHook(void);
void XStl_MemScrubOcmHook(void);
void XStl_MemScrubPmuHook(void);
void XStl_AdmaTestHook(void);
void XStl_XmpuErrInjHook(void);
void XStl_OcmErrInjHook(void);
void XStl_ClkErrInjHook(void);
void XStl_IntcIntegrityChkHook(void);
void XStl_IntcInvApbAccTestHook(void);
void XStl_IntcMemListAddMemHook(void);
void XStl_IntcMemListRemMemHook(void);
void XStl_AtbErrInjHook(void);
void XStl_RpuLSErrInjHook(void);
void XStl_PMUECCErrInjHook(void);
void XStl_PMUTmrErrInjHook(void);
void XStl_SysmonErrInjHook(void);
void XStl_UartLpbkTestHook(void);
void XStl_CanLoopbackTestHook(void);
void XStl_TtcTestCntrHook(void);
void XStl_TtcTestEvntTmrHook(void);
void XStl_MbistClearHook(void);
void XStl_GemLoopbackTestHook(void);
void XStl_XppuErrInjHook(void);
void XStl_WdtErrInjHook(void);
void XStl_IntcInvApbListCtrlHook(void);

void XStl_IpiHandler(const XPfw_Module_t *StlModulePtr, u32 IpiNum, u32 SrcMask, const u32* Payload, u8 Len);

#endif		/* end of protection macro */
#endif /* End of  __MICROBLAZE__ macro */
#endif

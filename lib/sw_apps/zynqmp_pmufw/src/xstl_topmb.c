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
* @file xstl_topmb.c
*  This file contains STL top level interface MB specific routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/08/2016  Initial creation
* 1.1  Naresh  04/18/2016  Updated file header copyright information
* 1.2  Naresh  04/23/2016  Fixed Misra-C violations
* 1.3  Naresh  05/16/2016  Addressed the review comments,Fixed Misra Violations
* 1.4  Naresh  05/23/2016  Added logic to enable the IPI interrupts to PMU
* 1.5  Naresh  10/14/2016  Added new hooks
* 1.6  Naresh  10/20/2016  Added hook for TTC event test timer
* 1.7  Naresh  10/21/2016  Fixed Misra-C Violations
* 1.8  Naresh  10/24/2016  Removed macro IPI_PMU_0_IER
* 1.9  Naresh  10/24/2016  Removed extra argument from XPfw_IpiReadMessage
* 1.10 Naresh  10/26/2016  Updated code to address CR#962582
* 1.11 Rama    01/03/2017  Added hooks for MBIST, GEM STLs
* 1.12 Rama    02/16/2017  Added hooks for XPPU, WDT STLs
* 1.13 Rama	   02/23/2017  Fixed Misra-C Violations
* 1.14 Rama	   03/15/2017  Corrected WDT and Register Check API hooks
* 1.15 Naresh  04/18/2017  Updated code to address CR#969232
* 1.16 Naresh  04/26/2017  Updated code to address CR#974513 and also updated
*						   code for non-periodic STL execution as per changes
*						   from PR#2112
* 1.17 Rama	   05/15/2017  Updated code to address CR#976231
* 1.18 Rama	   12/19/2018  Updated to address CR#1016540
* 1.19 Rama	   04/01/2019  Corrected the error action processing for R5STL
*						   errors, CR#1026804, CR#1026671
* 1.20 Naresh  04/08/2019  Updated code to address CR#1018587
* 1.21 Kvn     05/06/2019  To avoid unused parameters warnings
*                          that are platform dependent, a void
*                          statement is used.
*                          This change address CR#1029822.
* 1.22 Rama	   04/02/2020  Updates Bit 29 in PMu Global General Storage_5
* 						   Register when XStl_RegChkInitHook is triggered.
* 						   This will indicate PMU to start  RPU run mode check
* 						   This change is to address CR#1052625.
* 1.23 Trinendra 04/22/2026 Added SDT Support
* </pre>
*
******************************************************************************/
#ifdef ENABLE_SAFETY
#ifdef SDT
#include "xstl_config.h"
#endif
#ifdef __MICROBLAZE__
/***************************** Include Files *********************************/
#include "xstl_defs.h"
#include "xstl_common.h"
#include "xipipsu.h"
#include "xstl_top.h"
#include "xpfw_core.h"
#include "xpfw_ipi_manager.h"
#include "xstl_topmb.h"

#ifdef XSTL_ON_PMU_CANLPBT
#include "xstl_can.h"
#endif

#ifdef XSTL_ON_PMU_TTCTEST
#include "xstl_ttc.h"
#endif

/************************** Variable Definitions *****************************/
static u32 XStl_ArgList[XSTL_NUM_STLS_ON_PMU][5] = {0U};

static XStl_PmuStlList XStl_PmuStlHooks[XSTL_NUM_STLS_ON_PMU] =
{
	{XStl_RegChkInitHook, &XStl_ArgList[0][0]},
	{NULL, NULL},
	{XStl_RegCheckTopHook, &XStl_ArgList[1][0]},
	{XStl_MemScrubOcmHook, &XStl_ArgList[2][0]},
	{XStl_MemScrubPmuHook, &XStl_ArgList[3][0]},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{XStl_AdmaTestHook, &XStl_ArgList[4][0]},
	{XStl_XmpuErrInjHook, &XStl_ArgList[5][0]},
	{XStl_OcmErrInjHook, &XStl_ArgList[6][0]},
	{XStl_ClkErrInjHook, &XStl_ArgList[7][0]},
	{XStl_IntcIntegrityChkHook, &XStl_ArgList[8][0]},
	{XStl_IntcInvApbAccTestHook, &XStl_ArgList[9][0]},
	{XStl_IntcMemListAddMemHook, &XStl_ArgList[10][0]},
	{XStl_IntcMemListRemMemHook, &XStl_ArgList[11][0]},
	{XStl_AtbErrInjHook, &XStl_ArgList[12][0]},
	{XStl_RpuLSErrInjHook, &XStl_ArgList[13][0]},
	{NULL, NULL},
	{NULL, NULL},
	{XStl_PMUECCErrInjHook, &XStl_ArgList[14][0]},
	{XStl_PMUTmrErrInjHook, &XStl_ArgList[15][0]},
	{XStl_SysmonErrInjHook, &XStl_ArgList[16][0]},
	{XStl_UartLpbkTestHook, &XStl_ArgList[17][0]},
	{XStl_CanLoopbackTestHook, &XStl_ArgList[18][0]},
	{XStl_TtcTestCntrHook, &XStl_ArgList[19][0]},
	{XStl_TtcTestEvntTmrHook, &XStl_ArgList[20][0]},
	{XStl_MbistClearHook, &XStl_ArgList[21][0]},
	{XStl_GemLoopbackTestHook, &XStl_ArgList[22][0]},
	{XStl_XppuErrInjHook, &XStl_ArgList[23][0]},
	{XStl_WdtErrInjHook, &XStl_ArgList[24][0]},
	{XStl_IntcInvApbListCtrlHook, &XStl_ArgList[25][0]},
};

static XPfw_Module_t *StlModPtr;

/************************** Constant Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* STL interface for initializing STL module on PMU.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XStl_PmuStlInit(void)
{
	StlModPtr = (XPfw_Module_t *)XPfw_CoreCreateMod();
	XStl_printf("PMU FW STL module created, ID:%d\r\n", StlModPtr->ModId);

	/* Register the IPI handler for the STL module */
	if(StlModPtr != NULL) {
		(void)XPfw_CoreSetIpiHandler((const XPfw_Module_t *)StlModPtr,
									(XPfwModIpiHandler_t)XStl_IpiHandler, 0U);
		StlModPtr->IpiId = (u16)((XSTL_PMU_IPI_ID >> XSTL_PMU_IPI_ID_OFFSET) &0xFF);
	}
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for register check initialization.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_RegChkInitHook(void)
{
#ifdef XSTL_GCPY_CREATE_PMUREGS
	u32 *ArgListPtr = NULL;
	u32 RegVal = 0U;
	XStl_printf("\nIn XStl_RegChkInitHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_REG_GCPY].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_REG_GCPY].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_RegCheckInitTop((XStl_InstanceStr *)ArgListPtr[0],
								ArgListPtr[1], ArgListPtr[2], ArgListPtr[3]);
		/* This is the starting point of STL on PMU, Start RPU_Run mode check on PMU */
		RegVal = Xil_In32(XSTL_PMU_GLBL_GEN_STORAGE_5);
		RegVal |= XSTL_START_ON_PMU_MASK;
		Xil_Out32(XSTL_PMU_GLBL_GEN_STORAGE_5, XSTL_START_ON_PMU_MASK);
		XStl_printf("Regcheck_init started \n");
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for register checking.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_RegCheckTopHook(void)
{
#if defined(XSTL_ON_PMU_CHKCOMMREGS) || defined(XSTL_ON_PMU_CHKPMUREGS)
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_RegCheckTopHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_REGPMU_CHECK].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_REGPMU_CHECK].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_RegCheckTop((XStl_InstanceStr *)NULL,
										(XStl_InstanceStr *)ArgListPtr[0], 0U,
															0U, &ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for OCM memory scrubbing.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_MemScrubOcmHook(void)
{
#ifdef XSTL_ON_PMU_MEMSCRUBOCM
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_MemScrubOcmHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_MEMSCRUB_OCM].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_MEMSCRUB_OCM].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_MemScrubOcmTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
									&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for PMU RAM memory scrubbing.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_MemScrubPmuHook(void)
{
#ifdef XSTL_ON_PMU_MEMSCRUBPMU
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_MemScrubPmuHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_MEMSCRUB_PMU].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_MEMSCRUB_PMU].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_MemScrubPmuTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
									&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for ADMA testing.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_AdmaTestHook(void)
{
#ifdef XSTL_ON_PMU_ADMATEST
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_AdmaTestHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_ADMA_TEST].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_ADMA_TEST].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_AdmaTestTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
										&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for XMPU error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_XmpuErrInjHook(void)
{
#ifdef XSTL_ON_PMU_XMPU_ERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_XmpuErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_XMPU_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_XMPU_ERRINJ].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_XmpuErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
										&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for OCM error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_OcmErrInjHook(void)
{
#ifdef XSTL_ON_PMU_OCM_ERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_OcmErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_CLK_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_OCM_ERRINJ].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_OcmErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
										&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for clock monitor error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_ClkErrInjHook(void)
{
#ifdef XSTL_ON_PMU_CLK_ERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_ClkErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_CLK_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_CLK_ERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_ClkErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U,
										&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Interconnect integrity check.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_IntcIntegrityChkHook(void)
{
#ifdef XSTL_ON_PMU_INTCINTGRCHK
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_IntcIntegrityChkHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_INTC_INTGRCHK].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_INTC_INTGRCHK].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_IntcIntegrityChkTop((XStl_InstanceStr *)ArgListPtr[0],
													0U, 0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Interconnect invalid APB access test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_IntcInvApbAccTestHook(void)
{
#ifdef XSTL_ON_PMU_INTCAPBTEST
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_IntcInvApbAccTestHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_INTC_APBTEST].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_INTC_APBTEST].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_IntcInvApbAccTestTop((XStl_InstanceStr *)ArgListPtr[0],
													0U, 0U);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Interconnect add memory to list.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_IntcInvApbListCtrlHook(void)
{
#ifdef XSTL_ON_PMU_INTCAPBTEST
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_IntcInvApbListCtrlHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_INTC_APBLISTCTRL].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_INTC_APBLISTCTRL].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_IntcInvApbListCtrlTop((XStl_InstanceStr *)ArgListPtr[0],
									0U, 0U, ArgListPtr[1], ArgListPtr[2]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Interconnect add memory to list.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_IntcMemListAddMemHook(void)
{
#ifdef XSTL_ON_PMU_INTCINTGRCHK
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_IntcMemListAddMemHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_INTC_ADDMEM].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_INTC_ADDMEM].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_IntcMemListAddMemTop((XStl_InstanceStr *)ArgListPtr[0],
									0U, 0U, ArgListPtr[1], ArgListPtr[2]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Interconnect remove memory to list.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_IntcMemListRemMemHook(void)
{
#ifdef XSTL_ON_PMU_INTCINTGRCHK
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_IntcMemListRemMemHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_INTC_REMMEM].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_INTC_REMMEM].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_IntcMemListRemMemTop((XStl_InstanceStr *)ArgListPtr[0],
									0U, 0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for ATB error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_AtbErrInjHook(void)
{
#ifdef XSTL_ON_PMU_ATBERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_AtbErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_ATB_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_ATB_ERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_AtbErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U, 0U);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for RPU lockstep error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_RpuLSErrInjHook(void)
{
#ifdef XSTL_ON_PMU_LSERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_RpuLSErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_RPU_LSERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_RPU_LSERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_RpuLSErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
														0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for PMU RAM ECC error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_PMUECCErrInjHook(void)
{
#ifdef XSTL_ON_PMU_ECCERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_PMUECCErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_PMU_ECCERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_PMU_ECCERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_PMUECCErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
									0U, ArgListPtr[1], ArgListPtr[2]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for PMU TMR error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_PMUTmrErrInjHook(void)
{
#ifdef XSTL_ON_PMU_TMRERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_PMUTmrErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_PMU_TMRERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_PMU_TMRERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_PMUTMRErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
									0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Sysmon error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_SysmonErrInjHook(void)
{
#ifdef XSTL_ON_PMU_SYSERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_SysmonErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_SYSMON_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_SYSMON_ERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_SysmonErrInjTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
									0U, ArgListPtr[1], ArgListPtr[2]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for UART loopback test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_UartLpbkTestHook(void)
{
#ifdef XSTL_ON_PMU_UARTLPBT
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_UartLpbkTestHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_UART_LPBKTEST].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_UART_LPBKTEST].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_UartLpbkTestTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
												0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for CAN loopback test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_CanLoopbackTestHook(void)
{
#ifdef XSTL_ON_PMU_CANLPBT
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_CanLoopbackTestHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_CAN_LPBKTEST].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_CAN_LPBKTEST].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
#ifdef XSTL_ON_PMU_CANLPBT
		/* First set the CAN controller clock frequency */
		XStl_CanLoopbackSetClk(ArgListPtr[2]);
#endif
		(void)XStl_CanLoopbackTestTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
												0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for TTC counter test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_TtcTestCntrHook(void)
{
#ifdef XSTL_ON_PMU_TTCTEST
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_TtcTestCntrHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_TTC_INTRCHK].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_TTC_INTRCHK].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
#ifdef XSTL_ON_PMU_TTCTEST
		/* First set the TTC clock frequency */
		XStl_TtcSetClkHz(ArgListPtr[4]);
#endif
		(void)XStl_TtcTestCntrTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
												0U, &ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for TTC event timer test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_TtcTestEvntTmrHook(void)
{
#ifdef XSTL_ON_PMU_TTCTEST
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_TtcTestEvntTmrHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_TTC_EVTTMRCHK].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_TTC_EVTTMRCHK].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_TtcTestEvntTmrTop((XStl_InstanceStr *)ArgListPtr[0], 0U,
										0U, ArgListPtr[1], ArgListPtr[2]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for MBIST.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_MbistClearHook(void)
{
#ifdef XSTL_ON_PMU_MBISTCLR
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_MbistClearHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_MBIST_CLEAR].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_MBIST_CLEAR].ArgListPtr;
	}
	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_MbistClearTop((XStl_InstanceStr *)ArgListPtr[0], 0x0U,
										 0x0U, ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for GEM loopback test.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_GemLoopbackTestHook(void)
{
#ifdef XSTL_ON_PMU_GEMLPBT
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_GemLoopbackTestHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_GEM_LPBKTEST].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_GEM_LPBKTEST].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_GemLoopbackTestTop((XStl_InstanceStr *)ArgListPtr[0],0x0,
													  0x0,&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for Xppu Error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_XppuErrInjHook(void)
{
#ifdef XSTL_ON_PMU_XPPU_ERRINJ
	u32 *ArgListPtr = NULL;

	XStl_printf("\nIn XStl_XppuErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_XPPU_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_XPPU_ERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_XppuErrInjTop((XStl_InstanceStr *)ArgListPtr[0],0x0,
													  0x0,&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* STL scheduler callback hook for WDT Error injection.
*
* @param	.
*
* @return	.
*
* @note		.
*
*******************************************************************************/
void XStl_WdtErrInjHook(void)
{
#ifdef XSTL_ON_PMU_SWDT_ERRINJ
	u32 *ArgListPtr = NULL;


	XStl_printf("\nIn XStl_WdtErrInjHook\r\n");
	if(XStl_PmuStlHooks[XSTL_ID_SWDT_ERRINJ].ArgListPtr != NULL) {
		ArgListPtr = XStl_PmuStlHooks[XSTL_ID_SWDT_ERRINJ].ArgListPtr;
	}

	if((XStl_InstanceStr *)ArgListPtr[0] != NULL) {
		(void)XStl_SwdtErrInjTop((XStl_InstanceStr *)ArgListPtr[0],0x0,
													  0x0,&ArgListPtr[1]);
	}
#endif
}

/*****************************************************************************/
/**
*
* This function will transfer the IPI to RPU channel-1.
*
* @param	StlInstPtr : Pointer to the STL instance data structure.
* @param	StlArg0 : Argument 0.
* @param	StlArg1 : Argument 1.
* @param	StlArg2 : Argument 2.
* @param	StlArg3 : Argument 3.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XStl_IpiTransfer(XStl_InstanceStr *StlInstPtr, u32 StlArg0,
							u32 StlArg1, u32 StlArg2, u32 StlArg3)
{
	u32 Status = (u32)XST_FAILURE;
	u32 IpiBuf[8] = {0U};

	XStl_AssertNonvoid(StlInstPtr != NULL);

	/* Frame the IPI payload buffer */
	IpiBuf[0] = StlInstPtr->StlCmd;
	IpiBuf[1] = (u32)StlInstPtr;
	IpiBuf[2] = StlArg0;
	IpiBuf[3] = StlArg1;
	IpiBuf[4] = StlArg2;
	IpiBuf[5] = StlArg3;

	/*
	 * Load the IPI message payload into the target (PMU MB Channel 0 or RPU
	 * IPI channel 1) payload buffer, trigger the IPI and wait for its ACK
	 */
	Status = (u32)XPfw_IpiWriteMessage(StlModPtr, XSTL_IPI_TARGET_MASK,
										&IpiBuf[0], XSTL_IPI_MSG_LEN);
	if(Status != (u32)XST_SUCCESS) {
		XStl_printf("Writing message to target IPI payload buffer failed\r\n");
	}
	else {
		XStl_printf("Triggering IPI and Waiting for Response...\r\n");
		(void)XPfw_IpiTrigger(XSTL_IPI_TARGET_MASK);
	}
	return Status;
}

/*****************************************************************************/
/**
*
* STL IPI interrupt handler.
*
* @param	StlModulePtr : Pointer to the STL module.
* @param	IpiNum : IPI number
* @param	SrcMask : Mask of the IPI source
*
* @return	None.
*
* @note		.
*
*******************************************************************************/
void XStl_IpiHandler(const XPfw_Module_t *StlModulePtr, u32 IpiNum, u32 SrcMask,
													const u32* Payload, u8 Len)
{
	XStl_InstanceStr *StlInstPtr = NULL;
	u32 StlCommand;
	u32 StlId;
	u32 StlFreq = 0U;
	u32 Idx;
	u32 Status = (u32)XST_FAILURE;
	u32 IpiBuf[8] = {0U};

	(void) Payload;
	(void) Len;

	XStl_printf("In STL IPI ISR\r\n");
	if(IpiNum > 0U) {
		XStl_printf("STL not expecting IPI from other than on PMU-0\r\n");
		return;
	}

	/* Check if the IPI is from the expected source i.e, RPU channel-1 */
	if((SrcMask & (u32)XSTL_IPI_SOURCE_MASK) == 0U) {
		XStl_printf("IPI not from valid source\r\n");
		return;
	}
	else {
		/* Valid IPI */
		XStl_printf("Valid IPI from RPU (regisr:%08x, regval:%08x, "
					"mask:%08x)\r\n", XSTL_IPI_TARGET_REGISR, SrcMask,
					XSTL_IPI_SOURCE_MASK);
	}

	/* Now read the IPI payload buffer from the source */
	Status = (u32)XPfw_IpiReadMessage(XSTL_IPI_SOURCE_MASK,
											&IpiBuf[0], XSTL_IPI_MSG_LEN);
	if(Status != (u32)XST_SUCCESS) {
		XStl_printf("IPI payload buffer read failed\r\n");
	}
	else {
		/* Dump the IPI payload buffer data */
		XStl_printf("IPI payload buffer word0-7:\r\n");
		XStl_printf("%08x %08x %08x %08x %08x %08x %08x %08x\r\n", IpiBuf[0],
									IpiBuf[1], IpiBuf[2], IpiBuf[3], IpiBuf[4],
									IpiBuf[5], IpiBuf[6], IpiBuf[7]);

		/* Check the STL command */
		StlCommand = (IpiBuf[0] >> XSTL_IPI_CMD_OFFSET) & XSTL_IPI_CMD_MASK;
		if((XStl_InstanceStr *)IpiBuf[1] != NULL) {
			StlInstPtr = (XStl_InstanceStr *)IpiBuf[1];
		}
		switch(StlCommand) {
			case XSTL_STL_ERR_EXEC:
				/* STL error IPI from the counterpart, R5->PMU */
				XStl_StlErrAction(StlInstPtr);
				break;
			case XSTL_PMU_STL_CMD_CFG:
				/*
				 * Config PMU STL input arguments. Received IPI payload syntax:
				 * word0-StlCmd, word1-StlInstPtr, word2-5:STL input arguments
				 */
				StlId = StlInstPtr->StlId;
				for(Idx = 0U;Idx < 5U;Idx++) {
					XStl_PmuStlHooks[StlId].ArgListPtr[Idx] = IpiBuf[Idx+1];
				}
				StlFreq = StlInstPtr->StlFreq;
				XStl_printf("StlAttr:%x, StlFreq:%x, StlInstPtr:%x\r\n",
								StlInstPtr->StlAttr, StlFreq, (u32)StlInstPtr);
				/* Add STL to the PMU FW scheduler */
				if(XStl_PmuStlHooks[StlId].StlFuncPtr != NULL) {
					Status = (u32)XPfw_CoreScheduleTask(StlModulePtr, StlFreq,
									   XStl_PmuStlHooks[StlId].StlFuncPtr);
				}
				if(Status == (u32)XST_SUCCESS) {
					StlInstPtr->ExecState = XSTL_ENABLE;
				}
				else {
					StlInstPtr->ExecState = Status;
				}
				break;
			case XSTL_PMU_STL_CMD_CTRL:
				/*
				 * Enable or disable the PMU STL execution. Rx IPI payload syn:
				 * word0-StlCmd, word1-StlInstPtr, word2-Control, word3-5:Rsvd
				 */
				StlId = StlInstPtr->StlId;
				if(IpiBuf[2] == XSTL_ENABLE) {
					/* Add STL to the PMU FW scheduler */
					StlFreq = StlInstPtr->StlFreq;
					Status = (u32)XPfw_CoreScheduleTask(StlModulePtr, StlFreq,
										XStl_PmuStlHooks[StlId].StlFuncPtr);
				}
				else {
					/* Remove STL from the PMU FW scheduler */
					Status = (u32)XPfw_CoreRemoveTask(StlModulePtr, StlFreq,
										XStl_PmuStlHooks[StlId].StlFuncPtr);
				}
				if(Status == (u32)XST_SUCCESS) {
					StlInstPtr->ExecState = IpiBuf[2];
				}
				else {
					StlInstPtr->ExecState = Status;
				}
				break;
			default:
				Status = (u32)XST_FAILURE;
				XStl_printf("Invalid STL command, StlId:%x, StlCommand:%x\r\n",
												StlInstPtr->StlId, StlCommand);
				break;
		}
	}
	/* Clear the buffer to frame response */
	for(Idx = 0U; Idx < XPFW_IPI_MAX_MSG_LEN; Idx++) {
		IpiBuf[Idx] = 0U;
	}
	/* Update Word 0 with Status */
	XStl_printf("Stl IPI handler Status %u \n", Status);
	IpiBuf[0] = Status;
	(void)XPfw_IpiWriteResponse(StlModulePtr, SrcMask, &IpiBuf[0],
													XPFW_IPI_MAX_MSG_LEN);
	return;
}


/*****************************************************************************/
/**
*
* This function will take appropriate action upon STL error.
*
* @param	StlInstPtr : Pointer to the STL instance data structure.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XStl_StlErrAction(XStl_InstanceStr *StlInstPtr)
{
	u32 RegAddr = 0U;
	u32 RegVal = 0U;
	u32 StlErrAct;

	XStl_AssertVoid(StlInstPtr != NULL);

	/*
	 * On PMU, first write the error code and STL ID to the persistent register
	 * PERS_GLOB_GEN_STORAGE7, that retains value only upon SRST and not POR
	 */
	RegVal = (StlInstPtr->ErrInfo.ErrCode << 8) | StlInstPtr->StlId;
	XStl_Out32(0xFFD8006CU, RegVal);

	/*
	 * Write to the corresponding PMU global register to trigger the chosen
	 * HW action i.e., SRST/POR/INTR/NO_ACTION
	 */
	StlErrAct = (StlInstPtr->StlAttr & 0x7U);
	if((StlErrAct == XSTL_PMU_ERR_NOACT) || (StlErrAct > XSTL_PMU_ERR_PSERR)) {
		return;
	}
	switch(StlErrAct) {
		case XSTL_PMU_ERR_SRST:
			RegAddr = 0xFFD80578U; /* ERROR_SRST_EN_2 */
			break;
		case XSTL_PMU_ERR_POR:
			RegAddr = 0xFFD80560U; /* ERROR_POR_EN_2 */
			break;
		case XSTL_PMU_ERR_INTR:
			RegAddr = 0xFFD80548U; /* ERROR_INT_EN_2 */
			break;
		case XSTL_PMU_ERR_PSERR:
			RegAddr = 0xFFD80590U; /* ERROR_SIG_EN_2 */
			break;
		default:
			/* NO ACTION */
			break;
	}

	/* Set appropriate bit in the PMU FW err (ERROR_STATUS_2) */
	XStl_Out32(RegAddr, XSTL_PMU_HW_ERR_BITMASK);

	RegVal = XStl_In32(XSTL_PMU_SERV_ERR_REG);
	/* Set bit 31 in the PMU SERV ERR Register */
	RegVal |= XSTL_PMU_SERV_ERR_BITMASK;
	XStl_Out32(XSTL_PMU_SERV_ERR_REG, RegVal);
}
#endif /* End of  __MICROBLAZE__ macro */
#endif /* End of  ENABLE_SAFETY macro */
/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_platform.h
*
* This header file contains platform initialization function declarations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 arm     04/02/26  Initial version
*
* </pre>
*
******************************************************************************/

#ifndef MMI_DPDC_PLATFORM_H_
#define MMI_DPDC_PLATFORM_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdcsub.h"
#include "mmi_dc_nonlive_test.h"

/************************** Constant Definitions *****************************/

#define DC_BASEADDR            	0xEDD00000	/**< DC base address */
#define DCDMA_BASEADDR         	0xEDD10000	/**< DC DMA base address */
#define DP_BASEADDR   		    0xEDE00000	/**< DP base address */

/************************** Variable Definitions *****************************/

/* External descriptor declarations */
extern XDcDma_Descriptor *AudDesc0;

/************************** Function Prototypes ******************************/

u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr);
u32 XDpDc_InitClkWiz(RunConfig *RunCfgPtr);
u32 XDpDc_InitDcSubsystem(RunConfig *RunCfgPtr);
void XDpDc_SetupInterrupts(RunConfig *RunCfgPtr);
u32 XDpDc_InitPlatform(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DPDC_PLATFORM_H_ */

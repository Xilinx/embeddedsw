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
******************************************************************************/

#ifndef MMI_DPDC_PLATFORM_H_
#define MMI_DPDC_PLATFORM_H_

#include "mmi_dc_nonlive_test.h"

/* External descriptor declarations */
extern XDcDma_Descriptor *AudDesc0;

/* Function prototypes */

u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr);
u32 XDpDc_InitClkWiz(RunConfig *RunCfgPtr);
u32 XDpDc_InitDcSubsystem(RunConfig *RunCfgPtr);
void XDpDc_SetupInterrupts(RunConfig *RunCfgPtr);
u32 XDpDc_InitPlatform(RunConfig *RunCfgPtr);

#endif /* MMI_DPDC_PLATFORM_H_ */

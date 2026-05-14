/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_mixed_test.h
*
* This file declares the interface for the mixed mode display test where
* Stream1 is sourced from a live video input (AVPG) and Stream2 is sourced
* from a non-live DMA frame buffer.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   arm  05/01/26  Initial release
* </pre>
*
******************************************************************************/

#ifndef __MMI_DC_MIXED_TEST_H__
#define __MMI_DC_MIXED_TEST_H__  /**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "mmi_dpdc_example.h"
#include "xparameters.h"

/************************** Function Prototypes ******************************/

u32 XDpDc_MmiDcMixedTest(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __MMI_DC_MIXED_TEST_H__ */

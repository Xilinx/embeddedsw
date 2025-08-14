/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_coreinit.h
* @addtogroup vprocss Overview
* @{
* @brief
*
* This header file contains the video processing engine sub-cores
* initialization routines and helper functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco  07/21/15   Initial Release

* </pre>
*
******************************************************************************/
#ifndef XVPROCSS_COREINIT_H__  /* prevent circular inclusions */
#define XVPROCSS_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvprocss.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
#ifndef SDT
static int ComputeSubcoreAbsAddr(UINTPTR subsys_baseaddr,
		                         UINTPTR subsys_highaddr,
		                         u32 subcore_offset,
					 UINTPTR *subcore_baseaddr);

#endif
int XVprocSs_SubcoreInitResetAxis(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitResetAximm(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitRouter(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitCsc(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitHScaler(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVScaler(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitHCrsmplr(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVCrsmpleIn(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVCrsmpleOut(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitLetterbox(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVdma(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitDeinterlacer(XVprocSs *XVprocSsPtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */

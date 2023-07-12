/******************************************************************************
* Copyright (C) 2017 - 2023 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditxss_coreinit.h
* @addtogroup v_sditxss Overview
* @{
*
* This header file contains the sdi tx subsystem sub-cores
* initialization routines and helper functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  jsr  07/17/17 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_SDITXSS_COREINIT_H__  /* prevent circular inclusions */
#define XV_SDITXSS_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
#include "xv_sditxss.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_SdiTxSs_SubcoreInitSdiTx(XV_SdiTxSs *SdiTxSsPtr);
int XV_SdiTxSs_SubcoreInitVtc(XV_SdiTxSs *SdiTxSsPtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */

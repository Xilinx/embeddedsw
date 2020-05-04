/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirxss_coreinit.h
* @addtogroup v_sdirxss_v3_0
* @{
*
* This header file contains the sdi rx subsystem sub-cores
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
#ifndef XV_SDIRXSS_COREINIT_H__ /* prevent circular inclusions */
#define XV_SDIRXSS_COREINIT_H__ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
#include "xv_sdirxss.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_SdiRxSs_SubcoreInitSdiRx(XV_SdiRxSs *SdiRxSsPtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */

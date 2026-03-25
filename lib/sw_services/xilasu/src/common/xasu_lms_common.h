/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_lms_common.h
 *
 * This file contains the LMS function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   03/24/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_lms_common_apis LMS Common APIs
 * @{
*/

#ifndef XASU_LMS_COMMON_H_
#define XASU_LMS_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_lmsinfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_LmsValidateParams(const XAsu_LmsHssSignVerifyParams *LmsParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_LMS_COMMON_H_ */
/** @} */

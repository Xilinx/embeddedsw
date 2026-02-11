/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_lms.h
 *
 * This file Contains the LMS client function prototypes, defines and macros for
 * the LMS module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   01/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_lms_client_apis LMS Client APIs
 * @{
*/
#ifndef XASU_LMS_H_
#define XASU_LMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_lmsinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_LmsSignVerify(XAsu_ClientParams *ClientParamsPtr,
		XAsu_LmsHssSignVerifyParams *LmsParamsPtr);
s32 XAsu_HssSignVerify(XAsu_ClientParams *ClientParamsPtr,
		XAsu_LmsHssSignVerifyParams *HssParamsPtr);
s32 XAsu_LmsKat(XAsu_ClientParams *ClientParamsPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_LMS_H_ */
/** @} */

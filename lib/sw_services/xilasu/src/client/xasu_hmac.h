/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_hmac.h
 *
 * This file Contains the HMAC client function prototypes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  01/02/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_hmac_client_apis HMAC Client APIs
 * @{
*/

#ifndef XASU_HMAC_H
#define XASU_HMAC_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_hmacinfo.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_HmacKat(XAsu_ClientParams *ClientParamsPtr);
s32 XAsu_HmacCompute(XAsu_ClientParams *ClientParamsPtr, XAsu_HmacParams *HmacParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_HMAC_H */
/** @} */

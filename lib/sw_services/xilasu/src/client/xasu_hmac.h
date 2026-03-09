/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       kp   02/26/26 Added client-side HMAC SHA3-256 KAT prototype
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_hmac_client_apis HMAC Client APIs
 * @{
*/

#ifndef XASU_HMAC_H_
#define XASU_HMAC_H_

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
s32 XAsu_HmacSha3Kat(void);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_HMAC_H_ */
/** @} */

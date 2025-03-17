/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap.h
 *
 * This file Contains the key wrap unwrap client function prototypes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasu_keywrap_client_apis Keywrap Client APIs
* @{
*/

#ifndef XASU_KEYWRAP_H_
#define XASU_KEYWRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_keywrapinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KeyWrap(XAsu_ClientParams *ClientParamsPtr, XAsu_KeyWrapParams *KeyWrapParamsPtr);
s32 XAsu_KeyUnwrap(XAsu_ClientParams *ClientParamsPtr, XAsu_KeyWrapParams *KeyUnwrapParamsPtr);
s32 XAsu_KeyWrapKat(XAsu_ClientParams *ClientParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYWRAP_H_ */
/** @} */

/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecies.h
 *
 * This file Contains the ECIES client function prototypes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  02/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecies_client_apis ECIES Client APIs
 * @{
*/

#ifndef XASU_ECIES_H_
#define XASU_ECIES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_eciesinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_EciesKat(XAsu_ClientParams *ClientParamsPtr);
s32 XAsu_EciesEncrypt(XAsu_ClientParams *ClientParamsPtr, XAsu_EciesParams *EciesParamsPtr);
s32 XAsu_EciesDecrypt(XAsu_ClientParams *ClientParamsPtr, XAsu_EciesParams *EciesParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_ECIES_H_ */
/** @} */

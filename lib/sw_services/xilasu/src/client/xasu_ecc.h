/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecc.h
 *
 * This file Contains the ECC client function prototypes, defines and macros for
 * the ECC hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/19/24 Initial release
 *       yog  09/26/24 Added doxygen grouping and fixed doxygen comments
 *       ss   12/02/24 Added support for ECDH
 *       yog  02/21/25 Added XAsu_EccValidateCurveInfo() API prototype
 *       yog  03/25/25 Added support for public key generation.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecc_client_apis ECC Client APIs
 * @{
*/

#ifndef XASU_ECC_H_
#define XASU_ECC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_eccinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_EccKat(XAsu_ClientParams *ClientParamsPtr);
s32 XAsu_EccGenSign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr);
s32 XAsu_EccVerifySign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr);
s32 XAsu_EccGenPubKey(XAsu_ClientParams *ClientParamsPtr, XAsu_EccKeyParams *EccKeyParamsPtr);
s32 XAsu_EcdhGenSharedSecret(XAsu_ClientParams *ClientParamsPtr, XAsu_EcdhParams *EcdhParamsPtr);
s32 XAsu_EcdhKat(XAsu_ClientParams *ClientParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_ECC_H_ */
/** @} */

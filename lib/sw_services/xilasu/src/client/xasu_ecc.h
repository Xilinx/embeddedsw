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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecc_client_apis ECC Client APIs
 * @{
*/

#ifndef XASU_ECC_H
#define XASU_ECC_H

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

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_ECC_H */
/** @} */

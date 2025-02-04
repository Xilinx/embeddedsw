/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa_common.h
 * @addtogroup Overview
 * @{
 *
 * This file contains the RSA function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/04/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_RSA_COMMON_H
#define XASU_RSA_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_rsainfo.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_RsaValidateInputParams(const XAsu_RsaParams *RsaParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_RSA_COMMON_H */
/** @} */
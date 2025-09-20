/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdf_common.h
 *
 * This file contains the KDF function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  09/11/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_kdf_common_apis KDF Common APIs
 * @{
*/

#ifndef XASU_KDF_COMMON_H_
#define XASU_KDF_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_kdfinfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_ValidateKdfParameters(const XAsu_KdfParams *KdfParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDF_COMMON_H_ */
/** @} */

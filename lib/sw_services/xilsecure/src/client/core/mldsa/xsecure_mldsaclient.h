/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_mldsaclient.h
*
* This file Contains the client function prototypes, defines and macros for the MLDSA hardware
* module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  03/03/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_mldsa_client_apis XilSecure MLDSA Client APIs
* @{
*/
#ifndef XSECURE_MLDSA_CLIENT_H
#define XSECURE_MLDSA_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"

/***************************************** Type Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Variable Definitions ****************************************/

/*************************************** Function Prototypes **************************************/
int XSecure_MldsaSignVerifyClient(XSecure_ClientInstance *InstancePtr, u64 MldsaParamAddr);
int XSecure_MldsaSignGenerateClient(XSecure_ClientInstance *InstancePtr, u64 MldsaParamAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_MLDSA_CLIENT_H */
/** @} */

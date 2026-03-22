/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsaclient.h
*
* This file contains the client function prototypes, defines and macros for the SLHDSA
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  03/20/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_client_apis XilSecure SLHDSA Client APIs
* @{
*/
#ifndef XSECURE_SLHDSA_CLIENT_H
#define XSECURE_SLHDSA_CLIENT_H

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
int XSecure_SlhdsaSignVerifyClient(XSecure_ClientInstance *InstancePtr, u64 SlhdsaParamAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_SLHDSA_CLIENT_H */
/** @} */

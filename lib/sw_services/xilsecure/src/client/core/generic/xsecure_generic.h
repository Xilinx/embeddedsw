/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file client/core/generic/xsecure_generic.h
*
* This file contains declarations for the generic request API that handles both SMC and IPI
* mailbox communication for xilsecure client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xsecure_client_apis XilSecure Client APIs
 * @{
 */
#ifndef XSECURE_GENERIC_H
#define XSECURE_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

int XSecure_SendRequest(const XSecure_ClientInstance *InstancePtr, u32 *PayloadBuf,
			u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen);

/** @} */
#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_GENERIC_H */

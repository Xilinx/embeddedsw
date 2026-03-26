/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xnvm_generic.h
*
* This file contains declarations for the generic request API that handles
* both SMC and IPI mailbox communication for xilnvm client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 3.7   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
*
* </pre>
*
**************************************************************************************************/
#ifndef XNVM_GENERIC_H
#define XNVM_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xnvm_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

int XNvm_SendRequest(const XNvm_ClientInstance *InstancePtr, u32 *PayloadBuf, u32 PayloadLen,
			u32 *ResponseBuf, u32 ResponseLen);

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_GENERIC_H */

/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xloader_generic.h
*
* This file contains declarations for the generic request API that handles
* both SMC and IPI mailbox communication for xilloader client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 2.4   sms  04/16/26 Initial release for unified SMC/Mailbox handling
*
* </pre>
*
**************************************************************************************************/
#ifndef XLOADER_GENERIC_H
#define XLOADER_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xloader_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

int XLoader_SendRequest(XLoader_ClientInstance *InstancePtr, u32 *PayloadBuf, u32 PayloadLen,
			u32 *ResponseBuf, u32 ResponseLen);

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_GENERIC_H */

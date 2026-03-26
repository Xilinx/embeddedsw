/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_generic.h
 *
 * This file contains the implementation of the generic request API that handles
 * both SMC and IPI mailbox communication for xilplmi client library
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 2.4   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
 *
 * </pre>
 *
 *************************************************************************************************/
#ifndef XPLMI_GENERIC_H
#define XPLMI_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xplmi_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

int XPlmi_SendRequest(XPlmi_ClientInstance *InstancePtr, u32 *PayloadBuf, u32 PayloadLen,
			u32 *ResponseBuf, u32 ResponseLen);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_GENERIC_H */

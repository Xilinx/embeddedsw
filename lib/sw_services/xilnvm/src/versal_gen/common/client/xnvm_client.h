/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xnvm_client.h
 *
 * This file contains declarations of the client interface functions which are common to the whole
 * XilNvm library and are not specific to BBRAM or eFUSE.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 3.8   tus  08/10/26 Initial release with the library version query
 *                     and server library information
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xnvm_client_apis XilNvm Client APIs
 * @{
 */
#ifndef XNVM_CLIENT_H
#define XNVM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xil_cryptoalginfo.h"
#include "xnvm_mailbox.h"
#include "xnvm_defs.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
int XNvm_GetClientLibVersion(u32 *Version);
int XNvm_GetServerLibInfo(XNvm_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo);

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_CLIENT_H */
/** @} */

/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.h
* @addtogroup xsecure_sha3_client_apis XilSecure SHA3 Versal Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the SHA-384 hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/17/21 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_SHACLIENT_H
#define XSECURE_SHACLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_sha3alginfo.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XSecure_ShaInitialize(XSecure_ClientInstance *InstancePtr, XSecure_ShaMode ShaMode);
int XSecure_ShaUpdate(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, u32 Size, u32 EndLast);
int XSecure_ShaFinish(XSecure_ClientInstance *InstancePtr, const u64 OutDataAddr, u32 Size);
int XSecure_ShaDigest(XSecure_ClientInstance *InstancePtr, XSecure_ShaMode ShaMode,
        const u64 InDataAddr, const u64 OutDataAddr, u32 DataSize, u32 HashSize);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_SHACLIENT_H */

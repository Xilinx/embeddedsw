/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_katclient.h
* @addtogroup xsecure_kat_client_apis XilSecure KAT Versal Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the KAT APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  07/19/22 Initial release
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_KAT_CLIENT_H
#define XSECURE_KAT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_AesDecryptKat(XSecure_ClientInstance *InstancePtr);
int XSecure_AesDecryptCmKat(XSecure_ClientInstance *InstancePtr);
int XSecure_RsaPublicEncKat(XSecure_ClientInstance *InstancePtr);
int XSecure_Sha3Kat(XSecure_ClientInstance *InstancePtr);
int XSecure_EllipticSignVerifyKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveClass);
int XSecure_AesEncryptKat(XSecure_ClientInstance *InstancePtr);
int XSecure_RsaPrivateDecKat(XSecure_ClientInstance *InstancePtr);
int XSecure_EllipticSignGenKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveType);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_CLIENT_H */

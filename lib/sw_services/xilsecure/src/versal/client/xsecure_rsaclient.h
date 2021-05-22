/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*@file xsecure_rsaclient.h
* @addtogroup xsecure_rsa_client_apis XilSecure RSA Versal Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the RSA hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_RSA_CLIENT_H
#define XSECURE_RSA_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_RsaPrivateDecrypt(const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr);
int XSecure_RsaPublicEncrypt(const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr);
int XSecure_RsaSignVerification(const u64 SignAddr, const u64 HashAddr,
				const u32 Size);
int XSecure_RsaKat(void);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_RSA_CLIENT_H */

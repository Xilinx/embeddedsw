/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_rsaclient.h
*
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
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_RsaKat into xsecure_katclient.c
* 5.2   mmd  07/09/23 Included header file for crypto algorithm information
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_client_apis XilSecure RSA Client APIs
* @{
*/
#ifndef XSECURE_RSA_CLIENT_H
#define XSECURE_RSA_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_rsaalginfo.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_RsaPrivateDecrypt(XSecure_ClientInstance *InstancePtr, const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr);
int XSecure_RsaPublicEncrypt(XSecure_ClientInstance *InstancePtr, const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr);
int XSecure_RsaSignVerification(XSecure_ClientInstance *InstancePtr, const u64 SignAddr, const u64 HashAddr,
				const u32 Size);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_RSA_CLIENT_H */
/** @} */

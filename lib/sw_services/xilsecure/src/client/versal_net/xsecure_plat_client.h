/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_client.h
*
* This file Contains the client function prototypes, defines and macros for
* the platform specific client APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.1   kpt  01/14/23 Initial release
* 5.2   vns  07/07/23 Added separate IPI commands for Crypto Status update
*       kpt  07/09/23 Added APIs related to Key wrap and unwrap
* 5.3   kpt  12/13/23 Added RSA quiet mode support
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*       kpt  06/13/24 Added XSecure_ReleaseRsaKey
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_client_apis Platform specific helper APIs in Xilsecure client
* @{
*/
#ifndef XSECURE_PLAT_CLIENT_H
#define XSECURE_PLAT_CLIENT_H

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

int XSecure_UpdateHnicCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask);
int XSecure_UpdateCpm5NCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask);
int XSecure_UpdatePkiCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask);
int XSecure_UpdatePcideCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask);
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_KeyUnwrap(XSecure_ClientInstance *InstancePtr, XSecure_KeyWrapData *KeyWrapData);
int XSecure_ReleaseRsaKey(XSecure_ClientInstance *InstancePtr);
/**
 * @}
 * @endcond
 */


#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_CLIENT_H */
/** @} */

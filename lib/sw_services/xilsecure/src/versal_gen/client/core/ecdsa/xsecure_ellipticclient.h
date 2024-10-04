/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_ellipticclient.h
*
* This file Contains the client function prototypes, defines and macros for
* the ECDSA hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_EllipticKat in to xsecure_katclient.c
* 5.2   mmd  07/09/23 Included header file for crypto algorithm information
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_client_apis XilSecure ECDSA Client APIs
* @{
*/
#ifndef XSECURE_ELLIPTIC_CLIENT_H
#define XSECURE_ELLIPTIC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_ellipticalginfo.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_EllipticGenerateSign(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 HashAddr, u32 Size,
                        u64 PrivKeyAddr, u64 EPrivKeyAddr, u64 SignAddr);
int XSecure_EllipticGenerateKey(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 PrivKeyAddr, u64 PubKeyAddr);
int XSecure_EllipticValidateKey(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 KeyAddr);
int XSecure_EllipticVerifySign(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 HashAddr, u32 Size,
                        u64 PubKeyAddr, u64 SignAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_ELLIPTIC_CLIENT_H */
/** @} */

/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_ellipticclient.h
* @addtogroup xsecure_ecdsa_client_apis XilSecure Elliptic Versal Client APIs
* @{
* @cond xsecure_internal
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
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_ELLIPTIC_CLIENT_H
#define XSECURE_ELLIPTIC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_EllipticGenerateSign(u32 CurveType, u64 HashAddr, u32 Size,
                        u64 PrivKeyAddr, u64 EPrivKeyAddr, u64 SignAddr);
int XSecure_EllipticGenerateKey(u32 CurveType, u64 PrivKeyAddr, u64 PubKeyAddr);
int XSecure_EllipticValidateKey(u32 CurveType, u64 KeyAddr);
int XSecure_EllipticVerifySign(u32 CurveType, u64 HashAddr, u32 Size,
                        u64 PubKeyAddr, u64 SignAddr);
int XSecure_EllipticKat(u32 CurveType);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_ELLIPTIC_CLIENT_H */

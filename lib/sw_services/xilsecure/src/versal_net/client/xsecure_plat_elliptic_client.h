/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_elliptic_client.h
* @addtogroup xsecure_plat_elliptic_client_apis XilSecure Elliptic Client APIs
* for Versal Net
* @{
* @cond xsecure_internal
* This file contains the function prototypes for the elliptic client APIs for
* Versal Net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   har  06/15/23 Initial release
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_PLAT_ELLIPTIC_CLIENT_H
#define XSECURE_PLAT_ELLIPTIC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_plat_defs.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_GenSharedSecret(XSecure_ClientInstance *InstancePtr, u32 CrvType, const u8* PrivateKey,
	const u8* PublicKey, u8 *SharedSecret);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_CLIENT_H */
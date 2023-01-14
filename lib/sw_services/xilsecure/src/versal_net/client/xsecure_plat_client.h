/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_client.h
* @addtogroup xsecure_plat_client_apis XilSecure Versal net Plat Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the platform specific client APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.1   kpt  01/14/23 Initial release
*
* </pre>
* @note
*
******************************************************************************/

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

int XSecure_UpdateCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
		u32 NodeId, u32 CryptoMask);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_CLIENT_H */

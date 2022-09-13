/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_katclient.h
* @addtogroup xsecure_kat_client_apis XilSecure KAT Versal net Client APIs
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
* 5.0   kpt  07/19/22 Initial release
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_KAT_CLIENT_PLAT_H
#define XSECURE_KAT_CLIENT_PLAT_H

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
int XSecure_TrngKat(XSecure_ClientInstance *InstancePtr);
int XSecure_UpdateKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatId KatOp, XSecure_KatId KatId);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_CLIENT_H */

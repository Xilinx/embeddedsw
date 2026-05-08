/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/hmac/xsecure_hmacclient.h
*
* This file contains the function prototypes for HMAC client interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   vss  04/29/26 Initial release
*
* </pre>
*
***************************************************************************************************/
#ifndef XSECURE_HMACCLIENT_H
#define XSECURE_HMACCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_plat_defs.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/************************************ Function Prototypes *****************************************/
int XSecure_HmacOperation(XSecure_ClientInstance *InstancePtr, const XSecure_HmacParams *ParamsPtr);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_HMACCLIENT_H */

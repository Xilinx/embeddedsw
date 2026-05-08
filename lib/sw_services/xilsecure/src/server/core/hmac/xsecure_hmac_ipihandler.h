/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/hmac/xsecure_hmac_ipihandler.h
*
* This file contains the function prototypes for HMAC IPI handler
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
#ifndef XSECURE_HMAC_IPIHANDLER_H
#define XSECURE_HMAC_IPIHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xil_types.h"
#include "xplmi_cmd.h"

/*************************************** Constant Definitions *************************************/

/****************************************** Type Definitions **************************************/

/************************************** Function Prototypes ***************************************/
int XSecure_HmacIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_HMAC_IPIHANDLER_H */
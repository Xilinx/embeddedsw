/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
* @file versal_2vp/xsecure_plat_katclient.h
*
* This file contains the client function prototypes, defines and macros for the KAT APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kat_client_apis XilSecure KAT Client APIs
* @{
*/
#ifndef XSECURE_PLAT_KATCLIENT_H
#define XSECURE_PLAT_KATCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xsecure_mailbox.h"

/****************************************** Type Definitions **************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/**************************************** Variable Definitions ************************************/

/************************************** Function Prototypes ***************************************/
int XSecure_TrngKat(XSecure_ClientInstance *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_KATCLIENT_H */
/** @} */

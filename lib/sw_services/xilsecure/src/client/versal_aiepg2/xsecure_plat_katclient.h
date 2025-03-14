/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_katclient.h
*
* This file Contains the client function prototypes, defines and macros for
* the KAT APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kal  03/13/25 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_client_apis XilSecure KAT Client APIs
* @{
*/
#ifndef XSECURE_PLAT_KATCLIENT_H
#define XSECURE_PLAT_KATCLIENT_H

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

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_KATCLIENT_H */
/** @} */

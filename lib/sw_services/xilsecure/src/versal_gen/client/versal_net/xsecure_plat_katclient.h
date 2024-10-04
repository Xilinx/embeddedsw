/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.0   kpt  07/19/22 Initial release
* 5.2   vns  07/07/23 Added separate IPI commands for KAT status updates
* 5.4   yog  04/29/24 Fixed doxygen grouping.
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
int XSecure_UpdateHnicKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask);
int XSecure_UpdateCpm5NKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask);
int XSecure_UpdatePcideKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask);
int XSecure_UpdatePkiKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_KATCLIENT_H */
/** @} */

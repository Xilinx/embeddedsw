/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_client.h
* @addtogroup xocp_client_apis XilOcp Client APIs
* @{
* @cond xocp_internal
* This file Contains the client function prototypes, defines and macros for
* the OCP hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*       am   01/10/23 Added client side API for dme
* 1.2   kpt  06/02/23 Updated XOcp_GetHwPcrLog prototype
*
* </pre>
*
* @note
*
* @endcond
******************************************************************************/

#ifndef XOCP_CLIENT_H
#define XOCP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xocp_mailbox.h"
#include "xocp_def.h"
#include "xocp_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XOcp_ClientInit(XOcp_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);
int XOcp_ExtendPcr(XOcp_ClientInstance *InstancePtr, XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 Size);
int XOcp_GetPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u64 PcrBufAddr, u32 PcrBufSize);
int XOcp_GetHwPcrLog(XOcp_ClientInstance *InstancePtr, u64 HwPcrEventAddr, u64 HwPcrLogInfoAddr,
		u32 NumOfLogEntries);
int XOcp_GenDmeResp(XOcp_ClientInstance *InstancePtr, u64 NonceAddr, u64 DmeStructResAddr);
int XOcp_GetX509Cert(XOcp_ClientInstance *InstancePtr, u64 GetX509CertAddr);
int XOcp_ClientAttestWithDevAk(XOcp_ClientInstance *InstancePtr, u64 AttestWithDevAk);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_CLIENT_H */

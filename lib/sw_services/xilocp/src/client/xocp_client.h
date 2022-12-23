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
int XOcp_ExtendPcr(XOcp_ClientInstance *InstancePtr, XOcp_RomHwPcr PcrNum, u64 ExtHashAddr);
int XOcp_GetPcr(XOcp_ClientInstance *InstancePtr, XOcp_RomHwPcr PcrNum, u64 PcrBufAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_CLIENT_H */
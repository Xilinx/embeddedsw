/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_client.h
*
* This file Contains the client function prototypes, defines and macros for
* the TPM hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pre  03/09/26 Initial release
*       pre  03/21/26 Added GetPcrLog client API
*
* </pre>
*
******************************************************************************/
#ifndef XTPM_CLIENT_H
#define XTPM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xtpm_mailbox.h"
#include "xtpm_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XTpm_Init(XTpm_ClientInstance *InstancePtr);
int XTpm_Startup(XTpm_ClientInstance *InstancePtr);
int XTpm_SelfTest(XTpm_ClientInstance *InstancePtr);
int XTpm_PcrEvent(XTpm_ClientInstance *InstancePtr, u32 PcrIndex, u64 DataAddr, u32 DataLength);
int XTpm_PcrRead(XTpm_ClientInstance *InstancePtr, u32 PcrIndex, u8 HashAlgo, u64 RespBufferAddr);
int XTpm_GetPcrLog(XTpm_ClientInstance *InstancePtr, u64 TpmPcrEventAddr, u64 TpmPcrLogInfoAddr,
		u32 NumOfLogEntries);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XTPM_CLIENT_H */

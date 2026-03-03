/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_hwpcr.h
* @addtogroup xocp_hwpcr_client_apis XilOcp HwPcr Client APIs
* @{
* @cond xocp_internal
* This file Contains the client function prototypes, defines and macros for the Hardware PCR
* interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 1.7   rpu  02/18/26 Initial release
*
* </pre>
*
* @note
*
* @endcond
***************************************************************************************************/

#ifndef XOCP_HWPCR_H
#define XOCP_HWPCR_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************** Include Files *********************************************/
#include "xil_types.h"
#include "xocp_mailbox.h"
#include "xocp_def.h"
#include "xocp_common.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/************************************ Variable Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/************************************** Function Prototypes ***************************************/
int XOcp_ExtendHwPcr(XOcp_ClientInstance *InstancePtr, XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 Size);
int XOcp_GetHwPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u64 PcrBufAddr, u32 PcrBufSize);
int XOcp_GetHwPcrLog(XOcp_ClientInstance *InstancePtr, u64 HwPcrEventAddr, u64 HwPcrLogInfoAddr,
		u32 NumOfLogEntries);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_HWPCR_H */
/** @} */
/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_swpcr.h
*
* This file Contains the client function prototypes, defines and macros for the Software PCR
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
***************************************************************************************************/

#ifndef XOCP_SWPCR_H
#define XOCP_SWPCR_H

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
int XOcp_ExtendSwPcr(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrExtendParams *ExtendParams);
int XOcp_GetSwPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u8 *PcrBuf, u32 PcrBufSize);
int XOcp_GetSwPcrLog(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrLogReadData *LogParams);
int XOcp_GetSwPcrData(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrReadData *DataParams);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_SWPCR_H */
/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dme.h
*
* This file Contains the client function prototypes, defines and macros for the DME interface.
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

#ifndef XOCP_DME_H
#define XOCP_DME_H

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
int XOcp_GenDmeResp(XOcp_ClientInstance *InstancePtr, u64 NonceAddr, u64 DmeStructResAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_DME_H */
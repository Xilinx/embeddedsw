/**************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_plat.h
* @addtogroup xil_ocpapis APIs to communicate with ASUFW for OCP functionalities
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   rmv  07/17/25 Initial release
*
* </pre>
*
**************************************************************************************************/
#ifndef XOCP_PLAT_H
#define XOCP_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xil_types.h"

#ifdef PLM_OCP_ASUFW_KEY_MGMT

/********************************** Constant Definitions *****************************************/

/************************************ Type Definitions *******************************************/

/**************************** Macros (Inline Functions) Definitions ******************************/

/************************************ Function Prototypes ****************************************/
int XOcp_StoreOcpSubsysIDs(u32 SubsystemIdListLen, const u32 *SubsystemIdList);
int XOcp_StoreSubsysDigest(u32 SubsystemId, u64 Hash);
int XOcp_GetSubsysDigest(u32 SubsystemId, u32 SubsysHashAddrPtr);

/********************************** Variable Definitions *****************************************/

#ifdef __cplusplus
}
#endif

#endif /* PLM_OCP_ASUFW_KEY_MGMT */
#endif /* XOCP_PLAT_H */

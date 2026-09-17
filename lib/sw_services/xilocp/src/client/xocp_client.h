/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_client.h
*
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
*       kal  06/02/23 Added client side API for SW PCR
* 1.7   rpu  02/18/26 Refactor OCP client library
* 1.8   sri  08/24/26 Add DME and key mgmt client includes for Versal_2VP_P
*       tus  08/07/26 Added XOcp_GetClientLibVersion and
*                     XOcp_GetServerLibInfo
*       tbk  09/01/26 Include HW PCR and SW PCR headers for Versal_2VP_P
*       rpu  09/10/26 Include SW PCR, DME and key mgmt headers for Versal_2VP
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XOCP_CLIENT_H
#define XOCP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xil_cryptoalginfo.h"
#include "xocp_mailbox.h"
#include "xocp_def.h"
#include "xocp_common.h"
#if defined(VERSAL_2VE_2VM)
#include "xocp_swpcr.h"
#include "xocp_hwpcr.h"
#elif defined(VERSAL_NET) || defined(VERSAL_2VP_P)
#include "xocp_swpcr.h"
#include "xocp_hwpcr.h"
#include "xocp_dme.h"
#include "xocp_key_mgmt.h"
#elif defined(VERSAL_2VP)
#include "xocp_swpcr.h"
#include "xocp_dme.h"
#include "xocp_key_mgmt.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XOcp_ClientInit(XOcp_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);
int XOcp_GetClientLibVersion(u32 *Version);
int XOcp_GetServerLibInfo(XOcp_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_CLIENT_H */

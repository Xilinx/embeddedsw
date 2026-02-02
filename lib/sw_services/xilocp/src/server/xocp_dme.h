/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_dme.h
* @addtogroup xilocp_dme_apis XilOcp DME APIs
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*
* </pre>
*
**************************************************************************************************/
#ifndef XOCP_DME_H
#define XOCP_DME_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xocp_common.h"

#ifdef PLM_OCP

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr);
XOcp_DmeResponse* XOcp_GetDmeResponse(void);
u32 XOcp_IsDmeChlAvail(void);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_DME_H */
/** @} */

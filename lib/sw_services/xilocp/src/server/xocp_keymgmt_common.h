/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_keymgmt_common.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*       rpu  02/18/26 Fixed Doxygen warnings
* </pre>
*
**************************************************************************************************/
#ifndef XOCP_KEYMGMT_COMMON_H
#define XOCP_KEYMGMT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MGMT
#include "xsecure_hmac.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int XOcp_ValidateDiceCdi(void);
int XOcp_KeyGenDevAkSeed(u32 CdiAddr, u32 CdiLen, u32 DataAddr, u32 DataLen, XSecure_HmacRes *Out);

#endif /* PLM_OCP_KEY_MGMT */

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_KEYMGMT_COMMON_H */

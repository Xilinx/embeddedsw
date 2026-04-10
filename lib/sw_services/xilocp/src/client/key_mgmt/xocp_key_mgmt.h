/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_key_mgmt.h
*
* This file contains the client function prototypes, defines and macros for the Key Mgmt
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

#ifndef XOCP_KEY_MGMT_H
#define XOCP_KEY_MGMT_H

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
int XOcp_GetX509Cert(XOcp_ClientInstance *InstancePtr, u64 GetX509CertAddr);
int XOcp_ClientAttestWithDevAk(XOcp_ClientInstance *InstancePtr, u64 AttestWithDevAk);
int XOcp_GenSharedSecretWithDevAk(XOcp_ClientInstance *InstancePtr, const u8* PubKey, u8 *SharedSecret);
int XOcp_ClientAttestWithKeyWrapDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttnPloadAddr, u32 AttnPloadSize, u32 PubKeyOffset, u64 SignatureAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_KEY_MGMT_H */
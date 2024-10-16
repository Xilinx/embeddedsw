/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.h
*
* This file Contains the client function prototypes, defines and macros for
* the SHA-384 hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/17/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       kpt  04/28/21 Added enum XSecure_ShaState to update sha driver states
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_Sha3Kat into xsecure_katclient.c
* 5.2	mmd  07/09/23 Included header file for crypto algorithm information
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_sha_client_apis XilSecure SHA Client APIs
* @{
*/
#ifndef XSECURE_SHACLIENT_H
#define XSECURE_SHACLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_sha3alginfo.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum {
	XSECURE_SHA_UNINITIALIZED = 0,
	XSECURE_SHA_INITIALIZED,
	XSECURE_SHA_UPDATE
}XSecure_ShaState;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XSecure_Sha3Initialize(void);
int XSecure_Sha3Update(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, u32 Size);
int XSecure_Sha3Finish(XSecure_ClientInstance *InstancePtr, const u64 OutDataAddr);
int XSecure_Sha3Digest(XSecure_ClientInstance *InstancePtr, const u64 InDataAddr, const u64 OutDataAddr, u32 Size);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_SHACLIENT_H */
/** @} */

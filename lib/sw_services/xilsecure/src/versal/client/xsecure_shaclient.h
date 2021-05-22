/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.h
* @addtogroup xsecure_sha3_client_apis XilSecure SHA3 Versal Client APIs
* @{
* @cond xsecure_internal
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
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_SHACLIENT_H
#define XSECURE_SHACLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

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
int XSecure_Sha3Update(const u64 InDataAddr, u32 Size);
int XSecure_Sha3Finish(const u64 OutDataAddr);
int XSecure_Sha3Digest(const u64 InDataAddr, const u64 OutDataAddr, u32 Size);
int XSecure_Sha3Kat(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_SHACLIENT_H */

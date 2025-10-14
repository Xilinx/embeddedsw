/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.h
* @addtogroup xsecure_sha3_client_apis XilSecure SHA Versal_2Ve_2Vm Client APIs
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
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_shaalginfo.h"

/************************** Constant Definitions *****************************/
#define XSECURE_SHA_START       (0x1U)	/**< Operation flags for SHA start  */
#define XSECURE_SHA_UPDATE      (0x2U)	/**< Operation flags for SHA update */
#define XSECURE_SHA_FINISH      (0x4U)	/**< Operation flags for SHA finish */

/**************************** Type Definitions *******************************/
/** SHA modes */
typedef enum {
        XSECURE_SHA_INVALID_MODE = -1,	/**< Invalid mode */
        XSECURE_SHA3_384,	/**< SHA3-384 mode */
        XSECURE_SHA2_384,	/**< SHA2-384 mode */
        XSECURE_SHA2_256,	/**< SHA2-256 mode */
        XSECURE_SHAKE_256,	/**< SHAKE-256 mode */
        XSECURE_SHA2_512,	/**< SHA2-512 mode */
        XSECURE_SHA3_256,	/**< SHA3-256 mode */
        XSECURE_SHA3_512	/**< SHA3-512 mode */
} XSecure_ShaMode;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XSecure_Sha3Operation(XSecure_ClientInstance *InstancePtr, XSecure_ShaOpParams *Sha3Params);
int XSecure_Sha2Operation(XSecure_ClientInstance *InstancePtr, XSecure_ShaOpParams *Sha2Params);
int XSecure_Sha2Kat(XSecure_ClientInstance *InstancePtr);

/************************** Variable Definitions *****************************/

/** @endcond */
#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_SHACLIENT_H */
/** @} */
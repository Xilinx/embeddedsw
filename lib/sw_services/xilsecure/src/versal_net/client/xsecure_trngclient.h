/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trngclient.h
* @addtogroup xsecure_trng_client_apis XilSecure TRNG Versal Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the TRNG hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   am   06/13/22 Initial release
*       kpt  07/24/22 moved XSecure_TrngKat into xsecure_katclient_plat.c
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_TRNGCLIENT_H
#define XSECURE_TRNGCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"

/************************** Constant Definitions *****************************/

#define XSECURE_TRNG_SEC_STRENGTH_IN_BYTES 32U

/************************** Function Prototypes ******************************/

int XSecure_TrngGenerareRandNum(XSecure_ClientInstance *InstancePtr, u64 RandBufAddr, u32 Size);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_TRNGCLIENT_H */

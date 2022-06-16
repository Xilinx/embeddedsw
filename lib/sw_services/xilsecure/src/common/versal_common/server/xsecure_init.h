/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_init.h
 *
 * Header file for xsecure_init.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  07/01/2020 Initial release
 * 4.3   rpo  07/01/2020 Updated file version to sync with library version
 *       am   09/24/2020 Resolved MISRA C violations
 *       har  10/12/2020 Addressed security review comments
 * 4.5   bm   05/13/2021 Add common crypto instances
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XSECURE_INIT_H_
#define XSECURE_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes.h"
#include "xsecure_rsa_core.h"
#include "xsecure_sha.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_Init(void);
void XSecure_TamperInterruptHandler(const u32 ErrorNodeId, const u32 ErrorMask);
XSecure_Sha3 *XSecure_GetSha3Instance(void);
XSecure_Aes *XSecure_GetAesInstance(void);
XSecure_Rsa *XSecure_GetRsaInstance(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_INIT_H_ */

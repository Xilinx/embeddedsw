/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_tamper.h
 *
 * This file contains APIs for tamper processing routines
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  06/25/2020 Initial release
 * 4.3   rpo  06/25/2020 Updated file version to sync with library version
 *       am   09/24/2020 Resolved MISRA C violations
 *       har  10/12/2020 Addressed security review comments
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XSECURE_TAMPER_H
#define XSECURE_TAMPER_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XSecure_ProcessTamperResponse(void);
void XSecure_SecureLockDown(void);
void XSecure_EnableTamperInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_TAMPER_H */

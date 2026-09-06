/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/utils/xsecure_libver.h
*
* This file contains function prototypes and definitions for the XilSecure client-server library
* version compatibility check.
*
* The client library is built with a compile-time version (XSECURE_MAJOR_VERSION.
* XSECURE_MINOR_VERSION from xsecure_ver.h). The server (PLM) is built with its own version.
* When the two binaries are produced/updated independently, their versions may diverge. These
* APIs let a user application query the server library version and verify that the client it is
* linked against is compatible with the running server before issuing crypto requests.
*
* Compatibility rule:
*  - Major version MUST match between client and server (a major bump indicates a breaking change).
*  - Client minor version MUST be less than or equal to the server minor version. A newer server
*    stays backward compatible with an older client (an older client only uses features the newer
*    server still supports), so client minor < server minor is acceptable; only client minor >
*    server minor is reported as a minor mismatch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.8   tus     08/19/26 Initial release with XSecure_GetLibInfo() and capability
*                        query helper
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_LIBVER_H
#define XSECURE_LIBVER_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xil_cryptoalginfo.h"
#include "xsecure_defs.h"
#include "xsecure_mailbox.h"

/************************************** Constant Definitions **************************************/
/*
 * Return codes for XSecure_VerifyLibCompatibility().
 *
 * On success it returns XST_SUCCESS (from xstatus.h); on a version mismatch it returns one of the
 * dedicated codes below so the caller can tell the negative scenarios apart from a single return
 * value (no separate out-parameter needed). Any other value is a transport/parameter error
 * propagated from the underlying server query (XST_INVALID_PARAM, XST_FAILURE, etc.).
 *
 * The codes use a distinct high base (0x2000) so they never collide with the low-numbered standard
 * codes in xstatus.h (XST_FAILURE = 1, XST_INVALID_VERSION = 4, ...) or with the server error enum.
 */
#define XSECURE_LIB_VERSION_MAJOR_MISMATCH	(0x2000)	/**< Major versions differ - breaking change */
#define XSECURE_LIB_VERSION_MINOR_MISMATCH	(0x2001)	/**< Client minor newer than server */

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/
/** Extract the major version field from a packed XIL_BUILD_VERSION word */
#define XSECURE_GET_MAJOR_VERSION(Ver)	(((u32)(Ver) >> 16U) & 0xFFFFU)
/** Extract the minor version field from a packed XIL_BUILD_VERSION word */
#define XSECURE_GET_MINOR_VERSION(Ver)	((u32)(Ver) & 0xFFFFU)

/*************************************** Function Prototypes **************************************/
int XSecure_GetClientLibVersion(u32 *Version);
int XSecure_GetServerLibVersion(XSecure_ClientInstance *InstancePtr, u32 *Version);
int XSecure_VerifyLibCompatibility(XSecure_ClientInstance *InstancePtr);
int XSecure_GetServerLibInfo(XSecure_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo);
u32 XSecure_IsCapabilitySupported(const Xil_LibInfo *LibInfo, u32 CapBit);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LIBVER_H */

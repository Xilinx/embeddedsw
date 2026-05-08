/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/lms_hss/xsecure_lmsclient.h
*
* This file contains the function prototypes for LMS client interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  05/03/26 Initial release
*
* </pre>
*
***************************************************************************************************/
#ifndef XSECURE_LMSCLIENT_H
#define XSECURE_LMSCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_plat_defs.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/************************************ Function Prototypes *****************************************/

/** @addtogroup xsecure_lms_client_apis XilSecure LMS Client APIs
 * @{
 */

int XSecure_LmsSignVerify(XSecure_ClientInstance *InstancePtr,
			 const XSecure_LmsParams *ParamsPtr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMSCLIENT_H */

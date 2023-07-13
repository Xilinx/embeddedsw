/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_keyunwrap.h
* @addtogroup xsecure_keyunwrap_apis XilSecure Key Unwrap APIs
* @{
* @cond xsecure_internal
* This file contains APIs related to key unwrap
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt  06/30/23 Initial release
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_KEY_UNWRAP_H_
#define XSECURE_KEY_UNWRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PLM_RSA_EXCLUDE

#include "xpmcdma.h"
#include "xsecure_plat_defs.h"

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/***************************** Function Prototypes ***************************/

int XSecure_KeyUnwrap(XSecure_KeyWrapData *KeyWrapData, XPmcDma *DmaPtr);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KEY_UNWRAP_H_ */
/* @} */

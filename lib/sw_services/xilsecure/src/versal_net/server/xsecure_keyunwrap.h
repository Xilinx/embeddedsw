/******************************************************************************
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_keyunwrap.h
*
* This file contains APIs related to key unwrap
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt  06/30/23 Initial release
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_keyunwrap_server_apis XilSecure Key Unwrap Server APIs
* @{
*/
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

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
*       kpt  06/13/24 Added AES key unwrap with padding support.
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

#ifndef XSECURE_KEY_SLOT_ADDR
#define XSECURE_KEY_STORE_ADDR				   (0x00000000U) /**< Key store address */
#else
#define XSECURE_KEY_STORE_ADDR                 (XSECURE_KEY_SLOT_ADDR)  /**< Key store address */
#endif

/***************************** Type Definitions ******************************/

/***************************** Function Prototypes ***************************/

int XSecure_KeyUnwrap(XSecure_KeyWrapData *KeyWrapData);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KEY_UNWRAP_H_ */
/** @} */

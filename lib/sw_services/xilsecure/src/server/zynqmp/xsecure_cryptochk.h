/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file server/zynqmp/xsecure_cryptochk.h
*
* This file contains macros and functions common to AES, SHA and RSA
* for Zynqmp
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.6   kal     08/18/21 Initial Release
* 4.7   am      11/26/21 Resolved doxygen warning
*
* </pre>
*
*******************************************************************************/
/**
 * @addtogroup xsecure_cryptochk_apis XilSecure Crypto Check APIs
 * @{
 */
#ifndef XSECURE_CRYPTOCHK_H
#define XSECURE_CRYPTOCHK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions ****************************/

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSECURE_CRYPTO_DISABLED_ERROR		(0x40U) /**< Crypto disabled
                                                         * error */

/*****************************************************************************/

/************************** Function Prototypes ******************************/
u32 XSecure_CryptoCheck(void);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_CRYPTOCHK_H_ */

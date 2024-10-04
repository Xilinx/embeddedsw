/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticcrvs.h
*
* This file contains the macros and types related to elliptic curve information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  08/24/20 First Release
* 4.3   har  08/24/20 Updated file version to sync with library version
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       har  10/14/20 Replaced ecdsa with elliptic in names of function and
*                     macros
* 4.6   har  07/14/21 Fixed doxygen warnings
* 5.2   yog  06/07/23 Added support for P-256 Curve
*       har  07/31/23 Redefined XSecure_EllipticCrvClass enum and moved to xsecure_defs.h
*       yog  02/23/24 Removed XSECURE_ECC_SUPPORT_NIST_P521 and XSECURE_ECC_SUPPORT_NIST_P256
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
#ifndef XSECURE_ELLIPTICCRVS_H_
#define XSECURE_ELLIPTICCRVS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xil_types.h"
#include <Ecdsa.h>
#include "xsecure_defs.h"

/************************** Constant Definitions ****************************/
/**
 * @name Supported ECC curves
 * @{
 */
/**< Macros to enable /disable support of NIST P-384 and NIST P-521 curve */
#define XSECURE_ECC_SUPPORT_NIST_P384

/** @} */

/***************************** Type Definitions ******************************/
extern EcdsaCrvInfo XSecure_EllipticCrvsDb[];

/***************************** Function Prototypes ***************************/
u32 XSecure_EllipticCrvsGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTICCRVS_H */

#endif
/** @} */

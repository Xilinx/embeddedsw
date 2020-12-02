/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XSECURE_ELLIPTICCRVS_H_
#define XSECURE_ELLIPTICCRVS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "Ecdsa.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECC_SUPPORT_NIST_P384
#define XSECURE_ECC_SUPPORT_NIST_P521

/***************************** Type Definitions ******************************/
typedef enum {
	XSECURE_ECC_NIST_P384 = 4,
	XSECURE_ECC_NIST_P521 = 5
} XSecure_EllipticCrvTyp;

typedef enum {
	XSECURE_ECC_PRIME = 0,
	XSECURE_ECC_BINARY = 1,
} XSecure_EllipticCrvClass;

extern EcdsaCrvInfo XSecure_EllipticCrvsDb[];

/***************************** Function Prototypes ***************************/
u32 XSecure_EllipticCrvsGetCount(void);

#ifdef __cplusplus
}
#endif /* XSECURE_ELLIPTICCRVS_H */

#endif

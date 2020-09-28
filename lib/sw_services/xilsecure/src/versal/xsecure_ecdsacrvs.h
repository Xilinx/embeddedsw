/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ecdsacrvs.h
*
* This file contains the macros and types related to ECDSA curve information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  08/24/20 First Release
* 4.3   har  08/24/20 Updated file version to sync with library version
*		am	 09/24/20 Resolved MISRA C violations
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XSECURE_ECDSACRVS_H_
#define XSECURE_ECDSACRVS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "Ecdsa.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECDSA_SUPPORT_NIST_P384
#define XSECURE_ECDSA_SUPPORT_NIST_P521

/***************************** Type Definitions ******************************/
typedef enum {
	XSECURE_ECDSA_NIST_P384 = 4,
	XSECURE_ECDSA_NIST_P521 = 5
} XSecure_EcdsaCrvTyp;

typedef enum {
	XSECURE_ECDSA_PRIME = 0,
	XSECURE_ECDSA_BINARY = 1,
} XSecure_EcdsaCrvClass;

extern EcdsaCrvInfo XSecure_EcdsaCrvsDb[];

/***************************** Function Prototypes ***************************/
u32 XSecure_EcdsaCrvsGetCount(void);

#ifdef __cplusplus
}
#endif /* XSECURE_ECDSACRVS_H */

#endif
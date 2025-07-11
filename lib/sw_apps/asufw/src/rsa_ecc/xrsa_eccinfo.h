/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xrsa_eccinfo.h
*
* This file contains the macros and types related to elliptic curve information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  07/11/24 Initial release
*       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
*       ss   12/02/24 Added support for NIST curves P-256,P-384
*       yog  07/11/25 Added support for Edward curves.
*
* </pre>
*
* @note
*
***************************************************************************************************/
/**
* @addtogroup xrsa_ecc_server_apis RSA ECC Server APIs
* @{
*/
#ifndef XRSA_ECCINFO_H_
#define XRSA_ECCINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "Ecdsa.h"

/************************************ Constant Definitions ***************************************/

/** This typedef is used to know the prime and binary curve values. */
typedef enum {
	XRSA_ECC_PRIME = 0,		/**< Prime curve value in Ecdsa.h */
	XRSA_ECC_BINARY = 1,		/**< Binary curve value in Ecdsa.h */
	XRSA_ECC_ED_PH = 2,		/**< EdDSA PH curve value in Ecdsa.h */
} XRsa_EccCrvClass;

/************************************** Type Definitions *****************************************/
extern EcdsaCrvInfo XRsa_EccCrvsDb[];

/************************************* Function Prototypes ***************************************/
u32 XRsa_EccCrvsGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* XRSA_ECCINFO_H_ */
/** @} */

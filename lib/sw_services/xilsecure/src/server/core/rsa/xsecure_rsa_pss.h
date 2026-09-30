/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/rsa/xsecure_rsa_pss.h
* This file contains RSA-PSS (SHA3-384) signature verification server APIs.
*
******************************************************************************/

/**
 * @addtogroup xsecure_rsa_pss_server_apis XilSecure RSA PSS server APIs
 * @{
 */
#ifndef XSECURE_RSA_PSS_H_
#define XSECURE_RSA_PSS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PLM_RSA_EXCLUDE

/***************************** Include Files *********************************/
#include "xsecure_defs.h"
#include "xsecure_rsa_core.h"
#include "xsecure_sha.h"

/************************** Constant Definitions ****************************/
#define XSECURE_RSA_PSS_SIGNED_INPUT_DATA_LEN	(512U)
	/**< Signed input / EM workspace length in bytes */
#define XSECURE_RSA_PSS_DATA_BLOCK_LEN		(480U)
	/**< Data block workspace length in bytes */

/***************************** Type Definitions ******************************/
/**
 * The RSA-PSS sign verification input structure. A pointer to this structure
 * is passed to XSecure_RsaPssSignVerify().
 *
 * The two workspaces are caller supplied so that a caller which already owns a
 * suitable scratch region, such as the PMC chunk memory used by XilLoader, does
 * not pay for a second copy.
 */
typedef struct {
	u64 MsgHashAddr; /**< 64-bit address of the SHA3-384 message hash */
	u64 SignAddr; /**< 64-bit address of the RSA signature */
	u32 KeySize; /**< RSA key size in bytes. RsaInstPtr->SizeInWords is fixed
		      *  to 4096 bits at initialization, so it does not carry this */
	XSecure_Rsa *RsaInstPtr; /**< Pointer to initialized RSA instance */
	XSecure_Sha *ShaInstPtr; /**< Pointer to SHA3 instance */
	u8 *SignedInputData; /**< Encoded message (EM) workspace of at least
			      *  KeySize bytes */
	u8 *DataBlock; /**< Data block (DB) workspace of at least KeySize - 49 bytes,
			*  up to XSECURE_RSA_PSS_DATA_BLOCK_LEN */
} XSecure_RsaPssSignVerifyInput;

/***************************** Function Prototypes ***************************/

/* RSA-PSS sign verification */
int XSecure_RsaPssSignVerify(const XSecure_RsaPssSignVerifyInput *Input);

#endif /* PLM_RSA_EXCLUDE */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XSECURE_RSA_PSS_H_ */

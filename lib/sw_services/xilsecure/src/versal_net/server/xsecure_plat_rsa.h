/******************************************************************************
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.h
* @addtogroup xsecure_plat_rsa_apis XilSecure Platform RSA APIs
* @{
* @cond xsecure_internal
* This file contains hardware interface related information for RSA device
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt  06/25/23 Initial release
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_PLAT_RSA_H_
#define XSECURE_PLAT_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PLM_RSA_EXCLUDE

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xsecure_rsa_core.h"
#include "xsecure_mgf.h"

/************************** Constant Definitions ****************************/

#ifndef XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES
#define XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES (XSECURE_RSA_3072_SIZE_WORDS * 4U) /**< RSA default key size in bytes */
#endif

/***************************** Type Definitions ******************************/

typedef struct {
	u64 InputDataAddr;       /**< Input data address */
	u64 OutputDataAddr;      /**< Output data address */
	u64 OptionalLabelAddr;   /**< Optional label address */
	u32 InputDataSize;       /**< Input data size */
	u32 OutputDataSize;      /**< Output data size */
	u32 OptionalLabelSize;    /**< Optional label size */
	void *ShaInstancePtr;    /**< SHA instance for MGF */
	XSecure_ShaType ShaType; /**< SHA type for MGF */
} XSecure_RsaOaepParam;

typedef struct {
	u8 *Modulus;  /**< Modulus */
	u8 *ModExt;   /**< Modulus extension */
	u8 *Exponent; /**< Exponent */
} XSecure_RsaKey;

/***************************** Function Prototypes ***************************/

int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam);
int XSecure_RsaOaepDecrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam);
XSecure_RsaKey *XSecure_GetRsaPrivateKey(void);
XSecure_RsaKey *XSecure_GetRsaPublicKey(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_RSA_H_ */
/* @} */

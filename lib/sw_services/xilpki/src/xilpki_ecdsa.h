/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilpki_ecdsa.h
 * @addtogroup xilpki-api XilPKI Library APIs
 *
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  -------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 * 2.0   Nava  06/21/23  Added PKI multi-queue support for ECC operations.
 * 2.0   Nava  09/11/23  Fixed doxygen warnings.
 *
 * </pre>
 *
 *
 ******************************************************************************/
#ifndef XILPKI_ECDSA_H
#define XILPKI_ECDSA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xilpki.h"

/************************** Constant Definitions *****************************/
#define NIST_P192_LEN_BYTES	24U /**< Bytes */
#define NIST_P384_LEN_BYTES	48U /**< Bytes */
#define NIST_P256_LEN_BYTES	32U /**< Bytes */
#define NIST_P521_LEN_BYTES	66U /**< Bytes */

/*
@cond internal
*/
#define XPKI_ECDSA_NON_SUPPORTED_CRV		0x1U	/**< Unsupported curve */
#define XPKI_ECDSA_INVALID_PARAM		0x2U	/**< Invalid parameters */
#define XPKI_ECDSA_SIGN_GEN_ERR			0x3U	/**< Ecdsa signature gneration error */
#define XPKI_ECDSA_SIGN_VERIFY_ERR		0x4U	/**< Ecdsa signature verification error */
#define XPKI_ECDSA_POINT_MULT_ERR		0x5U	/**< Ecc point multiplication error */
#define XPKI_MOD_ADD_ERR			0x6U	/**< Modulus Addtion error */
#define XPKI_DONE_STATUS_ERR			0x7U	/**< PKI done status error */
/** @endcond*/
/**************************** Type Definitions *******************************/

typedef enum {
	ECC_NIST_P192 = 1,	/**< NIST P-192 curve */
	ECC_NIST_P256,	/**< NIST P-256 curve */
	ECC_NIST_P384,	/**< NIST P-384 curve */
	ECC_NIST_P521	/**< NIST P-251 curve */
} XPki_EcdsaCrvType;

typedef enum {
	ECC_BINARY = 1,		/**< ECC Binary curve */
	ECC_PRIME		/**< ECC Prime curve */
} XPki_EcdsaCrv;

typedef struct {
	XPki_EcdsaCrvType CrvType;	/**< Curve type(NIST-P192, P256, P384, and P521) */
	XPki_EcdsaCrv Crv;	/**< Type of ECC curve class either prime or binary class */
} XPki_EcdsaCrvInfo;

typedef struct {
	const u8 *Qx;	/**< ECC Public key curve point x */
	const u8 *Qy;	/**< ECC Public key curve point y */
} XPki_EcdsaKey;

typedef struct {
	u8 *SignR;	/**< The signature component R */
	u8 *SignS;	/**< The signature component S */
} XPki_EcdsaSign;

typedef struct {
	const u8 *Gx;	/**< ECC Curve Generator point x */
	const u8 *Gy;	/**< ECC Curve Generator curve point y */
	u8 Gxlen;    /**< Length of Generator point x */
	u8 Gylen;     /**< Length of Generator point y */
} XPki_EcdsaGpoint;

typedef struct {
	XPki_EcdsaCrvType CrvType; /**< ECC Curve type */
	u8 Hashlen;		 /**< Data hash length */
	u8 Dlen;		/**< ECC Private key length */
	u8 Klen;		/**< ECC ephemeral key lenth */
	const u8 *Hash;		/**< Pointer to the data hash */
	const u8 *D;		/**< Pointer to the private key */
	const u8 *K;		/**< Pointer to the ephemeral key */
} XPki_EcdsaSignInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;	/**< ECC Curve type */
	XPki_EcdsaKey PubKey;		/**< Structure to the ECC public key */
	XPki_EcdsaSign Sign;		/**< Structure to the ECC signature */
	u8 Hashlen;			/**< Data hash length */
	const u8 *Hash;			/**< Pointer to the Data Hash */
} XPki_EcdsaVerifyInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;	/**< ECC Curve type */
	XPki_EcdsaGpoint Gpoint;	/**< Structure to the ECC genetator points */
	u8 Dlen;			/**< ECC Private key length */
	const u8 *D;			/**< Pointer to the private key */
} XPki_EcdsaPointMultiInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;	/**< ECC Curve type */
	const u8 *Order;		/**< Pointer to the ECC Curve Order */
	u8 Dlen;			/**< ECC Private key length */
	const u8 *D;			/**< Pointer to the private key */
} XPki_ModAddInputData;

#ifdef __cplusplus
}
#endif

#endif  /* XILPKI_ECDSA_H */
/**@}*/

/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilpki_ecdsa.h
 * @addtogroup xilpki Overview
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
 *
 * </pre>
 *
 * @note
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
#define NIST_P384_LEN_BYTES	48U /* Bytes */
#define NIST_P256_LEN_BYTES	32U /* Bytes */
#define NIST_P521_LEN_BYTES	66U /* Bytes */

#define XPKI_ECDSA_NON_SUPPORTED_CRV		0x1U
#define XPKI_ECDSA_INVALID_PARAM		0x2U
#define XPKI_ECDSA_SIGN_GEN_ERR			0x3U
#define XPKI_ECDSA_SIGN_VERIFY_ERR		0x4U
#define XPKI_ECDSA_POINT_MULT_ERR		0x5U
#define XPKI_MOD_ADD_ERR			0x6U
#define XPKI_DONE_STATUS_ERR			0x7U
/**************************** Type Definitions *******************************/

typedef enum {
	ECC_NIST_P256 = 1,
	ECC_NIST_P384,
	ECC_NIST_P521
} XPki_EcdsaCrvType;

typedef enum {
	ECC_BINARY = 1,
	ECC_PRIME
} XPki_EcdsaCrv;

typedef struct {
	XPki_EcdsaCrvType CrvType;
	XPki_EcdsaCrv Crv;
} XPki_EcdsaCrvInfo;

typedef struct {
	const u8 *Qx;	/* Public key curve point x */
	const u8 *Qy;	/* Public key curve point y */
} XPki_EcdsaKey;

typedef struct {
	u8 *SignR;	/* The signature component R */
	u8 *SignS;	/* The signature component S */
} XPki_EcdsaSign;

typedef struct {
	const u8 *Gx;	/* Generator curve point x */
	const u8 *Gy;	/* Generator curve point y */
	u8 Gxlen;
	u8 Gylen;
} XPki_EcdsaGpoint;

typedef struct {
	XPki_EcdsaCrvType CrvType;
	u8 Hashlen;
	u8 Dlen;
	u8 Klen;
	const u8 *Hash;
	const u8 *D;
	const u8 *K;
} XPki_EcdsaSignInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;
	XPki_EcdsaKey PubKey;
	XPki_EcdsaSign Sign;
	u8 Hashlen;
	const u8 *Hash;
} XPki_EcdsaVerifyInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;
	XPki_EcdsaGpoint Gpoint;
	u8 Dlen;
	const u8 *D;
} XPki_EcdsaPointMultiInputData;

typedef struct {
	XPki_EcdsaCrvType CrvType;
	const u8 *Order;
	u8 Dlen;
	const u8 *D;
} XPki_ModAddInputData;

#endif  /* XILPKI_ECDSA_H */

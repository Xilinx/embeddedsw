/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilpki_kat.c
 * @addtogroup xilpki-api XilPKI Library APIs
 * This file contains the definitions of known answer test(KAT) functions.
 *
 * @{
 * @details
 *
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  ------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 * 2.0   Nava  06/21/23  Added PKI multi-queue support for ECC operations.
 * 2.0   Nava  09/11/23  Fixed doxygen warnings.
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilpki.h"

/***************** Macros (Inline Functions) Definitions ********************/
/*
@cond internal
*/
#define ECC_MAX_BUF_LEN_BYTES		66U
#define XPKI_REQ_SUBMITTED		0U
#define XPKI_REQ_COMPLETED		1U
#define XPKI_MAX_WAIT_COUNT		1000000U

/************************** Function Prototypes ******************************/
static void XPki_SignGenearte_CompletionCallBack(u32 RequestID, u32 Status);
static void XPki_SignVerify_CompletionCallBack(u32 RequestID, u32 Status);
static void XPki_PrivKeyGen_CompletionCallBack(u32 RequestID, u32 Status);
static void XPki_PubKeyGen_CompletionCallBack(u32 RequestID, u32 Status);
/************************** Variable Definitions *****************************/
/** Public key used for signature verification */
static const u8 PubkeyQx_P192[] = {
	0xA2, 0xA9, 0x06, 0xF6, 0x63, 0x21, 0xC6, 0xBC,
	0xBA, 0x87, 0x12, 0x0A, 0x5A, 0xCD, 0xB8, 0x4E,
	0x50, 0x4B, 0x88, 0x47, 0xC6, 0xAA, 0xA2, 0xFB
};

static const u8 PubkeyQy_P192[] = {
	0xA5, 0x4A, 0xD8, 0x50, 0x88, 0xD9, 0x65, 0x48,
	0xEF, 0xC8, 0xC9, 0x71, 0x8B, 0xE3, 0x9E, 0xD7,
	0x27, 0x4F, 0xEF, 0x05, 0xCC, 0xD4, 0xE6, 0xDA
};

static const u8 PubkeyQx_P256[] = {
	0x19, 0x7A, 0xBC, 0x89, 0x08, 0xCD, 0x01, 0x82,
	0xA3, 0xA2, 0x9E, 0x1E, 0xAD, 0xA0, 0xB3, 0x62,
	0x1C, 0xBA, 0x98, 0x47, 0x73, 0x8C, 0xDC, 0xF1,
	0xD3, 0xBA, 0x94, 0xFE, 0xFD, 0x8A, 0xE0, 0xB7
};

static const u8 PubkeyQy_P256[] = {
	0x09, 0x5E, 0xCC, 0x06, 0xC6, 0xBB, 0x63, 0xB5,
	0x61, 0x9E, 0x52, 0x43, 0xAE, 0xC7, 0xAD, 0x63,
	0x90, 0x72, 0x28, 0x19, 0xE4, 0x26, 0xB2, 0x4B,
	0x7A, 0xBF, 0x9D, 0x95, 0x47, 0xF7, 0x03, 0x36
};

static const u8 PubkeyQx_P384[] = {
	0xB5, 0x5C, 0x0F, 0xEF, 0xD9, 0x89, 0x64, 0x08,
	0xF9, 0x82, 0x7B, 0x65, 0x74, 0xAB, 0xD2, 0xB9,
	0x1F, 0x47, 0xE6, 0x61, 0x3C, 0x24, 0x71, 0x50,
	0x36, 0x87, 0xB4, 0xF2, 0x90, 0x43, 0x56, 0xF2,
	0x26, 0x91, 0xF0, 0x43, 0x53, 0x45, 0xF1, 0xD5,
	0xB4, 0x36, 0x9D, 0x9E, 0xBC, 0x01, 0xF7, 0x3B
};

static const u8 PubkeyQy_P384[] = {
	0x55, 0xB8, 0x8D, 0xB6, 0xB5, 0xA9, 0x44, 0x03,
	0xA3, 0xCD, 0xCF, 0x00, 0x2D, 0x38, 0xE5, 0xFF,
	0x95, 0x62, 0x2A, 0x55, 0x83, 0x55, 0x32, 0x99,
	0x31, 0x44, 0x01, 0x51, 0x7A, 0x13, 0x5B, 0xF7,
	0x6F, 0xAA, 0xBD, 0xCC, 0x55, 0x38, 0x53, 0x8D,
	0xE6, 0x52, 0xF9, 0xFB, 0xEA, 0x58, 0xA3, 0xD1
};

static const u8 PubkeyQx_P521[] = {
	0xC4, 0xD5, 0x85, 0x18, 0x17, 0x8B, 0xF1, 0x8D,
	0xB6, 0xFE, 0xEC, 0x0D, 0x03, 0xAC, 0xD8, 0x05,
	0x30, 0x4B, 0xE5, 0xB1, 0x56, 0x70, 0xA3, 0x67,
	0xA4, 0x6C, 0xDC, 0x6B, 0x3A, 0x40, 0xDF, 0x59,
	0x6E, 0xA1, 0xCC, 0x10, 0x64, 0x8F, 0xAB, 0xE9,
	0x55, 0xB2, 0x96, 0xD7, 0x8E, 0xDA, 0xC1, 0x17,
	0xF1, 0xF5, 0x53, 0xB4, 0xFA, 0x52, 0x9C, 0x30,
	0x22, 0x28, 0x45, 0x68, 0x9A, 0xEF, 0x1E, 0xE9,
	0x98, 0x00
};

static const u8 PubkeyQy_P521[] = {
	0x2E, 0xBC, 0x46, 0xD9, 0x50, 0xFD, 0x32, 0x46,
	0x9A, 0x99, 0x92, 0xF0, 0xF5, 0x13, 0xE3, 0x26,
	0x15, 0x23, 0x27, 0x83, 0x66, 0xAD, 0x83, 0x4B,
	0x2C, 0x71, 0x00, 0x29, 0xB7, 0x76, 0x43, 0x55,
	0xE8, 0x7B, 0xF1, 0x5E, 0x98, 0x31, 0x7F, 0x8E,
	0xD2, 0xD7, 0x48, 0x6A, 0x8D, 0xB7, 0xB4, 0x50,
	0x61, 0x65, 0x15, 0x9B, 0x4C, 0x36, 0xA4, 0x1B,
	0xCA, 0x1C, 0xFC, 0xEC, 0x1A, 0x32, 0x0C, 0x35,
	0x64, 0x01
};

/* Signature */
static const u8 SignR_P192[] = {
	0x21, 0x28, 0x80, 0xB9, 0x4F, 0xD0, 0x81, 0x4D,
	0xA5, 0x7D, 0x8B, 0x2A, 0x8E, 0xA1, 0xC5, 0x9C,
	0x39, 0xDE, 0x8C, 0xB8, 0x72, 0xBA, 0xEC, 0xF0
};

static const u8 SignS_P192[] = {
	0xED, 0xE8, 0x21, 0x6D, 0x0B, 0x14, 0xFA, 0x54,
	0xF8, 0x00, 0xBF, 0xDA, 0xF5, 0x40, 0x20, 0xBD,
	0xB2, 0xFA, 0xB1, 0xE2, 0x4A, 0x3D, 0x6D, 0x1E
};

static const u8 SignR_P256[] = {
	0x4F, 0x10, 0x46, 0xCA, 0x9A, 0xB6, 0x25, 0x73,
	0xF5, 0x3E, 0x0B, 0x1F, 0x6F, 0x31, 0x4C, 0xE4,
	0x81, 0x0F, 0x50, 0xB1, 0xF3, 0xD1, 0x65, 0xFF,
	0x65, 0x41, 0x7F, 0xD0, 0x76, 0xF5, 0x42, 0x2B
};

static const u8 SignS_P256[] = {
	0xF1, 0xFA, 0x63, 0x6B, 0xDB, 0x9B, 0x32, 0x4B,
	0x2C, 0x26, 0x9D, 0xE6, 0x6F, 0x88, 0xC1, 0x98,
	0x81, 0x2A, 0x50, 0x89, 0x3A, 0x99, 0x3A, 0x3E,
	0xCD, 0x92, 0x63, 0x2D, 0x12, 0xC2, 0x42, 0xDC
};

static const u8 SignR_P384[] = {
	0x88, 0x70, 0xA2, 0x01, 0xAF, 0x9F, 0xEF, 0xFC,
	0x07, 0xD5, 0xC7, 0x2C, 0xD0, 0x95, 0x27, 0x33,
	0x1A, 0xD4, 0x7D, 0xB5, 0xD9, 0x40, 0xD0, 0x13,
	0xB3, 0xA3, 0x0E, 0xB4, 0xA3, 0x66, 0x9F, 0xDA,
	0xCA, 0xC7, 0x13, 0x81, 0x06, 0x6F, 0x75, 0x08,
	0x82, 0x8D, 0xD3, 0xC0, 0x4F, 0x51, 0xEA, 0x30
};

static const u8 SignS_P384[] = {
	0xD8, 0x3A, 0x68, 0x49, 0xD5, 0x46, 0x1C, 0x82,
	0xFB, 0x0B, 0x62, 0xBE, 0x5D, 0x7D, 0x1F, 0xFD,
	0x4E, 0xB9, 0xB7, 0xFB, 0x75, 0x68, 0x47, 0xC4,
	0x66, 0xAA, 0xFC, 0xD6, 0x22, 0x39, 0xA4, 0x67,
	0xF0, 0xAD, 0x78, 0xBF, 0xBC, 0x27, 0x90, 0x6C,
	0xF4, 0x14, 0xE4, 0x4B, 0x50, 0x8E, 0x80, 0xCC
};

static const u8 SignR_P521[] = {
	0xBA, 0xE2, 0xC3, 0x5D, 0xFE, 0xCC, 0x54, 0x92,
	0x1A, 0x64, 0x96, 0x9B, 0x7C, 0xC3, 0x91, 0x28,
	0x73, 0x8A, 0x4A, 0x2B, 0x6E, 0xD5, 0xA9, 0x32,
	0x04, 0x8C, 0x2F, 0xF3, 0xEE, 0xEE, 0x26, 0x25,
	0x1C, 0xCD, 0xDA, 0x9A, 0x26, 0x79, 0xDC, 0x8F,
	0x55, 0x1D, 0xFC, 0x51, 0x24, 0xE6, 0x2D, 0x1E,
	0xD8, 0x74, 0xAD, 0xD3, 0xDD, 0x40, 0xA2, 0xE7,
	0xF7, 0xE3, 0x8C, 0x10, 0x57, 0xCA, 0xED, 0xC8,
	0x40, 0x01
};

static const u8 SignS_P521[] = {
	0xBF, 0xAD, 0x27, 0xE6, 0xB6, 0x21, 0xF9, 0x1E,
	0x24, 0x7F, 0xF8, 0x56, 0xFA, 0xFC, 0x27, 0xC6,
	0x66, 0xD3, 0x02, 0xA3, 0x67, 0x19, 0xC6, 0x4F,
	0xC8, 0x46, 0x5F, 0xEC, 0x69, 0xEC, 0x03, 0xC8,
	0xD8, 0x00, 0xE3, 0x98, 0x00, 0x1C, 0x1B, 0x8E,
	0x39, 0xD5, 0x95, 0x85, 0x61, 0x7C, 0xCA, 0x71,
	0xB7, 0x0D, 0xD2, 0x0E, 0x44, 0xBF, 0xD7, 0xEB,
	0xED, 0x08, 0xE8, 0x58, 0x2D, 0x49, 0x88, 0x51,
	0xB2, 0x00
};

/* D: Private key used for signature generation */
static const u8 D_P192[] = {
	0x5B, 0x46, 0xA7, 0x23, 0x99, 0x50, 0xD2, 0x64, 0xE5,
	0xCC, 0x47, 0x1F, 0x4B, 0xB4, 0x36, 0xF6, 0x57, 0x80,
	0xFD, 0x32, 0x60, 0x68, 0x91, 0x78
};

static const u8 D_P256[] = {
	0x96, 0xBF, 0x85, 0x49, 0xC3, 0x79, 0xE4, 0x04, 0xED,
	0xA1, 0x08, 0xA5, 0x51, 0xF8, 0x36, 0x23, 0x12, 0xD8,
	0xD1, 0xB2, 0xA5, 0xFA, 0x57, 0x06, 0xE2, 0xCC, 0x22,
	0x5C, 0xF6, 0xF9, 0x77, 0xC4
};

static const u8 D_P384[] = {
	0x08, 0x8A, 0x3F, 0xD8, 0x57, 0x4B, 0x22, 0xD1, 0x14,
	0x97, 0x6B, 0x5E, 0x56, 0xA8, 0x93, 0xE3, 0x0A, 0x6A,
	0x2E, 0x39, 0xFC, 0x3D, 0xE7, 0x55, 0x04, 0xCB, 0x6A,
	0xFC, 0x4A, 0xAE, 0xFA, 0xB4, 0xE3, 0xA3, 0xE3, 0x6C,
	0x1C, 0x4B, 0x58, 0xC0, 0x48, 0x4B, 0x9E, 0x62, 0xED,
	0x02, 0x2C, 0xF9
};

static const u8 D_P521[] = {
	0x22, 0x17, 0x96, 0x4F, 0xB2, 0x14, 0x35, 0x33, 0xBA,
	0x93, 0xAA, 0x35, 0xFE, 0x09, 0x37, 0xA6, 0x69, 0x5E,
	0x20, 0x87, 0x27, 0x07, 0x06, 0x44, 0x99, 0x21, 0x7C,
	0x5F, 0x6A, 0xB8, 0x09, 0xDF, 0xEE, 0x4E, 0x18, 0xDC,
	0x78, 0x14, 0xBA, 0x5B, 0xB4, 0x55, 0x19, 0x50, 0x98,
	0xFC, 0x4B, 0x30, 0x8E, 0x88, 0xB2, 0xC0, 0x28, 0x30,
	0xB3, 0x7E, 0x1B, 0xB1, 0xB8, 0xE1, 0xB8, 0x47, 0x5F,
	0x08, 0x00, 0x01
};

/* K: Ehimeral key used for signature generation */
static const u8 K_P192[] = {
	0x47, 0xD8, 0x69, 0x0A, 0xF8, 0xC0, 0xA9, 0xDE, 0xEE,
	0x6D, 0x6B, 0xA0, 0x8A, 0xF0, 0x44, 0x07, 0x8B, 0x70,
	0x2F, 0xEF, 0xA0, 0xB0, 0x6C, 0xD0
};

static const u8 K_P256[] = {
	0xAE, 0x50, 0xEE, 0xFA, 0x27, 0xB4, 0xDB, 0x14, 0x9F,
	0xE1, 0xFB, 0x04, 0xF2, 0x4B, 0x50, 0x58, 0x91, 0xE3,
	0xAC, 0x4D, 0x2A, 0x5D, 0x43, 0xAA, 0xCA, 0xC8, 0x7F,
	0x79, 0x52, 0x7E, 0x1A, 0x7A
};

static const u8 K_P384[] = {
	0xEF, 0x3F, 0xF4, 0xC2, 0x6C, 0xE0, 0xCA, 0xED, 0x85,
	0x3F, 0xC4, 0x9F, 0x74, 0xE0, 0x78, 0x08, 0x68, 0x37,
	0x01, 0x4F, 0x05, 0x5F, 0xD9, 0x2E, 0x9E, 0x74, 0x01,
	0x47, 0x53, 0x9B, 0x45, 0x2A, 0x84, 0xA7, 0xC6, 0x1E,
	0xA8, 0xDD, 0xE3, 0x94, 0x83, 0xEA, 0x0B, 0x8C, 0x1F,
	0xEF, 0x44, 0x2E
};

static const u8 K_P521[] = {
	0xBF, 0xD6, 0x31, 0xA2, 0xA6, 0x47, 0x31, 0x70, 0xB8,
	0x16, 0x6D, 0x33, 0x25, 0x06, 0xBE, 0x62, 0xE5, 0x48,
	0x5A, 0xD0, 0xBE, 0x76, 0xBA, 0x74, 0xA1, 0x09, 0x7C,
	0x59, 0x5F, 0x57, 0x70, 0xCD, 0xCE, 0x70, 0xE3, 0x63,
	0x7D, 0x2F, 0x17, 0xBA, 0x52, 0xB4, 0xEA, 0xCD, 0xAE,
	0xD3, 0x22, 0xD9, 0xAA, 0xB6, 0x19, 0x18, 0xD5, 0x9D,
	0xE3, 0x2D, 0x2D, 0xA2, 0x6C, 0xEF, 0x49, 0x23, 0x1E,
	0xC9, 0x00, 0x00
};

/* H: Data hash */
static const u8 H_P192[] = {
	0xDB, 0x25, 0x48, 0x37, 0x0A, 0x4B, 0x65, 0xEE, 0xBA,
	0x31, 0xEB, 0xEE, 0x5C, 0x61, 0x5C, 0x73, 0x0B, 0x6F,
	0x37, 0x1B, 0x00, 0x00, 0x00, 0x00
};

static const u8 H_P256[] = {
	0xC4, 0xA8, 0xC8, 0x99, 0x28, 0xCF, 0x80, 0xB6, 0xE4,
	0x42, 0xD5, 0xBD, 0x28, 0x4D, 0xE3, 0xFD, 0x3A, 0x13,
	0xD8, 0x65, 0x0C, 0x41, 0x1C, 0x21, 0x48, 0x95, 0x79,
	0x2A, 0xA1, 0x41, 0x1A, 0xA4
};

static const u8 H_P384[] = {
	0x89, 0x1E, 0x78, 0x0A, 0x0E, 0xF7, 0x8A, 0x2B, 0xCB,
	0xD6, 0x30, 0x6C, 0x9D, 0x14, 0x11, 0x74, 0x5A, 0x8B,
	0x3F, 0x0B, 0x5E, 0x9F, 0x52, 0xC9, 0x99, 0x02, 0xEE,
	0x49, 0x70, 0xBC, 0xDB, 0x6A, 0x6C, 0x83, 0x6D, 0x12,
	0x20, 0x7D, 0x05, 0x35, 0x1B, 0x6E, 0x4F, 0x1C, 0x7D,
	0x18, 0xEA, 0x5A
};

static const u8 H_P521[] = {
	0xCC, 0xDB, 0x1C, 0x06, 0x38, 0x5D, 0x9F, 0x03, 0x49,
	0x2E, 0x3A, 0x07, 0xC4, 0xA3, 0x01, 0x91, 0x4D, 0x3F,
	0x9C, 0x67, 0x20, 0xDE, 0xDF, 0x2A, 0xB3, 0x02, 0xB4,
	0x41, 0x9D, 0x82, 0xFE, 0x11, 0x1C, 0x12, 0xDE, 0xFE,
	0xE9, 0xA8, 0xAF, 0x88, 0x1D, 0xCF, 0x2C, 0x8C, 0xCF,
	0x83, 0x39, 0xC1, 0x46, 0x11, 0x80, 0x8E, 0x00, 0x1B,
	0x8A, 0xCB, 0xF5, 0x35, 0x1F, 0xC0, 0x5A, 0xFB, 0x88,
	0xEF, 0x00, 0x00
};
/** @endcond*/

/** @cond xilpki_internal */

static volatile u32 RQ_SignGen_Status;
static volatile u32 RQ_SignGen_State;
static volatile u32 RQ_SignGenCallBack_Rid;

static volatile u32 RQ_SignVerify_Status;
static volatile u32 RQ_SignVerify_State;
static volatile u32 RQ_SignVerifyCallBack_Rid;

static volatile u32 RQ_PrivKeyGen_Status;
static volatile u32 RQ_PrivKeyGen_State;
static volatile u32 RQ_PrivKeyGenCallBack_Rid;

static volatile u32 RQ_PubKeyGen_Status;
static volatile u32 RQ_PubKeyGen_State;
static volatile u32 RQ_PubKeyGenCallBack_Rid;

/**  @endcond */
/*****************************************************************************/
/**
 * @brief	This function returns ECC public key to perform KAT
 *
 * @param  CrvInfo Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *		ECC public key
 *
 *****************************************************************************/
static XPki_EcdsaKey *XPki_GetKatEccPublicKey(XPki_EcdsaCrvInfo *CrvInfo)
{
	static XPki_EcdsaKey ExpPubKey = {0U};

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		ExpPubKey.Qx = PubkeyQx_P192;
		ExpPubKey.Qy = PubkeyQy_P192;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		ExpPubKey.Qx = PubkeyQx_P256;
		ExpPubKey.Qy = PubkeyQy_P256;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		ExpPubKey.Qx = PubkeyQx_P384;
		ExpPubKey.Qy = PubkeyQy_P384;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		ExpPubKey.Qx = PubkeyQx_P521;
		ExpPubKey.Qy = PubkeyQy_P521;
	} else {
		ExpPubKey.Qx = NULL;
		ExpPubKey.Qy = NULL;
	}

	return &ExpPubKey;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC expected signature to perform KAT
 *
 * @param       CrvInfo  Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *		ECC expected signature
 *
 *****************************************************************************/
static XPki_EcdsaSign *XPki_GetKatEccExpSign(XPki_EcdsaCrvInfo *CrvInfo)
{
	static XPki_EcdsaSign ExpSign = {0U};

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		ExpSign.SignR = (u8 *)SignR_P192;
		ExpSign.SignS = (u8 *)SignS_P192;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		ExpSign.SignR = (u8 *)SignR_P256;
		ExpSign.SignS = (u8 *)SignS_P256;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		ExpSign.SignR = (u8 *)SignR_P384;
		ExpSign.SignS = (u8 *)SignS_P384;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		ExpSign.SignR = (u8 *)SignR_P521;
		ExpSign.SignS = (u8 *)SignS_P521;
	} else {
		ExpSign.SignR = NULL;
		ExpSign.SignS = NULL;
	}

	return &ExpSign;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC private key to perform KAT
 *
 * @param       CrvInfo  Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *		ECC private key
 *
 *****************************************************************************/
static const u8 *XPki_GetKatEccPrivateKey(XPki_EcdsaCrvInfo *CrvInfo)
{
	static const u8 *D;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		D = D_P192;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		D = D_P256;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		D = D_P384;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		D = D_P521;
	} else {
		D = NULL;
	}

	return D;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC ephemeral key to perform KAT
 *
 * @param   CrvInfo Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *		ECC ephemeral key
 *
 *****************************************************************************/
static const u8 *XPki_GetKatEccEphimeralKey(XPki_EcdsaCrvInfo *CrvInfo)
{
	static const u8 *K;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		K = K_P192;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		K = K_P256;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		K = K_P384;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		K = K_P521;
	} else {
		K = NULL;
	}

	return K;
}

/*****************************************************************************/
/**
 * @brief       This function returns ECC Data hash to perform KAT
 *
 * @param       CrvInfo Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *              ECC Data hash
 *
 *****************************************************************************/
static const u8 *XPki_GetKatEccDataHash(XPki_EcdsaCrvInfo *CrvInfo)
{
	static const u8 *H;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		H = H_P192;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		H = H_P256;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		H = H_P384;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		H = H_P521;
	} else {
		H = NULL;
	}

	return H;
}

/*****************************************************************************/
/**
 * @brief       This function returns ECC Data length to perform KAT
 *
 * @param       CrvInfo Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *              ECC Data length
 *
 *****************************************************************************/
static u8 XPki_GetKatEccCurveLen(XPki_EcdsaCrvInfo *CrvInfo)
{
	static u8 CurveLen;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		CurveLen = NIST_P192_LEN_BYTES;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		CurveLen = NIST_P256_LEN_BYTES;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		CurveLen = NIST_P384_LEN_BYTES;
	} else if (CrvInfo->CrvType == ECC_NIST_P521) {
		CurveLen = NIST_P521_LEN_BYTES;
	} else {
		CurveLen = 0;
	}

	return CurveLen;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign verify known answer test(KAT)
 *
 * @param   InstancePtr  Pointer to the XPki instance
 * @param	CrvInfo  Type of ECC curve class either prime/binary curve
 *		and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *		- XST_SUCCESS - when KAT passes
 *		- Errorcode - when KAT fails
 *
 *****************************************************************************/
int XPki_EcdsaVerifySignKat(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo)
{
	XPki_EcdsaKey *PubKey = XPki_GetKatEccPublicKey(CrvInfo);
	XPki_EcdsaSign *ExpSign = XPki_GetKatEccExpSign(CrvInfo);
	u8 const *Hash = XPki_GetKatEccDataHash(CrvInfo);
	XPki_EcdsaVerifyInputData VerifyParams = {0};
	u8 Hlen = XPki_GetKatEccCurveLen(CrvInfo);
	XPki_Request_Info Request_Info = {0};
	volatile int Status = XST_FAILURE;
	u32 RequestID = 0U;
	u32 Count = 0U;

	if (PubKey->Qx == NULL || PubKey->Qy == NULL || ExpSign->SignR == NULL ||
	    ExpSign->SignS == NULL || Hash == NULL) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	VerifyParams.CrvType = CrvInfo->CrvType;
	VerifyParams.PubKey.Qx = PubKey->Qx;
	VerifyParams.PubKey.Qy = PubKey->Qy;
	VerifyParams.Sign.SignR = (u8 *)ExpSign->SignR;
	VerifyParams.Sign.SignS = (u8 *)ExpSign->SignS;
	VerifyParams.Hashlen = Hlen;
	VerifyParams.Hash = Hash;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_SIGN_VERIFY;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_SIGN_VERIFY;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_SIGN_VERIFY;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_SIGN_VERIFY;
	}

	Request_Info.PtrInputData = (void *)&VerifyParams;
	Request_Info.XPki_CompletionCallBack = XPki_SignVerify_CompletionCallBack;

	RQ_SignVerify_Status = 0U;
	RQ_SignVerify_State = XPKI_REQ_SUBMITTED;
	RQ_SignVerifyCallBack_Rid = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);

	while (RQ_SignVerify_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	Status = (int)RQ_SignVerify_Status;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief It is a callback API for Signature Verification Request.
 *
 * @param  RequestID  Unique ID for the submited request
 * @param   Status  Status of the request
 *
 * @return
 * 		- XST_SUCCESS - On success
 * 		- XST_FAILURE  - On failure
 *
******************************************************************************/
static void XPki_SignVerify_CompletionCallBack(u32 RequestID, u32 Status)
{
	RQ_SignVerify_State = XPKI_REQ_COMPLETED;
	RQ_SignVerifyCallBack_Rid = RequestID;
	RQ_SignVerify_Status = Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign generate known answer test(KAT)
 *
 * @param  InstancePtr  Pointer to the XPki instance
 * @param	CrvInfo  Type of ECC curve class either prime or binary class
 *		and curve type(NIST-P192,P256,P384, and P521)
 *
 * @return
 *	-	XST_SUCCESS - when KAT passes
 *	-	Errorcode - when KAT fails
 *
 *****************************************************************************/
int XPki_EcdsaSignGenerateKat(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo)
{
	XPki_EcdsaSign *ExpSign = XPki_GetKatEccExpSign(CrvInfo);
	u8 const *K = XPki_GetKatEccEphimeralKey(CrvInfo);
	u8 const *Hash = XPki_GetKatEccDataHash(CrvInfo);
	u8 const *D = XPki_GetKatEccPrivateKey(CrvInfo);
	u8 Size = XPki_GetKatEccCurveLen(CrvInfo);
	XPki_EcdsaSignInputData  SignParams = {0};
	XPki_Request_Info Request_Info = {0};
	volatile int Status = XST_FAILURE;
	u8 SignR[ECC_MAX_BUF_LEN_BYTES];
	u8 SignS[ECC_MAX_BUF_LEN_BYTES];
	XPki_EcdsaSign GeneratedSign;
	u32 RequestID = 0U;
	u32 Count = 0U;

	if ((ExpSign->SignR == NULL) || (ExpSign->SignS == NULL) ||
	    (D == NULL) || (K == NULL)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	SignParams.CrvType = CrvInfo->CrvType;
	SignParams.D = D;
	SignParams.K = K;
	SignParams.Hash = Hash;
	SignParams.Dlen = Size;
	SignParams.Klen = Size;
	SignParams.Hashlen = Size;
	GeneratedSign.SignR = (u8 *)SignR;
	GeneratedSign.SignS = (u8 *)SignS;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_SIGN;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_SIGN;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_SIGN;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_SIGN;
	}

	Request_Info.PtrInputData = (void *)&SignParams;
	Request_Info.PtrOutputData = (void *)&GeneratedSign;
	Request_Info.XPki_CompletionCallBack = XPki_SignGenearte_CompletionCallBack;

	RQ_SignGen_Status = 0U;
	RQ_SignGen_State = XPKI_REQ_SUBMITTED;
	RQ_SignGenCallBack_Rid = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);

	while (RQ_SignGen_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	if ((RQ_SignGen_Status != XST_SUCCESS) ||
	    (RQ_SignGenCallBack_Rid != RequestID)) {
		goto END;
	}


	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_DeQueue, InstancePtr,
			       &Request_Info, RequestID);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCmp_CT,
			       GeneratedSign.SignR, Size, ExpSign->SignR, Size, Size);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCmp_CT,
			       GeneratedSign.SignS, Size, ExpSign->SignS, Size, Size);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	It is a callback API for Signature Generation Request.
 *
 * @param  RequestID  Unique ID for the submited request
 * @param  Status  Status of the request
 *
 * @return
 *		- XST_SUCCESS - On success
 *		- XST_FAILURE - On failure
 *
******************************************************************************/
static void XPki_SignGenearte_CompletionCallBack(u32 RequestID, u32 Status)
{
	RQ_SignGen_State = XPKI_REQ_COMPLETED;
	RQ_SignGenCallBack_Rid = RequestID;
	RQ_SignGen_Status = Status;
}

/*****************************************************************************/
/**
 * @brief       This function performs ECC pairwise consistency test on
 *		ECC Curves
 *
 * @param       InstancePtr  Pointer to the XPki instance
 * @param       CrvInfo  Type of ECC curve class either prime or binary class
 *              and curve type(NIST-P192,P256,P384, and P521)
 * @param       PubKey  ECC public key
 * @param       PrivKey  ECC private key
 *
 * @return
 *		- XST_SUCCESS - when KAT passes
 *		- Errorcode - when KAT fails
 *
 *****************************************************************************/
int XPki_EcdsaPwct(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo, XPki_EcdsaKey *PubKey, u8 *PrivKey)
{
	u8 const *K = XPki_GetKatEccEphimeralKey(CrvInfo);
	u8 const *Hash = XPki_GetKatEccDataHash(CrvInfo);
	XPki_EcdsaVerifyInputData VerifyParams = {0};
	u8 Size = XPki_GetKatEccCurveLen(CrvInfo);
	XPki_EcdsaSignInputData SignParams = {0};
	XPki_Request_Info Request_Info = {0};
	volatile int Status = XST_FAILURE;
	u8 SignR[ECC_MAX_BUF_LEN_BYTES];
	u8 SignS[ECC_MAX_BUF_LEN_BYTES];
	XPki_EcdsaSign GeneratedSign;
	u32 RequestID = 0U;
	u32 Count = 0U;

	if ((Hash == NULL) || (K == NULL)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	SignParams.CrvType = CrvInfo->CrvType;
	SignParams.D = PrivKey;
	SignParams.K = K;
	SignParams.Hash = Hash;
	SignParams.Dlen = Size;
	SignParams.Klen = Size;
	SignParams.Hashlen = Size;
	GeneratedSign.SignR = SignR;
	GeneratedSign.SignS = SignS;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_SIGN;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_SIGN;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_SIGN;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_SIGN;
	}

	Request_Info.PtrInputData = (void *)&SignParams;
	Request_Info.PtrOutputData = (void *)&GeneratedSign;
	Request_Info.XPki_CompletionCallBack = XPki_SignGenearte_CompletionCallBack;

	RQ_SignGen_Status = 0U;
	RQ_SignGen_State = XPKI_REQ_SUBMITTED;
	RQ_SignGenCallBack_Rid = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);

	while (RQ_SignGen_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	if ((RQ_SignGen_Status != XST_SUCCESS) ||
	    (RQ_SignGenCallBack_Rid != RequestID)) {
		goto END;
	}

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_DeQueue, InstancePtr,
			       &Request_Info, RequestID);

	VerifyParams.CrvType = CrvInfo->CrvType;
	VerifyParams.PubKey.Qx = PubKey->Qx;
	VerifyParams.PubKey.Qy = PubKey->Qy;
	VerifyParams.Sign.SignR = SignR;
	VerifyParams.Sign.SignS = SignS;
	VerifyParams.Hashlen = Size;
	VerifyParams.Hash = Hash;

	if (CrvInfo->CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_SIGN_VERIFY;
	} else if (CrvInfo->CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_SIGN_VERIFY;
	} else if (CrvInfo->CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_SIGN_VERIFY;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_SIGN_VERIFY;
	}

	Request_Info.PtrInputData = (void *)&VerifyParams;
	Request_Info.XPki_CompletionCallBack = XPki_SignVerify_CompletionCallBack;

	RQ_SignVerify_Status = 0U;
	RQ_SignVerify_State = XPKI_REQ_SUBMITTED;
	RQ_SignVerifyCallBack_Rid = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);
	Count = 0U;
	while (RQ_SignVerify_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	Status = (int)RQ_SignVerify_Status;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function Generate key pairs for the ECC curves NIST-P192,
 *              P256, P384, and P521.
 *
 * @param       InstancePtr	Pointer to the XPki instance
 * @param       CrvType		Type of ECC curve(NIST-P192,P256,P384, and P521)
 * @param       PubKey		Pointer to the pulic-key buffer
 * @param       PrivKey		Pointer to the private-key buffer
 *
 * @return
 *		- XST_SUCCESS - when passes
 *		- Errorcode - when fails
 *
 *****************************************************************************/
int XPki_EcdsaGenerateKeyPair(XPki_Instance *InstancePtr, XPki_EcdsaCrvType CrvType,
				XPki_EcdsaKey *PubKey, u8 *PrivKey)
{
	XPki_Request_Info Request_Info = {0};
	volatile int Status = XST_FAILURE;
	u32 RequestID = 0U;
	u32 Count = 0U;

	if (CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_KEY_PRIV_GEN;
	} else if (CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_KEY_PRIV_GEN;
	} else if (CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_KEY_PRIV_GEN;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_KEY_PRIV_GEN;
	}

	Request_Info.PtrOutputData = (void *)PrivKey;
	Request_Info.XPki_CompletionCallBack = XPki_PrivKeyGen_CompletionCallBack;

	RQ_PrivKeyGen_Status = 0U;
	RQ_PrivKeyGen_State = XPKI_REQ_SUBMITTED;
	RQ_PrivKeyGenCallBack_Rid = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);

	while (RQ_PrivKeyGen_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	if ((RQ_PrivKeyGen_Status != XST_SUCCESS) ||
	    (RQ_PrivKeyGenCallBack_Rid != RequestID)) {
		goto END;
	}


	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_DeQueue, InstancePtr,
			       &Request_Info, RequestID);

	if (CrvType == ECC_NIST_P192) {
		Request_Info.OpsType = PKI_ECC_NIST_P192_KEY_PUB_GEN;
	} else if (CrvType == ECC_NIST_P256) {
		Request_Info.OpsType = PKI_ECC_NIST_P256_KEY_PUB_GEN;
	} else if (CrvType == ECC_NIST_P384) {
		Request_Info.OpsType = PKI_ECC_NIST_P384_KEY_PUB_GEN;
	} else {
		Request_Info.OpsType = PKI_ECC_NIST_P521_KEY_PUB_GEN;
	}

	Request_Info.PtrInputData = (void *)PrivKey;
	Request_Info.PtrOutputData = (void *)PubKey;
	Request_Info.XPki_CompletionCallBack = XPki_PubKeyGen_CompletionCallBack;

	RQ_PubKeyGen_Status = 0U;
	RQ_PubKeyGen_State = XPKI_REQ_SUBMITTED;
	RQ_PubKeyGenCallBack_Rid = 0U;
	Count = 0U;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_EnQueue, InstancePtr,
			       &Request_Info, &RequestID);

	while (RQ_PubKeyGen_State != XPKI_REQ_COMPLETED) {
		if (Count++ == XPKI_MAX_WAIT_COUNT) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	if ((RQ_PubKeyGen_Status != XST_SUCCESS) ||
            (RQ_PubKeyGenCallBack_Rid != RequestID)) {
		goto END;
	}


	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XilPki_DeQueue, InstancePtr,
			       &Request_Info, RequestID);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       It is a completion callback API for Private-key generation
 *		Request.
 *
 * @param       RequestID       Unique ID for the submited request
 * @param       Status          Status of the Request
 *
 * @return
 *		- XST_SUCCESS - On success
 *		- -XST_FAILURE  - On failure *
******************************************************************************/
static void XPki_PrivKeyGen_CompletionCallBack(u32 RequestID, u32 Status)
{
	RQ_PrivKeyGen_State = XPKI_REQ_COMPLETED;
	RQ_PrivKeyGenCallBack_Rid = RequestID;
	RQ_PrivKeyGen_Status = Status;
}

/*****************************************************************************/
/**
 * @brief       It is a completion callback API for Public-key generation
 *              request.
 *
 * @param RequestID  Unique ID for the submited request
 * @param Status  Status of the request
 *
 * @return
 *		- XST_SUCCESS - On success
*		- XST_FAILURE - On failure
 *
******************************************************************************/
static void XPki_PubKeyGen_CompletionCallBack(u32 RequestID, u32 Status)
{
	RQ_PubKeyGen_State = XPKI_REQ_COMPLETED;
	RQ_PubKeyGenCallBack_Rid = RequestID;
	RQ_PubKeyGen_Status = Status;
}
/**@}*/

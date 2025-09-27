/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_kat.c
 *
 * This file contains the code for SHA2/SHA3, RSA, ECC, ECDH and HMAC KAT command supported by
 * ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/15/24 Initial release
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       ma   07/08/24 Add task based approach at queue level
 *       ss   08/20/24 Added RSA KAT
 *       yog  08/21/24 Added kat support for ECC
 *       am   08/06/24 Added AES KAT
 *       am   08/24/24 Added AES DPA CM KAT support
 *       yog  08/25/24 Integrated FIH library
 *       ss   09/26/24 Fixed doxygen comments
 *       am   10/22/24 Replaced XSHA_SHA_256_HASH_LEN with XASU_SHA_256_HASH_LEN
 * 1.1   ss   12/02/24 Added kat support for ECDH
 *       ma   12/12/24 Updated resource allocation logic
 *       yog  01/02/25 Added HMAC KAT
 *       ma   01/15/25 Added KDF KAT
 *       am   01/28/25 Fixed compilation error
 *       yog  02/21/25 Added ECIES KAT
 *       ss   02/24/25 Added Key wrap unwrap KAT
 *       am   03/14/25 Fix code spell warning
 *       LP   04/07/25 Added HKDF KAT
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_kat.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_cmd.h"
#include "xasu_eccinfo.h"
#include "xfih.h"
#include "xasu_rsainfo.h"
#include "xasufw_util.h"
#include "xasu_hmacinfo.h"
#include "xkdf.h"
#include "xhkdf.h"
#include "xecies.h"
#include "xaes_hw.h"
#include "xkeywrap.h"
#include "xtrng.h"
#include "xtrng_hw.h"
#include "xasufw_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_ValidateDpaIntermediateValues(const u32 *MaskedOutput0, const u32 *MaskedOutput1,
	const u32 *Mask0, const u32 *Mask1);
static void XAsufw_ApplyMask(const u32 *Data, u32 *MaskedData, u32 Mask, u32 Length);
static s32 RunKdfAndDependentKat(XAsufw_Dma *AsuDmaPtr);
#if defined(XASU_KEYWRAP_ENABLE) || defined(XASU_RSA_PADDING_ENABLE)
static s32 RunKeyWrapAndDependentKat(XAsufw_Dma *AsuDmaPtr);
#endif

/************************************ Variable Definitions ***************************************/
#define XASUFW_KAT_MSG_LENGTH_IN_BYTES			(32U)	/**< SHA KAT message length in bytes */
#define XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES		(256U)	/**< RSA KAT message length in bytes */
#define XASUFW_AES_KEY_LEN_IN_BYTES			(32U)	/**< AES KAT Key length in bytes */
#define XASUFW_AES_IV_LEN_IN_BYTES			(12U)	/**< AES KAT Iv length in bytes */
#define XASUFW_AES_AAD_LEN_IN_BYTES			(16U)	/**< AES KAT AAD length in bytes */
#define XASUFW_AES_TAG_LEN_IN_BYTES			(16U)	/**< AES KAT Tag length in bytes */
#define XASUFW_DOUBLE_P256_SIZE_IN_BYTES		(64U)	/**< Double the size of P256 curve length */
#define XASUFW_DOUBLE_P448_SIZE_IN_BYTES		(114U)	/**< Double the size of P448 curve length */
#define XASUFW_DOUBLE_P384_SIZE_IN_BYTES		(96U)	/**< Double the size of P384 curve length */
#define XASUFW_AES_CM_LEN_IN_WORDS			(4U) /**< AES CM input and output buffer lengths in words. */
#define XASUFW_AES_CM_SPLIT_MASK			(0xFFFFFFFFU) /**< AES CM split mask. */
#define XASUFW_AES_CM_SPLIT_ALIGNED_LENGTH		(0x10U) /**< AES CM split configuration aligned length. */
#define XASUFW_AES_CM_DATA_LEN_IN_BYTES			(16U) /**< AES CM data length in bytes. */
#define XASUFW_AES_CM_MASK_BUF_WORD_LEN			(32U) /**< AES CM mask data buffer word length. */
#define XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES		(31U)	/**< Key wrap unwrap KAT message length in
								bytes*/
#define XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES		(5U)	/**< RSA OAEP optional data length
									in bytes */
#define XASUFW_KEYWRAP_INPUT_PADDING_SIZE_IN_BYTES	(9U)	/**< Key wrap padding length for
									KAT message in bytes */
#define XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES	(XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES + \
							XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES + \
							XASUFW_KEYWRAP_INPUT_PADDING_SIZE_IN_BYTES)
						/**< Key wrap unwrap KAT message output length in
							bytes */
#define XASUFW_RSA_INPUT_LEN_IN_BYTES			(80U)	/**< RSA input length in bytes */
#define XASUFW_RSA_PSS_SALT_LEN_IN_BYTES		(20U)

/* KAT message */
static const u8 KatMessage[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* SHA2 256 hash for above KAT message */
static const u8 ExpSha2_256Hash[XASU_SHA_256_HASH_LEN] = {
	0x03U, 0xFBU, 0xA3U, 0xBBU, 0x9BU, 0x90U, 0xBBU, 0x5FU,
	0xF1U, 0x07U, 0xC0U, 0x43U, 0x63U, 0xF7U, 0x99U, 0x34U,
	0xECU, 0xE9U, 0x56U, 0xB2U, 0x3CU, 0xFAU, 0x13U, 0xC7U,
	0xDFU, 0x79U, 0xB9U, 0xFEU, 0x5DU, 0x1DU, 0xADU, 0x3BU
};

/* SHA2 512 hash for above KAT message */
static const u8 ExpSha2_512Hash[XASU_SHA_512_HASH_LEN] = {
	0x63U, 0x9DU, 0xDFU, 0x40U, 0x52U, 0x7DU, 0xACU, 0x45U,
	0x48U, 0xC0U, 0xA8U, 0xBEU, 0xCEU, 0x54U, 0x71U, 0x71U,
	0x42U, 0x94U, 0x3FU, 0x5AU, 0x4EU, 0x95U, 0x9CU, 0xDEU,
	0x81U, 0x81U, 0x4AU, 0x56U, 0x63U, 0xA8U, 0x5AU, 0x5FU,
	0x4BU, 0xD4U, 0x94U, 0x06U, 0x99U, 0x0CU, 0x54U, 0x22U,
	0x32U, 0xC0U, 0xDEU, 0x7CU, 0x47U, 0x32U, 0x1AU, 0x4FU,
	0xCCU, 0x41U, 0x9AU, 0x5FU, 0x82U, 0x9FU, 0x8DU, 0xD5U,
	0xB9U, 0xF2U, 0x45U, 0x20U, 0xD4U, 0xF4U, 0x5FU, 0xDCU
};

/* SHA3 256 hash for above KAT message */
static const u8 ExpSha3_256Hash[XASU_SHA_256_HASH_LEN] = {
	0xBCU, 0x64U, 0xDCU, 0xBBU, 0x66U, 0xDDU, 0x08U, 0xA5U,
	0xC1U, 0x11U, 0xE6U, 0xB3U, 0x55U, 0x60U, 0xF1U, 0xC3U,
	0x3EU, 0x6DU, 0xE8U, 0x6AU, 0x15U, 0xDFU, 0x6CU, 0xF3U,
	0xA5U, 0xD1U, 0xB4U, 0x31U, 0xC0U, 0x67U, 0x5CU, 0xDDU
};

#ifdef XASU_HMAC_ENABLE
static const u8 ExpHmacOutput[XASU_SHA_256_HASH_LEN] = {
	0x72U, 0xC5U, 0x39U, 0x7DU, 0x23U, 0x49U, 0x3EU, 0xFFU,
	0xF9U, 0x16U, 0xC8U, 0x4BU, 0x08U, 0x11U, 0x3EU, 0x26U,
	0x2FU, 0xA8U, 0x55U, 0x76U, 0xAFU, 0xFEU, 0x96U, 0x4AU,
	0x09U, 0x79U, 0xAAU, 0x05U, 0xBEU, 0xAFU, 0x5FU, 0x2DU
};
#endif

#ifdef XASU_KDF_ENABLE
static const u8 ExpKdfOutput[XASU_SHA_256_HASH_LEN] = {
	0x91U, 0x53U, 0x33U, 0xF0U, 0x62U, 0x3DU, 0xBCU, 0x73U,
	0x7EU, 0x92U, 0x3CU, 0x1BU, 0xB3U, 0xA9U, 0x92U, 0x31U,
	0x4EU, 0x66U, 0xC4U, 0x6EU, 0x2CU, 0x8DU, 0x81U, 0x45U,
	0xD2U, 0xE4U, 0x70U, 0x4EU, 0x75U, 0xD0U, 0x3AU, 0x77U
};
#endif

#if defined(XASU_RSA_PADDING_ENABLE) || defined(XASU_ECIES_ENABLE)
static const u8 RsaData[XASUFW_RSA_INPUT_LEN_IN_BYTES] = {
	0x05U, 0x95U, 0x87U, 0x3FU, 0xA5U, 0x76U, 0x50U, 0xBFU, 0x76U, 0x6FU, 0x2CU, 0xB3U, 0x97U, 0x80U,
	0x8BU, 0x7BU, 0x99U, 0xE8U, 0x56U, 0x2FU, 0x4BU, 0xCDU, 0x66U, 0x25U, 0x6FU, 0xA9U, 0x51U, 0x7CU,
	0x42U, 0x57U, 0x95U, 0x95U, 0x6AU, 0xF0U, 0xF8U, 0x7EU, 0x56U, 0x0DU, 0x28U, 0x07U, 0xE7U, 0x1CU,
	0x7CU, 0xD8U, 0x9EU, 0xF2U, 0xD6U, 0x37U, 0xE8U, 0x7AU, 0x5AU, 0xDBU, 0xA5U, 0xB5U, 0xD2U, 0x93U,
	0xFAU, 0x29U, 0x28U, 0x76U, 0xB8U, 0xCEU, 0xC3U, 0x66U, 0xFFU, 0x15U, 0xFBU, 0x28U, 0x3BU, 0xC2U,
	0x82U, 0x32U, 0xB7U, 0x5FU, 0xE4U, 0xD5U, 0x54U, 0x8EU, 0xE5U, 0x43U
};
#endif

#if defined(XASU_RSA_PADDING_ENABLE) || defined(XASU_KEYWRAP_ENABLE)
static const u8 RsaModulus[XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES] = {
	0xAEU, 0xFEU, 0xC6U, 0x93U, 0xF1U, 0x06U, 0x01U, 0x47U, 0x17U, 0x28U, 0xA2U, 0x49U,
	0x6FU, 0xA3U, 0x1DU, 0x8CU, 0xDBU, 0xF9U, 0xB3U, 0x57U, 0x67U, 0xEFU, 0x31U, 0xCDU,
	0xACU, 0x13U, 0x1CU, 0x20U, 0xE0U, 0x9DU, 0x55U, 0x55U, 0xFDU, 0x0CU, 0x30U, 0xB7U,
	0x83U, 0x28U, 0xA5U, 0x4CU, 0x77U, 0xC0U, 0x85U, 0x11U, 0x70U, 0xB4U, 0x4AU, 0xFCU,
	0x98U, 0xDFU, 0x75U, 0x69U, 0xD8U, 0xF9U, 0x21U, 0x6AU, 0x65U, 0xAAU, 0x30U, 0x84U,
	0xCFU, 0x2EU, 0xCBU, 0x6CU, 0x91U, 0xD5U, 0x6AU, 0x0DU, 0x46U, 0xE7U, 0x37U, 0xB0U,
	0x3FU, 0x1FU, 0x71U, 0xD9U, 0x5DU, 0x0BU, 0xD6U, 0x5CU, 0x61U, 0xCBU, 0xD2U, 0x8AU,
	0x96U, 0xB9U, 0x7EU, 0x1BU, 0xA9U, 0x33U, 0x69U, 0xD3U, 0xBAU, 0x50U, 0xA1U, 0x10U,
	0xFEU, 0x53U, 0x1FU, 0x5CU, 0x63U, 0xB1U, 0xF6U, 0x3BU, 0xB0U, 0xE8U, 0x83U, 0x0BU,
	0x5FU, 0x30U, 0x00U, 0x35U, 0x1FU, 0xA3U, 0x4EU, 0x7AU, 0x3EU, 0xE1U, 0x51U, 0xAEU,
	0x7CU, 0x62U, 0xAFU, 0x06U, 0x99U, 0x5EU, 0x14U, 0x73U, 0xF5U, 0x7CU, 0x35U, 0x40U,
	0xBBU, 0xB7U, 0xA3U, 0x3CU, 0x13U, 0xE0U, 0x92U, 0x7EU, 0x03U, 0x16U, 0x73U, 0xADU,
	0x78U, 0x26U, 0x0BU, 0x13U, 0x29U, 0x2FU, 0x5FU, 0x29U, 0x40U, 0xD8U, 0xBFU, 0x5EU,
	0x73U, 0xFAU, 0x55U, 0x2DU, 0x3EU, 0xABU, 0x7FU, 0x3CU, 0xB3U, 0x58U, 0xA5U, 0x9FU,
	0xA8U, 0x3CU, 0x12U, 0x58U, 0x27U, 0xC5U, 0xE3U, 0x0BU, 0x11U, 0xCDU, 0x3FU, 0x1DU,
	0xAAU, 0x98U, 0x77U, 0x0FU, 0x6DU, 0x99U, 0x26U, 0xE2U, 0x73U, 0x28U, 0x31U, 0x8EU,
	0x9CU, 0x40U, 0x51U, 0xC8U, 0x58U, 0x52U, 0xA1U, 0x07U, 0xADU, 0xAFU, 0x36U, 0xB8U,
	0xC8U, 0x08U, 0x49U, 0xD7U, 0xCEU, 0x28U, 0x7FU, 0xE3U, 0xB2U, 0xF1U, 0xB1U, 0xE9U,
	0x6AU, 0x59U, 0x43U, 0xDCU, 0xD8U, 0x4DU, 0x68U, 0xC0U, 0x11U, 0x21U, 0xEEU, 0xEDU,
	0xB0U, 0x0BU, 0xA8U, 0x24U, 0xE0U, 0xD5U, 0x36U, 0xC2U, 0xFDU, 0x3EU, 0x35U, 0xC3U,
	0x37U, 0xC4U, 0xA2U, 0x84U, 0xA4U, 0xC2U, 0xD6U, 0xECU, 0xAEU, 0xFFU, 0xFBU, 0xC5U,
	0xD1U, 0x05U, 0xD2U, 0x23U
};

static const u8 RsaPvtExp[XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES] = {
	0x0AU, 0xA8U, 0x1BU, 0x41U, 0xB1U, 0x20U, 0xD3U, 0x7DU, 0x17U, 0xCCU, 0xF2U, 0xADU,
	0x14U, 0x2EU, 0x53U, 0xC3U, 0x5BU, 0x36U, 0x06U, 0x94U, 0xE1U, 0x10U, 0x70U, 0xF0U,
	0xFCU, 0x74U, 0xA1U, 0x76U, 0xE3U, 0x16U, 0xD1U, 0xB6U, 0x8DU, 0xD5U, 0x6BU, 0x36U,
	0x11U, 0xB7U, 0xACU, 0xF1U, 0x4EU, 0x2DU, 0x9CU, 0x2CU, 0xE6U, 0xB7U, 0x24U, 0x05U,
	0xE3U, 0xEDU, 0x5FU, 0xC2U, 0x15U, 0x63U, 0x7EU, 0x84U, 0x73U, 0x32U, 0x7DU, 0x07U,
	0xE9U, 0x72U, 0x09U, 0x13U, 0x50U, 0x82U, 0x35U, 0x96U, 0x1FU, 0x66U, 0x3FU, 0x3EU,
	0xEDU, 0x69U, 0x25U, 0xCEU, 0xBDU, 0xDAU, 0xD5U, 0xB0U, 0x04U, 0x88U, 0x9CU, 0x06U,
	0xB2U, 0x8DU, 0x13U, 0x3FU, 0xEDU, 0xFAU, 0xE2U, 0x8BU, 0xF1U, 0x41U, 0xADU, 0xBDU,
	0x52U, 0x2FU, 0x8FU, 0xAEU, 0x59U, 0xA7U, 0xE1U, 0xBDU, 0xDAU, 0xD5U, 0x1DU, 0xFDU,
	0xD8U, 0x4BU, 0x1DU, 0x08U, 0x1FU, 0x28U, 0x1BU, 0xC4U, 0x58U, 0x05U, 0xF2U, 0xAAU,
	0x74U, 0x8AU, 0xB1U, 0xEBU, 0xEDU, 0xF5U, 0x0BU, 0xBBU, 0xB6U, 0x16U, 0x8DU, 0x2BU,
	0xE3U, 0x81U, 0xC5U, 0x23U, 0xC8U, 0x34U, 0x37U, 0x6DU, 0xE0U, 0xE6U, 0xF3U, 0xA8U,
	0x57U, 0xAFU, 0xA2U, 0xABU, 0x74U, 0xAEU, 0xA1U, 0x33U, 0x6EU, 0x81U, 0x0BU, 0x73U,
	0x23U, 0x39U, 0xE2U, 0xCBU, 0xD6U, 0xA0U, 0xE5U, 0xBFU, 0x6DU, 0x4AU, 0x23U, 0x10U,
	0x1BU, 0x5BU, 0xAAU, 0x6EU, 0xDAU, 0x76U, 0x11U, 0x7CU, 0xB5U, 0xFBU, 0xCAU, 0xE2U,
	0xF8U, 0xB5U, 0x54U, 0x10U, 0x29U, 0x5CU, 0x30U, 0x19U, 0x0DU, 0x09U, 0x85U, 0x9AU,
	0x2DU, 0xFBU, 0x7AU, 0xB7U, 0xA2U, 0xFBU, 0xCBU, 0xA7U, 0x83U, 0x08U, 0xB2U, 0x87U,
	0x81U, 0xDDU, 0x6BU, 0x52U, 0x91U, 0xC1U, 0x10U, 0x4DU, 0x1DU, 0x55U, 0xA1U, 0x5EU,
	0xACU, 0xFCU, 0x3CU, 0x6AU, 0x1CU, 0x0FU, 0xDCU, 0x55U, 0x64U, 0x0FU, 0x56U, 0x2CU,
	0x37U, 0x2FU, 0xF7U, 0xE6U, 0x90U, 0xE8U, 0x99U, 0xE3U, 0x06U, 0x34U, 0xF8U, 0xF2U,
	0xE2U, 0x90U, 0x1CU, 0x5CU, 0xD9U, 0xA8U, 0x45U, 0x72U, 0x40U, 0x94U, 0x5CU, 0x3CU,
	0x28U, 0x32U, 0x44U, 0xD1U
};

static const u32 RsaPublicExp = 0x1000100U;

static const char RsaOpt[XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES + 1U] = "ASUFW";
#endif

static const u8 EccPrivKey[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
	0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
	0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
	0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU
};

static const u8 EccPrivKey25519[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0x9DU, 0x61U, 0xB1U, 0x9DU, 0xEFU, 0xFDU, 0x5AU, 0x60U,
	0xBAU, 0x84U, 0x4AU, 0xF4U, 0x92U, 0xECU, 0x2CU, 0xC4U,
	0x44U, 0x49U, 0xC5U, 0x69U, 0x7BU, 0x32U, 0x69U, 0x19U,
	0x70U, 0x3BU, 0xACU, 0x03U, 0x1CU, 0xAEU, 0x7FU, 0x60U
};

static const u8 EccPrivKey448[XASU_ECC_P448_SIZE_IN_BYTES] = {
	0x6CU, 0x82U, 0xA5U, 0x62U, 0xCBU, 0x80U, 0x8DU, 0x10U,
	0xD6U, 0x32U, 0xBEU, 0x89U, 0xC8U, 0x51U, 0x3EU, 0xBFU,
	0x6CU, 0x92U, 0x9FU, 0x34U, 0xDDU, 0xFAU, 0x8CU, 0x9FU,
	0x63U, 0xC9U, 0x96U, 0x0EU, 0xF6U, 0xE3U, 0x48U, 0xA3U,
	0x52U, 0x8CU, 0x8AU, 0x3FU, 0xCCU, 0x2FU, 0x04U, 0x4EU,
	0x39U, 0xA3U, 0xFCU, 0x5BU, 0x94U, 0x49U, 0x2FU, 0x8FU,
	0x03U, 0x2EU, 0x75U, 0x49U, 0xA2U, 0x00U, 0x98U, 0xF9U, 0x5BU
};

static const u8 EccHash[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0x02U, 0xBFU, 0x58U, 0x5CU, 0x72U, 0x89U, 0x45U, 0x9CU,
	0xDDU, 0x20U, 0x61U, 0xD1U, 0x67U, 0xE5U, 0x40U, 0xC0U,
	0x1EU, 0x40U, 0x56U, 0xB4U, 0x65U, 0xCAU, 0xE1U, 0x5FU,
	0xA3U, 0x45U, 0xEDU, 0xADU, 0x93U, 0x88U, 0x54U, 0x6DU
};

static const u8 EccEphemeralKey[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0xBFU, 0xD6U, 0x31U, 0xA2U, 0xA6U, 0x47U, 0x31U, 0x70U,
	0xB8U, 0x16U, 0x6DU, 0x33U, 0x25U, 0x06U, 0xBEU, 0x62U,
	0xE5U, 0x48U, 0x5AU, 0xD0U, 0xBEU, 0x76U, 0xBAU, 0x74U,
	0xA1U, 0x09U, 0x7CU, 0x59U, 0x5FU, 0x57U, 0x70U, 0xCDU
};

static const u8 EccExpPubKeyP256[XASUFW_DOUBLE_P256_SIZE_IN_BYTES] = {
	0x36U, 0x77U, 0xFBU, 0xF9U, 0xBBU, 0x2DU, 0x96U, 0xA3U,
	0x1BU, 0x01U, 0x11U, 0x08U, 0x57U, 0x93U, 0x8CU, 0xC4U,
	0x9DU, 0x9AU, 0x30U, 0xA4U, 0xE0U, 0x0EU, 0x9CU, 0xD4U,
	0xB5U, 0x5DU, 0x97U, 0x77U, 0x58U, 0x0CU, 0x84U, 0xC7U,
	0x0CU, 0x67U, 0x48U, 0x94U, 0xE8U, 0x53U, 0xD3U, 0x6BU,
	0xBEU, 0xC6U, 0xC2U, 0x1FU, 0xDCU, 0xFCU, 0x7BU, 0xD1U,
	0xF8U, 0x2BU, 0x72U, 0xD3U, 0xA4U, 0xC2U, 0x8EU, 0x10U,
	0xD8U, 0x25U, 0x5DU, 0x21U, 0x33U, 0xD5U, 0xCAU, 0x38U
};

static const u8 EccExpPubKey25519[XASUFW_DOUBLE_P256_SIZE_IN_BYTES] = {
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0xD7U, 0x5AU, 0x98U, 0x01U, 0x82U, 0xB1U, 0x0AU, 0xB7U,
	0xD5U, 0x4BU, 0xFEU, 0xD3U, 0xC9U, 0x64U, 0x07U, 0x3AU,
	0x0EU, 0xE1U, 0x72U, 0xF3U, 0xDAU, 0xA6U, 0x23U, 0x25U,
	0xAFU, 0x02U, 0x1AU, 0x68U, 0xF7U, 0x07U, 0x51U, 0x1AU
};

static const u8 EccExpPubKeyP448[XASUFW_DOUBLE_P448_SIZE_IN_BYTES] = {
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x5FU, 0xD7U, 0x44U, 0x9BU, 0x59U, 0xB4U, 0x61U,
	0xFDU, 0x2CU, 0xE7U, 0x87U, 0xECU, 0x61U, 0x6AU, 0xD4U,
	0x6AU, 0x1DU, 0xA1U, 0x34U, 0x24U, 0x85U, 0xA7U, 0x0EU,
	0x1FU, 0x8AU, 0x0EU, 0xA7U, 0x5DU, 0x80U, 0xE9U, 0x67U,
	0x78U, 0xEDU, 0xF1U, 0x24U, 0x76U, 0x9BU, 0x46U, 0xC7U,
	0x06U, 0x1BU, 0xD6U, 0x78U, 0x3DU, 0xF1U, 0xE5U, 0x0FU,
	0x6CU, 0xD1U, 0xFAU, 0x1AU, 0xBEU, 0xAFU, 0xE8U, 0x25U, 0x61U, 0x80U
};

static u8 EccExpSignP256[XASUFW_DOUBLE_P256_SIZE_IN_BYTES] = {
	0x7AU, 0x67U, 0xE9U, 0x44U, 0xC7U, 0x93U, 0x90U, 0xB2U,
	0x2AU, 0xEBU, 0x4FU, 0x03U, 0xEFU, 0x12U, 0xDAU, 0xE6U,
	0x5FU, 0x1BU, 0xF0U, 0x42U, 0xFCU, 0xC2U, 0x6EU, 0x5FU,
	0x10U, 0xECU, 0x94U, 0x9BU, 0x39U, 0xB4U, 0x1FU, 0x6AU,
	0x33U, 0x01U, 0x16U, 0x6CU, 0x6DU, 0xBCU, 0x5FU, 0xA3U,
	0x8BU, 0x1AU, 0x10U, 0xCAU, 0x50U, 0xA7U, 0x08U, 0x98U,
	0x47U, 0xE5U, 0x74U, 0xCDU, 0x91U, 0x0AU, 0xC3U, 0x1DU,
	0x04U, 0xE9U, 0xA6U, 0x99U, 0x54U, 0x9AU, 0x5CU, 0x5AU
};

static u8 EccExpSign25519[XASUFW_DOUBLE_P256_SIZE_IN_BYTES] = {
	0xE5U, 0x56U, 0x43U, 0x00U, 0xC3U, 0x60U, 0xACU, 0x72U,
	0x90U, 0x86U, 0xE2U, 0xCCU, 0x80U, 0x6EU, 0x82U, 0x8AU,
	0x84U, 0x87U, 0x7FU, 0x1EU, 0xB8U, 0xE5U, 0xD9U, 0x74U,
	0xD8U, 0x73U, 0xE0U, 0x65U, 0x22U, 0x49U, 0x01U, 0x55U,
	0x5FU, 0xB8U, 0x82U, 0x15U, 0x90U, 0xA3U, 0x3BU, 0xACU,
	0xC6U, 0x1EU, 0x39U, 0x70U, 0x1CU, 0xF9U, 0xB4U, 0x6BU,
	0xD2U, 0x5BU, 0xF5U, 0xF0U, 0x59U, 0x5BU, 0xBEU, 0x24U,
	0x65U, 0x51U, 0x41U, 0x43U, 0x8EU, 0x7AU, 0x10U, 0x0BU
};

static u8 EccExpSignP448[XASUFW_DOUBLE_P448_SIZE_IN_BYTES] = {
	0x53U, 0x3AU, 0x37U, 0xF6U, 0xBBU, 0xE4U, 0x57U, 0x25U,
	0x1FU, 0x02U, 0x3CU, 0x0DU, 0x88U, 0xF9U, 0x76U, 0xAEU,
	0x2DU, 0xFBU, 0x50U, 0x4AU, 0x84U, 0x3EU, 0x34U, 0xD2U,
	0x07U, 0x4FU, 0xD8U, 0x23U, 0xD4U, 0x1AU, 0x59U, 0x1FU,
	0x2BU, 0x23U, 0x3FU, 0x03U, 0x4FU, 0x62U, 0x82U, 0x81U,
	0xF2U, 0xFDU, 0x7AU, 0x22U, 0xDDU, 0xD4U, 0x7DU, 0x78U,
	0x28U, 0xC5U, 0x9BU, 0xD0U, 0xA2U, 0x1BU, 0xFDU, 0x39U,
	0x80U, 0xFFU, 0x0DU, 0x20U, 0x28U, 0xD4U, 0xB1U, 0x8AU,
	0x9DU, 0xF6U, 0x3EU, 0x00U, 0x6CU, 0x5DU, 0x1CU, 0x2DU,
	0x34U, 0x5BU, 0x92U, 0x5DU, 0x8DU, 0xC0U, 0x0BU, 0x41U,
	0x04U, 0x85U, 0x2DU, 0xB9U, 0x9AU, 0xC5U, 0xC7U, 0xCDU,
	0xDAU, 0x85U, 0x30U, 0xA1U, 0x13U, 0xA0U, 0xF4U, 0xDBU,
	0xB6U, 0x11U, 0x49U, 0xF0U, 0x5AU, 0x73U, 0x63U, 0x26U,
	0x8CU, 0x71U, 0xD9U, 0x58U, 0x08U, 0xFFU, 0x2EU, 0x65U, 0x26U, 0x00U
};

/* AES key */
static const u8 AesKey[XASUFW_AES_KEY_LEN_IN_BYTES] = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/* AES Iv */
static const u8 AesIv[XASUFW_AES_IV_LEN_IN_BYTES] = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U, 0x62U, 0x98U,
	0xDFU, 0xEAU, 0x5AU, 0xB2U
};

/* AES AAD data */
static const u8 AesAad[XASUFW_AES_AAD_LEN_IN_BYTES] = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

/* AES-ECB expected cipherText */
static const u8 ExpAesEcbCt[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {
	0x97U, 0xFFU, 0x15U, 0x52U, 0x9AU, 0x8AU, 0x24U, 0x83U,
	0x10U, 0x22U, 0xE1U, 0x33U, 0x04U, 0xDEU, 0xCEU, 0xFDU,
	0x27U, 0x69U, 0x5DU, 0x9BU, 0x81U, 0x4EU, 0xCCU, 0x5CU,
	0x8EU, 0xAEU, 0xACU, 0x3EU, 0x48U, 0xD6U, 0xA6U, 0xD7U
};

/* AES-GCM expected cipherText */
static const u8 ExpAesGcmCt[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

/* AES expected tag */
static const u8 ExpAesGcmTag[XASUFW_AES_TAG_LEN_IN_BYTES] = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

#ifdef XASU_AES_CM_ENABLE
/* AES counter measure plaintext */
static const u32 AesCmPt[XASUFW_AES_CM_LEN_IN_WORDS] = {0xE2BEC16BU, 0x969F402EU, 0x117E3DE9U, 0x2A179373U};

/* AES counter measure expected ciphertext */
static const u32 AesCmCt[XASUFW_AES_CM_LEN_IN_WORDS] = {0x91614D87U, 0x26E320B6U, 0x6468EF1BU, 0xCEB60D99U};

/* AES counter measure key */
static const u32 AesCmKey[XASUFW_AES_CM_LEN_IN_WORDS] = {0x2B7E1516U, 0x28AED2A6U, 0xABF71588U, 0x9CF4F3CU};

/* AES counter measure Iv */
static const u32 AesCmIv[XASUFW_AES_CM_LEN_IN_WORDS] = {0xF0F1F2F3U, 0xF4F5F6F7U, 0xF8F9FAFBU, 0xFCFDFEFFU};
#endif

static const u8 EcdhPrivKey[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0xC9U, 0x80U, 0x68U, 0x98U, 0xA0U, 0x33U, 0x49U, 0x16U, 0xC8U, 0x60U, 0x74U, 0x88U, 0x80U,
	0xA5U, 0x41U, 0xF0U, 0x93U, 0xB5U, 0x79U, 0xA9U, 0xB1U, 0xF3U, 0x29U, 0x34U, 0xD8U, 0x6CU,
	0x36U, 0x3CU, 0x39U, 0x80U, 0x03U, 0x57U
};
static const u8 EcdhPubKey1[XASUFW_DOUBLE_P256_SIZE_IN_BYTES] = {
	0XF6U, 0X83U, 0X6AU, 0X8AU, 0XDDU, 0X91U, 0XCBU, 0X18U, 0X2DU, 0X8DU, 0X25U, 0X8DU, 0XDAU,
	0X66U, 0X80U, 0X69U, 0X0EU, 0XB7U, 0X24U, 0XA6U, 0X6DU, 0XC3U, 0XBBU, 0X60U, 0XD2U, 0X32U,
	0X25U, 0X65U, 0XC3U, 0X9EU, 0X4AU, 0XB9U, 0X1FU, 0X83U, 0X7AU, 0XA3U, 0X28U, 0X64U, 0X87U,
	0X0CU, 0XB8U, 0XE8U, 0XD0U, 0XACU, 0X2FU, 0XF3U, 0X1FU, 0X82U, 0X4EU, 0X7BU, 0XEDU, 0XDCU,
	0X4BU, 0XB7U, 0XADU, 0X72U, 0XC1U, 0X73U, 0XADU, 0X97U, 0X4BU, 0X28U, 0X9DU, 0XC2U
};

static const u8 EcdhExpSharedSecret[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0x1DU, 0xB8U, 0x09U, 0xC2U, 0x76U, 0xF2U, 0x16U, 0x10U, 0x79U, 0x11U, 0x68U, 0x52U, 0x8EU,
	0xFAU, 0x01U, 0x85U, 0x11U, 0x2EU, 0x78U, 0x65U, 0x50U, 0x36U, 0xAEU, 0xEDU, 0x87U, 0xC7U,
	0x15U, 0xA2U, 0x90U, 0x45U, 0xFDU, 0xFCU
};

#ifdef XASU_ECIES_ENABLE
static const u8 EciesRxPrivKey[XASU_ECC_P384_SIZE_IN_BYTES] = {
	0x02U, 0x87U, 0xF6U, 0x2AU, 0x5AU, 0xA8U, 0x43U, 0x2FU,
	0xF5U, 0xE9U, 0x56U, 0x18U, 0xECU, 0x8FU, 0x9CU, 0xCAU,
	0xA8U, 0x70U, 0xDDU, 0xE9U, 0x9CU, 0x30U, 0xB5U, 0x1BU,
	0x76U, 0x73U, 0x37U, 0x8EU, 0xFEU, 0x4CU, 0xCAU, 0xC5U,
	0x98U, 0xF4U, 0xBBU, 0xEBU, 0xBFU, 0xD8U, 0x99U, 0x3FU,
	0x9AU, 0xBBU, 0x74U, 0x7BU, 0x6AU, 0xD6U, 0x38U, 0xB9U
};

static const u8 EciesRxPubKey[XASUFW_DOUBLE_P384_SIZE_IN_BYTES] = {
	0xB3U, 0x64U, 0x18U, 0xA3U, 0x01U, 0x40U, 0x74U, 0xECU,
	0x9BU, 0xBCU, 0xC6U, 0xA4U, 0xB2U, 0x36U, 0x7AU, 0x4FU,
	0xB4U, 0x64U, 0xCCU, 0xA7U, 0xECU, 0x0AU, 0x32U, 0x4CU,
	0xB6U, 0x86U, 0x70U, 0xD5U, 0xC5U, 0xE0U, 0x3EU, 0x7AU,
	0x7EU, 0xB0U, 0x7DU, 0xA1U, 0x17U, 0xC5U, 0xEAU, 0x50U,
	0xB6U, 0x65U, 0xABU, 0x62U, 0xBDU, 0x02U, 0xA4U, 0x91U,
	0x4EU, 0xA2U, 0x99U, 0xC3U, 0x0EU, 0x7DU, 0x76U, 0xE2U,
	0xC5U, 0x90U, 0x5BU, 0xABU, 0xADU, 0xA2U, 0xD3U, 0xBBU,
	0x4EU, 0xE5U, 0xEBU, 0x35U, 0xA5U, 0xA2U, 0x36U, 0x05U,
	0xCDU, 0xB0U, 0xD5U, 0x13U, 0x34U, 0x71U, 0xA5U, 0x3EU,
	0xB9U, 0xE6U, 0x75U, 0x8EU, 0x49U, 0x10U, 0x5AU, 0x4EU,
	0xAFU, 0x29U, 0xD2U, 0x26U, 0x7BU, 0xA8U, 0x4EU, 0xF2U
};

static const u8 EciesIv[XASU_AES_IV_SIZE_96BIT_IN_BYTES] = {
	0x99U, 0xD1U, 0x58U, 0x32U, 0xCCU, 0x65U, 0xF6U, 0xB4U,
	0xC5U, 0xC5U, 0xC3U, 0x4FU
};
#endif

#ifdef XASU_KEYWRAP_ENABLE
static const u8 KeyWrapInput[XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES] = {
	0xffU, 0xe9U, 0x52U, 0x60U, 0x48U, 0x34U, 0xbfU, 0xf8U, 0x99U, 0xe6U, 0x36U, 0x58U, 0xf3U,
	0x42U, 0x46U, 0x81U, 0x5cU, 0x91U, 0x59U, 0x7eU, 0xb4U, 0x0aU, 0x21U, 0x72U, 0x9eU, 0x0aU,
	0x8aU, 0x95U, 0x9bU, 0x61U, 0xf2U
};

static u8 WrappedResult[XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES];
#endif

/*************************************************************************************************/
/**
 * @brief	This function executes the kat task handler.
 *
 * @param	KatTask		Pointer to the KAT task instance.
 *
 * @return
 *	- Always returns XASUFW_SUCCESS.
 *
 *************************************************************************************************/
s32 XAsufw_RunKatTaskHandler(void *KatTask)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XTask_TaskNode *SelfKatTask = (XTask_TaskNode *)KatTask;
	XSha *XAsufw_Sha3Ptr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	XTrng *XAsufw_TrngPtr = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

	/** Get the DMA instance pointer. */
	XAsufw_Dma *AsuDmaPtr = XAsufw_GetDmaInstance(ASUDMA_0_DEVICE_ID);

	XAsufw_Printf(DEBUG_GENERAL, "Running KAT tasks\r\n");

	/** Run AES KAT. */
	Status = XAsufw_AesOperationKat(AsuDmaPtr, XASU_AES_GCM_MODE);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_AES);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_AES_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}
	else {
		Status = XAsufw_AesOperationKat(AsuDmaPtr, XASU_AES_ECB_MODE);
		if (Status != XASUFW_SUCCESS) {
			/** On failure of KAT, disable the root resource. */
			XAsufw_DisableResource(XASUFW_AES);
			/** Mark KAT status as failed in RTCA area. */
			XASUFW_MARK_KAT_FAILED(XASU_MODULE_AES_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
			goto SLD;
#endif
		}
		else {
#ifdef XASU_AES_CM_ENABLE
			Status = XAsufw_AesDpaCmKat(AsuDmaPtr);
#endif
			if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_KAT_NOT_SUPPORTED_ON_QEMU)) {
				/** On failure of KAT, disable the root resource. */
				XAsufw_DisableResource(XASUFW_AES);
				/** Mark KAT status as failed in RTCA area. */
				XASUFW_MARK_KAT_FAILED(XASU_MODULE_AES_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
				goto SLD;
#endif
			}
			else {
				/** Set KAT status as passed in RTCA area. */
				XASUFW_SET_KAT_PASSED(XASU_MODULE_AES_ID);

				/** Set KAT status as passed in RTCA area. */
				XASUFW_SET_KAT_PASSED(XASU_MODULE_KEYWRAP_ID);
			}
		}
	}

	/** Run Keywrap and dependent KAT's. */
#if defined(XASU_KEYWRAP_ENABLE) || defined(XASU_RSA_PADDING_ENABLE)
	Status = RunKeyWrapAndDependentKat(AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}
#endif

	/** Run KDF and dependent(HMAC, SHA2-256/512) module. */
	Status = RunKdfAndDependentKat(AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}

	/** Run SHA3 KAT. */
	Status = XAsufw_ShaKat(XAsufw_Sha3Ptr, AsuDmaPtr, XASUFW_SHA3, XASU_SHA_MODE_256);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_SHA3);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_SHA3_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}
	else {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_SHA3_ID);
	}

	/** Run TRNG KAT. */
	Status = XTrng_PreOperationalSelfTests(XAsufw_TrngPtr);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_TRNG);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_TRNG_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}
	else {
		Status = XTrng_EnableDefaultMode(XAsufw_TrngPtr);
		if (Status != XASUFW_SUCCESS) {
			/** On failure of KAT, disable the root resource. */
			XAsufw_DisableResource(XASUFW_TRNG);
			/** Mark KAT status as failed in RTCA area. */
			XASUFW_MARK_KAT_FAILED(XASU_MODULE_TRNG_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
			goto SLD;
#endif
		}
		else {
			/** Set KAT status as passed in RTCA area. */
			XASUFW_SET_KAT_PASSED(XASU_MODULE_TRNG_ID);
		}
	}

	/** Run ECC signature generation and verification KAT on ECC core for P-256 curve. */
	Status = XAsufw_EccCoreKat(AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_ECC);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_ECC_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
		goto SLD;
#endif
	}
	else {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_ECC_ID);
#ifdef XASU_RSA_PADDING_ENABLE
		if(!(XASUFW_IS_KAT_PASSED(XASU_MODULE_RSA_ID))) {
			/* If RSA KAT is not passed, skip ECC KAT on RSA core. */
			goto END;
		}
#endif
#ifdef XASU_ECC_SUPPORT_EDWARD_P25519
		/** Run ECC KAT on RSA core using ED25519 curve. */
		Status = XAsufw_RsaEccKat(AsuDmaPtr, XASU_ECC_NIST_ED25519);
#endif /* XASU_ECC_SUPPORT_EDWARD_P25519 */
		if (Status != XASUFW_SUCCESS) {
			/** On failure of KAT, disable the root resource. */
			XAsufw_DisableResource(XASUFW_RSA);
			/** Mark KAT status as failed in RTCA area. */
			XASUFW_MARK_KAT_FAILED(XASU_MODULE_RSA_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
			goto SLD;
#endif
		}
		else {
#ifdef XASU_ECC_SUPPORT_EDWARD_P448
			/** Run ECC KAT on RSA core using ED448 curve. */
			Status = XAsufw_RsaEccKat(AsuDmaPtr, XASU_ECC_NIST_ED448);
#endif /* XASU_ECC_SUPPORT_EDWARD_P448 */
			if (Status != XASUFW_SUCCESS) {
				/** On failure of KAT, disable the root resource. */
				XAsufw_DisableResource(XASUFW_RSA);
				/** Mark KAT status as failed in RTCA area. */
				XASUFW_MARK_KAT_FAILED(XASU_MODULE_RSA_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
				goto SLD;
#endif
			}
			else {
#ifdef XASU_ECC_SUPPORT_NIST_P256
				/** Run ECC KAT on RSA core using P-256 curve. */
				Status = XAsufw_RsaEccKat(AsuDmaPtr, XASU_ECC_NIST_P256);
#endif /* XASU_ECC_SUPPORT_NIST_P256 */
				if (Status != XASUFW_SUCCESS) {
					/** On failure of KAT, disable the root resource. */
					XAsufw_DisableResource(XASUFW_RSA);
					/** Mark KAT status as failed in RTCA area. */
					XASUFW_MARK_KAT_FAILED(XASU_MODULE_RSA_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
					goto SLD;
#endif
				}
				else {
					/** Set KAT status as passed in RTCA area. */
					XASUFW_SET_KAT_PASSED(XASU_MODULE_RSA_ID);
				}
			}
		}
	}

#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
SLD:
	/* TODO: To send IPI to PLM to trigger secure lockdown. */
#endif

#ifdef XASU_RSA_PADDING_ENABLE
END:
#endif
	XAsufw_RMW(XASU_RTCA_KAT_EXEC_STATUS_ADDR, XASU_RTCA_KAT_EXEC_STATUS_MASK,
			XASU_RTCA_KAT_EXEC_STATUS_VALUE);
	XAsufw_Printf(DEBUG_GENERAL, "KATs execution completed.\r\n");

	/** Delete self task after completion. */
	XTask_Delete(SelfKatTask);

	/** Returning success as we have disabled the module for which the KAT has failed. */
	return XASUFW_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief	This function runs SHA KAT on the given SHA instance for specified mode.
 *
 * @param	XAsufw_ShaInstance	Pointer to the SHA instance.
 * @param	AsuDmaPtr		ASU DMA instance pointer.
 * @param	ShaResource		SHA2/SHA3 resource.
 * @param	ShaMode			SHA mode (256/512).
 *
 * @return
 *	- XASUFW_SUCCESS, if SHA KAT is successful.
 *	- XASUFW_SHA_HASH_COMPARISON_FAILED, if expected and generated hash comparison fails.
 *	- XASUFW_SHA_KAT_FAILED, if XAsufw_ShaKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, XAsufw_Dma *AsuDmaPtr, XAsufw_Resource ShaResource,
	u32 ShaMode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 OutVal[XASU_SHA_512_HASH_LEN] = {0};
	u32 HashLen = 0U;
	const u8 *ExpHash = NULL;

	/** Determine expected hash based on mode and resource. */
	switch (ShaMode) {
		case XASU_SHA_MODE_256:
			ExpHash = (ShaResource == XASUFW_SHA2) ? ExpSha2_256Hash : ExpSha3_256Hash;
			HashLen = XASU_SHA_256_HASH_LEN;
			break;
		case XASU_SHA_MODE_512:
			ExpHash = ExpSha2_512Hash;
			HashLen = XASU_SHA_512_HASH_LEN;
			break;
		default:
			Status = XASUFW_SHA_INVALID_SHA_MODE;
			goto END;
	}

	/** Perform SHA start operation. */
	Status = XSha_Start(XAsufw_ShaInstance, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Perform SHA update operation. */
	Status = XSha_Update(XAsufw_ShaInstance, AsuDmaPtr, (UINTPTR)KatMessage,
		XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Perform SHA finish operation. */
	Status = XSha_Finish(XAsufw_ShaInstance, AsuDmaPtr, (u32 *)OutVal, HashLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Compare generated hash with expected hash. */
	Status = Xil_SMemCmp(OutVal, HashLen, ExpHash, HashLen, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_HASH_COMPARISON_FAILED;
	}

END_CLR:
	/** Securely zeroize output buffer. */
	SStatus = Xil_SMemSet(&OutVal[0U], HashLen, 0U, HashLen);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

END:
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_KAT_FAILED);
	}

	return Status;
}

#ifdef XASU_RSA_PADDING_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function runs RSA-OAEP KAT for encryption and decryption using 2048 bit key
 * 		and SHA2-256.
 *
 * @param	AsuDmaPtr	Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if RSA OAEP KAT is successful.
 *	- XASUFW_MEM_COPY_FAIL, if copying of data fails.
 * 	- XASUFW_RSA_OAEP_ENCODE_ERROR, if OAEP encode operation fails.
 * 	- XASUFW_RSA_OAEP_DECODE_ERROR, if OAEP decode operation fails.
 * 	- XASUFW_RSA_KAT_FAILED, if XAsufw_RsaEncDecOaepOpKat API fails.
 * 	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaEncDecOaepOpKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 EncOutput[XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES];
	u8 DecOutput[XASUFW_RSA_INPUT_LEN_IN_BYTES];
	XAsu_RsaOaepPaddingParams RsaOaepPaddingParam;
	XAsu_RsaPubKeyComp PubKeyParam;
	XAsu_RsaPvtKeyComp PvtKeyParam;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

	PubKeyParam.Keysize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaModulus,
		XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	PvtKeyParam.PubKeyComp = PubKeyParam;
	PvtKeyParam.PrimeCompOrTotientPrsnt = 0U;

	Status = Xil_SMemCpy(PvtKeyParam.PvtExp, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaPvtExp,
		XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	RsaOaepPaddingParam.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)RsaData;
	RsaOaepPaddingParam.XAsu_RsaOpComp.OutputDataAddr = (u64)(UINTPTR)EncOutput;
	RsaOaepPaddingParam.XAsu_RsaOpComp.Len = XASUFW_RSA_INPUT_LEN_IN_BYTES;
	RsaOaepPaddingParam.XAsu_RsaOpComp.KeySize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	RsaOaepPaddingParam.XAsu_RsaOpComp.KeyCompAddr = (u64)(UINTPTR)&PubKeyParam;
	RsaOaepPaddingParam.XAsu_RsaOpComp.ExpoCompAddr = 0U;
	RsaOaepPaddingParam.ShaType = XASU_SHA2_TYPE;
	RsaOaepPaddingParam.ShaMode = XASU_SHA_MODE_256;
	RsaOaepPaddingParam.OptionalLabelAddr = (u64)(UINTPTR)RsaOpt;
	RsaOaepPaddingParam.OptionalLabelSize = XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES;

	Status = XRsa_OaepEncode(AsuDmaPtr, Sha2Ptr, &RsaOaepPaddingParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_ENCODE_ERROR;
		goto END;
	}

	RsaOaepPaddingParam.XAsu_RsaOpComp.KeyCompAddr =  (u64)(UINTPTR)&PvtKeyParam;
	RsaOaepPaddingParam.XAsu_RsaOpComp.Len = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	RsaOaepPaddingParam.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)EncOutput;
	RsaOaepPaddingParam.XAsu_RsaOpComp.OutputDataAddr = (u64)(UINTPTR)DecOutput;

	Status = XRsa_OaepDecode(AsuDmaPtr, Sha2Ptr, &RsaOaepPaddingParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_DECODE_ERROR;
		goto END;
	}

	Status = Xil_SMemCmp_CT(RsaData, XASUFW_RSA_INPUT_LEN_IN_BYTES,
		(u8 *)(UINTPTR)RsaOaepPaddingParam.XAsu_RsaOpComp.OutputDataAddr,
		XASUFW_RSA_INPUT_LEN_IN_BYTES, XASUFW_RSA_INPUT_LEN_IN_BYTES);

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&EncOutput[0U], XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, 0U,
		XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	SStatus |= Xil_SMemSet(&DecOutput[0U], XASUFW_RSA_INPUT_LEN_IN_BYTES, 0U,
		XASUFW_RSA_INPUT_LEN_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}
	else {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_KAT_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs RSA-PSS KAT for sign generation and verification using
 * 		2048 bit key and SHA2-256.
 *
 * @param	AsuDmaPtr	Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if RSA PSS signature generation and verification KAT is successful.
 *	- XASUFW_MEM_COPY_FAIL, if copying of data fails.
 * 	- XASUFW_RSA_PSS_SIGN_GEN_ERROR, if PSS sign generation operation fails.
 * 	- XASUFW_RSA_PSS_SIGN_VER_ERROR, if PSS sign verification operation fails.
 * 	- XASUFW_RSA_KAT_FAILED, if XAsufw_RsaPssSignGenAndVerifOpKat API fails.
 * 	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaPssSignGenAndVerifOpKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 RsaSignatureOutput[XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES];
	XAsu_RsaPaddingParams RsaPaddingParam;
	XAsu_RsaPubKeyComp PubKeyParam;
	XAsu_RsaPvtKeyComp PvtKeyParam;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

	PubKeyParam.Keysize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaModulus,
		    XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	PvtKeyParam.PubKeyComp = PubKeyParam;
	PvtKeyParam.PrimeCompOrTotientPrsnt = 0U;

	Status = Xil_SMemCpy(PvtKeyParam.PvtExp, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaPvtExp,
		    XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	RsaPaddingParam.XAsu_RsaOpComp.KeySize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	RsaPaddingParam.XAsu_RsaOpComp.KeyCompAddr = (u64)(UINTPTR)&PvtKeyParam;
	RsaPaddingParam.XAsu_RsaOpComp.ExpoCompAddr = 0U;
	RsaPaddingParam.SaltLen = XASUFW_RSA_PSS_SALT_LEN_IN_BYTES;
	RsaPaddingParam.InputDataType = 0U;
	RsaPaddingParam.SignatureLen = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	RsaPaddingParam.SignatureDataAddr = 0U;
	RsaPaddingParam.ShaType = XASU_SHA2_TYPE;
	RsaPaddingParam.ShaMode = XASU_SHA_MODE_256;
	RsaPaddingParam.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)RsaData;
	RsaPaddingParam.XAsu_RsaOpComp.Len = XASUFW_RSA_INPUT_LEN_IN_BYTES;
	RsaPaddingParam.XAsu_RsaOpComp.OutputDataAddr = (u64)(UINTPTR)RsaSignatureOutput;

	Status = XRsa_PssSignGenerate(AsuDmaPtr, Sha2Ptr, &RsaPaddingParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PSS_SIGN_GEN_ERROR;
		goto END;
	}

	RsaPaddingParam.XAsu_RsaOpComp.OutputDataAddr = 0U;
	RsaPaddingParam.XAsu_RsaOpComp.KeyCompAddr = (u64)(UINTPTR)&PubKeyParam;
	RsaPaddingParam.SignatureDataAddr = (u64)(UINTPTR)RsaSignatureOutput;

	Status = XRsa_PssSignVerify(AsuDmaPtr, Sha2Ptr, &RsaPaddingParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PSS_SIGN_VER_ERROR;
	}

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&RsaSignatureOutput[0U], XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, 0U,
		XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}
	else {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_KAT_FAILED);
	}

	return Status;
}
#endif

/*************************************************************************************************/
/**
 * @brief	This function runs ECC KAT on ECC core for P-256 curve.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if ECC KAT is successful.
 *	- XASUFW_ECC_PUBKEY_COMPARISON_FAILED, if public key generation fails.
 *	- XASUFW_ECC_SIGNATURE_COMPARISON_FAILED, if generated and expected signature
	comparison fails.
 *	- XASUFW_ECC_KAT_FAILED, if XAsufw_EccCoreKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_EccCoreKat(XAsufw_Dma *AsuDmaPtr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 GenPubKey[XASUFW_DOUBLE_P256_SIZE_IN_BYTES];
	u8 GenSign[XASUFW_DOUBLE_P256_SIZE_IN_BYTES];
	XEcc *EccInstance = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** Generate the public key using the provided private key. */
	Status = XEcc_GeneratePublicKey(EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, (u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated public key with expected public key. */
	Status = Xil_SMemCmp(GenPubKey, XASUFW_DOUBLE_P256_SIZE_IN_BYTES, EccExpPubKeyP256,
			     XASUFW_DOUBLE_P256_SIZE_IN_BYTES, XASUFW_DOUBLE_P256_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_PUBKEY_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Validate the generated ECC public key. */
	Status = XEcc_ValidatePublicKey(EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Generate signature using core API XEcc_GenerateSignature. */
	Status = XEcc_GenerateSignature(EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, EccEphemeralKey,
					(u64)(UINTPTR)EccHash, XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated signature with the expected signature. */
	Status = Xil_SMemCmp(GenSign, XASUFW_DOUBLE_P256_SIZE_IN_BYTES, EccExpSignP256,
			     XASUFW_DOUBLE_P256_SIZE_IN_BYTES, XASUFW_DOUBLE_P256_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_SIGNATURE_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Verify signature using core API XEcc_VerifySignature. */
	Status = XEcc_VerifySignature(EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
				      XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenPubKey,
				      (u64)(UINTPTR)EccHash, XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

END:
	/** Zeroize local copy of public key. */
	SStatus = Xil_SMemSet(&GenPubKey[0U], XASUFW_DOUBLE_P256_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P256_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of generated signature. */
	SStatus = Xil_SMemSet(&GenSign[0U], XASUFW_DOUBLE_P256_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P256_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_KAT_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs ECC KAT on RSA core for user specified curve.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 * @param	CurveType	Type of elliptic curve specified by user.
 *
 * @return
 *	- XASUFW_SUCCESS, if ECC KAT is successful.
 *	- XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED, if public key generation fails.
 *	- XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED, if generated and expected signature
 *	  comparison fails.
 *	- XASUFW_RSA_ECC_KAT_FAILED, if XAsufw_RsaEccKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaEccKat(XAsufw_Dma *AsuDmaPtr, u8 CurveType)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 GenPubKey[XASUFW_DOUBLE_P448_SIZE_IN_BYTES];
	u8 GenSign[XASUFW_DOUBLE_P448_SIZE_IN_BYTES];
	u8 CurveSize = 0U;
	u8 InputLen = 0U;
	const u8 *PrivateKey = NULL;
	const u8 *ExpPubKey = NULL;
	const u8 *ExpSign = NULL;

	switch (CurveType) {
		case XASU_ECC_NIST_P256:
			PrivateKey = EccPrivKey;
			ExpPubKey = EccExpPubKeyP256;
			ExpSign = EccExpSignP256;
			CurveSize = XASU_ECC_P256_SIZE_IN_BYTES;
			InputLen = XASU_ECC_P256_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_ED25519:
			PrivateKey = EccPrivKey25519;
			ExpPubKey = EccExpPubKey25519;
			ExpSign = EccExpSign25519;
			CurveSize = XASU_ECC_P256_SIZE_IN_BYTES;
			InputLen = 0U;
			break;
		case XASU_ECC_NIST_ED448:
			PrivateKey = EccPrivKey448;
			ExpPubKey = EccExpPubKeyP448;
			ExpSign = EccExpSignP448;
			CurveSize = XASU_ECC_P448_SIZE_IN_BYTES;
			InputLen = 0U;
			break;

		default:
			Status = XASUFW_RSA_ECC_INVALID_PARAM;
			goto END;
	}

	/** Generate the public key using the provided private key. */
	Status = XRsa_EccGeneratePubKey(AsuDmaPtr, CurveType, CurveSize,
					(u64)(UINTPTR)PrivateKey, (u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated public key with expected public key. */
	Status = Xil_SMemCmp(GenPubKey, XASUFW_DOUBLE_VALUE(CurveSize), ExpPubKey,
			     XASUFW_DOUBLE_VALUE(CurveSize), XASUFW_DOUBLE_VALUE(CurveSize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Validate the generated ECC public key. */
	Status = XRsa_EccValidatePubKey(AsuDmaPtr, CurveType, CurveSize,
					(u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Generate signature using RSA core API XRsa_EccGenerateSignature. */
	Status = XRsa_EccGenerateSignature(AsuDmaPtr, CurveType,
					   CurveSize, (u64)(UINTPTR)PrivateKey, EccEphemeralKey,
					   (u64)(UINTPTR)EccHash, InputLen, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated signature message with the expected. */
	Status = Xil_SMemCmp(GenSign, XASUFW_DOUBLE_VALUE(CurveSize), ExpSign,
			     XASUFW_DOUBLE_VALUE(CurveSize), XASUFW_DOUBLE_VALUE(CurveSize));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Verify signature using RSA core API XRsa_EccVerifySignature. */
	Status = XRsa_EccVerifySignature(AsuDmaPtr, CurveType, CurveSize,
					 (u64)(UINTPTR)ExpPubKey, (u64)(UINTPTR)EccHash,
					 InputLen, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

END:
	/** Zeroize local copy of public key. */
	SStatus = Xil_SMemSet(&GenPubKey[0U], XASUFW_DOUBLE_P448_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P448_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of generated signature. */
	SStatus = Xil_SMemSet(&GenSign[0U], XASUFW_DOUBLE_P448_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P448_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_KAT_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes AES KAT using the user-specified engine mode (GCM/ECB).
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 * @param	EngineMode	AES engine mode.
 *
 * @return
 *	- XASUFW_SUCCESS, if AES GCM KAT execution is successful.
 *	- XASUFW_AES_WRITE_KEY_FAILED, if Key write to AES USER key register fails.
 *	- XASUFW_AES_INIT_FAILED, if initialization of AES engine fails.
 *	- XASUFW_AES_UPDATE_FAILED, if update of data to AES engine fails.
 *	- XASUFW_AES_FINAL_FAILED, if completion of final AES operation fails.
 *	- XASUFW_AES_KAT_FAILED, if XAsufw_AesOperationKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_AesOperationKat(XAsufw_Dma *AsuDmaPtr, u8 EngineMode)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XAes *AesInstance = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAsu_AesKeyObject KeyObject;
	const u8 *AesExpectedCt = (EngineMode == XASU_AES_GCM_MODE) ? ExpAesGcmCt : ExpAesEcbCt;
	const u8 *IvPtr = NULL;
	const u8 *TagPtr = NULL;
	u8 IvLen;
	u8 TagLen;
	u8 AesOutData[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {0};
	u8 AesTag[XASUFW_AES_TAG_LEN_IN_BYTES] = {0};

	/** Configure key object. */
	KeyObject.KeyAddress = (u64)(UINTPTR)AesKey;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeySize = XASU_AES_KEY_SIZE_256_BITS;

	/** Write key to AES registers. */
	Status = XAes_WriteKey(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
		goto END;
	}

	/** Determine IV based on mode and operation. */
	IvPtr = (EngineMode == XASU_AES_GCM_MODE) ? AesIv : NULL;
	IvLen = (EngineMode == XASU_AES_GCM_MODE) ? XASUFW_AES_IV_LEN_IN_BYTES : 0U;

	/**
	 * Initialize the AES engine and load the provided key and IV to the AES engine for
	 * encryption operation.
	 */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)IvPtr,
		IvLen, EngineMode, XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		goto END;
	}

	/** Process AAD, only for AES-GCM mode. */
	if (EngineMode == XASU_AES_GCM_MODE) {
		Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)AesAad, 0U,
			XASUFW_AES_AAD_LEN_IN_BYTES, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
			goto END;
		}
	}

	/** Process input data. */
	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)KatMessage,
		(u64)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		goto END;
	}

	TagPtr = (EngineMode == XASU_AES_GCM_MODE) ? AesTag : NULL;
	TagLen = (EngineMode == XASU_AES_GCM_MODE) ? XASUFW_AES_TAG_LEN_IN_BYTES : 0U;

	/** Perform AES final operation for encryption. */
	Status = XAes_Final(AesInstance, AsuDmaPtr, (u32)(UINTPTR)TagPtr, TagLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
		goto END;
	}

	/** Compare encrypted data with expected ciphertext data. */
	Status = Xil_SMemCmp_CT(AesExpectedCt, XASUFW_KAT_MSG_LENGTH_IN_BYTES,
		AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Compare generated GCM tag with the expected GCM tag. */
	if (EngineMode == XASU_AES_GCM_MODE) {
		Status = Xil_SMemCmp_CT(ExpAesGcmTag, TagLen, TagPtr, TagLen, TagLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * Initialize the AES engine and load the provided key and IV to the AES engine for
	 * decryption operation.
	 */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)IvPtr,
		IvLen, EngineMode, XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		goto END;
	}

	/** Process AAD, only for AES-GCM mode. */
	if (EngineMode == XASU_AES_GCM_MODE) {
		Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)AesAad, 0U,
			XASUFW_AES_AAD_LEN_IN_BYTES, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
			goto END;
		}
	}

	/** Process input data. */
	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)AesExpectedCt,
		(u64)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		goto END;
	}

	/** Perform AES final operation for encryption. */
	Status = XAes_Final(AesInstance, AsuDmaPtr, (u32)(UINTPTR)TagPtr, TagLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
		goto END;
	}

	/** Compare decrypted data with expected input data. */
	Status = Xil_SMemCmp_CT(KatMessage, XASUFW_KAT_MSG_LENGTH_IN_BYTES, AesOutData,
		XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASUFW_KAT_MSG_LENGTH_IN_BYTES);

END:
	/** Zeroize local copy of buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	ClearStatus |= Xil_SecureZeroize((u8 *)(UINTPTR)AesTag, XASUFW_AES_TAG_LEN_IN_BYTES);
	if ((Status == XASUFW_SUCCESS) && (Status == XASUFW_SUCCESS)) {
		Status = ClearStatus;
	}
	else {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_KAT_FAILED);
	}

	/** Clear the key written to the XASU_AES_USER_KEY_0 key source. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(AesInstance, KeyObject.KeySrc));

	return Status;
}

#ifdef XASU_AES_CM_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function executes AES DPA CM encryption and decryption KAT.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if AES DPA CM KAT is successful.
 *	- XASUFW_AES_DPA_CM_ENC_OP_FAILED, if AES DPA CM KAT encrypt operation fails.
 *	- XASUFW_AES_DPA_CM_DEC_OP_FAILED, if AES DPA CM KAT encrypt operation fails.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK_FAILED, if intermediate DPA CM check fails.
 *	- XASUFW_AES_DPA_CM_KAT_FAILED, if XAsufw_AesDpaCmKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_AesDpaCmKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XAes *InstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	u32 Output0[XASUFW_AES_CM_LEN_IN_WORDS];
	u32 Output1[XASUFW_AES_CM_LEN_IN_WORDS];
	u32 MaskedOutput0[XASUFW_AES_CM_LEN_IN_WORDS + XASUFW_AES_CM_LEN_IN_WORDS];
	u32 MaskedOutput1[XASUFW_AES_CM_LEN_IN_WORDS + XASUFW_AES_CM_LEN_IN_WORDS];
	u32 MaskedPt[XASUFW_AES_CM_MASK_BUF_WORD_LEN];
	u32 MaskedKey[XASUFW_AES_CM_MASK_BUF_WORD_LEN];
	u32 Index;
	u32 InputDataAddr = (u32)(UINTPTR)XRsa_GetDataBlockAddr();

	if ((XASUFW_PLATFORM == PMC_TAP_VERSION_PLATFORM_QEMU) ||
		(XASUFW_PLATFORM == PMC_TAP_VERSION_PLATFORM_COSIM)) {
		XAsufw_Printf(DEBUG_INFO, "INFO: AES DPACM KAT is not supported on QEMU\r\n");
		Status = XASUFW_KAT_NOT_SUPPORTED_ON_QEMU;
		goto RET;
	}

	/** Mask the input data. */
	XAsufw_ApplyMask(AesCmPt, MaskedPt, XASUFW_AES_CM_SPLIT_MASK, sizeof(AesCmPt));

	/** Mask the key. */
	XAsufw_ApplyMask(AesCmKey, MaskedKey, XASUFW_AES_CM_SPLIT_MASK, sizeof(AesCmKey));

	/** Load mask data to plaintext address. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		XAsufw_WriteReg((InputDataAddr + (Index * XASUFW_WORD_LEN_IN_BYTES)),
			XASUFW_AES_CM_SPLIT_MASK);
	}

	/** Load masked data to plaintext address. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		XAsufw_WriteReg(((InputDataAddr + XASUFW_AES_CM_SPLIT_ALIGNED_LENGTH) +
			(Index * XASUFW_WORD_LEN_IN_BYTES)),MaskedPt[Index]);
	}

	/** First encryption run. */
	Status = XAsufw_AesDpaCmOperation(InstancePtr, AsuDmaPtr, InputDataAddr,
		MaskedOutput0, MaskedKey, AesCmIv, XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_ENC_OP_FAILED;
		goto END;
	}

	/** Calculate unmasked result for first run. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		Output0[Index] = MaskedOutput0[Index] ^
			MaskedOutput0[XASUFW_AES_CM_LEN_IN_WORDS + Index];
	}

	/** Compare encrypted data with expected ciphertext data. */
	Status = Xil_SMemCmp_CT(AesCmCt, XASUFW_AES_CM_DATA_LEN_IN_BYTES, Output0,
		XASUFW_AES_CM_DATA_LEN_IN_BYTES, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Second encryption run. */
	Status = XAsufw_AesDpaCmOperation(InstancePtr, AsuDmaPtr, InputDataAddr,
		MaskedOutput1, MaskedKey, AesCmIv, XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_ENC_OP_FAILED;
		goto END;
	}

	/** Calculate unmasked result for second run. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		Output1[Index] = MaskedOutput1[Index] ^
			MaskedOutput1[XASUFW_AES_CM_LEN_IN_WORDS + Index];;
	}

	/** Compare encrypted data with expected ciphertext data. */
	Status = Xil_SMemCmp_CT(AesCmCt, XASUFW_AES_CM_DATA_LEN_IN_BYTES, Output1,
		XASUFW_AES_CM_DATA_LEN_IN_BYTES, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Validate intermediate output. */
	Status = XAsufw_ValidateDpaIntermediateValues(MaskedOutput0,
		&MaskedOutput0[XASUFW_AES_CM_LEN_IN_WORDS], MaskedOutput1,
		&MaskedOutput1[XASUFW_AES_CM_LEN_IN_WORDS]);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Output data masking for decryption. */
	XAsufw_ApplyMask(AesCmCt, MaskedPt, XASUFW_AES_CM_SPLIT_MASK, sizeof(AesCmCt));

	/** Load mask data to plaintext address. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		XAsufw_WriteReg((InputDataAddr + (Index * XASUFW_WORD_LEN_IN_BYTES)),
			XASUFW_AES_CM_SPLIT_MASK);
	}

	/** Load masked data to plaintext address. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		XAsufw_WriteReg(((InputDataAddr + XASUFW_AES_CM_SPLIT_ALIGNED_LENGTH) +
			(Index * XASUFW_WORD_LEN_IN_BYTES)),MaskedPt[Index]);
	}

	/** First decryption run. */
	Status = XAsufw_AesDpaCmOperation(InstancePtr, AsuDmaPtr, InputDataAddr,
		MaskedOutput0, MaskedKey, AesCmIv, XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_DEC_OP_FAILED;
		goto END;
	}

	/** Calculate unmasked result for second run. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		Output0[Index] = MaskedOutput0[Index] ^
			MaskedOutput0[XASUFW_AES_CM_LEN_IN_WORDS + Index];
	}

	/** Compare decrypted data with expected input data. */
	Status = Xil_SMemCmp_CT(AesCmPt, XASUFW_AES_CM_DATA_LEN_IN_BYTES, Output0,
		XASUFW_AES_CM_DATA_LEN_IN_BYTES, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Second decryption run. */
	Status = XAsufw_AesDpaCmOperation(InstancePtr, AsuDmaPtr, InputDataAddr,
		MaskedOutput1, MaskedKey, AesCmIv, XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_DEC_OP_FAILED;
		goto END;
	}

	/** Calculate unmasked result for second run. */
	for (Index = 0U; Index < XASUFW_AES_CM_LEN_IN_WORDS; Index++) {
		Output1[Index] = MaskedOutput1[Index] ^
			MaskedOutput1[XASUFW_AES_CM_LEN_IN_WORDS + Index];;;
	}

	/** Compare decrypted data with expected input data. */
	Status = Xil_SMemCmp_CT(AesCmPt, XASUFW_AES_CM_DATA_LEN_IN_BYTES, Output1,
		XASUFW_AES_CM_DATA_LEN_IN_BYTES, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Validate intermediate output. */
	Status = XAsufw_ValidateDpaIntermediateValues(MaskedOutput0,
		&MaskedOutput0[XASUFW_AES_CM_LEN_IN_WORDS], MaskedOutput1,
		&MaskedOutput1[XASUFW_AES_CM_LEN_IN_WORDS]);

END:
	/** Zeroize local copy of buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Output0, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	ClearStatus |= Xil_SecureZeroize((u8 *)(UINTPTR)Output1, XASUFW_AES_CM_DATA_LEN_IN_BYTES);
	if ((Status == XASUFW_SUCCESS) && (Status == XASUFW_SUCCESS)) {
		Status = ClearStatus;
	}
	else {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_DPA_CM_KAT_FAILED);
	}

RET:
	return Status;
}
#endif /* XASU_AES_CM_ENABLE */

#ifdef XASU_HMAC_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function runs HMAC KAT on the given HMAC instance using SHA2-256.
 *
 * @param	AsuDmaPtr		Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if HMAC KAT is successful.
 *	- XASUFW_HMAC_KAT_COMPARISON_FAILED, if expected and generated HMAC comparison fails.
 *	- XASUFW_HMAC_KAT_FAILED, if XAsufw_HmacOperationKat API fails.
 *
 *************************************************************************************************/
s32 XAsufw_HmacOperationKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XHmac *XAsufw_HmacInstance = XHmac_GetInstance();
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u8 HmacOutput[XASU_SHA_256_HASH_LEN] = {0U};

	/** Perform HMAC init operation. */
	Status = XHmac_Init(XAsufw_HmacInstance, AsuDmaPtr, Sha2Ptr, (u64)(UINTPTR)EccPrivKey,
			    XASU_ECC_P256_SIZE_IN_BYTES, XASU_SHA_MODE_256, XASU_SHA_256_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Perform HMAC update operation. */
	Status = XHmac_Update(XAsufw_HmacInstance, AsuDmaPtr, (u64)(UINTPTR)KatMessage,
			      XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Perform HMAC final operation. */
	Status = XHmac_Final(XAsufw_HmacInstance, AsuDmaPtr, (u32 *)HmacOutput);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Compare generated HMAC output with expected HMAC output. */
	Status = Xil_SMemCmp(HmacOutput, XASU_SHA_256_HASH_LEN, ExpHmacOutput,
			     XASU_SHA_256_HASH_LEN,
			     XASU_SHA_256_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_HMAC_KAT_COMPARISON_FAILED;
	}

END_CLR:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&HmacOutput[0U], XASU_SHA_256_HASH_LEN, 0U,
			      XASU_SHA_256_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}
END:
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_KAT_FAILED);
	}

	return Status;
}
#endif /* XASU_HMAC_ENABLE */

#ifdef XASU_KDF_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function runs KDF KAT using SHA2-256.
 *
 * @param	AsuDmaPtr		Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if KDF KAT is successful.
 *	- XASUFW_KDF_KAT_COMPARISON_FAILED, if expected and generated KDF comparison fails.
 *	- XASUFW_KDF_KAT_FAILED, if XAsufw_KdfOperationKat API fails.
 *
 *************************************************************************************************/
s32 XAsufw_KdfOperationKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u8 KdfOutput[XASU_SHA_256_HASH_LEN] = {0U};
	XAsu_KdfParams Params;

	Params.KeyInAddr = (u64)(UINTPTR)EccPrivKey;
	Params.KeyInLen = XASU_ECC_P256_SIZE_IN_BYTES;
	Params.ContextAddr = (u64)(UINTPTR)KatMessage;
	Params.ContextLen = XASUFW_KAT_MSG_LENGTH_IN_BYTES;
	Params.KeyOutAddr = (u64)(UINTPTR)KdfOutput;
	Params.KeyOutLen = XASU_SHA_256_HASH_LEN;
	Params.ShaMode = XASU_SHA_MODE_256;
	Params.ShaType = XASU_SHA2_TYPE;

	/** Perform KDF generate with known inputs. */
	Status = XKdf_Generate(AsuDmaPtr, Sha2Ptr, &Params);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Compare generated KDF key output with expected KDF key output. */
	Status = Xil_SMemCmp(KdfOutput, XASU_SHA_256_HASH_LEN, ExpKdfOutput,
			     XASU_SHA_256_HASH_LEN,
			     XASU_SHA_256_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KDF_KAT_COMPARISON_FAILED;
	}

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&KdfOutput[0U], XASU_SHA_256_HASH_LEN, 0U,
			      XASU_SHA_256_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_KAT_FAILED);
	}

	return Status;
}
#endif /* XASU_KDF_ENABLE */

#ifdef XASU_ECIES_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function runs ECIES KAT using ECC curve P-384, SHA2-256 and AES-GCM mode.
 *
 * @param	AsuDmaPtr	Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if ECIES KAT is successful.
 *	- XASUFW_ECIES_KAT_COMPARISON_FAILED, if expected and generated ECIES comparison fails.
 *	- XASUFW_ECIES_KAT_FAILED, if XAsufw_EciesOperationKat API fails.
 *
 *************************************************************************************************/
s32 XAsufw_EciesOperationKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAsu_EciesParams Params;
	u8 EciesTxPubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 EciesMac[XASUFW_AES_TAG_LEN_IN_BYTES];
	u8 EciesEncOut[XASUFW_KAT_MSG_LENGTH_IN_BYTES];
	u8 EciesDecOut[XASUFW_KAT_MSG_LENGTH_IN_BYTES];

	Params.AesKeySize = XASU_AES_KEY_SIZE_128_BITS;
	Params.DataLength = XASUFW_KAT_MSG_LENGTH_IN_BYTES;
	Params.EccCurveType = XASU_ECC_NIST_P384;
	Params.EccKeyLength = XASU_ECC_P384_SIZE_IN_BYTES;
	Params.InDataAddr = (u64)(UINTPTR)KatMessage;
	Params.IvAddr = (u64)(UINTPTR)EciesIv;
	Params.IvLength = XASU_AES_IV_SIZE_96BIT_IN_BYTES;
	Params.MacAddr = (u64)(UINTPTR)EciesMac;
	Params.MacLength = XASU_AES_MAX_TAG_LENGTH_IN_BYTES;
	Params.OutDataAddr = (u64)(UINTPTR)EciesEncOut;
	Params.TxKeyAddr = (u64)(UINTPTR)EciesTxPubKey;
	Params.RxKeyAddr = (u64)(UINTPTR)EciesRxPubKey;
	Params.ContextAddr = (u64)(UINTPTR)RsaData;
	Params.ContextLen = XASUFW_KAT_MSG_LENGTH_IN_BYTES;
	Params.ShaMode = XASU_SHA_MODE_256;
	Params.ShaType = XASU_SHA2_TYPE;

	/** Perform ECIES encryption with known inputs. */
	Status = XEcies_Encrypt(AsuDmaPtr, Sha2Ptr, AesInstancePtr, &Params);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Params.InDataAddr = (u64)(UINTPTR)EciesEncOut;
	Params.OutDataAddr = (u64)(UINTPTR)EciesDecOut;
	Params.RxKeyAddr = (u64)(UINTPTR)EciesRxPrivKey;

	/** Perform ECIES decryption with known inputs. */
	Status = XEcies_Decrypt(AsuDmaPtr, Sha2Ptr, AesInstancePtr, &Params);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Compare generated decrypted output with input message. */
	Status = Xil_SMemCmp(EciesDecOut, XASUFW_KAT_MSG_LENGTH_IN_BYTES, KatMessage,
			     XASUFW_KAT_MSG_LENGTH_IN_BYTES,
			     XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_KAT_COMPARISON_FAILED;
		goto END;
	}

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&EciesEncOut[0U], XASUFW_KAT_MSG_LENGTH_IN_BYTES, 0U,
			      XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	SStatus = Xil_SMemSet(&EciesDecOut[0U], XASUFW_KAT_MSG_LENGTH_IN_BYTES, 0U,
			      XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	SStatus = Xil_SMemSet(&EciesTxPubKey[0U], (XASU_ECC_P521_SIZE_IN_BYTES +
			      XASU_ECC_P521_SIZE_IN_BYTES), 0U, (XASU_ECC_P521_SIZE_IN_BYTES +
			      XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	SStatus = Xil_SMemSet(&EciesMac[0U], XASUFW_AES_TAG_LEN_IN_BYTES, 0U,
			      XASUFW_AES_TAG_LEN_IN_BYTES);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_KAT_FAILED);
	}

	return Status;
}
#endif /* XASU_ECIES_ENABLE */

#ifdef XASU_KEYWRAP_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function runs key wrap unwrap KAT using SHA2-256.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if Key wrap unwrap KAT is successful.
 *	- XASUFW_KEYWRAP_UNWRAPPED_DATA_COMPARISON_FAILED, if expected and generated unwrapped message
 *	comparison fails.
 *	- XASUFW_KEYWRAP_KAT_FAILED, if XAsufw_KeyWrapOperationKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_KeyWrapOperationKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 OutLengthVal = 0U;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAes *AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAsu_KeyWrapParams KwpunwpParam;
	XAsu_RsaPubKeyComp PubKeyParam;
	XAsu_RsaPvtKeyComp PvtKeyParam;
	u8 UnwrappedResult[XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES];

	PubKeyParam.Keysize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	/** Copy required parameters for Key wrap unwrap operation. */
	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaModulus,
			     XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	PvtKeyParam.PubKeyComp = PubKeyParam;
	PvtKeyParam.PrimeCompOrTotientPrsnt = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PvtKeyParam.PvtExp, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, RsaPvtExp,
		    XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES, XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	KwpunwpParam.InputDataAddr = (u64)(UINTPTR)KeyWrapInput;
	KwpunwpParam.OutputDataAddr = (u64)(UINTPTR)WrappedResult;
	KwpunwpParam.ExpoCompAddr = 0U;
	KwpunwpParam.KeyCompAddr = (u64)(UINTPTR)&PubKeyParam;
	KwpunwpParam.OptionalLabelAddr = (u64)(UINTPTR)RsaOpt;
	KwpunwpParam.ActualOutuputDataLenAddr = (u64)(UINTPTR)&OutLengthVal;
	KwpunwpParam.InputDataLen = XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES;
	KwpunwpParam.RsaKeySize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	KwpunwpParam.OptionalLabelSize = XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES;
	KwpunwpParam.OutuputDataLen = XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES;
	KwpunwpParam.AesKeySize = XASU_AES_KEY_SIZE_128_BITS;
	KwpunwpParam.ShaType = XASU_SHA2_TYPE;
	KwpunwpParam.ShaMode = XASU_SHA_MODE_256;

	/** Perform key wrap operation with known inputs. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyWrap(&KwpunwpParam, AsuDmaPtr, Sha2Ptr, AesPtr, &OutLengthVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL;
		goto END;
	}

	KwpunwpParam.InputDataAddr = (u64)(UINTPTR)WrappedResult;
	KwpunwpParam.OutputDataAddr = (u64)(UINTPTR)UnwrappedResult;
	KwpunwpParam.ExpoCompAddr = 0U;
	KwpunwpParam.KeyCompAddr = (u64)(UINTPTR)&PvtKeyParam;
	KwpunwpParam.OptionalLabelAddr = (u64)(UINTPTR)RsaOpt;
	KwpunwpParam.ActualOutuputDataLenAddr = (u64)(UINTPTR)&OutLengthVal;
	KwpunwpParam.InputDataLen = XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES;
	KwpunwpParam.RsaKeySize = XASUFW_RSA_KAT_KEY_LENGTH_IN_BYTES;
	KwpunwpParam.OptionalLabelSize = XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES;
	KwpunwpParam.OutuputDataLen = XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES;
	KwpunwpParam.AesKeySize = XASU_AES_KEY_SIZE_128_BITS;
	KwpunwpParam.ShaType = XASU_SHA2_TYPE;
	KwpunwpParam.ShaMode = XASU_SHA_MODE_256;

	/** Perform key unwrap operation with known inputs. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyUnwrap(&KwpunwpParam, AsuDmaPtr, Sha2Ptr, AesPtr, &OutLengthVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_GEN_UNWRAPPED_KEY_OPERATION_FAIL;
		goto END;
	}

	/** Compare generated unwrapped message with expected input. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(UnwrappedResult, XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES, KeyWrapInput,
				XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES, XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_UNWRAPPED_DATA_COMPARISON_FAILED;
		goto END;
	}

END:
	/** Zeroize local copy of wrapped output value. */
	SStatus = Xil_SMemSet(&WrappedResult[0U], XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES, 0U,
				XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of unwrapped output value. */
	SStatus = Xil_SMemSet(&UnwrappedResult[0U], XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES, 0U,
				XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
	  Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_KAT_FAILED);
	}

	return Status;
}
#endif /* XASU_KEYWRAP_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function runs ECDH KAT on RSA core for P-256 curve.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 * 		- XASUFW_SUCCESS, if ECDH KAT is successful.
 * 		- XASUFW_ECDH_SECRET_COMPARISON_FAILED, if generated and expected shared secret
 * 		  comparison fails.
 * 		- XASUFW_FAILURE, if any other failure happens.
 *
 *************************************************************************************************/
s32 XAsufw_P256EcdhKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 SharedSecret[XASU_ECC_P256_SIZE_IN_BYTES];

	/** Generate the shared secret using the known private key and public key. */
	Status = XRsa_EcdhGenSharedSecret(AsuDmaPtr, XASU_ECC_NIST_P256,
					  XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)EcdhPrivKey,
					  (u64)(UINTPTR)EcdhPubKey1,
					  (u64)(UINTPTR)SharedSecret, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL;
		goto END_CLR;
	}

	/** Compare the generated shared secret with expected shared secret. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(SharedSecret, XASU_ECC_P256_SIZE_IN_BYTES, EcdhExpSharedSecret,
			     XASU_ECC_P256_SIZE_IN_BYTES, XASU_ECC_P256_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECDH_SECRET_COMPARISON_FAILED;
	}

END_CLR:

	/** Zeroize local copy of shared secret. */
	SStatus = Xil_SMemSet(&SharedSecret[0U], XASU_ECC_P256_SIZE_IN_BYTES, 0U,
			      XASU_ECC_P256_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECDH_KAT_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes the KDF and dependent module KATs, after successful
 * 		execution it sets dependent module KAT status in RTCA area and on failure
 * 		respective module will be disabled.
 *
 * @param	AsuDmaPtr	Pointer to the dma instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if KATs are successful.
 * 		- XASUFW_FAILURE, if any other failure happens.
 *
 *************************************************************************************************/
static s32 RunKdfAndDependentKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XSha *XAsufw_Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

#ifdef XASU_KDF_ENABLE
	/** Run KDF with SHA2-256 KAT. */
	Status = XAsufw_KdfOperationKat(AsuDmaPtr);
	if (Status == XASUFW_SUCCESS) {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_KDF_ID);

		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_HMAC_ID);

		goto RUN_SHA2_512;
	}

	/** On failure of KAT, disable the root resource. */
	XAsufw_DisableResource(XASUFW_KDF);

	/** Mark KAT status as failed in RTCA area. */
	XASUFW_MARK_KAT_FAILED(XASU_MODULE_KDF_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
	goto END;
#endif /* XASU_TRIGGER_SLD_ON_KAT_FAILURE */
#endif /* XASU_KDF_ENABLE */

#ifdef XASU_HMAC_ENABLE
	/** Run HMAC with SHA2-256 KAT. */
	Status = XAsufw_HmacOperationKat(AsuDmaPtr);
	if (Status == XASUFW_SUCCESS) {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_HMAC_ID);

		goto RUN_SHA2_512;
	}

	/** On failure of KAT, disable the root resource. */
	XAsufw_DisableResource(XASUFW_HMAC);

	/** Mark KAT status as failed in RTCA area. */
	XASUFW_MARK_KAT_FAILED(XASU_MODULE_HMAC_ID);

#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
	goto END;
#endif /* XASU_TRIGGER_SLD_ON_KAT_FAILURE */
#endif /* XASU_HMAC_ENABLE */

	/** Run SHA2-256 KAT. */
	Status = XAsufw_ShaKat(XAsufw_Sha2Ptr, AsuDmaPtr, XASUFW_SHA2, XASU_SHA_MODE_256);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_SHA2);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_SHA2_ID);
		goto END;
	}

RUN_SHA2_512:
	Status = XAsufw_ShaKat(XAsufw_Sha2Ptr, AsuDmaPtr, XASUFW_SHA2, XASU_SHA_MODE_512);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_SHA2);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_SHA2_ID);
	}
	else {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_SHA2_ID);
	}

END:
	return Status;
}

#if defined(XASU_KEYWRAP_ENABLE) || defined(XASU_RSA_PADDING_ENABLE)
/*************************************************************************************************/
/**
 * @brief	This function executes the Keywrap and dependent module KATs, after successful
 * 		execution it sets dependent module KAT status in RTCA area and on failure
 * 		respective module will be disabled.
 *
 * @param	AsuDmaPtr	Pointer to the DMA instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if KATs are successful.
 * 		- XASUFW_FAILURE, if any other failure happens.
 *
 *************************************************************************************************/
static s32 RunKeyWrapAndDependentKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

#ifdef XASU_KEYWRAP_ENABLE
	/** Run Keywrap with SHA2-256 KAT. */
	Status = XAsufw_KeyWrapOperationKat(AsuDmaPtr);
	if (Status == XASUFW_SUCCESS) {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_KEYWRAP_ID);
#ifdef XASU_RSA_PADDING_ENABLE
		goto SKIP_RSA_OAEP;
#else
		goto END;
#endif /* XASU_RSA_PADDING_ENABLE */
	}

	/** On failure of KAT, disable the root resource. */
	XAsufw_DisableResource(XASUFW_KEYWRAP);

	/** Mark KAT status as failed in RTCA area. */
	XASUFW_MARK_KAT_FAILED(XASU_MODULE_KEYWRAP_ID);
#ifdef XASU_TRIGGER_SLD_ON_KAT_FAILURE
	goto END;
#endif /* XASU_TRIGGER_SLD_ON_KAT_FAILURE */
#endif /* XASU_KEYWRAP_ENABLE */

#ifdef XASU_RSA_PADDING_ENABLE
	/** Run RSA OAEP encrypt and decrypt KAT. */
	Status = XAsufw_RsaEncDecOaepOpKat(AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_RSA);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_RSA_ID);
		goto END;
	}
SKIP_RSA_OAEP:
	/** Run RSA PSS signature generation and verification KAT. */
	Status = XAsufw_RsaPssSignGenAndVerifOpKat(AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		/** On failure of KAT, disable the root resource. */
		XAsufw_DisableResource(XASUFW_RSA);
		/** Mark KAT status as failed in RTCA area. */
		XASUFW_MARK_KAT_FAILED(XASU_MODULE_RSA_ID);
	}
	else {
		/** Set KAT status as passed in RTCA area. */
		XASUFW_SET_KAT_PASSED(XASU_MODULE_RSA_ID);
	}

#endif /* XASU_RSA_PADDING_ENABLE */
END:
	return Status;
}
#endif /* XASU_KEYWRAP_ENABLE || XASU_RSA_PADDING_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function performs validation checks for AES DPA CM KAT output.
 *
 * @param 	MaskedOutput0	Pointer to the masked output0 array of size 4 words.
 * @param 	MaskedOutput1	Pointer to the masked output1 array of size 4 words.
 * @param 	Mask0		Pointer to the mask0 array of size 4 words.
 * @param 	Mask1		Pointer to the mask1 array of size 4 words.
 *
 * @return
 *	- XASUFW_SUCCESS, if DPA CM checks are passed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK_FAILED, if DPA CM checks fail.
 *
 *************************************************************************************************/
static s32 XAsufw_ValidateDpaIntermediateValues(const u32 *MaskedOutput0, const u32 *MaskedOutput1,
	const u32 *Mask0, const u32 *Mask1)
{
	CREATE_VOLATILE(Status, XASUFW_AES_DPA_CM_KAT_CHECK_FAILED);

	/** Check whether masked output data are zero or not. */
	if (((MaskedOutput0[XASUFW_BUFFER_INDEX_ZERO] == 0U) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_ONE] == 0U) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_TWO] == 0U) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_THREE] == 0U)) ||
			((MaskedOutput1[XASUFW_BUFFER_INDEX_ZERO] == 0U) &&
			(MaskedOutput1[XASUFW_BUFFER_INDEX_ONE] == 0U) &&
			(MaskedOutput1[XASUFW_BUFFER_INDEX_TWO] == 0U) &&
			(MaskedOutput1[XASUFW_BUFFER_INDEX_THREE] == 0U)) ||
			((Mask0[XASUFW_BUFFER_INDEX_ZERO] == 0U) &&
			(Mask0[XASUFW_BUFFER_INDEX_ONE] == 0U) &&
			(Mask0[XASUFW_BUFFER_INDEX_TWO] == 0U) &&
			(Mask0[XASUFW_BUFFER_INDEX_THREE] == 0U)) ||
			((Mask1[XASUFW_BUFFER_INDEX_ZERO] == 0U) &&
			(Mask1[XASUFW_BUFFER_INDEX_ONE] == 0U) &&
			(Mask1[XASUFW_BUFFER_INDEX_TWO] == 0U) &&
			(Mask1[XASUFW_BUFFER_INDEX_THREE] == 0U))) {
		goto END;
	}

	/** Check whether masked outputs are different. */
	if ((MaskedOutput0[XASUFW_BUFFER_INDEX_ZERO] == MaskedOutput1[XASUFW_BUFFER_INDEX_ZERO]) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_ONE] ==
			MaskedOutput1[XASUFW_BUFFER_INDEX_ONE]) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_TWO] ==
			MaskedOutput1[XASUFW_BUFFER_INDEX_TWO]) &&
			(MaskedOutput0[XASUFW_BUFFER_INDEX_THREE] ==
			MaskedOutput1[XASUFW_BUFFER_INDEX_THREE])) {
		goto END;
	}

	/** Check whether masks are different. */
	if ((Mask0[XASUFW_BUFFER_INDEX_ZERO] == Mask1[XASUFW_BUFFER_INDEX_ZERO]) &&
			(Mask0[XASUFW_BUFFER_INDEX_ONE] == Mask1[XASUFW_BUFFER_INDEX_ONE]) &&
			(Mask0[XASUFW_BUFFER_INDEX_TWO] == Mask1[XASUFW_BUFFER_INDEX_TWO]) &&
			(Mask0[XASUFW_BUFFER_INDEX_THREE] == Mask1[XASUFW_BUFFER_INDEX_THREE])) {
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function applies a mask to given data using the XOR operation.
 *
 * @param	Data		Pointer to the data array.
 * @param	MaskedData	Pointer to the array where the masked data will be stored.
 * @param	Mask		Mask value.
 * @param	Length		Length of the Data and masked data arrays.
 *
 *************************************************************************************************/
static void XAsufw_ApplyMask(const u32 *Data, u32 *MaskedData, u32 Mask, u32 Length)
{
	u32 Index;
	for (Index = 0U; Index < Length; Index++) {
		MaskedData[Index] = (Data[Index] ^ Mask);
	}
}
/** @} */

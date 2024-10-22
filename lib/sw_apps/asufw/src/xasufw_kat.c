/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_kat.c
 *
 * This file contains the code for SHA2/SHA3, RSA and ECC KAT command supported by ASUFW.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_AesDpaCmChecks(const u32 *P, const u32 *Q, const u32 *R, const u32 *S);

/************************************ Variable Definitions ***************************************/
#define XASUFW_KAT_MSG_LENGTH_IN_BYTES		(32U)	/**< SHA KAT message length in bytes */
#define XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES	(256U)	/**< RSA KAT message length in bytes */
#define XASUFW_AES_KEY_LEN_IN_BYTES		(32U)	/**< AES KAT Key length in bytes */
#define XASUFW_AES_IV_LEN_IN_BYTES		(12U)	/**< AES KAT Iv length in bytes */
#define XASUFW_AES_AAD_LEN_IN_BYTES		(16U)	/**< AES KAT AAD length in bytes */
#define XASUFW_AES_TAG_LEN_IN_BYTES		(16U)	/**< AES KAT Tag length in bytes */
#define XASUFW_DOUBLE_P256_SIZE_IN_BYTES	(64U)	/**< Double the size of P256 curve length */
#define XASUFW_DOUBLE_P192_SIZE_IN_BYTES	(48U)	/**< Double the size of P192 curve length */
#define XASUFW_AES_DATA_SPLIT_SIZE_IN_BYTES	(16U)	/**< AES data split size in words */
#define XASUFW_AES_CM_KAT_KEY_SIZE_IN_BYTES	(32U)	/**< AES Key size in words */
#define XASUFW_AES_CM_KAT_DATA_SIZE_IN_BYTES	(64U)	/**< AES operation data in words */

/* KAT message */
static const u8 KatMessage[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* SHA2 256 hash for above KAT message */
static const u8 ExpSha2256Hash[XASU_SHA_256_HASH_LEN] = {
	0x03U, 0xFBU, 0xA3U, 0xBBU, 0x9BU, 0x90U, 0xBBU, 0x5FU,
	0xF1U, 0x07U, 0xC0U, 0x43U, 0x63U, 0xF7U, 0x99U, 0x34U,
	0xECU, 0xE9U, 0x56U, 0xB2U, 0x3CU, 0xFAU, 0x13U, 0xC7U,
	0xDFU, 0x79U, 0xB9U, 0xFEU, 0x5DU, 0x1DU, 0xADU, 0x3BU
};

/* SHA3 256 hash for above KAT message */
static const u8 ExpSha3256Hash[XASU_SHA_256_HASH_LEN] = {
	0xBCU, 0x64U, 0xDCU, 0xBBU, 0x66U, 0xDDU, 0x08U, 0xA5U,
	0xC1U, 0x11U, 0xE6U, 0xB3U, 0x55U, 0x60U, 0xF1U, 0xC3U,
	0x3EU, 0x6DU, 0xE8U, 0x6AU, 0x15U, 0xDFU, 0x6CU, 0xF3U,
	0xA5U, 0xD1U, 0xB4U, 0x31U, 0xC0U, 0x67U, 0x5CU, 0xDDU
};

static const u8 RsaData[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES] = {
	0x30U, 0x68U, 0x1BU, 0x08U, 0x2EU, 0x9BU, 0xAAU, 0x9DU, 0x14U, 0x81U, 0x7AU, 0x3AU,
	0xDFU, 0xA0U, 0xD8U, 0x4BU, 0x77U, 0x58U, 0xC9U, 0x20U, 0xC5U, 0x8AU, 0x08U, 0xE7U,
	0xB9U, 0x73U, 0x2EU, 0x8EU, 0x99U, 0x8BU, 0x0DU, 0x44U, 0x0CU, 0x59U, 0x61U, 0xA0U,
	0x05U, 0x32U, 0x6CU, 0x35U, 0xEEU, 0xACU, 0x47U, 0x6BU, 0xD6U, 0x3EU, 0x2BU, 0xF6U,
	0x69U, 0x04U, 0x5CU, 0x26U, 0xC2U, 0x97U, 0xD4U, 0xBAU, 0x53U, 0x0FU, 0xA6U, 0xDBU,
	0xE9U, 0xCDU, 0x2DU, 0x08U, 0xC3U, 0x38U, 0x40U, 0x3BU, 0x44U, 0x63U, 0x66U, 0xC3U,
	0x8AU, 0xF1U, 0xFCU, 0xBDU, 0xA8U, 0x4FU, 0x2AU, 0x6EU, 0x11U, 0xDEU, 0xB8U, 0x82U,
	0x53U, 0x91U, 0x9AU, 0xB3U, 0x47U, 0x4BU, 0xC2U, 0x44U, 0xA9U, 0x75U, 0xDDU, 0xEBU,
	0x6FU, 0x85U, 0x38U, 0xBDU, 0xEFU, 0xB1U, 0x13U, 0xD8U, 0xB3U, 0xC7U, 0x13U, 0x58U,
	0x05U, 0x9FU, 0xDEU, 0x15U, 0x40U, 0x91U, 0x90U, 0x0EU, 0x6BU, 0x0BU, 0x50U, 0x89U,
	0x64U, 0x20U, 0x2FU, 0x08U, 0x25U, 0x50U, 0xD0U, 0xA3U, 0xCDU, 0xCAU, 0x78U, 0x1AU,
	0xA8U, 0xC3U, 0x9FU, 0x7CU, 0xCDU, 0x03U, 0x21U, 0xC4U, 0xEBU, 0x2BU, 0x05U, 0x96U,
	0xACU, 0x6AU, 0xA7U, 0x09U, 0x3DU, 0x53U, 0x00U, 0xB9U, 0xCEU, 0x6FU, 0x5DU, 0x7CU,
	0x1CU, 0xF0U, 0x71U, 0xFCU, 0x20U, 0x34U, 0xE7U, 0xF9U, 0x93U, 0xA8U, 0xBEU, 0x47U,
	0xA1U, 0x05U, 0x7DU, 0x91U, 0x9FU, 0x0CU, 0x2CU, 0xAFU, 0xAEU, 0xA1U, 0xFAU, 0xBDU,
	0x2BU, 0x3DU, 0x83U, 0x91U, 0xBFU, 0xC2U, 0x81U, 0x76U, 0x57U, 0x6AU, 0xF5U, 0xD6U,
	0xA4U, 0x32U, 0xD2U, 0x44U, 0x4AU, 0x08U, 0x7DU, 0xA8U, 0xBCU, 0x0DU, 0x8EU, 0x6CU,
	0xB8U, 0x51U, 0xF2U, 0xF9U, 0x90U, 0x55U, 0xEDU, 0xC1U, 0xEBU, 0x7FU, 0x6BU, 0x24U,
	0xD7U, 0xC2U, 0x68U, 0xD3U, 0x02U, 0xE9U, 0x50U, 0x28U, 0xE1U, 0xEEU, 0x29U, 0x0BU,
	0x92U, 0x8AU, 0xEEU, 0xD4U, 0xFDU, 0xAFU, 0xEFU, 0x94U, 0x0CU, 0xA6U, 0xF5U, 0xD7U,
	0x62U, 0x14U, 0x97U, 0xB8U, 0xB3U, 0xF5U, 0x76U, 0xD5U, 0x4EU, 0x14U, 0x80U, 0x04U,
	0xE9U, 0x09U, 0x47U, 0x77U
};

static const u8 RsaModulus[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES] = {
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

static const u8 RsaExpectedCt[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES] = {
	0x3BU, 0xCEU, 0xC0U, 0x73U, 0xA9U, 0x93U, 0x58U, 0x71U, 0x94U, 0xAFU, 0xB8U, 0xE9U,
	0x6EU, 0x51U, 0x81U, 0x5AU, 0xE6U, 0x0FU, 0xCCU, 0x17U, 0x08U, 0xA9U, 0xD0U, 0x1FU,
	0xDFU, 0x72U, 0xF5U, 0xABU, 0x2BU, 0x6CU, 0xF7U, 0xF8U, 0x75U, 0xD1U, 0xE8U, 0x56U,
	0xC3U, 0x5DU, 0x19U, 0xD9U, 0x50U, 0x3CU, 0xE0U, 0xE3U, 0x1CU, 0x58U, 0x38U, 0xB0U,
	0x03U, 0x01U, 0xA4U, 0x9BU, 0x61U, 0xFBU, 0x5FU, 0x4EU, 0xF0U, 0x47U, 0x9BU, 0x79U,
	0x10U, 0xABU, 0x68U, 0x91U, 0xA4U, 0x37U, 0x5FU, 0x51U, 0xA8U, 0x6AU, 0x5CU, 0xD4U,
	0xF2U, 0x20U, 0x5AU, 0x76U, 0x35U, 0x6FU, 0x90U, 0x3EU, 0xD7U, 0x86U, 0xE0U, 0xE4U,
	0x12U, 0xACU, 0x54U, 0xF6U, 0x35U, 0x1FU, 0x56U, 0x41U, 0x77U, 0x91U, 0x3AU, 0x46U,
	0xE1U, 0x78U, 0xFBU, 0x0CU, 0xBFU, 0x6DU, 0x80U, 0xB3U, 0xEAU, 0xBEU, 0x2FU, 0x38U,
	0x31U, 0x05U, 0x03U, 0x3BU, 0x54U, 0xDCU, 0x17U, 0x0FU, 0x1DU, 0x13U, 0x75U, 0x53U,
	0xE5U, 0x3EU, 0x22U, 0x8FU, 0x98U, 0x94U, 0x6BU, 0x5FU, 0xBBU, 0xDFU, 0xCAU, 0x6EU,
	0x01U, 0x41U, 0xAFU, 0xE0U, 0xA7U, 0x9AU, 0x54U, 0x6FU, 0x8EU, 0x04U, 0x93U, 0x1AU,
	0xF8U, 0xCDU, 0xC9U, 0x77U, 0x94U, 0x7EU, 0x21U, 0xB7U, 0x94U, 0x57U, 0xC8U, 0x97U,
	0xF2U, 0x96U, 0x93U, 0xCCU, 0x61U, 0xD4U, 0x4AU, 0xDAU, 0x09U, 0x31U, 0xCEU, 0xE2U,
	0x5EU, 0xC1U, 0x9DU, 0x43U, 0xB3U, 0x0AU, 0xC8U, 0xC1U, 0x81U, 0xA0U, 0xA5U, 0x70U,
	0xCAU, 0x80U, 0x18U, 0x7AU, 0x16U, 0x2AU, 0x34U, 0x3BU, 0x66U, 0x92U, 0x30U, 0x23U,
	0x6DU, 0x25U, 0x1FU, 0xADU, 0x26U, 0x01U, 0xBAU, 0x8BU, 0x52U, 0x2EU, 0x46U, 0xF0U,
	0x76U, 0x70U, 0x5FU, 0x95U, 0x5DU, 0x31U, 0xD5U, 0xDAU, 0xB2U, 0x9EU, 0xF0U, 0x41U,
	0x3EU, 0xC2U, 0x60U, 0x55U, 0x89U, 0x3CU, 0xCDU, 0xF3U, 0xF1U, 0x0BU, 0xA3U, 0x45U,
	0xD6U, 0x80U, 0xC2U, 0xD9U, 0x3AU, 0xA1U, 0x62U, 0x5CU, 0xA9U, 0xA8U, 0xACU, 0x60U,
	0x3FU, 0x82U, 0x0CU, 0x55U, 0xB8U, 0x7BU, 0x69U, 0x41U, 0x2DU, 0x61U, 0xC5U, 0x32U,
	0x93U, 0x3BU, 0xC4U, 0x6BU
};

static const u32 RsaPublicExp = 0x1000100U;

static const u8 EccPrivKey[XECC_P256_SIZE_IN_BYTES] = {
	0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
	0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
	0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
	0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU
};

static const u8 EccHash[XECC_P256_SIZE_IN_BYTES] = {
	0x02U, 0xBFU, 0x58U, 0x5CU, 0x72U, 0x89U, 0x45U, 0x9CU,
	0xDDU, 0x20U, 0x61U, 0xD1U, 0x67U, 0xE5U, 0x40U, 0xC0U,
	0x1EU, 0x40U, 0x56U, 0xB4U, 0x65U, 0xCAU, 0xE1U, 0x5FU,
	0xA3U, 0x45U, 0xEDU, 0xADU, 0x93U, 0x88U, 0x54U, 0x6DU
};

static const u8 EccEphemeralKey[XECC_P256_SIZE_IN_BYTES] = {
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

static const u8 EccExpPubKeyP192[XASUFW_DOUBLE_P192_SIZE_IN_BYTES] = {
	0xB4U, 0x62U, 0x5EU, 0x18U, 0x7AU, 0x1AU, 0x03U, 0x8BU,
	0x4CU, 0xEAU, 0xF9U, 0xFFU, 0xDDU, 0xFCU, 0x9EU, 0x04U,
	0x62U, 0x7DU, 0xD1U, 0x62U, 0xE2U, 0xDEU, 0x92U, 0x97U,
	0x37U, 0x77U, 0x9DU, 0xBEU, 0xBCU, 0x4EU, 0x41U, 0xF0U,
	0x34U, 0x9DU, 0x65U, 0x68U, 0xD2U, 0xABU, 0xB0U, 0xBEU,
	0x6CU, 0x06U, 0xD9U, 0x18U, 0x67U, 0xCFU, 0xD3U, 0x41U
};

static const u8 EccExpSignP192[XASUFW_DOUBLE_P192_SIZE_IN_BYTES] = {
	0x5FU, 0x8EU, 0xB6U, 0xBEU, 0xECU, 0x55U, 0xE9U, 0x9DU,
	0x78U, 0x23U, 0xD8U, 0x73U, 0x9DU, 0xAEU, 0x77U, 0x50U,
	0xF0U, 0x87U, 0xC9U, 0xDCU, 0xBAU, 0xF3U, 0x6FU, 0x73U,
	0x1EU, 0xB0U, 0xFDU, 0xF2U, 0x34U, 0x15U, 0x26U, 0x54U,
	0xDEU, 0xB2U, 0x41U, 0xC1U, 0x5FU, 0xDDU, 0x08U, 0xA5U,
	0xF5U, 0x1EU, 0x6AU, 0xC1U, 0x95U, 0x32U, 0x3EU, 0x78U
};

/* AES key */
static const u8 AesKey[XASUFW_AES_KEY_LEN_IN_BYTES] = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/* AES IV */
static const u8 AesIv[XASUFW_AES_IV_LEN_IN_BYTES] = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U, 0x62U, 0x98U,
	0xDFU, 0xEAU, 0x5AU, 0xB2U
};

/* AES AAD data */
static const u8 AesAad[XASUFW_AES_AAD_LEN_IN_BYTES] = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

/* AES-GCM expected CipherText */
static const u8 ExpAesGcmCt[XASUFW_KAT_MSG_LENGTH_IN_BYTES] = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

/* AES expected Tag */
static const u8 ExpAesGcmTag[XASUFW_AES_TAG_LEN_IN_BYTES] = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

static const u8 AesCmKey[XASUFW_AES_CM_KAT_KEY_SIZE_IN_BYTES] = {
	0x98U, 0x07U, 0x69U, 0x56U, 0x4FU, 0x15U, 0x8CU, 0x97U,
	0x78U, 0xBAU, 0x50U, 0xF2U, 0x5FU, 0x76U, 0x63U, 0xE4U,
	0x97U, 0xE6U, 0x0CU, 0x2FU, 0x1BU, 0x55U, 0xA4U, 0x09U,
	0xDDU, 0x3AU, 0xCBU, 0xD8U, 0xB6U, 0x87U, 0xA0U, 0xEDU
};

static const u8 AesCmData[XASUFW_AES_CM_KAT_DATA_SIZE_IN_BYTES] = {
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x86U, 0xC2U, 0x37U, 0xCFU, 0xEAU, 0xD4U, 0x8AU, 0xC1U,
	0xA0U, 0xA6U, 0x0BU, 0x3DU, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
	0x24U, 0x81U, 0x32U, 0x2DU, 0x56U, 0x8DU, 0xD5U, 0xA8U,
	0xEDU, 0x5EU, 0x77U, 0xD0U, 0x88U, 0x1AU, 0xDEU, 0x93U
};

static const u8 AesCmCt[XASUFW_AES_DATA_SPLIT_SIZE_IN_BYTES] = {
	0x67U, 0x02U, 0x0AU, 0x3BU, 0x3AU, 0xDEU, 0xECU, 0xF6U,
	0x03U, 0x09U, 0xB3U, 0x78U, 0x6EU, 0xCAU, 0xD4U, 0xEBU
};

static const u8 AesCmMiC[XASUFW_AES_DATA_SPLIT_SIZE_IN_BYTES] = {
	0x64U, 0x00U, 0xD2U, 0x1FU, 0x63U, 0x63U, 0xFCU, 0x09U,
	0x06U, 0xD4U, 0xF3U, 0x79U, 0x88U, 0x09U, 0xCAU, 0x7EU
};

/*************************************************************************************************/
/**
 * @brief	This function runs SHA KAT on the given SHA instance for 256 bit digest size.
 *
 * @param	XAsufw_ShaInstance	Pointer to the SHA instance.
 * @param	QueueId			Queue Unique ID.
 * @param	ShaResource		SHA2/SHA3 resource.
 *
 * @return
 *	- XASUFW_SUCCESS on successful execution of the SHA KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_SHA_HASH_COMPARISON_FAILED, if expected and generated hash comparison fails.
 *	- XASUFW_SHA_KAT_FAILED, when XAsufw_ShaKat API fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, u32 QueueId, XAsufw_Resource ShaResource)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 OutVal[XASU_SHA_256_HASH_LEN];
	const u8 *ExpHash = NULL;

	/** Check resource availability (DMA,SHA) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(ShaResource, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	XAsufw_AllocateResource(ShaResource, QueueId);

	/** Perform SHA start operation. */
	Status = XSha_Start(XAsufw_ShaInstance, XASU_SHA_MODE_SHA256);
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
	Status = XSha_Finish(XAsufw_ShaInstance, (u64)(UINTPTR)OutVal, XASU_SHA_256_HASH_LEN, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (ShaResource == XASUFW_SHA2) {
		ExpHash = ExpSha2256Hash;
	} else {
		ExpHash = ExpSha3256Hash;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Compare generated hash with expected hash. */
	Status = Xil_SMemCmp(OutVal, XASU_SHA_256_HASH_LEN, ExpHash, XASU_SHA_256_HASH_LEN,
			     XASU_SHA_256_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_HASH_COMPARISON_FAILED;
	}

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&OutVal[0U], XASU_SHA_256_HASH_LEN, 0U, XASU_SHA_256_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_KAT_FAILED);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(ShaResource, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs RSA KAT for 2048 bit key size.
 *
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS on successful execution of the RSA KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED, if expected and generated encrypted message
 *	comparison fails.
 *	- XASUFW_RSA_KAT_FAILED, when XAsufw_RsaPubEncKat API fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaPubEncKat(u32 QueueId)
{
	s32 Status  = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XAsu_RsaPubKeyComp PubKeyParam;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 OutVal[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES];

	PubKeyParam.Keysize = XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	/** Copy required parameters. */
	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaModulus,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Check resource availability (DMA,RSA) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(RET);
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);

	/** Perform public exponentiation encryption operation. */
	Status = XRsa_PubExp(AsuDmaPtr, PubKeyParam.Keysize, (u64)(UINTPTR)RsaData,
			     (u64)(UINTPTR)OutVal, (u64)(UINTPTR)&PubKeyParam, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare generated encrypted message with expected. */
	Status = Xil_SMemCmp(OutVal, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaExpectedCt,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&OutVal[0U], XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, 0U,
			      XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_KAT_FAILED);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS ) {
		Status = XAsufw_UpdateErrorStatus(Status,
						  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs ECC KAT on ECC core for P-256 curve.
 *
 * @param	XAsufw_EccInstance	Pointer to the ECC instance.
 * @param	QueueId			Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS on successful execution of the ECC KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_ECC_PUBKEY_COMPARISON_FAILED, if public key generation fails.
 *	- XASUFW_ECC_SIGNATURE_COMPARISON_FAILED, if generated and expected signature
	comparison fails.
 *	- XASUFW_ECC_KAT_FAILED, when XAsufw_EccCoreKat API fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_EccCoreKat(XEcc *XAsufw_EccInstance, u32 QueueId)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 GenPubKey[XASUFW_DOUBLE_P256_SIZE_IN_BYTES];
	u8 GenSign[XASUFW_DOUBLE_P256_SIZE_IN_BYTES];

	/** Check resource availability (DMA,ECC) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_ECC, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}

	XAsufw_AllocateResource(XASUFW_ECC, QueueId);

	/** Generates the public key using the provided private key. */
	Status = XEcc_GeneratePublicKey(XAsufw_EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, (u64)(UINTPTR)GenPubKey);
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

	/** Validates the generated ECC public key. */
	Status = XEcc_ValidatePublicKey(XAsufw_EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Generate signature using core API XEcc_GenerateSignature. */
	Status = XEcc_GenerateSignature(XAsufw_EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
					XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, EccEphemeralKey,
					(u64)(UINTPTR)EccHash, XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated signature with the expected. */
	Status = Xil_SMemCmp(GenSign, XASUFW_DOUBLE_P256_SIZE_IN_BYTES, EccExpSignP256,
			     XASUFW_DOUBLE_P256_SIZE_IN_BYTES, XASUFW_DOUBLE_P256_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_SIGNATURE_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Verify signature using core API XEcc_VerifySignature. */
	Status = XEcc_VerifySignature(XAsufw_EccInstance, AsuDmaPtr, XECC_CURVE_TYPE_NIST_P256,
				      XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenPubKey,
				      (u64)(UINTPTR)EccHash, XECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
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

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_ECC, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs ECC KAT on RSA core for P-192 curve.
 *
 * @param	QueueId		Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS on successful execution of the ECC KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED, if public key generation fails.
 *	- XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED, if generated and expected signature
	comparison fails.
 *	- XASUFW_RSA_ECC_KAT_FAILED, when XAsufw_RsaEccKat API fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaEccKat(u32 QueueId)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 GenPubKey[XASUFW_DOUBLE_P192_SIZE_IN_BYTES];
	u8 GenSign[XASUFW_DOUBLE_P192_SIZE_IN_BYTES];

	/** Check resource availability (DMA,RSA) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);

	/** Generates the public key using the provided private key. */
	Status = XRsa_EccGeneratePubKey(AsuDmaPtr, XASU_ECC_NIST_P192, XRSA_ECC_P192_SIZE_IN_BYTES,
					(u64)(UINTPTR)EccPrivKey, (u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated public key with expected public key. */
	Status = Xil_SMemCmp(GenPubKey, XASUFW_DOUBLE_P192_SIZE_IN_BYTES, EccExpPubKeyP192,
			     XASUFW_DOUBLE_P192_SIZE_IN_BYTES, XASUFW_DOUBLE_P192_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Validates the generated ECC public key. */
	Status = XRsa_EccValidatePubKey(AsuDmaPtr, XASU_ECC_NIST_P192, XRSA_ECC_P192_SIZE_IN_BYTES,
					(u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Generate signature using RSA core API XRsa_EccGenerateSignature. */
	Status = XRsa_EccGenerateSignature(AsuDmaPtr, XASU_ECC_NIST_P192,
					   XRSA_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, EccEphemeralKey,
					   (u64)(UINTPTR)EccHash, XRSA_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Compare the generated signature message with the expected. */
	Status = Xil_SMemCmp(GenSign, XASUFW_DOUBLE_P192_SIZE_IN_BYTES, EccExpSignP192,
			     XASUFW_DOUBLE_P192_SIZE_IN_BYTES, XASUFW_DOUBLE_P192_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED;
		XFIH_GOTO(END);
	}

	/** Verify signature using RSA core API XRsa_EccVerifySignature. */
	Status = XRsa_EccVerifySignature(AsuDmaPtr, XASU_ECC_NIST_P192, XRSA_ECC_P192_SIZE_IN_BYTES,
					 (u64)(UINTPTR)EccExpPubKeyP192, (u64)(UINTPTR)EccHash,
					 XRSA_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

END:
	/** Zeroize local copy of public key. */
	SStatus = Xil_SMemSet(&GenPubKey[0U], XASUFW_DOUBLE_P192_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P192_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of generated signature. */
	SStatus = Xil_SMemSet(&GenSign[0U], XASUFW_DOUBLE_P192_SIZE_IN_BYTES, 0U,
			      XASUFW_DOUBLE_P192_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_KAT_FAILED);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES KAT on the given AES instance for 256 bit message length
 * 		in GCM mode.
 *
 * @param	AesInstance	Pointer to the AES instance.
 * @param	QueueId		Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, on successful execution of the AES KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_AES_WRITE_KEY_FAILED, if Key write to AES USER key register fails.
 *	- XASUFW_AES_INIT_FAILED, if initialization of AES engine fails.
 *	- XASUFW_AES_UPDATE_FAILED, if update of data to AES engine fails.
 *	- XASUFW_AES_FINAL_FAILED, if completion of final AES operation fails.
 *	- XASUFW_AES_KAT_FAILED, when XAsufw_AesGcmKat API fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
s32 XAsufw_AesGcmKat(XAes *AesInstance, u32 QueueId)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsu_AesKeyObject KeyObject;
	u8 AesOutData[XASUFW_KAT_MSG_LENGTH_IN_BYTES];
	u8 AesTag[XASUFW_AES_TAG_LEN_IN_BYTES];

	KeyObject.KeyAddress = (u64)(UINTPTR)AesKey;
	KeyObject.KeySrc = XASU_AES_USER_KEY_0;
	KeyObject.KeySize = XASU_AES_KEY_SIZE_256_BITS;

	/** Check resource availability (DMA,AES) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_AES, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}

	XAsufw_AllocateResource(XASUFW_AES, QueueId);

	/** Write the key into specified AES USER key registers. */
	Status = XAes_WriteKey(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
		XFIH_GOTO(END);
	}

	/**  Initializes the AES engine and load the provided key and IV to the AES engine
		for encryption. */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)AesIv,
			   XASUFW_AES_IV_LEN_IN_BYTES, XASU_AES_GCM_MODE, XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES update operation for AAD, message. */
	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)AesAad, 0U,
			     XASUFW_AES_AAD_LEN_IN_BYTES, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		XFIH_GOTO(END);
	}

	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)KatMessage,
			     (u64)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES final operation for encryption. */
	Status = XAes_Final(AesInstance, AsuDmaPtr, (u32)(UINTPTR)AesTag,
			    XASUFW_AES_TAG_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
		XFIH_GOTO(END);
	}

	/** Comparison of encrypted data with expected ciphertext data. */
	Status = Xil_SMemCmp_CT(ExpAesGcmCt, XASUFW_KAT_MSG_LENGTH_IN_BYTES, AesOutData,
				XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES KAT Failed at Encrypted data Comparison \r\n");
		XFIH_GOTO(END);
	}

	/** Comparison of generated GCM tag with the expected GCM tag. */
	Status = Xil_SMemCmp_CT(ExpAesGcmTag, XASUFW_AES_TAG_LEN_IN_BYTES, AesTag,
				XASUFW_AES_TAG_LEN_IN_BYTES, XASUFW_AES_TAG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES KAT Failed at Encrypted data Comparison \r\n");
		XFIH_GOTO(END);
	}

	/**  Initializes the AES engine and load the provided key and IV to the AES engine
		for decryption. */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)AesIv,
			   XASUFW_AES_IV_LEN_IN_BYTES, XASU_AES_GCM_MODE, XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES update operation for AAD, ciphertext. */
	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)AesAad, 0U,
			     XASUFW_AES_AAD_LEN_IN_BYTES, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		XFIH_GOTO(END);
	}

	Status = XAes_Update(AesInstance, AsuDmaPtr, (u64)(UINTPTR)ExpAesGcmCt,
			     (u64)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES final operation for decryption. */
	Status = XAes_Final(AesInstance, AsuDmaPtr, (u32)(UINTPTR)AesTag,
			    XASUFW_AES_TAG_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
		XFIH_GOTO(END);
	}

	/** Comparison of decrypted data with expected input data. */
	Status = Xil_SMemCmp_CT(KatMessage, XASUFW_KAT_MSG_LENGTH_IN_BYTES, AesOutData,
				XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES KAT Failed at Decrypted data Comparison \r\n");
		XFIH_GOTO(END);
	}

END:
	/** Zeroize local copy of buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)AesOutData, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	ClearStatus |= Xil_SecureZeroize((u8 *)(UINTPTR)AesTag, XASUFW_AES_TAG_LEN_IN_BYTES);
	if ((Status == XASUFW_SUCCESS) && (Status == XASUFW_SUCCESS)) {
		Status = ClearStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_KAT_FAILED);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_AES, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES DPA CM KAT on the given AES instance.
 *
 * @param	AesInstance	Pointer to the AES instance.
 * @param	QueueId		Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS on successful execution of the AES DPA CM KAT.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK1_FAILED, if check1 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK2_FAILED, if check2 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK3_FAILED, if check3 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK4_FAILED, if check4 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK5_FAILED, if check5 has failed.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_AesDecryptDpaCmKat(XAes *AesInstance, u32 QueueId)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsu_AesKeyObject KeyObject;
	const u32 *AesCmCtPtr = (u32 *)(UINTPTR)AesCmCt;
	const u32 *AesCmMiCPtr = (u32 *)(UINTPTR)AesCmMiC;

	u32 Output0[XASUFW_AES_CM_KAT_DATA_SIZE_IN_BYTES];
	u32 Output1[XASUFW_AES_CM_KAT_DATA_SIZE_IN_BYTES];

	const u32 *RM0 = &Output0[0U];
	const u32 *R0 = &Output0[4U];
	const u32 *Mm0 = &Output0[8U];
	const u32 *M0 = &Output0[12U];
	const u32 *RM1 = &Output1[0U];
	const u32 *R1 = &Output1[4U];
	const u32 *Mm1 = &Output1[8U];
	const u32 *M1 = &Output1[12U];

	KeyObject.KeyAddress = (u64)(UINTPTR)AesCmKey;
	KeyObject.KeySrc = XASU_AES_USER_KEY_0;
	KeyObject.KeySize = XASU_AES_KEY_SIZE_256_BITS;

	/** Check resource availability (DMA,AES) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_AES, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	XAsufw_AllocateResource(XASUFW_AES, QueueId);

	/** Run DPA CM KAT on AES engine to know performance integrity. */
	Status = XAes_DpaCmDecryptData(AesInstance, AsuDmaPtr, &KeyObject, (u32)(UINTPTR)AesCmData,
				       (u32)(UINTPTR)Output0, sizeof(Output0));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XASUFW_FAILURE;

	/** Rerun the AES DPA CM KAT with same vectors of previous run. */
	Status = XAes_DpaCmDecryptData(AesInstance, AsuDmaPtr, &KeyObject, (u32)(UINTPTR)AesCmData,
				       (u32)(UINTPTR)Output1, sizeof(Output1));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Perform AES DPA CM checks. */
	Status = XST_FAILURE;
	Status = XAsufw_AesDpaCmChecks(RM0, RM1, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_KAT_CHECK1_FAILED;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XAsufw_AesDpaCmChecks(RM1, RM0, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_KAT_CHECK2_FAILED;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XAsufw_AesDpaCmChecks(Mm0, RM0, RM1, Mm1);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_KAT_CHECK3_FAILED;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XAsufw_AesDpaCmChecks(Mm1, RM0, RM1, Mm0);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_AES_DPA_CM_KAT_CHECK4_FAILED;
		goto END;
	}

	if ((((R0[0U] ^ RM0[0U]) != AesCmCtPtr[0U])  || ((R0[1U] ^ RM0[1U]) != AesCmCtPtr[1U]) ||
	     ((R0[2U] ^ RM0[2U]) != AesCmCtPtr[2U])  || ((R0[3U] ^ RM0[3U]) != AesCmCtPtr[3U])) ||
	    (((M0[0U] ^ Mm0[0U]) != AesCmMiCPtr[0U]) || ((M0[1U] ^ Mm0[1U]) != AesCmMiCPtr[1U]) ||
	     ((M0[2U] ^ Mm0[2U]) != AesCmMiCPtr[2U]) || ((M0[3U] ^ Mm0[3U]) != AesCmMiCPtr[3U])) ||
	    (((R1[0U] ^ RM1[0U]) == AesCmCtPtr[0U])  || ((R1[1U] ^ RM1[1U]) == AesCmCtPtr[1U]) ||
	     ((R1[2U] ^ RM1[2U]) == AesCmCtPtr[2U])  || ((R1[3U] ^ RM1[3U]) == AesCmCtPtr[3U])) ||
	    (((M1[0U] ^ Mm1[0U]) == AesCmMiCPtr[0U]) || ((M1[1U] ^ Mm1[1U]) == AesCmMiCPtr[1U]) ||
	     ((M1[2U] ^ Mm1[2U]) == AesCmMiCPtr[2U]) || ((M1[3U] ^ Mm1[3U]) == AesCmMiCPtr[3U]))) {
		Status = XASUFW_AES_DPA_CM_KAT_CHECK5_FAILED;
	}

END:
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_DPA_CM_KAT_FAILED);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_AES, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs checks for AES DPA CM KAT ouptut.
 *
 * @param 	P is the pointer to the data array of size 4 words.
 * @param 	Q is the pointer to the data array of size 4 words.
 * @param 	R is the pointer to the data array of size 4 words.
 * @param 	S is the pointer to the data array of size 4 words.
 *
 * @return
 *	- XASUFW_SUCCESS - When check is passed
 *	- XASUFW_FAILURE - when check is failed
 *
 *************************************************************************************************/
static s32 XAsufw_AesDpaCmChecks(const u32 *P, const u32 *Q, const u32 *R, const u32 *S)
{
	s32 Status = XASUFW_FAILURE;

	/** Perform AES DPA CM checks. */
	if (((P[0U] == 0U) && (P[1U] == 0U) && (P[2U] == 0U) && (P[3U] == 0U)) ||
	    ((P[0U] == Q[0U]) && (P[1U] == Q[1U]) && (P[2U] == Q[2U]) && (P[3U] == Q[3U])) ||
	    ((P[0U] == R[0U]) && (P[1U] == R[1U]) && (P[2U] == R[2U]) && (P[3U] == R[3U])) ||
	    ((P[0U] == S[0U]) && (P[1U] == S[1U]) && (P[2U] == S[2U]) && (P[3U] == S[3U]))) {
		Status = XASUFW_FAILURE;
	} else {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}
/** @} */

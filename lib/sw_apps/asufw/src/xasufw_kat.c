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
#include "xasu_hmacinfo.h"
#include "xkdf.h"
#include "xhkdf.h"
#include "xecies.h"
#include "xaes_hw.h"
#include "xkeywrap.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_AesDpaCmChecks(const u32 *P, const u32 *Q, const u32 *R, const u32 *S);

/************************************ Variable Definitions ***************************************/
#define XASUFW_KAT_MSG_LENGTH_IN_BYTES		        (32U)	/**< SHA KAT message length in bytes */
#define XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES	        (256U)	/**< RSA KAT message length in bytes */
#define XASUFW_AES_KEY_LEN_IN_BYTES		            (32U)	/**< AES KAT Key length in bytes */
#define XASUFW_AES_IV_LEN_IN_BYTES		            (12U)	/**< AES KAT Iv length in bytes */
#define XASUFW_AES_AAD_LEN_IN_BYTES		            (16U)	/**< AES KAT AAD length in bytes */
#define XASUFW_AES_TAG_LEN_IN_BYTES		            (16U)	/**< AES KAT Tag length in bytes */
#define XASUFW_DOUBLE_P256_SIZE_IN_BYTES	        (64U)	/**< Double the size of P256 curve length */
#define XASUFW_DOUBLE_P192_SIZE_IN_BYTES	        (48U)	/**< Double the size of P192 curve length */
#define XASUFW_DOUBLE_P384_SIZE_IN_BYTES	        (96U)	/**< Double the size of P192 curve length */
#define XASUFW_AES_DATA_SPLIT_SIZE_IN_BYTES	        (16U)	/**< AES data split size in words */
#define XASUFW_AES_CM_KAT_KEY_SIZE_IN_BYTES	        (32U)	/**< AES Key size in words */
#define XASUFW_AES_CM_KAT_DATA_SIZE_IN_BYTES        (64U)	/**< AES operation data in words */
#define XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES	        (31U)	/**< Key wrap unwrap KAT message length in
								bytes*/
#define XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES		(5U)	/**< RSA OAEP optional data length
									in bytes */
#define XASUFW_KEYWRAP_INPUT_PADDING_SIZE_IN_BYTES	(9U)	/**< Key wrap padding length for
									KAT message in bytes */
#define XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES	(XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES + \
							XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES + \
							XASUFW_KEYWRAP_INPUT_PADDING_SIZE_IN_BYTES)
						/**< Key wrap unwrap KAT message output length in
							bytes */

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

static const u8 ExpHmacOutput[XASU_SHA_256_HASH_LEN] = {
	0x72U, 0xC5U, 0x39U, 0x7DU, 0x23U, 0x49U, 0x3EU, 0xFFU,
	0xF9U, 0x16U, 0xC8U, 0x4BU, 0x08U, 0x11U, 0x3EU, 0x26U,
	0x2FU, 0xA8U, 0x55U, 0x76U, 0xAFU, 0xFEU, 0x96U, 0x4AU,
	0x09U, 0x79U, 0xAAU, 0x05U, 0xBEU, 0xAFU, 0x5FU, 0x2DU
};

static const u8 ExpKdfOutput[XASU_SHA_256_HASH_LEN] = {
	0x91U, 0x53U, 0x33U, 0xF0U, 0x62U, 0x3DU, 0xBCU, 0x73U,
	0x7EU, 0x92U, 0x3CU, 0x1BU, 0xB3U, 0xA9U, 0x92U, 0x31U,
	0x4EU, 0x66U, 0xC4U, 0x6EU, 0x2CU, 0x8DU, 0x81U, 0x45U,
	0xD2U, 0xE4U, 0x70U, 0x4EU, 0x75U, 0xD0U, 0x3AU, 0x77U
};

static const u8 ExpHkdfOutput[XASU_SHA_256_HASH_LEN] = {
	0x10U, 0x27U, 0x1dU, 0x64U, 0xbaU, 0xafU, 0xd3U, 0x75U,
	0x75U, 0xedU, 0x28U, 0x73U, 0xe7U, 0x30U, 0x27U, 0x03U,
	0x43U, 0xacU, 0xa1U, 0xc7U, 0x64U, 0x40U, 0x79U, 0x03U,
	0xddU, 0x17U, 0x88U, 0xd0U, 0xb1U, 0xb4U, 0x48U, 0xd2U
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

static const u8 RsaPvtExp[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES] = {
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

static const u8 EccPrivKey[XASU_ECC_P256_SIZE_IN_BYTES] = {
	0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
	0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
	0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
	0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU
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

static const u8 EcdhPrivKey[XASU_ECC_P192_SIZE_IN_BYTES] = {
	0xE5U, 0xCEU, 0x89U, 0xA3U, 0x4AU, 0xDDU, 0xDFU, 0x25U, 0xFFU, 0x3BU, 0xF1U, 0xFFU, 0xE6U,
	0x80U, 0x3FU, 0x57U, 0xD0U, 0x22U, 0x0DU, 0xE3U, 0x11U, 0x87U, 0x98U, 0xEAU
};

static const u8 EcdhExpSharedSecret[XASU_ECC_P192_SIZE_IN_BYTES] = {
	0x12U, 0xF5U, 0xE2U, 0x72U, 0x5DU, 0x81U, 0x47U, 0x18U, 0x31U, 0x55U, 0x54U, 0xACU, 0x4DU,
	0x95U, 0x8BU, 0xDFU, 0xECU, 0x86U, 0x8CU, 0xA8U, 0x23U, 0xDCU, 0xE5U, 0x65U
};

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
static const u8 KeyWrapInput[XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES] = {
	0xffU, 0xe9U, 0x52U, 0x60U, 0x48U, 0x34U, 0xbfU, 0xf8U, 0x99U, 0xe6U, 0x36U, 0x58U, 0xf3U,
	0x42U, 0x46U, 0x81U, 0x5cU, 0x91U, 0x59U, 0x7eU, 0xb4U, 0x0aU, 0x21U, 0x72U, 0x9eU, 0x0aU,
	0x8aU, 0x95U, 0x9bU, 0x61U, 0xf2U
};

static const char RsaOpt[XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES + 1U] = "ASUFW";

static u8 WrappedResult[XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES];
/*************************************************************************************************/
/**
 * @brief	This function runs SHA KAT on the given SHA instance for 256-bit digest size.
 *
 * @param	XAsufw_ShaInstance	Pointer to the SHA instance.
 * @param	AsuDmaPtr			ASU DMA instance pointer.
 * @param	ShaResource			SHA2/SHA3 resource.
 *
 * @return
 *	- XASUFW_SUCCESS, if SHA KAT is successful.
 *	- XASUFW_SHA_HASH_COMPARISON_FAILED, if expected and generated hash comparison fails.
 *	- XASUFW_SHA_KAT_FAILED, if XAsufw_ShaKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, XAsufw_Dma *AsuDmaPtr, XAsufw_Resource ShaResource)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 OutVal[XASU_SHA_256_HASH_LEN];
	const u8 *ExpHash = NULL;

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
	Status = XSha_Finish(XAsufw_ShaInstance, (u32 *)OutVal, XASU_SHA_256_HASH_LEN,
			     XASU_FALSE);
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

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs RSA KAT for encryption and decryption using 2048 bit key.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if RSA KAT is successful.
 *	- XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED, if expected and generated encrypted message
 *	  comparison fails.
 *	- XASUFW_RSA_KAT_FAILED, if XAsufw_RsaEncDecKat API fails.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_RsaEncDecKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XAsu_RsaPubKeyComp PubKeyParam;
	XAsu_RsaPvtKeyComp PvtKeyParam;
	u8 Result[XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES];

	PubKeyParam.Keysize = XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	/** Copy required parameters. */
	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaModulus,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	PvtKeyParam.PubKeyComp = PubKeyParam;
	PvtKeyParam.PrimeCompOrTotientPrsnt = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PvtKeyParam.PvtExp, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaPvtExp,
		    XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Perform RSA private exponentiation operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PvtExp(AsuDmaPtr, PvtKeyParam.PubKeyComp.Keysize, (u64)(UINTPTR)RsaExpectedCt,
			     (u64)(UINTPTR)Result, (u64)(UINTPTR)&PvtKeyParam, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PVT_OP_ERROR;
		goto END;
	}

	/** Compare generated encrypted message with expected encrypted message. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(Result, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaData,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DECRYPT_DATA_COMPARISON_FAILED;
		goto END;
	}

	/** Perform RSA public exponentiation encryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PubExp(AsuDmaPtr, PubKeyParam.Keysize, (u64)(UINTPTR)RsaData,
			     (u64)(UINTPTR)Result, (u64)(UINTPTR)&PubKeyParam, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_OP_ERROR;
		goto END;
	}

	/** Compare generated encrypted message with expected encrypted message. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(Result, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaExpectedCt,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED;
		goto END;
	}

END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&Result[0U], XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, 0U,
			      XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_KAT_FAILED);
	}

	return Status;
}

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
 * @brief	This function runs ECC KAT on RSA core for P-192 curve.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
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
s32 XAsufw_RsaEccKat(XAsufw_Dma *AsuDmaPtr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 GenPubKey[XASUFW_DOUBLE_P192_SIZE_IN_BYTES];
	u8 GenSign[XASUFW_DOUBLE_P192_SIZE_IN_BYTES];

	/** Generate the public key using the provided private key. */
	Status = XRsa_EccGeneratePubKey(AsuDmaPtr, XASU_ECC_NIST_P192, XASU_ECC_P192_SIZE_IN_BYTES,
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

	/** Validate the generated ECC public key. */
	Status = XRsa_EccValidatePubKey(AsuDmaPtr, XASU_ECC_NIST_P192, XASU_ECC_P192_SIZE_IN_BYTES,
					(u64)(UINTPTR)GenPubKey);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Generate signature using RSA core API XRsa_EccGenerateSignature. */
	Status = XRsa_EccGenerateSignature(AsuDmaPtr, XASU_ECC_NIST_P192,
					   XASU_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)EccPrivKey, EccEphemeralKey,
					   (u64)(UINTPTR)EccHash, XASU_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
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
	Status = XRsa_EccVerifySignature(AsuDmaPtr, XASU_ECC_NIST_P192, XASU_ECC_P192_SIZE_IN_BYTES,
					 (u64)(UINTPTR)EccExpPubKeyP192, (u64)(UINTPTR)EccHash,
					 XASU_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)GenSign);
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

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES KAT on the given AES instance for 256-bit message length
 * 		in GCM mode.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if AES GCM KAT execution is successful.
 *	- XASUFW_AES_WRITE_KEY_FAILED, if Key write to AES USER key register fails.
 *	- XASUFW_AES_INIT_FAILED, if initialization of AES engine fails.
 *	- XASUFW_AES_UPDATE_FAILED, if update of data to AES engine fails.
 *	- XASUFW_AES_FINAL_FAILED, if completion of final AES operation fails.
 *
 *************************************************************************************************/
s32 XAsufw_AesGcmKat(XAsufw_Dma *AsuDmaPtr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XAes *AesInstance = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAsu_AesKeyObject KeyObject;
	u8 AesOutData[XASUFW_KAT_MSG_LENGTH_IN_BYTES];
	u8 AesTag[XASUFW_AES_TAG_LEN_IN_BYTES];

	KeyObject.KeyAddress = (u64)(UINTPTR)AesKey;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeySize = XASU_AES_KEY_SIZE_256_BITS;

	/** Write the key into specified AES USER key registers. */
	Status = XAes_WriteKey(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
		XFIH_GOTO(END);
	}

	/**
	 * Initialize the AES engine and load the provided key and IV to the AES engine for
	 * encryption operation.
	 */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)AesIv,
			   XASUFW_AES_IV_LEN_IN_BYTES, XASU_AES_GCM_MODE, XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES update operation for AAD and message. */
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

	/** Compare encrypted data with expected ciphertext data. */
	Status = Xil_SMemCmp_CT(ExpAesGcmCt, XASUFW_KAT_MSG_LENGTH_IN_BYTES, AesOutData,
				XASUFW_KAT_MSG_LENGTH_IN_BYTES, XASUFW_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES KAT Failed at Encrypted data Comparison \r\n");
		XFIH_GOTO(END);
	}

	/** Compare generated GCM tag with the expected GCM tag. */
	Status = Xil_SMemCmp_CT(ExpAesGcmTag, XASUFW_AES_TAG_LEN_IN_BYTES, AesTag,
				XASUFW_AES_TAG_LEN_IN_BYTES, XASUFW_AES_TAG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES KAT Failed at Encrypted data Comparison \r\n");
		XFIH_GOTO(END);
	}

	/**
	 * Initialize the AES engine and load the provided key and IV to the AES engine for
	 * decryption operation.
	 */
	Status = XAes_Init(AesInstance, AsuDmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)AesIv,
			   XASUFW_AES_IV_LEN_IN_BYTES, XASU_AES_GCM_MODE, XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		XFIH_GOTO(END);
	}

	/** Perform AES update operation for AAD and ciphertext. */
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

	/** Compare decrypted data with expected input data. */
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

	/** Clear the key written to the XASU_AES_USER_KEY_7 key source. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(AesInstance, KeyObject.KeySrc));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES DPA CM KAT on the given AES instance.
 *
 * @param	AsuDmaPtr	ASU DMA instance pointer.
 *
 * @return
 *	- XASUFW_SUCCESS, if AES DPA CM KAT is successful.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK1_FAILED, if check1 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK2_FAILED, if check2 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK3_FAILED, if check3 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK4_FAILED, if check4 has failed.
 *	- XASUFW_AES_DPA_CM_KAT_CHECK5_FAILED, if check5 has failed.
 *	- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_AesDecryptDpaCmKat(XAsufw_Dma *AsuDmaPtr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	XAes *AesInstance = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
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
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeySize = XASU_AES_KEY_SIZE_256_BITS;

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

	return Status;
}

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
			    XASU_ECC_P256_SIZE_IN_BYTES, XASU_SHA_MODE_SHA256, XASU_SHA_256_HASH_LEN);
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
	Params.ShaMode = XASU_SHA_MODE_SHA256;

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

/*************************************************************************************************/
/**
 * @brief	This function runs HKDF KAT using SHA2-256.
 *
 * @param	AsuDmaPtr	Pointer to the ASU DMA instance.
 *
 * @return
 *	- XASUFW_SUCCESS, if HKDF KAT is executed successfully.
 *	- XASUFW_HKDF_KAT_COMPARISON_FAILED, if expected and generated HKDF comparison fails.
 *
 *************************************************************************************************/
s32 XAsufw_HkdfOperationKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u8 HkdfOutput[XASU_SHA_256_HASH_LEN] = {0U};
	XAsu_HkdfParams Params;

	Params.KdfParams.KeyInAddr = (u64)(UINTPTR)EccPrivKey;
	Params.KdfParams.KeyInLen = XASU_ECC_P256_SIZE_IN_BYTES;
	Params.KdfParams.ContextAddr = (u64)(UINTPTR)KatMessage;
	Params.KdfParams.ContextLen = XASUFW_KAT_MSG_LENGTH_IN_BYTES;
	Params.KdfParams.KeyOutAddr = (u64)(UINTPTR)HkdfOutput;
	Params.KdfParams.KeyOutLen = XASU_SHA_256_HASH_LEN;
	Params.KdfParams.ShaMode = XASU_SHA_MODE_SHA256;
	Params.SaltAddr = (u64)(UINTPTR)EccHash;
	Params.SaltLen = XASU_ECC_P256_SIZE_IN_BYTES;

	/** Perform HKDF generate with known inputs. */
	Status = XHkdf_Generate(AsuDmaPtr, Sha2Ptr, &Params);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_GENERATE_FAILED);
		goto END;
	}

	/** Compare generated HKDF key output with expected HKDF key output. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(HkdfOutput, XASU_SHA_256_HASH_LEN, ExpHkdfOutput,
                                XASU_SHA_256_HASH_LEN, XASU_SHA_256_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_HKDF_KAT_COMPARISON_FAILED;
	}

END:
	/** Zeroize local copy of output value. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SMemSet(&HkdfOutput[0U], XASU_SHA_256_HASH_LEN,
					0U, XASU_SHA_256_HASH_LEN));

	return Status;
}

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
	Params.ShaMode = XASU_SHA_MODE_SHA256;
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

	PubKeyParam.Keysize = XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES;
	PubKeyParam.PubExp = RsaPublicExp;

	/** Copy required parameters for Key wrap unwrap operation. */
	Status = Xil_SMemCpy(PubKeyParam.Modulus, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaModulus,
			     XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	PvtKeyParam.PubKeyComp = PubKeyParam;
	PvtKeyParam.PrimeCompOrTotientPrsnt = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PvtKeyParam.PvtExp, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, RsaPvtExp,
		    XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES, XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES);
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
	KwpunwpParam.RsaKeySize = XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES;
	KwpunwpParam.OptionalLabelSize = XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES;
	KwpunwpParam.OutuputDataLen = XASUFW_KEYWRAP_OUTPUT_SIZE_IN_BYTES;
	KwpunwpParam.AesKeySize = XASU_AES_KEY_SIZE_128_BITS;
	KwpunwpParam.ShaType = XASU_SHA2_TYPE;
	KwpunwpParam.ShaMode = XASU_SHA_MODE_SHA256;

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
	KwpunwpParam.RsaKeySize = XASUFW_RSA_KAT_MSG_LENGTH_IN_BYTES;
	KwpunwpParam.OptionalLabelSize = XASUFW_RSA_OPTIONAL_DATA_SIZE_IN_BYTES;
	KwpunwpParam.OutuputDataLen = XASUFW_KEYWRAP_INPUT_SIZE_IN_BYTES;
	KwpunwpParam.AesKeySize = XASU_AES_KEY_SIZE_128_BITS;
	KwpunwpParam.ShaType = XASU_SHA2_TYPE;
	KwpunwpParam.ShaMode = XASU_SHA_MODE_SHA256;

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

/*****************************************************************************/
/**
 * @brief	This function performs checks for AES DPA CM KAT output.
 *
 * @param 	P is the pointer to the data array of size 4 words.
 * @param 	Q is the pointer to the data array of size 4 words.
 * @param 	R is the pointer to the data array of size 4 words.
 * @param 	S is the pointer to the data array of size 4 words.
 *
 * @return
 *	- XASUFW_SUCCESS, if DPA CM checks are passed.
 *	- XASUFW_FAILURE, if DPA CM checks fail.
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

/*************************************************************************************************/
/**
 * @brief	This function runs ECDH KAT on RSA core for P-192 curve.
 *
 * @param	AsuDmaPtr		ASU DMA instance pointer.
 *
 * @return
 * 		- XASUFW_SUCCESS, if ECC KAT is successful.
 * 		- XASUFW_ECDH_SECRET_COMPARISON_FAILED, if generated and expected shared secret
 * 		  comparison fails.
 * 		- XASUFW_FAILURE, if any other failure happens.
 *
 *************************************************************************************************/
s32 XAsufw_P192EcdhKat(XAsufw_Dma *AsuDmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u8 SharedSecret[XASU_ECC_P192_SIZE_IN_BYTES];

	/** Generate the shared secret using the known private key and public key. */
	Status = XRsa_EcdhGenSharedSecret(AsuDmaPtr, XRSA_ECC_CURVE_TYPE_NIST_P192,
					  XASU_ECC_P192_SIZE_IN_BYTES, (u64)(UINTPTR)EcdhPrivKey,
					  (u64)(UINTPTR)EccExpPubKeyP192,
					  (u64)(UINTPTR)SharedSecret, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	/** Compare the generated shared secret with expected shared secret. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(SharedSecret, XASU_ECC_P192_SIZE_IN_BYTES, EcdhExpSharedSecret,
			     XASU_ECC_P192_SIZE_IN_BYTES, XASU_ECC_P192_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECDH_SECRET_COMPARISON_FAILED;
	}

END_CLR:

	/** Zeroize local copy of shared secret. */
	SStatus = Xil_SMemSet(&SharedSecret[0U], XASU_ECC_P192_SIZE_IN_BYTES, 0U,
			      XASU_ECC_P192_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECDH_KAT_FAILED);
	}

	return Status;
}
/** @} */

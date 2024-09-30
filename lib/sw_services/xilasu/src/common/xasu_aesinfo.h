/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aesinfo.h
 *
 * This file contains the AES definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   07/16/24 Initial release
 *       am   08/01/24 Replaced enums with macros
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_AESINFO_H
#define XASU_AESINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define XASU_AES_BLOCK_SIZE_IN_BYTES			(16U) /**< AES block size in bytes */

#define XASU_AES_EVEN_MODULUS				(2U) /**< Modulus to determine evenness */

#define XASU_AES_IV_SIZE_96BIT_IN_WORDS			(3U) /**< AES 96bit Iv size in words */
#define XASU_AES_IV_SIZE_128BIT_IN_WORDS		(4U) /**< AES 128bit Iv size in words */
#define XASU_AES_IV_SIZE_96BIT_IN_BYTES			(12U) /**< AES 96bit Iv size in bytes */
#define XASU_AES_IV_SIZE_128BIT_IN_BYTES		(16U) /**< AES 128bit Iv size in bytes */

#define XASU_AES_KEY_SIZE_128BIT_IN_WORDS		(4U) /**< AES 128bit key size in words */
#define XASU_AES_KEY_SIZE_256BIT_IN_WORDS		(8U) /**< AES 256bit key size in words */
#define XASU_AES_KEY_SIZE_128BIT_IN_BYTES		(16U) /**< AES 128bit key size in bytes */
#define XASU_AES_KEY_SIZE_256BIT_IN_BYTES		(32U) /**< AES 256bit key size in bytes */

#define XASU_AES_MIN_TAG_LENGTH_IN_BYTES		(4U) /**< AES minimum tag length in bytes */
#define XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES	(8U) /**< AES NIST recommended minimum tag length in bytes */
#define XASU_AES_MAX_TAG_LENGTH_IN_BYTES		(16U) /**< AES maximum tag length in bytes */

/*************************** Macros (Inline Functions) Definitions *******************************/
/* AES module command IDs */
#define XASU_AES_OPERATION_CMD_ID	(0x0U) /**< Command ID for AES operation command */
#define XASU_AES_KAT_CMD_ID		(0x1U) /**< Command ID for AES KAT command */
#define XASU_AES_GET_INFO_CMD_ID	(0x2U) /**< Command ID for AES Get Info command */

/* AES operation flags */
#define XASU_AES_INIT			(0x1U) /**< AES initialize operation flag */
#define XASU_AES_UPDATE			(0x2U) /**< AES update operation flag */
#define XASU_AES_FINAL			(0x4U) /**< AES final operation flag */

/* AES engine mode */
#define XASU_AES_CBC_MODE		(0x0U) /**< AES CBC mode */
#define XASU_AES_CFB_MODE		(0x1U) /**< AES CFB mode */
#define XASU_AES_OFB_MODE		(0x2U) /**< AES OFB mode */
#define XASU_AES_CTR_MODE		(0x3U) /**< AES CTR mode */
#define XASU_AES_ECB_MODE		(0x4U) /**< AES ECB mode */
#define XASU_AES_CCM_MODE		(0x5U) /**< AES CCM mode */
#define XASU_AES_GCM_MODE		(0x6U) /**< AES GCM mode */
#define XASU_AES_CMAC_MODE		(0x8U) /**< AES CMAC mode */
#define XASU_AES_GHASH_MODE		(0xEU) /**< AES GHASH mode */

/* AES key source */
#define XASU_AES_EFUSE_KEY_RED_0	(0x0U) /**< EFUSE Red key 0 */
#define XASU_AES_EFUSE_KEY_RED_1	(0x1U) /**< EFUSE Red key 1 */
#define XASU_AES_USER_KEY_0		(0x2U) /**< User Key 0 */
#define XASU_AES_USER_KEY_1		(0x3U) /**< User Key 1 */
#define XASU_AES_USER_KEY_2		(0x4U) /**< User Key 2 */
#define XASU_AES_USER_KEY_3		(0x5U) /**< User Key 3 */
#define XASU_AES_USER_KEY_4		(0x6U) /**< User Key 4 */
#define XASU_AES_USER_KEY_5		(0x7U) /**< User Key 5 */
#define XASU_AES_USER_KEY_6		(0x8U) /**< User Key 6 */
#define XASU_AES_USER_KEY_7		(0x9U) /**< User Key 7 */

/* AES key size */
#define XASU_AES_KEY_SIZE_128_BITS	(0x0U) /**< 128 bits AES key size  */
#define XASU_AES_KEY_SIZE_256_BITS	(0x2U) /**< 256 bits AES key size */

/* AES Operation type */
#define XASU_AES_ENCRYPT_OPERATION	(0x0U) /**< AES encrypt operation */
#define XASU_AES_DECRYPT_OPERATION	(0x1U) /**< AES decrypt operation */

/************************************ Type Definitions *******************************************/
/**
 * @brief This structure contains AES key object information.
 */
typedef struct {
	u64 KeyAddress; /**< AES Key pointer */
	u32 KeySize; /**< AES key size */
	u32 KeySrc; /**< AES key source */
} XAsu_AesKeyObject;

/**
 * @brief This structure is common for both client and handler, which contains AES input and output
 *  parameters information.
 */
typedef struct {
	u64 InputDataAddr; /**< AES input data address */
	u64 OutputDataAddr; /**< AES output data address */
	u64 AadAddr; /**< AES Aad address */
	u64 KeyObjectAddr; /**< AES Key object address */
	u64 IvAddr; /**< AES Iv address */
	u64 TagAddr; /**< AES Tag address */
	u32 DataLen; /**< AES common input/output data length */
	u32 AadLen; /**< AES AAD length */
	u32 IvLen; /**< AES Iv length */
	u32 TagLen; /**< AES tag length */
	u8 EngineMode; /**< AES engine mode */
	u8 OperationFlags; /**< AES operation(INIT, UPDATE, FINAL) flag */
	u8 IsLast; /**< Indicates whether it is the last update of data to AES engine */
	u8 OperationType; /**< AES encrypt/decrypt operation type */
} Asu_AesParams __attribute__ ((aligned (32)));

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_AESINFO_H */
/** @} */
